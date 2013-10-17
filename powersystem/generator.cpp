
#include "generator.h"
using namespace elabtsaot;

//#include <string>
using std::string;
//#include <complex>
using std::complex;
#include <sstream>
using std::stringstream;

Generator::Generator() :
  extId(0), status(true), name("defgen"), busExtId(-1),
  Pgen(0), Qgen(0), Vss(complex<double>(1,0)),
  model(GENMODEL_0p0), xl(0), ra(0), xd(0), xd_1(1), xd_2(0), Td0_1(0), Td0_2(0),
  xq(0), xq_1(0), xq_2(0), Tq0_1(0), Tq0_2(0), M(1), D(0),
  _Ess(1), _deltass(0) {}

string Generator::serialize() const{

  stringstream ss;

  ss << extId << " ";
  ss << status << " ";
  ss << "name:" << name << ":name ";
  ss << busExtId << " ";

  ss << Pgen << " ";
  ss << Qgen << " ";
  ss << Vss << " ";

  ss << model << " ";
  ss << xl << " ";
  ss << ra << " ";
  ss << xd << " ";
  ss << xd_1 << " ";
  ss << xd_2 << " ";
  ss << Td0_1 << " ";
  ss << Td0_2 << " ";
  ss << xq << " ";
  ss << xq_1 << " ";
  ss << xq_2 << " ";
  ss << Tq0_1 << " ";
  ss << Tq0_2 << " ";
  ss << M << " ";
  ss << D << " ";

  ss << _Ess << " ";
  ss << _deltass << " ";

  return ss.str();
}

void Generator::updateVariables(){
  complex<double> I = conj(complex<double>(Pgen,Qgen)/Vss);
  complex<double> Z(ra,xd_1);
  complex<double> E = Vss + I*Z;
  _Ess = abs(E);
  _deltass = arg(E);
}

double Generator::Ess() const{return _Ess;}
double Generator::deltass() const{return _deltass;}
