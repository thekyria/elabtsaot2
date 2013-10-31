
#ifndef SSUTILS_H
#define SSUTILS_H

#include "powersystem.h"

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
namespace ublas = boost::numeric::ublas;

#include <complex>
typedef std::complex<double> complex;

namespace elabtsaot{

namespace ssutils{

/* 'Q=indexVector(V,p);'[cpp] should operate as 'Q=V(c)'[matlab] */
template<class T, class A>
ublas::vector<T,A> indexVector(ublas::vector<T,A> const& V,
                               ublas::vector<size_t> const& p){
  size_t length(p.size());
  ublas::vector<T,A> Q(length);
  for (size_t i(0); i!=length; ++i)
    Q(i)=V(p(i));
  return Q;
}

/* 'B=indexMatrix(M,rp,cp);'[cpp] should operate as 'B=M(rp,cp)'[matlab] */
template<class T, class L, class A>
ublas::matrix<T,L,A> indexMatrix(ublas::matrix<T,L,A> const& M,
                                 ublas::vector<size_t> const& rp,
                                 ublas::vector<size_t> const& cp){
  size_t rowCount(rp.size()), colCount(cp.size());
  ublas::matrix<T,L,A> B(rowCount,colCount); // B.clear();
  for (size_t r(0); r!=rowCount; ++r)
    for(size_t c(0); c!=colCount; ++c)
      B(r,c)=M(rp(r),cp(c));
  return B;
}

void buildY(Powersystem const& pws, ublas::matrix<complex,ublas::column_major>& Y);
void buildB(Powersystem const& pws, ublas::matrix<double,ublas::column_major>& B);
void buildBranchB(Powersystem const& pws, ublas::vector<double>& b);
void buildCd(Powersystem const& pws, ublas::matrix<int,ublas::column_major>& Cd);
void calculatePower(ublas::matrix<complex,ublas::column_major> const& Y,
                    ublas::vector<complex> const& V,
                    ublas::vector<complex>& S);
void calculateCurrent(ublas::matrix<complex,ublas::column_major> const& Y,
                      ublas::vector<complex> const& V,
                      ublas::vector<complex>& I);
ublas::vector<double> abs(ublas::vector<complex> const& v);
ublas::vector<complex> absComplex(ublas::vector<complex> const& v);
ublas::vector<complex> normComplexVec(ublas::vector<complex> const& v);
void calculateDSdV(ublas::matrix<complex,ublas::column_major> const& Y,
                   ublas::vector<complex> const& V,
                   ublas::matrix<complex,ublas::column_major>& dSdVm,
                   ublas::matrix<complex,ublas::column_major>& dSdVa);
void updatePowersystem(Powersystem& pws,
                       ublas::matrix<complex,ublas::column_major> const& Y,
                       ublas::vector<complex> const& V);
} // end of namespace ssutils

} // end of namespace elabtsaot

#endif // SSUTILS_H
