
#include "ssutils.h"
using namespace elabtsaot;

#include <boost/numeric/ublas/operation.hpp> // for axpy_prod
#include <boost/numeric/ublas/banded.hpp> // for diagonal_matrix
using namespace ublas;

using std::polar;

void ssutils::buildY(Powersystem const& pws, matrix<complex,column_major>& Y){
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
    if (tap==0.){
      tap = 1.;                      // assuming that (tap==0) => denotes line
      th  = 0.;
    }

    // Determine y-parameters
    // two-port y11-parameter
    complex y11 = (ys + yp_from) / complex(tap*tap , 0.0);
    // two-port y12-parameter
    complex y12 = -ys / (complex(tap,0.0) * exp(complex(0.0,-th)));
    // two-port y21-parameter
    complex y21 = -ys / (complex(tap,0.0) * exp(complex(0.0,th)));
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

// TODO: Sub-optimal implementation for clarity sake
void ssutils::buildB(Powersystem const& pws, matrix<double,column_major>& B){
  size_t N = pws.getBusCount();
  // Branch susceptance vector
  size_t Nbr = pws.getBranchCount();
  vector<double> b(Nbr);
  buildBranchB(pws,b);
  // Branch susceptance diagonal matrix
  diagonal_matrix<double,column_major> diagb(b.size(),b.data());
  // Directed connection matrix
  matrix<int,column_major> Cd(Nbr,N);
  buildCd(pws,Cd);
  // Intermediate: Bf is such that Bf * Va is the vector of real branch powers
  // injected at each branch's "from" bus
  matrix<double,column_major> Bf(Nbr,N);
  axpy_prod(diagb,Cd,Bf,true);
  // Simplified admittance matrix
  B.resize(N,N);
  B.clear();
  axpy_prod(trans(Cd),Bf,B,true);
}

void ssutils::buildBranchB(Powersystem const& pws, vector<double>& b){
  size_t Nbr = pws.getBranchCount();
  b.resize(Nbr);
  for (size_t k(0); k!=Nbr; ++k){
    Branch const* branch = pws.getBranch(k);
    double tap = branch->Xratio;
    if(tap==0.) tap=1.; // assuming that (tap==0) => denotes line
    b(k) = 1/branch->X/tap;
  }
}

void ssutils::buildCd(Powersystem const& pws, matrix<int,column_major>& Cd){
  size_t N = pws.getBusCount();
  size_t Nbr = pws.getBranchCount();
  Cd.resize(Nbr,N);
  Cd.clear();
  for (size_t k(0); k!=Nbr; ++k){
    Branch const* branch = pws.getBranch(k);
    size_t f = pws.getBus_intId(branch->fromBusExtId);  // from bus internal index
    size_t t = pws.getBus_intId(branch->toBusExtId);  // to bus internal index
    Cd(k,f) = 1;
    Cd(k,t) = -1;
  }
}

void ssutils::calculatePower(matrix<complex,column_major> const& Y,
                              vector<complex> const& V,
                              vector<complex>& S){
  vector<complex> I;
  calculateCurrent(Y,V,I);
  S.resize(V.size());
  S.clear();
  noalias(S) = element_prod(V,conj(I));
}

void ssutils::calculateCurrent(matrix<complex,column_major> const& Y,
                                vector<complex> const& V,
                                vector<complex>& I ){
  I.resize(V.size());
  I.clear();
  axpy_prod(Y,V,I,true);
}

vector<double> ssutils::abs(vector<complex> const& v){
  size_t N=v.size();
  vector<double> ans(N);
  for (size_t k=0; k!=N; ++k)
    ans(k) = std::abs(v(k));
  return ans;
}

vector<complex> ssutils::absComplex(vector<complex> const& v){
  size_t N=v.size();
  vector<complex> ans(N);
  for (size_t k=0; k!=N; ++k)
    ans(k) = complex(std::abs(v(k)),0.);
  return ans;
}

vector<complex> ssutils::normComplexVec(vector<complex> const& v){
  size_t N=v.size();
  vector<complex> ans(N);
  for (size_t k=0; k!=N; ++k)
    ans(k) = v(k)/std::abs(v(k));
  return ans;
}

void ssutils::calculateDSdV(matrix<complex,column_major> const& Y,
                             vector<complex> const& V,
                             matrix<complex,column_major>& dSdVm,
                             matrix<complex,column_major>& dSdVa){
  // Sparse provision

  size_t N=V.size();
  matrix<complex,column_major> temp(N,N);
  temp.clear();
  // ----- dPower against dVoltage_magnitude calculations -----
  dSdVm.resize(N,N);
  dSdVm.clear();
  // step1: dSdVm = Y*diagVnorm
  vector<complex> Vnorm(normComplexVec(V));
  diagonal_matrix<complex,column_major> diagVnorm(Vnorm.size(),Vnorm.data());
  axpy_prod(Y,diagVnorm,temp,true);
  // step2: dSdVm = conj( Y*diagVnorm )
  temp = conj(temp);
  // step3: dSdVm = diagV * conj(Y*diagVnorm)
  diagonal_matrix<complex,column_major> diagV(V.size(),V.data());
  axpy_prod(diagV,temp,dSdVm,true); // dSdVm = prod(diagV,dSdVm);
  // step4: dSdVm = diagV*conj(Y*diagVnorm) + diagconjI * diagVnorm;
  vector<complex> I;
  calculateCurrent(Y,V,I);
  diagonal_matrix<complex,column_major> diagI(I.size(),I.data()); // needed in dsdVa calculations
  I = conj(I); // now I temporarily holds conjI
  diagonal_matrix<complex,column_major> diagconjI(I.size(),I.data());
  axpy_prod(diagconjI,diagVnorm,dSdVm,false);

  temp.clear();
  // ----- dPower against dVoltage_angle calculations -----
  dSdVa.resize(N,N);
  dSdVa.clear();
  // step1: dsdVa = Y*diagV
  axpy_prod(Y,diagV,temp,true);
  // step2: dsdVa = diagI - Y*diagV
  temp = diagI - temp;
  // step3: dsdVa = conj( diagI-Y*diagV )
  temp = conj(temp);
  // step4: dSdVa = diagV * conj(diagI-Y*diagV)
  axpy_prod(diagV,temp,dSdVa,true); //dSdVa = prod(diagV,dSdVa);
  // step5: dSdVa = 1i * diagV*conj(diagI-Y*diagV)
  dSdVa *= complex(0,1);
}

void ssutils::updatePowersystem(Powersystem& pws,
                                 matrix<complex,column_major> const& Y,
                                 vector<complex> const& V){
  pws.set_status( PWSSTATUS_VALID );

  // Calculate power injections
  size_t N = pws.getBusCount();
  vector<complex> S;
  ssutils::calculatePower(Y,V,S);

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
