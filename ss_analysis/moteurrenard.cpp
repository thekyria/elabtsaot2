
#include "moteurrenard.h"
using namespace elabtsaot;

#include "logger.h"

#include <boost/numeric/ublas/lu.hpp> // for matrix operations
#include <boost/numeric/ublas/io.hpp> // for ublas::matrix '<<'
using namespace ublas;
//#include <boost/timer/timer.hpp>

#include <complex>
using std::polar;
#include <cmath>                      // for M_PI constant
#define _USE_MATH_DEFINES
#include <iostream>
using std::cout;
using std::endl;

enum MoteurRenardProperties{
  SSEMRN_PROPERTY_FLATSTART,
  SSEMRN_PROPERTY_TOL,
  SSEMRN_PROPERTY_MAXIT
};

MoteurRenard::MoteurRenard(Logger* log):
    SSEngine("MoteurRenard s.s. engine", log){

  // ----- Prepare properties -----
  property_type tempPt;
  tempPt.key = SSEMRN_PROPERTY_FLATSTART;
  tempPt.dataType = PROPERTYT_DTYPE_BOOL;
  tempPt.name = "Flat start";
  tempPt.description = "Start power flow computation from a flat point, ie. |V|=1 ang(V)=0 for all buses";
  tempPt.defaultValue = true;
  _properties[tempPt] = tempPt.defaultValue;

  tempPt.key = SSEMRN_PROPERTY_TOL;
  tempPt.dataType = PROPERTYT_DTYPE_DOUBLE;
  tempPt.name = "Convergeance tolerance";
  tempPt.description = "Termination tolerance on per unit P & Q mismatch";
  tempPt.defaultValue = 1e-8;
  tempPt.minValue = 1e-14;
  tempPt.maxValue = 1e-3;
  _properties[tempPt] = tempPt.defaultValue;

  tempPt.key = SSEMRN_PROPERTY_MAXIT;
  tempPt.dataType = PROPERTYT_DTYPE_INT;
  tempPt.name = "Maximum iteration count";
  tempPt.description = "Maximum number of iteration of the algorithm";
  tempPt.defaultValue = 20;
  tempPt.minValue = 5;
  tempPt.maxValue = 100;
  _properties[tempPt] = tempPt.defaultValue;
}

int MoteurRenard::do_solvePowerFlow(Powersystem const& pws,
                                    matrix<complex>& Y,
                                    vector<complex>& V) const{

//  boost::timer::auto_cpu_timer t; // when t goes out of scope it prints timing info

  // Before entering into the power flow main routine, check that the power system
  // has been validated
  if (pws.status()!=PWSSTATUS_VALID && pws.status()!=PWSSTATUS_PF)
    return -1;

  // Retrieve options
  /* flatStart: start iteration from a flat voltage profile (Vmag=1, Vang=0)
   tolerance: convergence criteria tolerance
   maxIterCount: maximum number of allowable iterations of main loop converge */
  bool flatStart;
  double tolerance;
  size_t maxIterCount;
  _getOptions( flatStart, tolerance, maxIterCount );

  // initialize
  size_t N = pws.getBusCount();
  bool converged(false);
  size_t iterCount(0);
//  cout << "init" << endl;

  // Build admittance matrix Y
  ssengine::buildY(pws, Y);
//  cout << "Y: " << Y << endl;

  // initialize voltage and power vectors
  V.resize(N);
  vector<complex> Sset(N);
  vector<double> Va(N), Vm(N);
  size_t Npv(0), Npq(0);
  std::vector<size_t> pv, pq; // old names: pv, pq
  for (size_t k=0;k!=N;++k){
    Bus const* bus = pws.getBus(k);
    V(k) = polar(bus->V,bus->theta);
    Va(k) = bus->theta;
    Vm(k) = bus->V;
    Sset(k) = complex(bus->P,bus->Q); // power setpoint
    if (bus->type==BUSTYPE_PV){
      Npv++;
      pv.push_back(k);
    }
    if (bus->type==BUSTYPE_PQ){
      Npq++;
      pq.push_back(k);
    }
  }
//  cout << "Sset: " << Sset << endl;

  // Evaluate F(x0)
  vector<complex> Smis(N);
  ssengine::calculatePower(Y,V,Smis); // temporarily Smis holds actual power at current V
//  cout << "Scur: " << Smis << endl;
  Smis -= Sset; // now it holds the power mismatch
//  cout << "Smis: " << Smis << endl;
  vector<double> F(Npv+Npq+Npq);
  for (size_t k=0; k!=Npv; ++k)
    F(k)         = real(Smis(pv[k]));
  for (size_t k=0; k!=Npq; ++k){
    F(Npv+k)     = real(Smis(pq[k]));
    F(Npv+Npq+k) = imag(Smis(pq[k]));
  }
//  cout << "F: " << F << endl;
//  cout << "norm_inf(F): " << norm_inf(F) << endl;
  // Check tolerance
  if (norm_inf(F)<tolerance)
    converged = true;

  // do Newton iterations
  while (!converged && iterCount<maxIterCount){
    ++iterCount;
//    cout << "itercount: " << iterCount << endl;

    // Evaluate Jacobian
    matrix<complex> dSdVm(N,N), dSdVa(N,N);
    ssengine::calculateDSdV(Y,V,dSdVm,dSdVa);
//    cout << "dSdVm: " << dSdVm << endl;
//    cout << "dSdVa: " << dSdVa << endl;
    matrix<double> J11(Npv+Npq,Npv+Npq), J12(Npv+Npq,Npq), J21(Npq,Npv+Npq), J22(Npq,Npq);
    for (size_t k=0; k!=Npv; ++k){
      for (size_t m=0; m!=Npv; ++m){
        J11(    k,    m) = real(dSdVa(pv[k],pv[m])); // Npv, Npv
      }
    }
    for (size_t k=0; k!=Npq; ++k){
      for (size_t m=0; m!=Npv; ++m){
        J11(Npv+k,    m) = real(dSdVa(pq[k],pv[m])); // Npq, Npv
        J21(    k,    m) = imag(dSdVa(pq[k],pv[m])); // Npq, Npv
      }
    }
    for (size_t k=0; k!=Npv; ++k){
      for (size_t m=0; m!=Npq; ++m){
        J11(    k,Npv+m) = real(dSdVa(pv[k],pq[m])); // Npv, Npq
        J12(    k,    m) = real(dSdVm(pv[k],pq[m])); // Npv, Npq
      }
    }
    for (size_t k=0; k!=Npq; ++k){
      for (size_t m=0; m!=Npq; ++m){
        J11(Npv+k,Npv+m) = real(dSdVa(pq[k],pq[m])); // Npq, Npq
        J12(Npv+k,    m) = real(dSdVm(pq[k],pq[m])); // Npq, Npq
        J21(    k,Npv+m) = imag(dSdVa(pq[k],pq[m])); // Npq, Npq
        J22(    k,    m) = imag(dSdVm(pq[k],pq[m])); // Npq, Npq
      }
    } // TODO: compact this into one loop with range checks!
//    cout << "J11: " << J11 << endl;
//    cout << "J12: " << J12 << endl;
//    cout << "J21: " << J21 << endl;
//    cout << "J22: " << J22 << endl;
    matrix<double> J(Npv+2*Npq,Npv+2*Npq);
    range r1(0,Npv+Npq), r2(Npv+Npq,Npv+2*Npq);
    project(J, r1, r1) = J11;
    project(J, r1, r2) = J12;
    project(J, r2, r1) = J21;
    project(J, r2, r2) = J22;
//    cout << "J: " << J << endl;

    // Compute update step
    permutation_matrix<size_t>* pm = new permutation_matrix<size_t>(J.size1());
    vector<double> dx = F;
    BOOST_TRY{
      int ans = lu_factorize(J,*pm);
      if (ans) return ans;
      lu_substitute(J,*pm,dx);
    }BOOST_CATCH(ublas::singular const& ex){
      cout << "Singularity likely!" << endl;
      cout << "Exception message: " << ex.what() << endl;
    }BOOST_CATCH(std::exception const& ex){
      cout << "Other exception caught!" << endl;
      cout << "Exception message: " << ex.what() << endl;
    }BOOST_CATCH(...){
      cout << "Operation failed!" << endl;
    }BOOST_CATCH_END
    dx = -dx;
//    cout << "dx: " << dx << endl;

    // Update voltage
    for (size_t k=0; k!=Npv; ++k)
      Va(pv[k]) += dx(k);
    for (size_t k=0; k!=Npq; ++k){
      Va(pq[k]) += dx(Npv+k);
      Vm(pq[k]) += dx(Npv+Npq+k);
    }
    for (size_t k=0;k!=N;++k){
      V(k) = polar(Vm(k),Va(k));
      Vm(k) = std::abs(V(k)); // Update Vm & Va in case
      Va(k) = std::arg(V(k)); // we wrapped around the angle
    }
//    cout << "Vm: " << Vm << endl;
//    cout << "Va: " << Va << endl;
    delete pm;

    // Evaluate F
    ssengine::calculatePower(Y,V,Smis); // temporarily Smis holds actual power at current V
    Smis -= Sset; // now it holds the power mismatch
//    cout << "Smis: " << Smis << endl;
    for (size_t k=0; k!=Npv; ++k)
      F(k)         = real(Smis(pv[k]));
    for (size_t k=0; k!=Npq; ++k){
      F(Npv+k)     = real(Smis(pq[k]));
      F(Npv+Npq+k) = imag(Smis(pq[k]));
    }
//    cout << "F: " << F << endl;
//    cout << "norm_inf(F): " << norm_inf(F) << endl;
    // Check tolerance
    if (norm_inf(F)<tolerance)
      converged = true;
  }
  if (!converged) return 2;
  return 0;
}

void MoteurRenard::_getOptions( bool& flatStart, double& tolerance, size_t& maxIterCount ) const{
  // Retrieve boost::any properties
  boost::any anyFlatStart = _getPropertyValueFromKey(SSEMRN_PROPERTY_FLATSTART);
  boost::any anyTolerance = _getPropertyValueFromKey(SSEMRN_PROPERTY_TOL);
  boost::any anyMaxIterCount = _getPropertyValueFromKey(SSEMRN_PROPERTY_MAXIT);
  // Store them in output arguments
  flatStart = boost::any_cast<bool>( anyFlatStart );
  tolerance = boost::any_cast<double>( anyTolerance );
  maxIterCount = boost::any_cast<int>( anyMaxIterCount );
}
