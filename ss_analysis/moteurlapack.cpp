
#include "moteurlapack.h"
using namespace elabtsaot;

#include "ssutils.h"
#include "logger.h"
#include "auxiliary.h"
using auxiliary::operator<<;

#include <boost/numeric/ublas/operation.hpp> // for axpy_prod
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
using namespace ublas;
#include <boost/numeric/bindings/traits/ublas_matrix.hpp>
#include <boost/numeric/bindings/traits/ublas_vector2.hpp>
#include <boost/numeric/bindings/lapack/gesv.hpp>
namespace lapack = boost::numeric::bindings::lapack;
#include <boost/timer/timer.hpp>

#include <complex>
using std::polar;
#include <cmath>                      // for M_PI constant
#define _USE_MATH_DEFINES
#include <iostream>
using std::cout;
using std::endl;

enum MoteurLapackProperties{
  SSEMLP_PROPERTY_FLATSTART,
  SSEMLP_PROPERTY_TOL,
  SSEMLP_PROPERTY_MAXIT,
  SSEMLP_PROPERTY_METHOD
};

enum MoteurLapackMethod{
  MLP_METHOD_NR = 0,
  MLP_METHOD_DC = 1
};

MoteurLapack::MoteurLapack(Logger* log):
    SSEngine("MoteurLapack s.s. engine", log){

  // ----- Prepare properties -----
  property_type tempPt;
  tempPt.key = SSEMLP_PROPERTY_FLATSTART;
  tempPt.dataType = PROPERTYT_DTYPE_BOOL;
  tempPt.name = "Flat start";
  tempPt.description = "Start power flow computation from a flat point, ie. |V|=1 ang(V)=0 for all buses";
  tempPt.defaultValue = true;
  _properties[tempPt] = tempPt.defaultValue;

  tempPt.key = SSEMLP_PROPERTY_TOL;
  tempPt.dataType = PROPERTYT_DTYPE_DOUBLE;
  tempPt.name = "Convergeance tolerance";
  tempPt.description = "Termination tolerance on per unit P & Q mismatch";
  tempPt.defaultValue = 1e-8;
  tempPt.minValue = 1e-14;
  tempPt.maxValue = 1e-3;
  _properties[tempPt] = tempPt.defaultValue;

  tempPt.key = SSEMLP_PROPERTY_MAXIT;
  tempPt.dataType = PROPERTYT_DTYPE_INT;
  tempPt.name = "Maximum iteration count";
  tempPt.description = "Maximum number of iteration of the algorithm";
  tempPt.defaultValue = 20;
  tempPt.minValue = 5;
  tempPt.maxValue = 100;
  _properties[tempPt] = tempPt.defaultValue;

  tempPt.key = SSEMLP_PROPERTY_METHOD;
  tempPt.dataType = PROPERTYT_DTYPE_INT;
  tempPt.name = "Method";
  tempPt.description = "Power method to use";
  tempPt.defaultValue = 0;
  tempPt.minValue = 0;
  tempPt.maxValue = 1;
  _properties[tempPt] = tempPt.defaultValue;
}

int MoteurLapack::do_solvePowerFlow(Powersystem const& pws, vector<complex>& V) const{

  boost::timer::auto_cpu_timer t; // when t goes out of scope it prints timing info

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
  int maxIterCount;
  int method;
  _getOptions(flatStart, tolerance, maxIterCount, method);

  bool converged(false);
  // initialize voltage and power vectors
  size_t N = pws.getBusCount();
  V.resize(N); // V.clear();
  vector<complex> Sset(N);
  vector<double> Va(N), Vm(N);
  size_t Npv(0), Npq(0), Nsl(0);
  std::vector<size_t> pv, pq, sl;
  for (size_t k=0;k!=N;++k){
    Bus const* bus = pws.getBus(k);
    if (flatStart){
      V(k) = complex(1,0);
      Va(k) = 0;
      Vm(k) = 1;
    } else {
      V(k) = polar(bus->V,bus->theta);
      Va(k) = bus->theta;
      Vm(k) = bus->V;
    }
    Sset(k) = complex(bus->P,bus->Q); // power setpoint
    if (bus->type==BUSTYPE_PV){
      Npv++;
      pv.push_back(k);
    } else if (bus->type==BUSTYPE_PQ){
      Npq++;
      pq.push_back(k);
    } else if (bus->type==BUSTYPE_SLACK){
      Nsl++;
      sl.push_back(k);
    }
  }

  switch(method){
  /*****************************************************************************
   * NEWTON-RAPHSON METHOD
   ****************************************************************************/
  case MLP_METHOD_NR:{
    int iterCount(0);
    // Build admittance matrix Y
    matrix<complex,column_major> Y;
    ssutils::buildY(pws, Y);

    // Evaluate F(x0)
    vector<complex> Smis(N);
    ssutils::calculatePower(Y,V,Smis); // temporarily Smis holds actual power at current V
    Smis -= Sset; // now it holds the power mismatch
    vector<double> F(Npv+Npq+Npq);
    for (size_t k=0; k!=Npv; ++k)
      F(k)         = real(Smis(pv[k]));
    for (size_t k=0; k!=Npq; ++k){
      F(Npv+k)     = real(Smis(pq[k]));
      F(Npv+Npq+k) = imag(Smis(pq[k]));
    }
    // Check tolerance
    if (norm_inf(F)<tolerance) converged = true;

    // do Newton iterations
    while (!converged && iterCount<maxIterCount){
      ++iterCount;

      // Evaluate Jacobian
      matrix<complex,column_major> dSdVm(N,N), dSdVa(N,N);
      ssutils::calculateDSdV(Y,V,dSdVm,dSdVa);
      matrix<double,column_major> J11(Npv+Npq,Npv+Npq), J12(Npv+Npq,Npq), J21(Npq,Npv+Npq), J22(Npq,Npq);
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
      matrix<double,column_major> J(Npv+2*Npq,Npv+2*Npq);
      range r1(0,Npv+Npq), r2(Npv+Npq,Npv+2*Npq);
      project(J, r1, r1) = J11;
      project(J, r1, r2) = J12;
      project(J, r2, r1) = J21;
      project(J, r2, r2) = J22;

      // Compute update step
      vector<double> dx = -F;
      int ans = lapack::gesv(J,dx);
      if (ans) return 1000+ans;

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

      // Evaluate F
      ssutils::calculatePower(Y,V,Smis); // temporarily Smis holds actual power at current V
      Smis -= Sset; // now it holds the power mismatch
      for (size_t k=0; k!=Npv; ++k)
        F(k)         = real(Smis(pv[k]));
      for (size_t k=0; k!=Npq; ++k){
        F(Npv+k)     = real(Smis(pq[k]));
        F(Npv+Npq+k) = imag(Smis(pq[k]));
      }
      // Check tolerance
      if (norm_inf(F)<tolerance) converged = true;
    }
    cout << "iteration count: " << iterCount << endl;

    break;
  }


  /*****************************************************************************
   * DC POWERFLOW METHOD
   ****************************************************************************/
  // TODO! if more slacks, only the first considered
  case MLP_METHOD_DC:{
    // Active power injected to bus by gens and loads
    vector<double> Pset = real(Sset);
    // Active power lost to shunt conductance
    vector<double> Pshunt(N);
    for (size_t k(0); k!=N; ++k){
      Bus const* bus = pws.getBus(k);
      Pshunt(k) = bus->Gsh; // When V=(1.0<0), P = G
    }
    // Active power due to phase shifters
    vector<double> Pshift(N);
    size_t Nbr(pws.getBranchCount());
    vector<double> branchB(Nbr);
    ssutils::buildBranchB(pws,branchB);
    vector<double> Pshiftf(Nbr);
    for (size_t k(0); k!=Nbr; ++k){
      Branch const* branch = pws.getBranch(k);
      Pshiftf(k) = -branchB(k)*branch->Xshift;
    }
    matrix<int,column_major> Cd(Nbr,N);
    ssutils::buildCd(pws,Cd);
    axpy_prod(trans(Cd),Pshiftf,Pshift,true);
    // Adjust bus power injections for phase shifters and shunt conductances
    vector<double> P = Pset - Pshift - Pshunt;

    // Simplified admittance matrix
    matrix<double,column_major> B(N,N);
    ssutils::buildB(pws,B);

    // Reorder matrices and vectors (index them)
    size_t Npvpq(pv.size()+pq.size());
    vector<size_t> pvpq(Npvpq);
    std::copy(pv.begin(), pv.end(), pvpq.begin());
    std::copy(pq.begin(), pq.end(), pvpq.begin()+pv.size());
    matrix<double,column_major> Bp( ssutils::indexMatrix(B,pvpq,pvpq) );
    vector<double> Pp(ssutils::indexVector(P,pvpq));

    // Compute slack bus correction
    vector<double> Bsl( column(B,sl[0]) );
    Bsl = ssutils::indexVector(Bsl,pvpq);
    vector<double> Psl( Bsl*Va(sl[0]));
    Pp -= Psl;

    // Compute intermediate angle result
    vector<double> Vap(Pp);
    int ans = lapack::gesv(Bp,Vap);
    if (ans) return 1000+ans;

    // Update voltage angles
    for (size_t k=0; k!=Npvpq; ++k)
      Va(pvpq(k)) = Vap(k);

    // Update voltage
    for (size_t k=0;k!=N;++k){
      V(k) = polar(Vm(k),Va(k));
      Vm(k) = std::abs(V(k)); // Update Vm & Va in case
      Va(k) = std::arg(V(k)); // we wrapped around the angle
    }

    converged = true;
    break;
  }

  /* Invalid method type! */
  default: return 10;
  }



  if (!converged) return 2;
  return 0;
}

void MoteurLapack::_getOptions(bool& flatStart, double& tolerance,
                               int& maxIterCount, int& method) const{
  // Retrieve boost::any properties
  boost::any anyFlatStart = _getPropertyValueFromKey(SSEMLP_PROPERTY_FLATSTART);
  boost::any anyTolerance = _getPropertyValueFromKey(SSEMLP_PROPERTY_TOL);
  boost::any anyMaxIterCount = _getPropertyValueFromKey(SSEMLP_PROPERTY_MAXIT);
  boost::any anyMethod = _getPropertyValueFromKey(SSEMLP_PROPERTY_METHOD);
  // Store them in output arguments
  flatStart = boost::any_cast<bool>( anyFlatStart );
  tolerance = boost::any_cast<double>( anyTolerance );
  maxIterCount = boost::any_cast<int>( anyMaxIterCount );
  method = boost::any_cast<int>( anyMethod );
}
