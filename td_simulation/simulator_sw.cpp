
#include "simulator_sw.h"
using namespace elabtsaot;

#include "auxiliary.h"
using auxiliary::operator <<;
#include "logger.h"
#include "powersystem.h"
#include "auxiliary.h"
#include "scenario.h"
#include "tdresults.h"
#include "tdresultsidentifier.h"
#include "ssutils.h"

#include <QProgressBar>

//#include <vector>
using std::vector;
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

#define DBL_E 0.0000000001
#define DOUBLE_EQ(x,v) (((v - DBL_E) < x) && (x <( v + DBL_E)))

Simulator_sw::Simulator_sw( Powersystem const* pws, Logger* log ) :
    TDEngine(pws, "Software simulator", 0.01, log), _pwsLocal(*pws) {}

int Simulator_sw::init(Powersystem const *pws){

  if (pws != NULL && pws != _pws){
    // A new powersystem has been supplied as argument to the Simulator_hw
    _pws = pws;
  } else {
    // The powersystem pointer remains the same (or no arg has been provided)
  }
  // Anyway update the internal copy of the powersystem, in order to account for
  // any changes in the pointed-to Powersystem
  _pwsLocal = *_pws;

  // Transient saliency is not supported! => xq_t = xd_t
  size_t genCount = _pwsLocal.getGenCount();
  for (size_t i=0; i!=genCount; ++i){
    Generator* gen = _pwsLocal.getGenerator(i);
    gen->xq_1 = gen->xd_1;
  }

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

  return 0;
}

bool Simulator_sw::do_isEngineCompatible(Scenario const& sce) const{

  bool sceCompatible(true);
  size_t eventSetSize = sce.getEventSetSize();
  for (size_t i=0; i!=eventSetSize; ++i){
    // Update event compatibility flag
    Event ev = sce.getEvent(i);
    bool evCompatible = false;
    switch (ev.element_type()){
    case EVENT_ELEMENT_TYPE_BUS:
      switch (ev.event_type()){
      case EVENT_EVENTTYPE_BUSFAULT:
        evCompatible = true;
        break;
      }
      break;

    case EVENT_ELEMENT_TYPE_BRANCH:
      switch (ev.event_type()){
      case EVENT_EVENTTYPE_BRFAULT:
        evCompatible = true;
        break;
      case EVENT_EVENTTYPE_BRTRIP:
        evCompatible = true;
        break;
      case EVENT_EVENTTYPE_BRSHORT:
        evCompatible = false;
        break;
      }
      break;

    case EVENT_ELEMENT_TYPE_GEN:
    case EVENT_ELEMENT_TYPE_LOAD:
    case EVENT_ELEMENT_TYPE_OTHER:
      evCompatible = false;
      break;
    }

    // Update global scenario compatibility flag
    if (!evCompatible){
      sceCompatible = false;
      break;
    }
  }
  return sceCompatible;
}

int Simulator_sw::do_simulate( Scenario const& sce, TDResults& res){

//  cout.precision(10);
  init(NULL);
  if (_pwsLocal.status()!=PWSSTATUS_PF) return 1;


  // **********************************
  // ***** GENERAL INITIALIZATION *****
  // **********************************
  Scenario _sce(sce);
  _sce.sort_t(); // Put the events in order

  double baseF = _pwsLocal.baseF;
  size_t genCount = _pwsLocal.getGenCount();
  size_t loadCount = _pwsLocal.getLoadCount();
  size_t busCount = _pwsLocal.getBusCount();
  ublas::vector<complex> Vbus(busCount, complex(1.0,0.0)); // bus voltages
  ublas::vector<complex> Ibus(busCount, complex(0.0,0.0)); // bus currents
  ublas::matrix<complex,ublas::column_major> augY;         // augmented Y matrix
  ublas::vector<complex> IN_1(busCount,0.0);               // Norton equivalent transient current source at buses
  /* For GENMODEL_0p0 ("classical" generator model):
       Xgen[*][0] -> delta in [rad]
       Xgen[*][1] -> omega in [rad/sec]
       Xgen[*][2] -> internal voltage E in [pu]
     For GENMODEL_1p1 ("1.1" generator model)
       Xgen[*][0] -> delta in [rad]
       Xgen[*][1] -> omega in [rad/sec]
       Xgen[*][2] -> transient quadrature axis voltage magnitude in [pu]
       Xgen[*][3] -> transient direct axis voltage magnitude in [pu]      */
  vector<vector<double> > Xgen(genCount, vector<double>(4,0.0) );
  vector<double> Pel(genCount,0.0);                        // Electical power
  vector<double> Efd(genCount,0.0);                        // Exitation current


  // ***************************************
  // ***** BUS AND GRID INITIALIZATION *****
  // ***************************************
  // Initialize bus voltages
  for (size_t k = 0 ; k != busCount ; ++k){
    Bus const* bus(_pwsLocal.getBus(k));
    Vbus(k) = std::polar(bus->V,bus->theta);
  }

  // Calculate and augment Y matrix
  ssutils::buildY(_pwsLocal, augY);
  _augmentYForLoads(augY,Vbus);
  _augmentYForGenerators(augY);

  // Factorize augmented Y matrix
  ublas::permutation_matrix<size_t> PaugY = ublas::permutation_matrix<size_t>(augY.size1());
  ublas::matrix<complex,ublas::column_major> LUaugY = augY;
  BOOST_TRY {
    ublas::lu_factorize(LUaugY, PaugY);
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

  // Check whether loads other than constant impedance are present in the system
  bool pwsHasNonZLoads = false;
  for (size_t k=0; k!=loadCount; ++k){
    Load const* load = _pwsLocal.getLoad(k);
    if (load->type()!=LOADTYPE_CONSTZ){
      pwsHasNonZLoads = true;
      break;
    }
  }


  // *********************************
  // ***** INITIALIZE GENERATORS *****
  // *********************************
  for (size_t i=0; i!=genCount; ++i){
    Generator* gen = _pwsLocal.getGenerator(i);
    int busExtId = gen->busExtId;
    int busIntId = _pwsLocal.getBus_intId(busExtId);

    switch (gen->model){

    case GENMODEL_0p0:{
      // Initialize dynamic variables
      complex Sgen(gen->Pgen,gen->Qgen);
      complex Vbus_ = Vbus(busIntId);
      double delta;  // = gen->deltass();
      double omega;
      double EQDm_1; // = gen->Ess();
      double Pel_;
      _init0p0(Sgen, Vbus_, gen->ra, gen->xd_1, baseF,
               delta, omega, EQDm_1, Pel_);

      // Update the global stores
      Xgen[i][0] = delta;
      Xgen[i][1] = omega;
      Xgen[i][2] = EQDm_1;
      Pel[i] = Pel_;
      break;}

    case GENMODEL_1p1:{
      // Initialize dynamic variables
      complex Sgen(gen->Pgen,gen->Qgen);
      complex Vbus_ = Vbus(busIntId);
      double delta;
      double omega;
      double Eq_1;
      double Ed_1;
      double Efd_;
      double Pel_;
      _init1p1(Sgen, Vbus_, gen->ra, gen->xq, gen->xd, gen->xq_1, gen->xd_1, baseF,
               delta, omega, Eq_1, Ed_1, Efd_, Pel_);

      // Update the global stores
      Xgen[i][0] = delta;
      Xgen[i][1] = omega;
      Xgen[i][2] = Eq_1;
      Xgen[i][3] = Ed_1;
      Efd[i] = Efd_;
      Pel[i] = Pel_;

      break;}
    }
  }


  // *********************************
  // *********** MAIN LOOP ***********
  // *********************************
  long int iterationCount = 0;
  size_t eventCounter = 0;

  // Result stores
  vector<double> time_store;
  vector<vector<double> > angles_store(genCount);
  vector<vector<double> > speeds_store(genCount);
  vector<vector<double> > eq_tr_store(genCount);
  vector<vector<double> > ed_tr_store(genCount);
  vector<vector<double> > Pel_store(genCount);
  vector<vector<complex> > Vbus_store(busCount);
  vector<vector<complex> > Ibus_store(busCount);

  vector<Event> curEvents; //concurrent events at specific t

  // Store the step size before doing changes in order to restore i in the end
  double currentTimeStep = _timeStep;
  double stopTime = _sce.stopTime();
  double t = _sce.startTime() - 2*currentTimeStep; // prevision in case there are events at t=0

//  cout << "INIT" << endl;
//  cout << "augY: " << augY << endl;
//  cout << "LUaugY: " << LUaugY << endl;
//  cout << "PaugY: " << PaugY << endl;

  // Xgen, Pel, Vbus change after here for each run
  while (t < stopTime+currentTimeStep){

    // Set currentTimeStep so that this iteration stops exactly at stopTime
    if (t+currentTimeStep > stopTime)
      currentTimeStep = stopTime-t;

//    cout << "ITERATION: " << iterationCount << endl;
//    cout << "TIME: " << t << endl;

    // ************************************
    // ****** HANDLING OF NON-ZLOADS ******
    // ************************************
    // ----- In case of non-zload recalculate augmented Y matrix -----
    if (pwsHasNonZLoads){ // COMMENTED-OUT ONLY FOR DEBUG PURPOSES
      // Calculate augmented Y matrix proper
      ssutils::buildY(_pwsLocal, augY);
      _augmentYForLoads(augY, Vbus);
      _augmentYForGenerators(augY);
      // Factorize augmented Y matrix
      PaugY = ublas::permutation_matrix<size_t>(augY.size1());
      LUaugY = augY;
      BOOST_TRY {
        ublas::lu_factorize(LUaugY, PaugY);
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
//      cout << "NON-ZLOADS" << endl;
//      cout << "augY: " << augY << endl;
//      cout << "LUaugY: " << LUaugY << endl;
//      cout << "PaugY: " << PaugY << endl;
    } // COMMENTED-OUT ONLY FOR DEBUG PURPOSES

    // ************************************
    // ******** MAIN SEQUENCE *************
    // ************************************
//    cout << "MAIN SEQUENCE" << endl;
    Xgen = _rungeKutta(LUaugY, PaugY, Xgen, Efd, currentTimeStep); // integration
//    cout << "Xgen: " << Xgen << endl;
    IN_1 = _calculateNortonCurrents(Xgen);                         // determine internal Norton currents
//    cout << "IN_1: " << IN_1 << endl;
    Vbus = _calculateGridVoltages(LUaugY, PaugY, IN_1);            // get grid voltage profile
//    cout << "Vbus: " << Vbus << endl;
    Ibus = ublas::prod(augY,Vbus);                                 // calculate bus currents
//    cout << "Ibus: " << Ibus << endl;
    Pel = _calculateGeneratorPowers(Vbus, Xgen);                   // calculate generator electric powers
//    cout << "Pel: " << Pel << endl;


    // ************************************
    // ******* STORE RESULTS **************
    // ************************************
    // Store results only for positive time
    if (t < 0.){
      t += currentTimeStep;
      continue;
    }
    // Save the data in the local stores; the final store would be after the loop to the TD results
    time_store.push_back(t);
    for (size_t k(0); k!=genCount; ++k){
      angles_store[k].push_back(Xgen[k][0]);
      speeds_store[k].push_back(Xgen[k][1]/(2*M_PI*baseF));
      eq_tr_store[k].push_back(Xgen[k][2]);
      ed_tr_store[k].push_back(Xgen[k][3]);
      Pel_store[k].push_back(Pel[k]);
    }
    for (size_t k(0); k!=busCount; ++k){
      Vbus_store[k].push_back(Vbus(k));
      Ibus_store[k].push_back(Ibus(k));
    }


    // ************************************
    // ********** EVENT HANDLING **********
    // ************************************
    // Check if next event occures in a smaller step
    if (eventCounter != _sce.getEventSetSize()){
      // Set only if there is an event left
      if ((t+currentTimeStep) >= _sce.getEvent(eventCounter).time()
          && _sce.getEvent(eventCounter).status()){
        // Event occured to a time smaller that time step
        currentTimeStep=_sce.getEvent(eventCounter).time()-t;
      }
    }
    // Check if event occured
    curEvents.clear();
    for (size_t i=eventCounter;i<_sce.getEventSetSize();++i){
      // Start from eventCounter, not from 0 but the next event, scenario must be in order
      if (DOUBLE_EQ(t,_sce.getEvent(i).time()) && _sce.getEvent(i).status()){
        curEvents.push_back(_sce.getEvent(i));
        eventCounter++;
      }
    }
    for (size_t i=0;i<curEvents.size();++i){
      if (curEvents[i].element_type() == EVENT_ELEMENT_TYPE_BUS)
        _parseBusFault(curEvents[i]); // Bus fault
      if (curEvents[i].element_type() == EVENT_ELEMENT_TYPE_BRANCH)
        _parseBrFault(curEvents[i]); // Branch fault
    }
    // The new parameters of the pws was set, running power flow now
    if (curEvents.size() != 0) {

      /* If event happened:
          - build Y matrix
          - calculate augmented Y matrix
          - LU factorize augmented Y matrix
          - solve power flow
          - calculate machine currents
          - calculate bus current injections */
      // Calculate augmented Y matrix
      ssutils::buildY(_pwsLocal, augY);
      _augmentYForLoads(augY,Vbus);
      _augmentYForGenerators(augY);
      // LU factorize augmented Y matrix
      ublas::permutation_matrix<size_t> pm(augY.size1());
      PaugY = pm;
      LUaugY = augY;
      BOOST_TRY {
        ublas::lu_factorize(LUaugY, PaugY);
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
//      cout << "EVENT" << endl;
//      cout << "augY: " << augY << endl;
//      cout << "LUaugY: " << LUaugY << endl;
//      cout << "PaugY: " << PaugY << endl;
      // Xgen is not changed but the following line is necessary to update the size of IN_1
      IN_1 = _calculateNortonCurrents(Xgen);              // determine internal Norton currents
//      cout << "IN_1: " << IN_1 << endl;
      Vbus = _calculateGridVoltages(LUaugY, PaugY, IN_1); // get grid voltage profile
//      cout << "Vbus: " << Vbus << endl;
      Ibus = ublas::prod(augY,Vbus);
//      cout << "Ibus: " << Ibus << endl;
      Pel = _calculateGeneratorPowers(Vbus, Xgen);        // calculate generator electric powers
//      cout << "Pel: " << Pel << endl;

      // Save again the values after the event at t for the t+ (just before, we saved them for t-)
      time_store.push_back(t);
      for (size_t k(0); k!=genCount; ++k){
        angles_store[k].push_back(Xgen[k][0]);
        speeds_store[k].push_back(Xgen[k][1]/(2*M_PI*baseF));
        eq_tr_store[k].push_back(Xgen[k][2]);
        ed_tr_store[k].push_back(Xgen[k][3]);
        Pel_store[k].push_back(Pel[k]);
      }
      for (size_t k(0); k!=busCount; ++k){
        Vbus_store[k].push_back(Vbus(k));
        Ibus_store[k].push_back(Ibus(k));
      }

      currentTimeStep = _timeStep; // The event occured, returning to oldstep
    }


    // ***************************************************
    // ***** UPDATE PROGRESS BAR AND TIMESTEP ************
    // ***************************************************
    if (_TDlog!=NULL) _TDlog->notifyProgress(100*t/stopTime);
    auxiliary::stayAlive();
    t += currentTimeStep;
    ++iterationCount;
  } // end of MAIN LOOP

  // Store all the computed data to TDresults to the user request
  _storeTDResults(time_store, angles_store, speeds_store, eq_tr_store, eq_tr_store,
                  Pel_store, Vbus_store, Ibus_store, res);

  if (_TDlog != NULL) _TDlog->notifyProgress(0);

  return 0;
}

Powersystem const* Simulator_sw::do_getPws() const{return _pws;}

void Simulator_sw::_augmentYForLoads(ublas::matrix<complex,ublas::column_major>& augY,
                                     ublas::vector<complex> const& Vbus) const{
  size_t loadCount = _pwsLocal.getLoadCount();
  for (size_t m=0; m!=loadCount; ++m){
    // Get load and bus
    Load const* load = _pwsLocal.getLoad(m);
    int busExtId = load->busExtId;
    int busIntId = _pwsLocal.getBus_intId(busExtId);
    Bus const* bus = _pwsLocal.getBus(busIntId);

    // Steady-state
    complex V0 = std::polar(bus->V, bus->theta);
    double Vm0 = std::abs(V0);
    complex V = Vbus(busIntId);
    double Vm = std::abs(V);
    complex S0 = complex(load->Pdemand,load->Qdemand);
    double P0 = S0.real();
    double Q0 = S0.imag();
    double f0 = _pwsLocal.baseF;

    // Current state
    double P = P0 * std::pow(Vm/Vm0, load->Vexpa); // *(1+load->kpf*(f-f0))
    double Q = Q0 * std::pow(Vm/Vm0, load->Vexpb); // *(1+load->kpf*(f-f0))
    complex S(P,Q);
    complex I = std::conj(S/V);
    double Im = std::abs(I);

    // Compute resulting Y
    complex Y = I/V;
    double Ym = std::abs(Y);

    complex Ytemp;
    switch ( load->type() ){
    case LOADTYPE_CONSTP:
      Ytemp = std::conj(S0)/std::abs(V0)/std::abs(V0) * (V0/V) * (V0/V);
      break;
    case LOADTYPE_CONSTI:
      Ytemp = std::conj(S0)/std::abs(V0)/std::abs(V0) * (V0/V);
      break;
    case LOADTYPE_CONSTZ:
      Ytemp = std::conj(S0)/std::abs(V0)/std::abs(V0);
      break;
    }
    augY.at_element(busIntId,busIntId) += Y;
  }
}

// Adding the shunt Norton impedance of the generator to the Y matrix of the grid
// See Padiyar fig. 12.4, p. 417
void Simulator_sw::_augmentYForGenerators(ublas::matrix<complex,ublas::column_major>& augY) const{
  size_t genCount = _pwsLocal.getGenCount();
  for (size_t g=0; g!=genCount; g++){
    Generator const* gen = _pwsLocal.getGenerator(g);
    int busExtId = gen->busExtId;
    int busIntId = _pwsLocal.getBus_intId(busExtId);
    complex ygen = 1.0/complex(gen->ra,gen->xd_1);
    augY.at_element(busIntId,busIntId) += ygen;
  }
}

ublas::vector<complex> Simulator_sw::_calculateGridVoltages(ublas::matrix<complex,ublas::column_major> const& LUaugY,
                                                            ublas::permutation_matrix<size_t> const& PaugY,
                                                            ublas::vector<complex> const& I)  const{
  ublas::vector<complex> V(I);            // initialize V with the current values
  ublas::lu_substitute(LUaugY, PaugY, V); // V contains the solution
  return V;
}

ublas::vector<complex> Simulator_sw::_calculateNortonCurrents(vector<vector<double> > const& Xgen) const{
  size_t genCount = _pwsLocal.getGenCount();
  size_t busCount = _pwsLocal.getBusCount();
  ublas::vector<complex> IN_1(busCount,complex(0.0,0.0));
  for ( size_t i = 0 ; i != genCount ; ++i ){
    Generator const* gen = _pwsLocal.getGenerator(i);
    int busExtId = gen->busExtId;
    int busIntId = _pwsLocal.getBus_intId(busExtId);
    switch (gen->model){
    case GENMODEL_0p0:{
      double EQDm_1 = Xgen[i][2];
      double delta = Xgen[i][0];
      IN_1(busIntId) += _calculateNortonCurrent_0p0(EQDm_1, delta, gen->ra, gen->xd_1);
      break;}
    case GENMODEL_1p1:{
      double delta = Xgen[i][0];
      double Eq_1 = Xgen[i][2];
      double Ed_1 = Xgen[i][3];
      IN_1(busIntId) += _calculateNortonCurrent_1p1(Eq_1, Ed_1, delta, gen->ra, gen->xq_1);
      break;}
    }
  }
  return IN_1;
}

vector<double> Simulator_sw::_calculateGeneratorPowers(ublas::vector<complex> const& Vbus,
                                                       vector<vector<double> > const& Xgen) const{
  size_t genCount = _pwsLocal.getGenCount();
  std::vector<double> Pel(genCount, 0.0);
  // Calculate machine powers
  for (size_t i(0); i!=genCount; ++i){
    Generator const* gen = _pwsLocal.getGenerator(i);
    int busExtId = gen->busExtId;
    int busIntId = _pwsLocal.getBus_intId(busExtId);
    switch (gen->model){
    case GENMODEL_0p0:{
      complex Vbus_ = Vbus(busIntId);
      double EQDm_1 = Xgen[i][2];
      double delta = Xgen[i][0];
      complex IQD = _calculateCurrent_0p0(Vbus_, EQDm_1, delta, gen->ra, gen->xd_1);
      Pel[i] = _calculateActivePower_0p0(Vbus_,IQD);
      break;}
    case GENMODEL_1p1:{
      complex Vbus_ = Vbus(busIntId);
      complex Iqd = _calculateCurrent_1p1(Vbus_, Xgen[i][0], Xgen[i][2], Xgen[i][3], gen->ra, gen->xd_1, gen->xq_1);
      Pel[i] = _calculateActivePower_1p1(Vbus_, Xgen[i][0], Iqd);;
      break;}
    }
  }
  return Pel;
}

////////////////////////////////////////////////////////////////////////////////
/// 0p0 model related functions
////////////////////////////////////////////////////////////////////////////////
void Simulator_sw::_init0p0(complex Sgen, complex Vbus_, double ra, double xd_1, double baseF,
                            double& delta, double& omega, double& EQDm_1, double& Pel_) const{
  complex Z_1(ra,xd_1);                           // transient impedance
  complex IQD = std::conj(Sgen)/std::conj(Vbus_); // armature current (DQ frame)
  complex EQD_1 = Vbus_ + Z_1*IQD;                // internal transient voltage (DQ frame)
  delta = std::arg(EQD_1);  // angle between the Q-axis (system frame of reference)
                            // and the q-axis (machine frame of reference)
  EQDm_1 = std::abs(EQD_1); // internal transient voltage magnitude
  omega = 2.*M_PI*baseF;    // Synchronous speed in [rad/sec]
  Pel_ = _calculateActivePower_0p0(Vbus_, IQD);
}

complex Simulator_sw::_calculateCurrent_0p0(complex Vbus_, double EQDm_1, double delta, double ra, double xd_1) const{
  complex EQD_1 = std::polar(EQDm_1,delta); // transient internal voltage (DQ frame)
  complex Z_1(ra,xd_1);                     // transient impedance
  complex IQD = (EQD_1-Vbus_)/Z_1;          // armature current (DQ frame)
  return IQD;
}

double Simulator_sw::_calculateActivePower_0p0(complex Vbus_, complex IQD) const{
  complex S = Vbus_ * std::conj(IQD);
  return S.real();
}

complex Simulator_sw::_calculateNortonCurrent_0p0(double EQDm_1, double delta, double ra, double xd_1) const{
  complex EQD_1 = std::polar(EQDm_1,delta); // transient internal voltage (DQ frame)
  complex Z_1(ra,xd_1);                     // transient impedance
  complex IN_1 = EQD_1/Z_1;                 // Norton equivalent transient current source
  return IN_1;
}

void Simulator_sw::_dynamics_0p0(complex Vbus_,
                                 double EQDm_1, double delta, double omega,
                                 double Pmech, double M, double D, double ra, double xd_1,
                                 double& ddelta, double& domega) const{
  double omegaNominal = 2.0*M_PI*_pwsLocal.baseF;
  complex IQD = _calculateCurrent_0p0(Vbus_, EQDm_1, delta, ra, xd_1);
  double Pel = _calculateActivePower_0p0(Vbus_, IQD);
  ddelta = omega - omegaNominal;
  domega = omegaNominal*(Pmech - Pel - D*(omega-omegaNominal))/M;
}

////////////////////////////////////////////////////////////////////////////////
/// 1p1 model related functions
////////////////////////////////////////////////////////////////////////////////
void Simulator_sw::_init1p1(complex Sgen, complex Vbus_, double ra, double xq, double xd, double xq_1, double xd_1, double baseF,
                            double& delta, double& omega, double& Eq_1, double& Ed_1, double& Efd, double& Pel_) const{
  // Steady-state and transient impedances
  complex Z(ra,xq);           // steady state impedance
  complex Z_1(ra,xq_1);       // transient impedance

  // Determine the armature current
  complex IQD = std::conj(Sgen)/std::conj(Vbus_); // armature current (DQ frame)

  // Set the q-axis
  complex EQD = Vbus_ + Z*IQD; // steady-state internal voltage (DQ frame)
                              // only has q-axis component! as per Padiyar p. 189
  delta = std::arg(EQD);     // angle between the Q-axis (system frame of reference)
                              // and the q-axis (machine frame of reference)

  // Determine the field voltage
  complex Iqd = IQD * exp(complex(0,-delta)); // armature current (dq frame)
//  double Iq = Iqd.real();     // q-axis armature current
  double Id = Iqd.imag();     // q-axis armature current
  Efd = std::abs(EQD) - (xd-xq) * Id;

  // Equivalent circuit of Padiyar figure 12.4, p. 417
  complex EQD_1 = Vbus_ + Z_1 * IQD;                // internal transient voltage (DQ frame)
  complex Eqd_1 = EQD_1 * exp(complex(0,-delta)); // internal transient voltage (dq frame)

//  // Alternatively, the following yields exactly the same result
//  complex Vdq = Vbus_ * exp(complex(0,-delta));    // bus voltage (dq frame)
//  complex Eqd_1 = Vdq + Z_1 * Iqd;

  Eq_1 = Eqd_1.real(); // q-axis internal transient voltage
  Ed_1 = Eqd_1.imag(); // d-axis internal transient voltage

  // Synchronous speed in [rad/sec]
  omega = 2.*M_PI*baseF;

  // Initialize power
  Pel_= _calculateActivePower_1p1(Vbus_, delta, Iqd);
}

complex Simulator_sw::_calculateCurrent_1p1(complex Vbus_, double delta, double Eq_1, double Ed_1, double ra, double xd_1, double xq_1) const{
  // Tranform Vbus_ to rotor frame of reference (dq), (counterclockwise) rotating the frame of reference by delta
  complex Vqd = Vbus_ * exp(complex(0,-delta)); // bus voltage (dq frame)
  double Vq = Vqd.real();     // q-axis bus voltage
  double Vd = Vqd.imag();     // d-axis bus voltage
  // Eq_1 and Ed_1 are already in the dq frame: Eqd_1 = Eq_1 + j*Ed_1

  // Transient saliency NOT neglected: xd_1 != xq_1
  // Armature resistance NOT neglected: ra != 0
  /* The following system has to be solved
   *   Vq = Eq_1 - xd_1*Id - ra*Iq
   *   Vd = Ed_1 - xq_1*Iq - ra*Id  */
  // Solution of the system above given in Padiyar (12.26), p. 414
  double Iq = (ra*(Eq_1-Vq) + xd_1*(Ed_1-Vd))/(ra*ra+xd_1*xq_1);  // q-axis armature current
  double Id = (-xq_1*(Eq_1-Vq) + ra*(Ed_1-Vd))/(ra*ra+xd_1*xq_1); // d-axis armature current
  complex Iqd(Iq,Id);                                             // armature current (dq frame)
  return Iqd;

//  // Transient saliency neglected: xd_1 == xq_1
//  // Armature resistance NOT neglected: ra != 0
//  double x_1 = xd_1;
//  complex Z(ra,x_1);
//  complex Eqd_1(Eq_1,Ed_1);
//  complex Iqd = (Eqd_1-Vqd)/Z;
//  return Iqd;

//  // Transient saliency NOT neglected: xd_1 != xq_1
//  // Armature resistance neglected: ra == 0
//  double Iq = (Eq_1 - Vd)/xq_1;
//  double Id = -(Ed_1 - Vq)/xd_1;
//  complex Iqd(Iq,Id);
//  return Iqd;
}

double Simulator_sw::_calculateActivePower_1p1(complex Vbus_, double delta, complex Iqd) const{
  // Tranform Vbus_ to rotor frame of reference (dq), (counterclockwise) rotating the frame of reference by delta
  complex Vqd = Vbus_ * exp(complex(0,-delta)); // bus voltage (dq frame)
  complex S = Vqd * std::conj(Iqd);
  return S.real();
}

complex Simulator_sw::_calculateNortonCurrent_1p1(double Eq_1, double Ed_1, double delta, double ra, double xq_1) const{
  complex Eqd_1 = complex(Eq_1,Ed_1);                   // internal voltage (qd frame)
  complex EQD_1 = Eqd_1 * std::exp(complex(0.0,delta)); // internal transient voltage (QD frame)
                                                        // bring Eqd into the QD frame: multiply by exp(1i*delta)
  complex Z_1 = complex(ra,xq_1);                       // transient impedance
  complex IN_1 = EQD_1/Z_1;                             // Norton equivalent transient current source
  return IN_1;
}

void Simulator_sw::_dynamics_1p1(complex Vbus_,
                   double delta, double omega, double Eq_1, double Ed_1, double Efd,
                   double Pmech, double M, double D, double ra, double xd, double xq, double xd_1, double xq_1, double Td0_1, double Tq0_1,
                   double& ddelta, double& domega, double& dEq_1, double& dEd_1) const{
  double omegaNominal = 2.0*M_PI*_pwsLocal.baseF;
  complex Iqd = _calculateCurrent_1p1(Vbus_, delta, Eq_1, Ed_1, ra, xd_1, xq_1);
  double Iq = Iqd.real(); // q-axis armature current
  double Id = Iqd.imag(); // d-axis armature current
  double Pel = _calculateActivePower_1p1(Vbus_, delta, Iqd);
  ddelta = omega - omegaNominal;
  domega = omegaNominal*(Pmech - Pel - D*(omega-omegaNominal))/M;
  dEq_1 = (Efd - Eq_1 + (xd-xd_1)*Id)/Td0_1;
  dEd_1 = (    - Ed_1 - (xq-xq_1)*Iq)/Tq0_1;
}

vector<vector<double> > Simulator_sw::_calculateGeneratorDynamics(
                                  vector<vector<double> > const& Xgen,
                                  vector<double> const& Efd,
                                  ublas::vector<complex> const& Vbus ) const{
  size_t genCount = _pwsLocal.getGenCount();
  vector<vector<double> > dXgen(genCount, vector<double>(4,0.0));
  for (size_t i=0; i!=genCount; ++i){
    Generator const* gen = _pwsLocal.getGenerator(i);
    int busExtId = gen->busExtId;
    int busIntId = _pwsLocal.getBus_intId(busExtId);
    complex Vbus_ = Vbus(busIntId);
    switch (gen->model){
    case GENMODEL_0p0:{
      double delta = Xgen[i][0];
      double omega = Xgen[i][1];
      double EQDm_1 = Xgen[i][2];
      double ddelta; // difference in delta; output variable
      double domega; // difference in omega; output variable
      _dynamics_0p0(Vbus_, EQDm_1, delta, omega,
                    gen->Pgen, gen->M, gen->D, gen->ra, gen->xd_1,
                    ddelta, domega);
      dXgen[i][0] = ddelta;
      dXgen[i][1] = domega;
      dXgen[i][2] = 0.0; // Eq doesnt change for model 1
      break;}

    case GENMODEL_1p1:{
      double delta = Xgen[i][0];
      double omega = Xgen[i][1];
      double Eq_1 = Xgen[i][2];
      double Ed_1 = Xgen[i][3];
      double ddelta; // difference in delta; output variable
      double domega; // difference in omega; output variable
      double dEq_1;  // difference in Eq_1; output variable
      double dEd_1;  // difference in Ed_1; output variable
      _dynamics_1p1(Vbus_, delta, omega, Eq_1, Ed_1, Efd[i],
                    gen->Pgen, gen->M, gen->D, gen->ra, gen->xd, gen->xq, gen->xd_1, gen->xq_1, gen->Td0_1, gen->Tq0_1,
                    ddelta, domega, dEq_1, dEd_1);
      dXgen[i][0] = ddelta;
      dXgen[i][1] = domega;
      dXgen[i][2] = dEq_1;
      dXgen[i][3] = dEd_1;
      break;}
    }
  }
  return dXgen;
}

vector<vector<double> > Simulator_sw::_rungeKutta(ublas::matrix<complex,ublas::column_major> const& LUaugY,
                                                  ublas::permutation_matrix<size_t> const& PaugY,
                                                  vector<vector<double> > const& Xgen,
                                                  vector<double> const& Efd,
                                                  double stepSize) const{

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
  size_t genCount = _pwsLocal.getGenCount();
  size_t busCount = _pwsLocal.getBusCount();
  vector<vector<double> > Kgen1, Kgen2, Kgen3, Kgen4;
  vector<vector<double> > XgenNew(genCount, vector<double>(4,0.0));
  ublas::vector<complex> IN_1(busCount,0.0); // Norton equivalent transient current source at buses
  ublas::vector<complex> Vbus(busCount,0.0); // bus voltages

  /* For each RK step S:
   * - based on the current state (Xgen) the Norton currents of the gens (IN_1) are calculated
   * - then the voltage profile (Vbus) is extracted by the network (LUaugY & PaugY)
   * - according to this voltage profile the generator dynamics are calculated (Kgen1, ...)
   */
  // ----- First step -----
  IN_1 = _calculateNortonCurrents(Xgen);
  Vbus = _calculateGridVoltages(LUaugY, PaugY, IN_1);
  Kgen1 = _calculateGeneratorDynamics(Xgen, Efd, Vbus);
  for (size_t k=0; k!=genCount; ++k){
    Generator const* gen = _pwsLocal.getGenerator(k);
    switch (gen->model){
    case GENMODEL_0p0:
      XgenNew[k][0] = Xgen[k][0] + stepSize*RKcoef_a[1][0]*Kgen1[k][0];
      XgenNew[k][1] = Xgen[k][1] + stepSize*RKcoef_a[1][0]*Kgen1[k][1];
      XgenNew[k][2] = Xgen[k][2] + stepSize*RKcoef_a[1][0]*Kgen1[k][2];
      break;
    case GENMODEL_1p1:
      XgenNew[k][0] = Xgen[k][0] + stepSize*RKcoef_a[1][0]*Kgen1[k][0];
      XgenNew[k][1] = Xgen[k][1] + stepSize*RKcoef_a[1][0]*Kgen1[k][1];
      XgenNew[k][2] = Xgen[k][2] + stepSize*RKcoef_a[1][0]*Kgen1[k][2];
      XgenNew[k][3] = Xgen[k][3] + stepSize*RKcoef_a[1][0]*Kgen1[k][3];
      break;
    }
  }

  // ----- Second step -----
  IN_1 = _calculateNortonCurrents(XgenNew);
  Vbus = _calculateGridVoltages(LUaugY, PaugY, IN_1);
  Kgen2 = _calculateGeneratorDynamics(XgenNew, Efd, Vbus);
  for (size_t k=0; k!=genCount; ++k){
    Generator const* gen = _pwsLocal.getGenerator(k);
    switch (gen->model){
    case GENMODEL_0p0:
      XgenNew[k][0] = Xgen[k][0] + stepSize*(RKcoef_a[2][0]*Kgen1[k][0] + RKcoef_a[2][1]*Kgen2[k][0]);
      XgenNew[k][1] = Xgen[k][1] + stepSize*(RKcoef_a[2][0]*Kgen1[k][1] + RKcoef_a[2][1]*Kgen2[k][1]);
      XgenNew[k][2] = Xgen[k][2] + stepSize*(RKcoef_a[2][0]*Kgen1[k][2] + RKcoef_a[2][1]*Kgen2[k][2]);
      break;
    case GENMODEL_1p1:
      XgenNew[k][0] = Xgen[k][0] + stepSize*(RKcoef_a[2][0]*Kgen1[k][0] + RKcoef_a[2][1]*Kgen2[k][0]);
      XgenNew[k][1] = Xgen[k][1] + stepSize*(RKcoef_a[2][0]*Kgen1[k][1] + RKcoef_a[2][1]*Kgen2[k][1]);
      XgenNew[k][2] = Xgen[k][2] + stepSize*(RKcoef_a[2][0]*Kgen1[k][2] + RKcoef_a[2][1]*Kgen2[k][2]);
      XgenNew[k][3] = Xgen[k][3] + stepSize*(RKcoef_a[2][0]*Kgen1[k][3] + RKcoef_a[2][1]*Kgen2[k][3]);
      break;
    }
  }

  // ----- Third step -----
  IN_1 = _calculateNortonCurrents(XgenNew);
  Vbus = _calculateGridVoltages(LUaugY, PaugY, IN_1);
  Kgen3 = _calculateGeneratorDynamics(XgenNew, Efd, Vbus);
  for (size_t k=0; k!=genCount; ++k){
    Generator const* gen = _pwsLocal.getGenerator(k);
    switch (gen->model){
    case GENMODEL_0p0:
      XgenNew[k][0] = Xgen[k][0] + stepSize*(RKcoef_a[3][0]*Kgen1[k][0] + RKcoef_a[3][1]*Kgen2[k][0] + RKcoef_a[3][2]*Kgen3[k][0]);
      XgenNew[k][1] = Xgen[k][1] + stepSize*(RKcoef_a[3][0]*Kgen1[k][1] + RKcoef_a[3][1]*Kgen2[k][1] + RKcoef_a[3][2]*Kgen3[k][1]);
      XgenNew[k][2] = Xgen[k][2] + stepSize*(RKcoef_a[3][0]*Kgen1[k][2] + RKcoef_a[3][1]*Kgen2[k][2] + RKcoef_a[3][2]*Kgen3[k][2]);
      break;
    case GENMODEL_1p1:
      XgenNew[k][0] = Xgen[k][0] + stepSize*(RKcoef_a[3][0]*Kgen1[k][0] + RKcoef_a[3][1]*Kgen2[k][0] + RKcoef_a[3][2]*Kgen3[k][0]);
      XgenNew[k][1] = Xgen[k][1] + stepSize*(RKcoef_a[3][0]*Kgen1[k][1] + RKcoef_a[3][1]*Kgen2[k][1] + RKcoef_a[3][2]*Kgen3[k][1]);
      XgenNew[k][2] = Xgen[k][2] + stepSize*(RKcoef_a[3][0]*Kgen1[k][2] + RKcoef_a[3][1]*Kgen2[k][2] + RKcoef_a[3][2]*Kgen3[k][2]);
      XgenNew[k][3] = Xgen[k][3] + stepSize*(RKcoef_a[3][0]*Kgen1[k][3] + RKcoef_a[3][1]*Kgen2[k][3] + RKcoef_a[3][2]*Kgen3[k][3]);
      break;
    }
  }

  // ----- Fourth step -----
  IN_1 = _calculateNortonCurrents(XgenNew);
  Vbus = _calculateGridVoltages(LUaugY, PaugY, IN_1);
  Kgen4 = _calculateGeneratorDynamics(XgenNew, Efd, Vbus);
  for (size_t k=0; k!=genCount; ++k){
    Generator const* gen = _pwsLocal.getGenerator(k);
    switch (gen->model){
    case GENMODEL_0p0:
      XgenNew[k][0] = Xgen[k][0] + stepSize*(  RKcoef_b[0]*Kgen1[k][0] + RKcoef_b[1]*Kgen2[k][0] + RKcoef_b[2]*Kgen3[k][0] + RKcoef_b[3]*Kgen4[k][0] );
      XgenNew[k][1] = Xgen[k][1] + stepSize*(  RKcoef_b[0]*Kgen1[k][1] + RKcoef_b[1]*Kgen2[k][1] + RKcoef_b[2]*Kgen3[k][1] + RKcoef_b[3]*Kgen4[k][1] );
      XgenNew[k][2] = Xgen[k][2] + stepSize*(  RKcoef_b[0]*Kgen1[k][2] + RKcoef_b[1]*Kgen2[k][2] + RKcoef_b[2]*Kgen3[k][2] + RKcoef_b[3]*Kgen4[k][2] );
      break;
    case GENMODEL_1p1:
      XgenNew[k][0] = Xgen[k][0] + stepSize*(  RKcoef_b[0]*Kgen1[k][0] + RKcoef_b[1]*Kgen2[k][0] + RKcoef_b[2]*Kgen3[k][0] + RKcoef_b[3]*Kgen4[k][0] );
      XgenNew[k][1] = Xgen[k][1] + stepSize*(  RKcoef_b[0]*Kgen1[k][1] + RKcoef_b[1]*Kgen2[k][1] + RKcoef_b[2]*Kgen3[k][1] + RKcoef_b[3]*Kgen4[k][1] );
      XgenNew[k][2] = Xgen[k][2] + stepSize*(  RKcoef_b[0]*Kgen1[k][2] + RKcoef_b[1]*Kgen2[k][2] + RKcoef_b[2]*Kgen3[k][2] + RKcoef_b[3]*Kgen4[k][2] );
      XgenNew[k][3] = Xgen[k][3] + stepSize*(  RKcoef_b[0]*Kgen1[k][3] + RKcoef_b[1]*Kgen2[k][3] + RKcoef_b[2]*Kgen3[k][3] + RKcoef_b[3]*Kgen4[k][3] );
      break;
    }
  }

  return XgenNew;
}

void Simulator_sw::_storeTDResults(vector<double> const& time_store,
                                   vector<vector<double> > const& angles_store,
                                   vector<vector<double> > const& speeds_store,
                                   vector<vector<double> > const& eq_tr_store,
                                   vector<vector<double> > const& ed_tr_store,
                                   vector<vector<double> > const& Pel_store,
                                   vector<vector<complex> > const& Vbus_store,
                                   vector<vector<complex> > const& Ibus_store,
                                   TDResults& res) const{
  vector<double> temp;
  for (size_t i=0; i!=res.dataExists().size(); ++i){
    temp.clear();
    // Only store if user selected so
    if (!res.dataExists().at(i)) continue;

    // Take the identifier for the specific data point
    TDResultIdentifier tdri = res.identifiers().at(i);

    switch (tdri.elementType) {
    case TDRESULTS_BUS:{
      int busIntId = _pwsLocal.getBus_intId(tdri.extId);

      switch (tdri.variable){
      case TDRESULTS_VOLTAGE_REAL:
        for (size_t k=0; k!=Vbus_store[busIntId].size(); ++k)
          temp.push_back(Vbus_store[busIntId][k].real());
        break;
      case TDRESULTS_VOLTAGE_IMAG:
        for (size_t k=0; k!=Vbus_store[busIntId].size(); ++k)
          temp.push_back(Vbus_store[busIntId][k].imag());
        break;
      case TDRESULTS_CURRENT_REAL:
        for (size_t k(0); k!=Ibus_store[busIntId].size(); ++k)
          temp.push_back(Ibus_store[busIntId][k].real());
        break;
      case TDRESULTS_CURRENT_IMAG:
        for (size_t k(0); k!=Ibus_store[busIntId].size(); ++k)
          temp.push_back(Ibus_store[busIntId][k].imag());
        break;
      }
      break;}

    case TDRESULTS_GENERATOR:{
      int genIntId = _pwsLocal.getGen_intId(tdri.extId);

      switch (tdri.variable){
      case TDRESULTS_ANGLE:
        temp = angles_store[genIntId];
        break;
      case TDRESULTS_SPEED:
        for (size_t k(0); k!=speeds_store[genIntId].size(); ++k)
          temp.push_back(speeds_store[genIntId][k] - 1);
          // -1 for uniformity with the hardware engine
        break;
      case TDRESULTS_ACCELERATING_POWER:{
        Generator const* gen = _pwsLocal.getGenerator(genIntId);
        double Pmech = gen->Pgen;
        for (size_t k=0 ; k!=Pel_store[genIntId].size() ; ++k)
          temp.push_back(Pmech - Pel_store[genIntId][k]);
        break;}
      case TDRESULTS_VOLTAGE_Q_TR:
        temp = eq_tr_store[genIntId];
        break;
      case TDRESULTS_VOLTAGE_D_TR:
        temp = ed_tr_store[genIntId];
        break;
      }
      break;}

    case TDRESULTS_OTHER:
      if (tdri.variable==TDRESULTS_TIME)
        temp = time_store;
      break;
    }
    res.set_data(i,temp);
  }
}

void Simulator_sw::_parseBusFault(Event &event){
  int busExtId = event.element_extId();
  int busIntId = _pwsLocal.getBus_intId(busExtId);
  Bus* bus = _pwsLocal.getBus(busIntId);

  switch (event.event_type()){
  case EVENT_EVENTTYPE_BUSFAULT:
    if (event.bool_arg()){
      // Storing the old
      _busintid.push_back(busIntId);
      _oldgsh.push_back(bus->Gsh);
      _oldbsh.push_back(bus->Bsh);
      double gsh;
      double bsh;
      gsh=1/event.double_arg_1();
      bsh=-1/event.double_arg_2();
      bus->Gsh = gsh;
      bus->Bsh = bsh;
    }

    // Restore the saved data
    else{
      for (size_t i=0;i<_busintid.size();++i){
        if (busIntId==_busintid[i]){
          bus->Gsh = _oldgsh[i];
          bus->Bsh = _oldbsh[i];
          _busintid.erase(_busintid.begin()+i);
          _oldgsh.erase(_oldgsh.begin()+i);
          _oldbsh.erase(_oldbsh.begin()+i);
        }
      }
    }
    break;
  }
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
        unsigned int maxextid = _pwsLocal.getBus_extId(0);
        for (size_t m = 0; m < _pwsLocal.getBusCount(); m++){
          if (_pwsLocal.getBus_extId(m)>maxextid)
            maxextid=_pwsLocal.getBus_extId(m);
        }
//        unsigned int maxextid;
//        for ( maxextid = 0 ; maxextid != 0 ; ++maxextid )
//        if ( _pwsLocal.getBus_intId( maxextid ) == -1 )
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
        unsigned int maxextidfrombr=_pwsLocal.getBr_extId(0);//Set the default first ext id
        for (size_t m=0;m<_pwsLocal.getBranchCount();m++){
          if(_pwsLocal.getBr_extId(m)>maxextidfrombr)
            maxextidfrombr=_pwsLocal.getBr_extId(m);
        }
//        unsigned int maxextidfrombr;
//        for ( maxextidfrombr = 0 ; maxextidfrombr != 0 ; ++maxextidfrombr )
//        if ( _pwsLocal.getBr_intId( maxextidfrombr ) == -1 )
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
//        _pwsLocal.getBranch(event.element_extId(),branchofevent);
        Branch* newtoline = new Branch();
        unsigned int maxextidtobr=_pwsLocal.getBr_extId(0);//Set the default first ext id
        for (size_t m=0;m<_pwsLocal.getBranchCount();m++){
          if (_pwsLocal.getBr_extId(m)>maxextidtobr)
            maxextidtobr=_pwsLocal.getBr_extId(m);
        }
//        unsigned int maxextidtobr;
//        for ( maxextidtobr = 0 ; maxextidtobr != 0 ; ++maxextidtobr )
//        if ( _pwsLocal.getBr_intId( maxextidtobr ) == -1 )
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
    branchofevent->X = 1e-14;
  }

  return 0;
}
