
#include "simulator_sw.h"
using namespace elabtsaot;

#include "logger.h"
#include "powersystem.h"
#include "auxiliary.h"
#include "precisiontimer.h"
#include "scenario.h"
#include "tdresults.h"
#include "tdresultsidentifier.h"
#include "ssengine.h"

#include <QProgressBar>

#include <list>
using std::list;
//#include <complex>
using std::complex;
//#include <string>
using std::string;
//#include <vector>
using std::vector;
//#include <map>
using std::map;
#include <iomanip>
#include <iostream>
using std::cout;
using std::cin;
using std::endl;
#include <cmath>
#define _USE_MATH_DEFINES
#include <stdio.h>
#include <stdlib.h>

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/vector_proxy.hpp>
#include <boost/numeric/ublas/triangular.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/operation.hpp>
//#include <boost/numeric/ublas/matrix.hpp>
//#include <boost/numeric/ublas/lu.hpp>
namespace ublas = boost::numeric::ublas;

#define DBL_EPSILON 0.0000000001
#define DOUBLE_EQ(x,v) (((v - DBL_EPSILON) < x) && (x <( v + DBL_EPSILON)))

Simulator_sw::Simulator_sw( Powersystem const* pws,
                            SSEngine const* const& sse,
                            Logger* log ) :
    TDEngine(pws, "Software simulator", 0.01, log),
    _sse(sse), _pwsLocal(*pws) {}

int Simulator_sw::init(Powersystem const *pws){

  if ( pws != NULL && pws != _pws ){
    // A new powersystem has been supplied as argument to the Simulator_hw
    _pws = pws;
  } else {
    // The powersystem pointer remains the same (or no arg has been provided)
  }

  // Anyway update the internal copy of the powersystem, in order to account for
  // any changes in the pointed to Powersystem
  _pwsLocal = *_pws;
  if ( _TDlog != NULL )
    _TDlog->notifyProgress(0);

  // Reset internal state of the simulator
  _busintid.clear();
  _oldgsh.clear();
  _oldbsh.clear();
  _br_frombus_intid.clear();
  _br_tobus_intid.clear();
  _br_frombus_oldgsh.clear();
  _br_frombus_oldbsh.clear();
  _br_tobus_oldgsh.clear();
  _br_tobus_oldbsh.clear();
  _br_branchoffault_extid.clear();
  _br_faultbus_extid.clear();

  _genBusIntId.clear();
  _loadBusIntId.clear();
  _genModel.clear();

  // Reset results
  _time.clear();
  _angles.clear();
  _speeds.clear();
  _eq_tr.clear();
  _ed_tr.clear();
  _powers.clear();
  _voltages.clear();
  _currents.clear();

  return 0;
}

bool Simulator_sw::do_isEngineCompatible(Scenario const& sce) const{

  bool compatibility=true;
  for ( size_t i = 0 ; i != sce.getEventSetSize() ; ++i){
    if (sce.getEvent(i).element_type()==EVENT_ELEMENT_TYPE_BUS)
      if (!sce.getEvent(i).event_type()==EVENT_EVENTTYPE_BUSFAULT)
        compatibility=false;
    if (sce.getEvent(i).element_type()==EVENT_ELEMENT_TYPE_BRANCH)
      if (!(sce.getEvent(i).event_type()==EVENT_EVENTTYPE_BRFAULT||
            sce.getEvent(i).event_type()==EVENT_EVENTTYPE_BRTRIP||
            sce.getEvent(i).event_type()==EVENT_EVENTTYPE_BRSHORT))
        compatibility=false;
    if (sce.getEvent(i).element_type()==EVENT_ELEMENT_TYPE_GEN)
      if (!(sce.getEvent(i).event_type()==EVENT_EVENTTYPE_GENTRIP||
            sce.getEvent(i).event_type()==EVENT_EVENTTYPE_GENPCHANGE||
            sce.getEvent(i).event_type()==EVENT_EVENTTYPE_GENQCHANGE))
        compatibility=false;
    if (sce.getEvent(i).element_type()==EVENT_ELEMENT_TYPE_LOAD)
      if (!( sce.getEvent(i).event_type()==EVENT_EVENTTYPE_LOADPCHANGE||
             sce.getEvent(i).event_type()==EVENT_EVENTTYPE_LOADQCHANGE))
        compatibility=false;
  }
  return compatibility;
}

int Simulator_sw::do_simulate( Scenario const& sce, TDResults& res){

//  cout.precision(10);
  init();
  if ( _pwsLocal.status() != PWSSTATUS_LF )
    return 1;

  PrecisionTimer timer;
  timer.Start();

  Scenario _sce(sce);
  _sce.sort_t();//Put the events in order
  double baseF = _pwsLocal.baseF();

  _genCount = _pwsLocal.getGenSet_size();
  _brCount = _pwsLocal.getBrSet_size();
  _loadCount = _pwsLocal.getLoadSet_size();
  _busCount = _pwsLocal.getBusSet_size();

  // Initialize generator specific internal variables
  for ( size_t k = 0 ; k != _genCount ; ++k ){
    Generator const* gen = _pwsLocal.getGenerator(k);
    if (!gen->status) continue;

    _genBusIntId.push_back(_pwsLocal.getBus_intId(gen->busExtId));
    _genModel.push_back(gen->model);
  }

  // Initialize load specific internal variables and check whether loads other
  // than constant impedance are present in the system
  bool pwsHasNonZLoads = false;
  for ( size_t k = 0 ; k != _loadCount ; ++k ){
    Load const* load = _pwsLocal.getLoad(k);

    _loadBusIntId.push_back( _pwsLocal.getBus_intId(load->busExtId) );
    if ( load->type() != LOADTYPE_CONSTZ )
      pwsHasNonZLoads = true;
  }

  // Initial bus voltages
  vector<complex<double> > Ubus0(_busCount);
  for ( size_t k = 0 ; k != _busCount ; ++k ){
    Bus const* bus(_pwsLocal.getBus(k));
    Ubus0[k] = bus->V * complex<double>(cos(bus->theta), sin(bus->theta));
  }
  vector<complex<double> > Ubus(Ubus0);

  // ----- Calculate initial state of generators -----
  // Electical power
  vector<double> Pel(_genCount,0.0);
  // Exitation current
  vector<double> Efd(_genCount,0.0);
  // Direct axis current
  vector<double> Id(_genCount,0.0);
  // Quadrature axis current
  vector<double> Iq(_genCount,0.0);
  /*
    For _genModel == 0 (classical "1.0" generator model):
      Xgen[*][0] -> delta in [rad]
      Xgen[*][1] -> omega in [rad/sec]
      Xgen[*][2] -> internal voltage E in [pu]
    For _genModel == 1 ("1.1" generator model)
      Xgen[*][0] -> delta in [rad]
      Xgen[*][1] -> electrical speed in [rad/sec]
      Xgen[*][2] -> quadrature axis voltage magnitude in [pu]
      Xgen[*][3] -> direct axis voltage magnitude Ed in [pu]
  */
  vector<vector<double> > Xgen(_genCount, vector<double>(4,0.0) );

  for ( size_t i = 0 ; i != _genCount ; ++i ) {
    Generator* tempGen;
    _pwsLocal.getGenerator(_pwsLocal.getGen_extId(i),tempGen);

    if (_genModel[i] == GENMODEL_0p0) {
      // Initial machine armature currents
//      complex<double> I0 = complex<double>( tempGen->pgen(), -tempGen->qgen() )
//                               / std::conj(Ubus[_genBusIntId[i]]);
//      // Initial steady-state internal EMF
//      complex<double> E0 = Ubus[_genBusIntId[i]]
//                  + I0 * complex<double>(0.0, tempGen->xd_1);
//      Xgen[i][0] = std::arg(E0);
//      Xgen[i][1] = 2*M_PI*baseF;
//      Xgen[i][2] = std::abs(E0);
      Xgen[i][0] = tempGen->deltass();
      Xgen[i][1] = 2*M_PI*baseF;
      Xgen[i][2] = tempGen->Ess();
    }

    else if (_genModel[i] == GENMODEL_1p1) {
      // Transient saliency is not supported => xd_t = xq_t
      tempGen->xq_1 = tempGen->xd_1;
      // Initial machine armature currents
      complex<double> Ia0 = complex<double>( tempGen->Pgen, -tempGen->Qgen )
                              / std::conj(Ubus[_genBusIntId[i]]);
      double phi0 = std::arg(Ia0);
      // Initial steady-state internal EMF
      complex<double> Eq0 = Ubus[_genBusIntId[i]]
                  + Ia0 * complex<double>(0.0, tempGen->xq);
      double delta0 = std::arg(Eq0);
      // Machine currents in dq frame
      Id[i] = -std::abs(Ia0) * std::sin( delta0 - phi0 );
      Iq[i] =  std::abs(Ia0) * std::cos( delta0 - phi0 );
      // Field voltage
      Efd[i] = std::abs(Eq0) - Id[i]*(tempGen->xd-tempGen->xq);
      //Initial transient internal EMF
      double Eq_tr0 = Efd[i] + Id[i]*(tempGen->xd-tempGen->xd_1);
      double Ed_tr0 = -Iq[i]*(tempGen->xq-tempGen->xq_1);
      Xgen[i][0] = delta0;
      Xgen[i][1] = 2*M_PI*baseF;
      Xgen[i][2] = Eq_tr0;
      Xgen[i][3] = Ed_tr0;
    }
  }
  _calculateMachineCurrents( Xgen, Ubus, _genBusIntId, Iq, Id, Pel );

  // Main Loop: Xgen,Pel, Ubus change after here for each run
  long int iterationCount = 0;
  size_t eventCounter = 0;

  vector<double> temp_angles;
  vector<double> temp_speeds;
  vector<double> temp_eq;
  vector<double> temp_ed;
  vector<complex<double> > temp_Ibus;
  vector<Event> curEvents; //concurrent events at specific t

  // Store the step size before doing changes in order to restore i in the end
  double currentTimeStep = _timeStep;
  double stopTime = _sce.stopTime();
  double t = _sce.startTime() - 0.02; // 0.02 without applying events

  // Augmented Y matrix
  ublas::matrix<complex<double> > augY;
  _calculateAugmentedYMatrix( Ubus, Ubus0, augY );

  // Factorize augmented Y matrix
  ublas::permutation_matrix<size_t> pmatrix(augY.size1());
  ublas::matrix<complex<double> > LUaugY(augY);
  BOOST_TRY {
    ublas::lu_factorize(LUaugY, pmatrix);
  } BOOST_CATCH(ublas::singular const& ex) {
    cout << "Singularity likely!" << endl;
    cout << "Exception message: " << ex.what() << endl;
  } BOOST_CATCH(std::exception const& ex) {
    cout << "Other exception caught!" << endl;
    cout << "Exception message: " << ex.what() << endl;
  } BOOST_CATCH(...){
    cout << "Operation failed!" << endl;
    return 2;
  } BOOST_CATCH_END

  while ( t< stopTime+currentTimeStep ){

    // Stop exactly at stop time
    if ( t+currentTimeStep > stopTime )
      currentTimeStep = stopTime-t;

    // ----- In case of powersystem with non-constant impedance loads -----
    // ----- recalculate augmented Y matrix -----
    if ( pwsHasNonZLoads ){
      // Calculate augmented Y matrix proper
      _calculateAugmentedYMatrix( Ubus, Ubus0, augY );

      // Factorize augmented Y matrix
      ublas::permutation_matrix<size_t> pm(augY.size1());
      pmatrix = pm;
      LUaugY = augY;
      BOOST_TRY {
        ublas::lu_factorize(LUaugY, pmatrix);
      } BOOST_CATCH(ublas::singular const& ex) {
        cout << "Singularity likely!" << endl;
        cout << "Exception message: " << ex.what() << endl;
      } BOOST_CATCH(std::exception const& ex) {
        cout << "Other exception caught!" << endl;
        cout << "Exception message: " << ex.what() << endl;
      } BOOST_CATCH(...){
        cout << "Operation failed!" << endl;
        return 3;
      } BOOST_CATCH_END
    }

    // ----- Perform RK integration of -----
    /*
      Xgen: generator dynamic variables (angles, speeds)
      Pel : generator electrical power
      Efd : excitation voltage
      Iq  : q-axis generator current
      Id  : d-axis generator current
      Ubus: bus voltages
    */
    _rungeKutta( LUaugY, pmatrix, _genBusIntId, currentTimeStep,
                 Xgen, Pel, Efd, Iq, Id, Ubus );
    temp_angles.resize( Xgen.size() );
    temp_speeds.resize( Xgen.size() );
    temp_eq.resize( Xgen.size() );
    temp_ed.resize( Xgen.size() );
    for ( size_t k = 0 ; k != Xgen.size() ; ++k ){
      temp_angles[k] = Xgen[k][0];
      temp_speeds[k] = Xgen[k][1]/(2*M_PI*baseF);
      temp_eq[k] = Xgen[k][2];
      temp_ed[k] = Xgen[k][3];
    }

    // Calculate the current injected at buses (from loads and gens)
    ublas::vector<complex<double> > U(Ubus.size());
    std::copy(Ubus.begin(),Ubus.end(),U.begin());
    ublas::vector<complex<double> > I( ublas::prod(U,LUaugY) );
    temp_Ibus.resize(I.size());
    std::copy(I.begin(),I.end(),temp_Ibus.begin());

    // Save the temporary data before the end of calibration
    // The final store would be after the loop to the TD results
    if ( t >= 0. ){
      if ( _TDlog != NULL )
        _TDlog->notifyProgress( t/stopTime*100 );
      auxiliary::stayAlive();
      _time.push_back(t);
      _voltages.push_back(Ubus);
      _currents.push_back(temp_Ibus);
      _powers.push_back(Pel);
      _angles.push_back(temp_angles);
      _speeds.push_back(temp_speeds);
      _eq_tr.push_back(temp_eq);
      _ed_tr.push_back(temp_ed);
    }

    // Check if next event occures in a smaller step
    if ( eventCounter != _sce.getEventSetSize() ){
      // Set only if there is an event left
      if (   (t+currentTimeStep) >= _sce.getEvent(eventCounter).time()
             && _sce.getEvent(eventCounter).status() ){
        // Event occured to a time smaller that time step
        currentTimeStep=_sce.getEvent(eventCounter).time()-t;
      }
    }

    // Check if event occured
    curEvents.clear();
    for (size_t i=eventCounter;i<_sce.getEventSetSize();++i){
      // Start from eventCounter, not from 0 but the next event, scenario must be in order
      if (    DOUBLE_EQ(t,_sce.getEvent(i).time())
           && _sce.getEvent(i).status() ){
        curEvents.push_back(_sce.getEvent(i));
        eventCounter++;
      }
    }
    for (size_t i=0;i<curEvents.size();++i){

      if (curEvents[i].element_type() == EVENT_ELEMENT_TYPE_BUS){ // Bus fault
        _parseBusFault(curEvents[i]);
      }
      if (curEvents[i].element_type() == EVENT_ELEMENT_TYPE_BRANCH){ // Branch fault
        _parseBrFault(curEvents[i]);
      }
      if (curEvents[i].element_type() == EVENT_ELEMENT_TYPE_GEN){ // Generator fault
        _parseGenFault(curEvents[i]);
      }
      if (curEvents[i].element_type() == EVENT_ELEMENT_TYPE_LOAD){ // Load fault
        _parseLoadFault(curEvents[i]);
      }
    }

    // The new parameters of the pws was set, running power flow now
    if (curEvents.size() != 0) {

      /*
        If event happened:
          - build Y matrix
          - calculate augmented Y matrix
          - LU factorize augmented Y matrix
          - solve power flow
          - calculate machine currents
          - calculate bus current injections
      */
      // Calculate augmented Y matrix
      _calculateAugmentedYMatrix( Ubus, Ubus0, augY );
      // LU factorize augmented Y matrix
      ublas::permutation_matrix<size_t> pm(augY.size1());
      pmatrix = pm;
      LUaugY = augY;
      BOOST_TRY {
        ublas::lu_factorize(LUaugY, pmatrix);
      } BOOST_CATCH(ublas::singular const& ex) {
        cout << "Singularity likely!" << endl;
        cout << "Exception message: " << ex.what() << endl;
      } BOOST_CATCH(std::exception const& ex) {
        cout << "Other exception caught!" << endl;
        cout << "Exception message: " << ex.what() << endl;
      } BOOST_CATCH(...){
        cout << "Operation failed!" << endl;
        return 4;
      } BOOST_CATCH_END
      // Solve power flow
      _solveNetwork( LUaugY, pmatrix, Xgen, _genBusIntId, Ubus );
      // Calculate machine currents
      _calculateMachineCurrents( Xgen, Ubus, _genBusIntId, Iq, Id, Pel );
      // Calculate bus current injections
      ublas::vector<complex<double> > U( Ubus.size() );
      std::copy(Ubus.begin(),Ubus.end(),U.begin());
      ublas::vector<complex<double> > I( ublas::prod(U,LUaugY) );
      temp_Ibus.resize(I.size());
      std::copy(I.begin(),I.end(),temp_Ibus.begin());

      // Save again the values after the event at t for the t+(just before, we saved them for t-)
      _time.push_back(t);
      _voltages.push_back(Ubus);
      _currents.push_back(temp_Ibus);
      _powers.push_back(Pel);
      _angles.push_back(temp_angles);
      _speeds.push_back(temp_speeds);
      _eq_tr.push_back(temp_eq);
      _ed_tr.push_back(temp_ed);
      currentTimeStep = _timeStep; // The event occured, returning to oldstep
    }
    t += currentTimeStep;
    ++iterationCount;
  }

//  double elapsedTime =
      timer.Stop();
//  cout << "Total sim time : "<< elapsedTime << " sec (";
//  cout << elapsedTime/iterationCount << "sec/iteration)" << endl;

  // Ask to store all the computed data to TDresults
  // according to the requested results from the user
  int ans = _storeTDResults(res);
  if (ans) return 5;

  return 0;
}

Powersystem const* Simulator_sw::do_getPws() const{ return _pws; }

// TODO: Building Y in _calculateAugmentedYMatrix inefficient!
int Simulator_sw::
_calculateAugmentedYMatrix( vector<complex<double> > const& Ubus,
                            vector<complex<double> > const& Ubus0,
                            ublas::matrix<complex<double> >& augY ){

  _sse->buildY(_pwsLocal, augY);
  size_t busCount = augY.size1();

  // Load power
  ublas::vector<complex<double> > loadS(_loadCount);
  for (size_t l=0;l<_loadCount;++l){
    Load const* load = _pwsLocal.getLoad(l);
    loadS.insert_element(l, complex<double>(load->Pdemand,-load->Qdemand));
  }

  // Equivvalent load admittance
  ublas::vector <complex<double> > yload(busCount);
  unsigned int busofld;
  for (size_t l=0;l<busCount;++l){
    yload.insert_element(l,0);
    for (size_t i=0;i<_loadCount;++i){
      busofld=_pwsLocal.getBus_intId(_pwsLocal.getLoad(i)->busExtId);
      if (busofld==l){
        switch ( _pwsLocal.getLoad(i)->type() ){
        case LOADTYPE_CONSTP:
          yload(l) = loadS[i]/std::abs(Ubus0[l])/std::abs(Ubus0[l]) * (Ubus0[l]/Ubus[l])* (Ubus0[l]/Ubus[l]);
          break;
        case LOADTYPE_CONSTI:
          yload(l) = loadS[i]/std::abs(Ubus0[l])/std::abs(Ubus0[l]) * (Ubus0[l]/Ubus[l]);
          break;
        case LOADTYPE_CONSTZ:
          yload(l) = loadS[i]/std::abs(Ubus0[l])/std::abs(Ubus0[l]);
          break;
        }
      }
    }
  }

  // Equivalent generator admittance
  vector<complex<double> > ygen( busCount, complex<double>(0.0,0.0) );
  for ( size_t i = 0 ; i < busCount ; ++i )
    for ( size_t g = 0 ; g < _genBusIntId.size() ; g++ )
      if ( i == _genBusIntId[g] )
        ygen[i] = 1.0/complex<double>(0.0,_pwsLocal.getGenerator(g)->xd_1);

  // Augmented Y matrix
  for ( size_t i = 0 ; i != busCount ; ++i)
    augY.at_element(i,i) += (ygen[i] + yload[i]);

  return 0;
}

void Simulator_sw::_solveNetwork( ublas::matrix<complex<double> > const& LUaugY,
                               ublas::permutation_matrix<size_t> const& pmatrix,
                               vector<vector<double> > const& Xgen,
                               vector<size_t> const& genBusIntId,
                               vector<complex<double> >& Ubus ){

  ublas::vector<complex<double> > Ibus(pmatrix.size(),complex<double>(0.0,0.0));
  for ( size_t i = 0 ; i != _genCount ; ++i ){

    complex<double> Vgen;
    if (_genModel[i]==GENMODEL_0p0){
      Vgen = complex<double>(Xgen[i][2],0.0)
                * std::exp(complex<double>(0.0,Xgen[i][0]));
    } else if (_genModel[i]==GENMODEL_1p1) {
      Vgen = complex<double>(Xgen[i][2],Xgen[i][3])
                * std::exp(complex<double>(0.0,Xgen[i][0]));
    }

    complex<double> Zgen = complex<double>(0.0, _pwsLocal.getGenerator(i)->xd_1);

    Ibus(genBusIntId[i]) += Vgen/Zgen;
  }
  ublas::lu_substitute(LUaugY, pmatrix, Ibus); // Ibus contains the solution

  Ubus.resize( Ibus.size() );
  std::copy( Ibus.begin(), Ibus.end(), Ubus.begin() );
}

void Simulator_sw::_calculateMachineCurrents( vector<vector<double> > const& Xgen,
                                              vector<complex<double> > const& Ubus,
                                              vector<size_t> const& genBusIntId,
                                              vector<double>& Iq,
                                              vector<double>& Id,
                                              vector<double>& Pel ){
  if ( Pel.size() != _genCount )
    Pel.resize(_genCount);

  for ( size_t i = 0 ; i < _genCount ; ++i ) {
    Generator const* gen = _pwsLocal.getGenerator(i);
    switch ( _genModel[i] ){
    case GENMODEL_0p0:
      Pel[i] = (1.0/gen->xd_1)
          * std::abs( Ubus[genBusIntId[i]] )
          * std::abs( Xgen[i][2] )
          * std::sin( Xgen[i][0]-std::arg(Ubus[genBusIntId[i]]) );
      break;
    case GENMODEL_1p1:{
      // Tranform U to rotor frame of reference
      double vd = -std::abs(Ubus[genBusIntId[i]])
                    * sin(Xgen[i][0]-std::arg(Ubus[genBusIntId[i]]));
      double vq =  std::abs(Ubus[genBusIntId[i]])
                    * cos(Xgen[i][0]-std::arg(Ubus[genBusIntId[i]]));
      Iq[i] = -(vd - Xgen[i][3])/gen->xq_1;
      Id[i] =  (vq - Xgen[i][2])/gen->xd_1;
      Pel[i] = Xgen[i][2]*Iq[i] + Xgen[i][3]*Id[i] + (gen->xd_1-gen->xq_1)*Id[i]*Iq[i];
      break;}
    }
  }
}

void
Simulator_sw::_calculateGeneratorDynamics( vector<vector<double> > const& Xgen,
                                           vector<double> const& Pel,
                                           vector<double> const& Efd,
                                           vector<double> const& Iq,
                                           vector<double> const& Id,
                                           vector<vector<double> >& dXgen ){

 double omegas = 2.0*M_PI*_pwsLocal.baseF();
 dXgen.resize( _genCount, vector<double>(4,0.0) );
 for ( size_t i = 0 ; i != _genCount ; ++i ){
   Generator const* gen = _pwsLocal.getGenerator(i);
   if (_genModel[i]==GENMODEL_0p0){
     dXgen[i][0] = Xgen[i][1] - omegas;
     dXgen[i][1] = omegas*(gen->Pgen - Pel[i] - gen->D*(Xgen[i][1]-omegas))
                   / gen->M;
     dXgen[i][2] = 0.0; // Eq doesnt change for model 1
   }
   else if (_genModel[i]==GENMODEL_1p1){
     dXgen[i][0] = Xgen[i][1] - omegas;
     dXgen[i][1] = omegas*(gen->Pgen - Pel[i] - gen->D*(Xgen[i][1]-omegas))
                   / gen->M;
     dXgen[i][2] = ( Efd[i] - Xgen[i][2] + (gen->xd-gen->xd_1)*Id[i] )
                   / gen->Td0_1;
     dXgen[i][3] = ( -Xgen[i][3]- (gen->xq-gen->xq_1)*Iq[i] )
                   / gen->Td0_1;
   }
 }
}

int Simulator_sw::_rungeKutta( ublas::matrix<complex<double> > const& LUaugY,
                               ublas::permutation_matrix<size_t> const& pmatrix,
                               vector<size_t> const& genBusIntId,
                               double stepSize,
                               vector<vector<double> >& Xgen,
                               vector<double>& Pel,
                               vector<double>& Efd,
                               vector<double>& Iq,
                               vector<double>& Id,
                               vector<complex<double> >& Ubus ){

  // ----- Construction of RK coefficients -----
  //   a = [ 0   0   0   0
  //         1/2 0   0   0
  //         0   1/2 0   0
  //         0   0   1   0   ];
  double RKcoef_a[4][4] = { {0  , 0  , 0, 0},
                            {0.5, 0  , 0, 0},
                            {0  , 0.5, 0, 0},
                            {0  , 0  , 1, 0} };
  //   b = [ 1/6 2/6 2/6 1/6 ];
  double RKcoef_b[4] = { 1.0/6.0, 2.0/6.0, 2.0/6.0, 1.0/6.0 };

  // ----- Initialization of variables -----
  vector<vector<double> > Kgen1, Kgen2, Kgen3, Kgen4;
  vector<vector<double> > XgenNew(_genCount, vector<double>(4,0.0));

  /* For each RK step S:
    - generator dynamics (KgenS) are calculated based on current state
    - generator dynamic variables are updated according to RK coefs and KgenS
    - the network is solved for bus voltages (Ubus)
    - machine currents (Iq, Id, and el. power Pel) are calculated for the new
      voltage profile
  */
  // ----- First step -----
  _calculateGeneratorDynamics(Xgen, Pel, Efd, Iq, Id, Kgen1);
  for (size_t gen=0;gen<_genCount;++gen){
    if (_genModel[gen]==GENMODEL_0p0){
      XgenNew[gen][0] = Xgen[gen][0] + stepSize*RKcoef_a[1][0]*Kgen1[gen][0];
      XgenNew[gen][1] = Xgen[gen][1] + stepSize*RKcoef_a[1][0]*Kgen1[gen][1];
      XgenNew[gen][2] = Xgen[gen][2] + stepSize*RKcoef_a[1][0]*Kgen1[gen][2];
    }
    else if (_genModel[gen]==GENMODEL_1p1){
      XgenNew[gen][0] = Xgen[gen][0] + stepSize*RKcoef_a[1][0]*Kgen1[gen][0];
      XgenNew[gen][1] = Xgen[gen][1] + stepSize*RKcoef_a[1][0]*Kgen1[gen][1];
      XgenNew[gen][2] = Xgen[gen][2] + stepSize*RKcoef_a[1][0]*Kgen1[gen][2];
      XgenNew[gen][3] = Xgen[gen][3] + stepSize*RKcoef_a[1][0]*Kgen1[gen][3];
    }
  }
  _solveNetwork( LUaugY, pmatrix, XgenNew, genBusIntId, Ubus );
  _calculateMachineCurrents( XgenNew, Ubus, genBusIntId, Iq, Id, Pel);

  // ----- Second step -----
  _calculateGeneratorDynamics(XgenNew, Pel, Efd, Iq, Id, Kgen2);
  for (size_t gen=0;gen<_genCount;++gen){
    if (_genModel[gen]==GENMODEL_0p0){
      XgenNew[gen][0] = Xgen[gen][0] + stepSize*(  RKcoef_a[2][0]*Kgen1[gen][0]
                                                + RKcoef_a[2][1]*Kgen2[gen][0]);
      XgenNew[gen][1] = Xgen[gen][1] + stepSize*(  RKcoef_a[2][0]*Kgen1[gen][1]
                                                + RKcoef_a[2][1]*Kgen2[gen][1]);
      XgenNew[gen][2] = Xgen[gen][2] + stepSize*(  RKcoef_a[2][0]*Kgen1[gen][2]
                                                + RKcoef_a[2][1]*Kgen2[gen][2]);
    }
    if (_genModel[gen]==GENMODEL_1p1){
      XgenNew[gen][0] = Xgen[gen][0] + stepSize*(  RKcoef_a[2][0]*Kgen1[gen][0]
                                                + RKcoef_a[2][1]*Kgen2[gen][0]);
      XgenNew[gen][1] = Xgen[gen][1] + stepSize*(  RKcoef_a[2][0]*Kgen1[gen][1]
                                                + RKcoef_a[2][1]*Kgen2[gen][1]);
      XgenNew[gen][2] = Xgen[gen][2] + stepSize*(  RKcoef_a[2][0]*Kgen1[gen][2]
                                                + RKcoef_a[2][1]*Kgen2[gen][2]);
      XgenNew[gen][3] = Xgen[gen][3] + stepSize*(  RKcoef_a[2][0]*Kgen1[gen][3]
                                                + RKcoef_a[2][1]*Kgen2[gen][3]);
    }
  }
  _solveNetwork( LUaugY, pmatrix, XgenNew, genBusIntId, Ubus );
  _calculateMachineCurrents( XgenNew, Ubus, genBusIntId, Iq, Id, Pel);

  // ----- Third step -----
  _calculateGeneratorDynamics(XgenNew, Pel, Efd, Iq, Id, Kgen3);
  for (size_t gen=0;gen<_genCount;++gen){
    if (_genModel[gen]==GENMODEL_0p0){
      XgenNew[gen][0] = Xgen[gen][0] + stepSize*(  RKcoef_a[3][0]*Kgen1[gen][0]
                                                + RKcoef_a[3][1]*Kgen2[gen][0]
                                                + RKcoef_a[3][2]*Kgen3[gen][0]);
      XgenNew[gen][1] = Xgen[gen][1] + stepSize*(  RKcoef_a[3][0]*Kgen1[gen][1]
                                                + RKcoef_a[3][1]*Kgen2[gen][1]
                                                + RKcoef_a[3][2]*Kgen3[gen][1]);
      XgenNew[gen][2] = Xgen[gen][2] + stepSize*(  RKcoef_a[3][0]*Kgen1[gen][2]
                                                + RKcoef_a[3][1]*Kgen2[gen][2]
                                                + RKcoef_a[3][2]*Kgen3[gen][2]);
    }
    if (_genModel[gen]==GENMODEL_1p1){
      XgenNew[gen][0] = Xgen[gen][0] + stepSize*(  RKcoef_a[3][0]*Kgen1[gen][0]
                                                + RKcoef_a[3][1]*Kgen2[gen][0]
                                                + RKcoef_a[3][2]*Kgen3[gen][0]);
      XgenNew[gen][1] = Xgen[gen][1] + stepSize*(  RKcoef_a[3][0]*Kgen1[gen][1]
                                                + RKcoef_a[3][1]*Kgen2[gen][1]
                                                + RKcoef_a[3][2]*Kgen3[gen][1]);
      XgenNew[gen][2] = Xgen[gen][2] + stepSize*(  RKcoef_a[3][0]*Kgen1[gen][2]
                                                + RKcoef_a[3][1]*Kgen2[gen][2]
                                                + RKcoef_a[3][2]*Kgen3[gen][2]);
      XgenNew[gen][3] = Xgen[gen][3] + stepSize*(  RKcoef_a[3][0]*Kgen1[gen][3]
                                                + RKcoef_a[3][1]*Kgen2[gen][3]
                                                + RKcoef_a[3][2]*Kgen3[gen][3]);
    }
  }
  _solveNetwork( LUaugY, pmatrix, XgenNew, genBusIntId, Ubus );
  _calculateMachineCurrents( XgenNew, Ubus, genBusIntId, Iq, Id, Pel);

  // ----- Fourth step -----
  _calculateGeneratorDynamics(XgenNew, Pel, Efd, Iq, Id, Kgen4);
  for (size_t gen=0;gen<_genCount;++gen){
    if (_genModel[gen]==GENMODEL_0p0){
      XgenNew[gen][0] = Xgen[gen][0] + stepSize*(  RKcoef_b[0]*Kgen1[gen][0]
                                                + RKcoef_b[1]*Kgen2[gen][0]
                                                + RKcoef_b[2]*Kgen3[gen][0]
                                                + RKcoef_b[3]*Kgen4[gen][0] );
      XgenNew[gen][1] = Xgen[gen][1] + stepSize*(  RKcoef_b[0]*Kgen1[gen][1]
                                                + RKcoef_b[1]*Kgen2[gen][1]
                                                + RKcoef_b[2]*Kgen3[gen][1]
                                                + RKcoef_b[3]*Kgen4[gen][1] );
      XgenNew[gen][2] = Xgen[gen][2] + stepSize*(  RKcoef_b[0]*Kgen1[gen][2]
                                                + RKcoef_b[1]*Kgen2[gen][2]
                                                + RKcoef_b[2]*Kgen3[gen][2]
                                                + RKcoef_b[3]*Kgen4[gen][2] );
    }
    if (_genModel[gen]==GENMODEL_1p1){
      XgenNew[gen][0] = Xgen[gen][0] + stepSize*(  RKcoef_b[0]*Kgen1[gen][0]
                                                + RKcoef_b[1]*Kgen2[gen][0]
                                                + RKcoef_b[2]*Kgen3[gen][0]
                                                + RKcoef_b[3]*Kgen4[gen][0] );
      XgenNew[gen][1] = Xgen[gen][1] + stepSize*(  RKcoef_b[0]*Kgen1[gen][1]
                                                + RKcoef_b[1]*Kgen2[gen][1]
                                                + RKcoef_b[2]*Kgen3[gen][1]
                                                + RKcoef_b[3]*Kgen4[gen][1] );
      XgenNew[gen][2] = Xgen[gen][2] + stepSize*(  RKcoef_b[0]*Kgen1[gen][2]
                                                + RKcoef_b[1]*Kgen2[gen][2]
                                                + RKcoef_b[2]*Kgen3[gen][2]
                                                + RKcoef_b[3]*Kgen4[gen][2] );
      XgenNew[gen][3] = Xgen[gen][3] + stepSize*(  RKcoef_b[0]*Kgen1[gen][3]
                                                + RKcoef_b[1]*Kgen2[gen][3]
                                                + RKcoef_b[2]*Kgen3[gen][3]
                                                + RKcoef_b[3]*Kgen4[gen][3] );
    }
  }
  _solveNetwork( LUaugY, pmatrix, XgenNew, genBusIntId, Ubus );
  _calculateMachineCurrents( XgenNew, Ubus, genBusIntId, Iq, Id, Pel);

  Xgen = XgenNew;

  return 0;
}

int Simulator_sw::_storeTDResults(TDResults& res){
  //Separate real and imag part of voltages and currents
  vector<double>volt_real;
  vector<double>volt_imag;
  vector<double>current_real;
  vector<double>current_imag;

  for (size_t i=0;i<res.dataExists().size();++i){
    // Only store if user selected so
    if (res.dataExists().at(i)){
      //Take the identifier for the specific data point
      TDResultIdentifier tdri=res.identifiers().at(i);
      volt_real.clear();
      volt_imag.clear();
      current_real.clear();
      current_imag.clear();
      if (tdri.elementType==TDRESULTS_BUS){
        int intid = _pwsLocal.getBus_intId(tdri.extId);
        for (size_t n=0;n<_voltages.size();++n){
          volt_real.push_back(std::real(_voltages.at(n).at(intid)));
          volt_imag.push_back(std::imag(_voltages.at(n).at(intid)));
        }
        if (tdri.variable==TDRESULTS_VOLTAGE_REAL)
          res.set_data(i,volt_real);
        else if (tdri.variable==TDRESULTS_VOLTAGE_IMAG)
          res.set_data(i,volt_imag);
      }
      else if (tdri.elementType==TDRESULTS_GENERATOR){
        int intid = _pwsLocal.getGen_intId(tdri.extId);
        for (size_t n=0;n<_voltages.size();++n){
          volt_real.push_back(std::real(_voltages.at(n).at(_genBusIntId[intid])));
          volt_imag.push_back(std::imag(_voltages.at(n).at(_genBusIntId[intid])));
        }
        for (size_t n=0;n<_currents.size();++n){
          current_real.push_back(std::real(_currents.at(n).at(_genBusIntId[intid])));
          current_imag.push_back(std::imag(_currents.at(n).at(_genBusIntId[intid])));
        }
        vector<double> temp_angles;
        vector<double> temp_speeds;
        vector<double> temp_internal_voltages;
        vector<double> temp_acc_power;
        double Pmech = _pwsLocal.getGenerator(intid)->Pgen;
        for ( size_t n=0 ; n<_angles.size() ; ++n ){
          temp_angles.push_back(_angles.at(n).at(intid));
          temp_speeds.push_back(_speeds.at(n).at(intid) - 1); // for uniformity
                                                             // with emu_hw res
          temp_internal_voltages.push_back(_eq_tr.at(n).at(intid));
          temp_acc_power.push_back( Pmech - _powers.at(n).at(intid) );
        }
        if (tdri.variable==TDRESULTS_ANGLE){
          res.set_data(i,temp_angles);
        }
        else if (tdri.variable==TDRESULTS_SPEED){
          res.set_data(i,temp_speeds);
        }
        else if (tdri.variable==TDRESULTS_ACCELERATING_POWER){
          res.set_data(i,temp_acc_power);
        }
        else if (tdri.variable==TDRESULTS_VOLTAGE_REAL){
          res.set_data(i,volt_real);
        }
        else if (tdri.variable==TDRESULTS_VOLTAGE_IMAG){
          res.set_data(i,volt_imag);
        }
        else if (tdri.variable==TDRESULTS_CURRENT_REAL){
          res.set_data(i,current_real);
        }
        else if (tdri.variable==TDRESULTS_CURRENT_IMAG){
          res.set_data(i,current_imag);
        }

      }
      else if (tdri.elementType==TDRESULTS_LOAD){
        int intid = _pwsLocal.getLoad_intId(tdri.extId);
        for (size_t n=0;n<_voltages.size();++n){
          volt_real.push_back(std::real(_voltages.at(n).at(_loadBusIntId[intid])));
          volt_imag.push_back(std::imag(_voltages.at(n).at(_loadBusIntId[intid])));
        }
        for (size_t n=0;n<_currents.size();++n){
          current_real.push_back(std::real(_currents.at(n).at(_loadBusIntId[intid])));
          current_imag.push_back(std::imag(_currents.at(n).at(_loadBusIntId[intid])));
        }
        if (tdri.variable==TDRESULTS_VOLTAGE_REAL){
          res.set_data(i,volt_real);
        }
        else if (tdri.variable==TDRESULTS_VOLTAGE_IMAG){
          res.set_data(i,volt_imag);
        }
        else if (tdri.variable==TDRESULTS_CURRENT_REAL){
          res.set_data(i,current_real);
        }
        else if (tdri.variable==TDRESULTS_CURRENT_IMAG){
          res.set_data(i,current_imag);
        }

      }
      else if (tdri.elementType==TDRESULTS_OTHER){//Probably time
        if (tdri.variable==TDRESULTS_TIME)
          res.set_data(i,_time);
      }
    }
  }
  return 0;
}

int Simulator_sw::_parseBusFault(Event &event){
  int intid=_pwsLocal.getBus_intId(event.element_extId());
  Bus* busofevent;
  int ans = _pwsLocal.getBus(event.element_extId(),busofevent);
  if (ans) return 1;

  // 0: 3ph fault
  if (event.event_type()==EVENT_EVENTTYPE_BUSFAULT){
    if (event.bool_arg()){
      //Storing the old
      _busintid.push_back(intid);
      _oldgsh.push_back(busofevent->Gsh);
      _oldbsh.push_back(busofevent->Bsh);
      double gsh;
      double bsh;
      gsh=1/event.double_arg_1();
      bsh=-1/event.double_arg_2();
      busofevent->Gsh = gsh;
      busofevent->Bsh = bsh;
    }

    //Restore the saved data
    else{
      for (size_t i=0;i<_busintid.size();++i){
        if (intid==_busintid[i]){
          busofevent->Gsh = _oldgsh[i];
          busofevent->Bsh = _oldbsh[i];
          _busintid.erase(_busintid.begin()+i);
          _oldgsh.erase(_oldgsh.begin()+i);
          _oldbsh.erase(_oldbsh.begin()+i);
        }
      }
    }
  }
  return 0;
}

int Simulator_sw::_parseBrFault(Event &event){

  Branch* branchofevent;
  int ans=_pwsLocal.getBranch(event.element_extId(),branchofevent);
  if (ans) return 1;

  // 0: 3ph fault,we have to handle the location of the fault accordingly, at
  // the location of the fault, we create a bus with 3phaze fault and two lines
  if (event.event_type()==EVENT_EVENTTYPE_BRFAULT){

    // Check first if it is on the bus
    if (event.bool_arg()){
      // Storing the old and putting fault -
      if (DOUBLE_EQ(event.double_arg(),0)){
        // - on the from bus of line
        Bus* busofevent; // the bus of fault
        int ans=_pwsLocal.getBus(branchofevent->fromBusExtId,busofevent);
        if (ans) return 2;

        _br_frombus_intid.push_back(_pwsLocal.getBus_intId(branchofevent->fromBusExtId));
        _br_frombus_oldgsh.push_back(busofevent->Gsh);
        _br_frombus_oldbsh.push_back(busofevent->Bsh);
        double gsh;
        double bsh;
        gsh=1/event.double_arg_1();
        bsh=-1/event.double_arg_2();
        busofevent->Gsh = gsh;
        busofevent->Bsh = bsh;
      }

      // - on the to bus of line
      else if(DOUBLE_EQ(event.double_arg(),1)){
        Bus* busofevent;//the bus of fault
        int ans=_pwsLocal.getBus(branchofevent->toBusExtId,busofevent);
        if (ans) return 3;

        _br_tobus_intid.push_back(_pwsLocal.getBus_intId(branchofevent->toBusExtId));
        _br_tobus_oldgsh.push_back(busofevent->Gsh);
        _br_tobus_oldbsh.push_back(busofevent->Bsh);
        double gsh = 1/event.double_arg_1();
        double bsh = -1/event.double_arg_2();
        busofevent->Gsh = gsh;
        busofevent->Bsh = bsh;
      }

      // - at an other location of line between two buses
      else {
        // Create a new fault bus in location
        double loc = event.double_arg();
        Bus* busofevent = new Bus();
        // Set the default first ext id
        int maxextid = _pwsLocal.getBus_extId(0);
        for (size_t m=0;m<_busCount;m++){
          if(_pwsLocal.getBus_extId(m)>maxextid)
            maxextid=_pwsLocal.getBus_extId(m);
        }
//        unsigned int maxextid;
//        for ( maxextid = 0 ; maxextid != 0 ; ++maxextid )
//        if ( _pws.getBus_intId( maxextid ) == -1 )
//        break;
        busofevent->extId = maxextid+1;
        double gsh = 1/event.double_arg_1();
        double bsh = -1/event.double_arg_2();
        busofevent->Gsh = gsh;
        busofevent->Bsh = bsh;
        branchofevent->status = false; //trip the old line
        // Create a new branch  frombus the new faulty bus
        _pwsLocal.addBus(*busofevent);

        Branch* newfromline = new Branch();
        int maxextidfrombr=_pwsLocal.getBr_extId(0);//Set the default first ext id
        for (size_t m=0;m<_pwsLocal.getBrSet_size();m++){
          if(_pwsLocal.getBr_extId(m)>maxextidfrombr)
            maxextidfrombr=_pwsLocal.getBr_extId(m);
        }
//        unsigned int maxextidfrombr;
//        for ( maxextidfrombr = 0 ; maxextidfrombr != 0 ; ++maxextidfrombr )
//        if ( _pws.getBr_intId( maxextidfrombr ) == -1 )
//        break;
        newfromline->extId = maxextidfrombr+1;
        newfromline->status = true;
        newfromline->fromBusExtId = branchofevent->fromBusExtId;
        newfromline->toBusExtId = maxextid+1;//the fault bus extid
        newfromline->R = branchofevent->R*loc;
        newfromline->X = branchofevent->X*loc;
        newfromline->Bfrom = branchofevent->Bfrom*loc;
        newfromline->Bto = branchofevent->Bto*loc;
        newfromline->Gfrom = branchofevent->Gfrom*loc;
        newfromline->Gto = branchofevent->Gto*loc;


        // Create a new branch  tobus the new faulty bus
//        _pws.getBranch(event.element_extId(),branchofevent);
        Branch* newtoline = new Branch();
        int maxextidtobr=_pwsLocal.getBr_extId(0);//Set the default first ext id
        for (size_t m=0;m<_pwsLocal.getBrSet_size();m++){
          if(_pwsLocal.getBr_extId(m)>maxextidtobr)
            maxextidtobr=_pwsLocal.getBr_extId(m);
        }
//        unsigned int maxextidtobr;
//        for ( maxextidtobr = 0 ; maxextidtobr != 0 ; ++maxextidtobr )
//        if ( _pws.getBr_intId( maxextidtobr ) == -1 )
//        break;
        newtoline->extId = maxextidtobr+2;
        newtoline->status = true;
        newtoline->toBusExtId = branchofevent->toBusExtId;
        newtoline->fromBusExtId = maxextid+1;
        newtoline->R = branchofevent->R*(1-loc);
        newtoline->X = branchofevent->X*(1-loc);
        newtoline->Bfrom = branchofevent->Bfrom*(1-loc);
        newtoline->Bto = branchofevent->Bto*(1-loc);
        newtoline->Gfrom = branchofevent->Gfrom*(1-loc);
        newtoline->Gto = branchofevent->Gto*(1-loc);

        _br_branchoffault_extid.push_back(branchofevent->extId);
        _br_faultbus_extid.push_back(maxextid+1);

        _pwsLocal.addBranch(*newfromline);
        _pwsLocal.addBranch(*newtoline);
        //We dont keep the old gsh bsh because in the bus we create is always 0
        //We keep only the exintid! of the fault bus and the two new lines

      }
    }

    else{
      //Restore the saved data -
      if (DOUBLE_EQ(event.double_arg(),0)){
        // - on the from bus of line
        Bus* busofevent; // the bus of fault
        int ans = _pwsLocal.getBus(branchofevent->fromBusExtId,busofevent);
        if ( ans ) return 7;

        for(size_t i=0;i< _br_frombus_intid.size();++i){
          if(_pwsLocal.getBus_intId(branchofevent->fromBusExtId)==_br_frombus_intid[i]){
            busofevent->Gsh = _br_frombus_oldgsh[i];
            busofevent->Bsh = _br_frombus_oldbsh[i];
            _br_frombus_intid.erase(_br_frombus_intid.begin()+i);
            _br_frombus_oldgsh.erase(_br_frombus_oldgsh.begin()+i);
            _br_frombus_oldbsh.erase(_br_frombus_oldbsh.begin()+i);
          }
        }
      }

      else if(DOUBLE_EQ(event.double_arg(),1)){
        // - on the to bus of line
        Bus* busofevent; // the bus of fault
        int ans=_pwsLocal.getBus(branchofevent->toBusExtId,busofevent);
        if (ans) return 8;

        for(size_t i=0;i< _br_tobus_intid.size();++i){
          if(_pwsLocal.getBus_intId(branchofevent->toBusExtId)==_br_tobus_intid[i]){
            busofevent->Gsh = _br_tobus_oldgsh[i];
            busofevent->Bsh = _br_tobus_oldbsh[i];
            _br_tobus_intid.erase(_br_tobus_intid.begin()+i);
            _br_tobus_oldgsh.erase(_br_tobus_oldgsh.begin()+i);
            _br_tobus_oldbsh.erase(_br_tobus_oldbsh.begin()+i);
          }
        }
      }

      else{
        // - at an other location of line between two buses
        // Search to find the fault bus that corresponds to the branch that had
        // a fault
        Bus* busofevent;
        for (size_t i=0;i<_br_branchoffault_extid.size();++i){
          // We found the line in the two vectors that corresponds to the
          // particular branch
          if (_br_branchoffault_extid[i]==branchofevent->extId){
            int ans=_pwsLocal.getBus(_br_faultbus_extid[i],busofevent);
            if (ans) return 9;

            // Removes the faulty bus, and also the two connected branches
            ans=_pwsLocal.deleteBus(busofevent->extId,true);
            if (ans) return 10;

            // Remove the fault entry
            _br_branchoffault_extid.erase(_br_branchoffault_extid.begin()+i);
            _br_faultbus_extid.erase(_br_faultbus_extid.begin()+i);

            branchofevent->status = true;
          }
        }
      }
    }
  }

  // 1: trip
  else if (event.event_type()==EVENT_EVENTTYPE_BRTRIP){
    if (event.bool_arg()){
      Bus* busofevent;
      // Search to find if the trip is in an already faulty branch
      for (size_t i=0;i<_br_branchoffault_extid.size();++i){
        if (_br_branchoffault_extid[i]==branchofevent->extId){

          // We found the line in the two vectors that corresponds to the
          // particular branch
          int ans=_pwsLocal.getBus(_br_faultbus_extid[i],busofevent);
          if (ans) return 11;

          // Removes the faulty bus, and also the two connected branches
          ans = _pwsLocal.deleteBus(busofevent->extId,true);
          if(ans) return 12;

          // Remove the fault entry
          _br_branchoffault_extid.erase(_br_branchoffault_extid.begin()+i);
          _br_faultbus_extid.erase(_br_faultbus_extid.begin()+i);
        }
      }

      branchofevent->status = false; //status == false meaning tripped
    }
    else{
      branchofevent->status = true;
    }
  }

  // 2: short
  else if (event.event_type()==EVENT_EVENTTYPE_BRSHORT){
    branchofevent->R = 1e-14; // near 0
    branchofevent->R = 1e-14;
  }

  return 0;
}

int Simulator_sw::_parseGenFault(Event &event){

  Generator* genofevent;
  int ans=_pwsLocal.getGenerator(event.element_extId(),genofevent);
  if ( ans ) return 1;

  if (event.event_type()==EVENT_EVENTTYPE_GENTRIP){//0: trip
    if (event.bool_arg()){
      genofevent->status = false; //status == false meaning tripped
      _genBusIntId.clear();
      for (size_t i=0;i<_genCount;++i){
        if (_pwsLocal.getGenerator(i)->status == true){
          _genBusIntId.push_back(_pwsLocal.getBus_intId(_pwsLocal.getGenerator(i)->busExtId));
        }
      }
      --_genCount;
    }
    else{
      genofevent->status = true;
      ++_genCount;
      _genBusIntId.clear();
      for (size_t i=0;i<_genCount;++i){
        if (_pwsLocal.getGenerator(i)->status == true){
          _genBusIntId.push_back(_pwsLocal.getBus_intId(_pwsLocal.getGenerator(i)->busExtId));
        }
      }
    }
  }
  else if (event.event_type()==EVENT_EVENTTYPE_GENPCHANGE){//1: p change
    genofevent->Pgen = event.double_arg();
  }
  else if (event.event_type()==EVENT_EVENTTYPE_GENQCHANGE){//2: q change
    genofevent->Qgen = event.double_arg();
  }

  return 0;
}

int Simulator_sw::_parseLoadFault(Event &event){

  Load* loadofevent;
  int ans = _pwsLocal.getLoad(event.element_extId(),loadofevent);
  if ( ans ) return 1;

  if (event.event_type()==EVENT_EVENTTYPE_LOADPCHANGE){//1: p change
    loadofevent->Pdemand = event.double_arg();
  }
  else if (event.event_type()==EVENT_EVENTTYPE_LOADQCHANGE){//2: q change
    loadofevent->Qdemand = event.double_arg();
  }
  return 0;
}
