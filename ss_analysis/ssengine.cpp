
#include "ssengine.h"
using namespace elabtsaot;

using namespace ublas;
using std::polar;

SSEngine::SSEngine( std::string description, Logger* log ) :
  _SSdescription(description), _SSlog(log){}

std::string SSEngine::SSDescription() const{ return _SSdescription; }
void SSEngine::setLogger(Logger* log){ _SSlog = log; }

int SSEngine::solvePowerFlow(Powersystem& pws) const{
  matrix<complex> Y;
  vector<complex> V;
  int ans = do_solvePowerFlow(pws,Y,V);
  if (ans) return ans ;
  ssengine::updatePowersystem(pws,Y,V);
  return 0;
}


////////////////////////////////////////////////////////////////////////////////
// ssengine ////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#include <boost/numeric/ublas/operation.hpp> // for axpy_prod
#include <boost/numeric/ublas/banded.hpp> // for diagonal_matrix

void ssengine::buildY(Powersystem const& pws, matrix<complex>& Y){
  size_t N = pws.getBusCount();
  Y.resize(N,N);
  Y.clear();
  for (size_t k=0; k!=pws.getBranchCount() ; ++k ){
    Branch const* branch = pws.getBranch(k);
    if (!branch->status) continue; // Check whether the branch is on-line

    // Retrieve data of the branch
    size_t f = pws.getBus_intId(branch->fromBusExtId);  // from bus internal index
    size_t t = pws.getBus_intId(branch->toBusExtId);  // to bus internal index

    complex ys = 1.0/(complex(branch->R,branch->X));  // series admittance
    complex yp_from = complex(branch->Gfrom, branch->Bfrom); // from end shunt admittance
    complex yp_to = complex(branch->Gto, branch->Bto); // to end shunt admittance

    double tap = branch->Xratio;              // Xformer tap ratio magnitude
    double th = branch->Xshift;               // Xformer phase shift angle
    if(tap==0.){
      tap=1.;                      // assuming that (tap==0) => denotes line
      th=0.;
    }

    // Determine y-parameters
    // two-port y11-parameter
    complex y11 = (ys + yp_from) / complex(tap*tap , 0.0);
    // two-port y12-parameter
    complex y12 = -ys / (complex(tap , 0.0) * exp(complex(0.0 , -th)));
    // two-port y21-parameter
    complex y21 = -ys / (complex(tap , 0.0) * exp(complex(0.0 , th)));
    // two-port y22-parameter
    complex y22 = ys + yp_to;

    // Update admittance matrix Y
    Y(f,f) += y11;
    Y(f,t) += y12;
    Y(t,f) += y21;
    Y(t,t) += y22;
  }

  // Include bus shunt admittances
  for (size_t k=0; k!=N; ++k)
    Y(k,k) += complex(pws.getBus(k)->Gsh, pws.getBus(k)->Bsh);
}

void ssengine::calculatePower(matrix<complex> const& Y,
                              vector<complex> const& V,
                              vector<complex>& S){
  vector<complex> I;
  calculateCurrent(Y,V,I);
  S.resize(V.size());
  S.clear();
  noalias(S) = element_prod(V,conj(I));
}

void ssengine::calculateCurrent(matrix<complex> const& Y,
                                vector<complex> const& V,
                                vector<complex>& I ){
  I.resize(V.size());
  I.clear();
  axpy_prod(Y,V,I,true);
}

vector<complex> ssengine::absComplex(vector<complex> const& v){
  size_t N=v.size();
  vector<complex> ans(N);
  for (size_t k=0; k!=N; ++k)
    ans(k) = complex(std::abs(v(k)),0.);
  return ans;
}

void ssengine::calculateDSdV(matrix<complex> const& Y,
                             vector<complex> const& V,
                             matrix<complex>& dSdVm,
                             matrix<complex>& dSdVa){
  // Sparse provision

  size_t N=V.size();
  // ----- dPower against dVoltage_magnitude calculations -----
  dSdVm.resize(N,N);
  dSdVm.clear();
  // step1: dSdVm = Y*diagVnorm
  vector<complex> Vnorm(absComplex(V));
  diagonal_matrix<complex> diagVnorm(Vnorm.size(),Vnorm.data());
  axpy_prod(Y,diagVnorm,dSdVm,true);
  // step2: dSdVm = conj( Y*diagVnorm )
  dSdVm = conj(dSdVm);
  // step3: dSdVm = diagV * conj(Y*diagVnorm)
  diagonal_matrix<complex> diagV(V.size(),V.data());
  dSdVm = prod(diagV,dSdVm);
  // step4: dSdVm = diagV*conj(Y*diagVnorm) + diagconjI * diagVnorm;
  vector<complex> I;
  calculateCurrent(Y,V,I);
  diagonal_matrix<complex> diagI(I.size(),I.data()); // needed in dsdVa calculations
  I = conj(I); // now I temporarily holds conjI
  diagonal_matrix<complex> diagconjI(I.size(),I.data());
  axpy_prod(diagconjI,diagVnorm,dSdVm,false);

  // ----- dPower against dVoltage_angle calculations -----
  dSdVa.resize(N,N);
  dSdVa.clear();
  // step1: dsdVa = Y*diagV
  axpy_prod(Y,diagV,dSdVa,true);
  // step2: dsdVa = diagI - Y*diagV
  dSdVa = diagI - dSdVa;
  // step3: dsdVa = conj( diagI-Y*diagV )
  dSdVa = conj(dSdVa);
  // step4: dSdVa = diagV * conj(diagI-Y*diagV)
  dSdVa = prod(diagV,dSdVa);
  // step5: dSdVa = 1i * diagV*conj(diagI-Y*diagV)
  dSdVa *= complex(0,1);
}

void ssengine::updatePowersystem(Powersystem& pws,
                                 matrix<complex> const& Y,
                                 vector<complex> const& V){
  pws.set_status( PWSSTATUS_VALID );

  // Calculate power injections
  size_t N = pws.getBusCount();
  vector<complex> S;
  ssengine::calculatePower(Y,V,S);

  // Store theta-U, P-Q results in the _busSet of the pws
  for ( size_t k = 0 ; k != N ; ++k ){
    Bus* bus = pws.getBus(k);
    bus->P     = real(S(k));
    bus->Q     = imag(S(k));
    bus->V     =  abs(V(k));
    bus->theta =  arg(V(k));
  }

  // Calculate machine steady state variables
  for (size_t k = 0 ; k != pws.getGenCount() ; ++k){
    Generator* gen = pws.getGenerator(k);
    int busIntId = pws.getBus_intId(gen->busExtId);
    Bus* bus = pws.getBus(busIntId);
    gen->Pgen = bus->P;
    gen->Qgen = bus->Q;
    gen->Vss  = polar(bus->V,bus->theta);
    gen->updateVariables();
  }

  // Calculate load steady state variables
  for ( size_t k = 0 ; k != pws.getLoadCount() ; ++k ){
    Load* load = pws.getLoad(k);
    int busIntId = pws.getBus_intId( load->busExtId );
    Bus* bus = pws.getBus(busIntId);
    load->Pdemand = -bus->P;
    load->Qdemand = -bus->Q;
    load->Vss = polar(bus->V,bus->theta);
  }

  pws.set_status( PWSSTATUS_PF );
}
