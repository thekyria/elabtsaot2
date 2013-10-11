
#include "generator.h"
using namespace elabtsaot;

//#include <string>
using std::string;
//#include <complex>
using std::complex;
#include <iostream>
using std::cout;
using std::cin;
using std::endl;
#include <sstream>
using std::stringstream;

Generator::Generator( unsigned int extId,
                      string const& name,
                      unsigned int busExtId,
                      double voltageSetpoint,
                      double pgen,
                      double qgen,
                      double pmin,
                      double pmax,
                      double qmin,
                      double qmax,
                      double xd_1,
                      double M,
                      bool status) :
  _extId(extId), _name(name), _busExtId(busExtId),
  _avr(true), _voltageSetpoint(voltageSetpoint),
  _pgen(pgen), _qgen(qgen), _pmin(pmin), _pmax(pmax), _qmin(qmin), _qmax(qmax),
  _status(status),
  _fss(50), _model(GENMODEL_0p0), _xl(0), _ra(0),
  _xd(0), _xd_1(xd_1), _xd_2(0), _Td0_1(0), _Td0_2(0),
  _xq(0), _xq_1(0), _xq_2(0), _Tq0_1(0), _Tq0_2(0),
  _M(M), _D(0),
  _Ess(1), _deltass(0), _Uss(complex<double>(1,0)) {}

Generator::~Generator(){ ; }

string Generator::serialize() const{

  stringstream ss;

  ss << _extId << " ";
  ss << "_name:" << _name << ":_name ";
  ss << _busExtId << " ";
  ss << _avr << " ";
  ss << _voltageSetpoint << " ";
  ss << _pgen << " ";
  ss << _qgen << " ";
  ss << _pmin << " ";
  ss << _pmax << " ";
  ss << _qmin << " ";
  ss << _qmax << " ";
  ss << _status << " ";

  ss << _fss << " ";
  ss << _model << " ";
  ss << _xl << " ";
  ss << _ra << " ";
  ss << _xd << " ";
  ss << _xd_1 << " ";
  ss << _xd_2 << " ";
  ss << _Td0_1 << " ";
  ss << _Td0_2 << " ";
  ss << _xq << " ";
  ss << _xq_1 << " ";
  ss << _xq_2 << " ";
  ss << _Tq0_1 << " ";
  ss << _Tq0_2 << " ";
  ss << _M << " ";
  ss << _D << " ";

  ss << _Ess << " ";
  ss << _deltass << " ";
  ss << _Uss << " ";

  return ss.str();
}

void Generator::display() const{
  cout << "Generator " << _extId << " (" << _name << ") ";
  cout << "at bus " << _busExtId << ": ";
//  cout << "\t" << _voltageSetpoint;
  cout << "\t" << _pgen;
  cout << "\t" << _qgen;
//  cout << "\t" << _pmin;
//  cout << "\t" << _pmax;
//  cout << "\t" << _qmin;
//  cout << "\t" << _qmax;
  cout << "\t" << _status << endl;
}

// --- getters ---
unsigned int Generator::extId() const{ return _extId; }
string Generator::name() const{ return _name; }
unsigned int Generator::busExtId() const{ return _busExtId; }
double Generator::voltageSetpoint() const{ return _voltageSetpoint; }
bool Generator::avr() const{ return _avr; }
double Generator::pgen() const{ return _pgen; }
double Generator::qgen() const{ return _qgen; }
double Generator::pmin() const{ return _pmin; }
double Generator::pmax() const{ return _pmax; }
double Generator::qmin() const{ return _qmin; }
double Generator::qmax() const{ return _qmax; }
bool Generator::status() const{ return _status; }

int Generator::model() const{ return _model; }
double Generator::fss() const{ return _fss; }
double Generator::xl() const{ return _xl; }
double Generator::ra() const{ return _ra; }
double Generator::xd() const{ return _xd; }
double Generator::xd_1() const{ return _xd_1; }
double Generator::xd_2() const{ return _xd_2; }
double Generator::Td0_1() const{ return _Td0_1; }
double Generator::Td0_2() const{ return _Td0_2; }
double Generator::xq() const{ return _xq; }
double Generator::xq_1() const{ return _xq_1; }
double Generator::xq_2() const{ return _xq_2; }
double Generator::Tq0_1() const{ return _Tq0_1; }
double Generator::Tq0_2() const{ return _Tq0_2; }
double Generator::M() const{ return _M; }
double Generator::D() const{ return _D; }

double Generator::Ess() const{ return _Ess; }
double Generator::deltass() const{ return _deltass; }
complex<double> Generator::Uss() const{ return _Uss; }


// --- setters ---
int Generator::set_extId(unsigned int val){
  _extId = val;
  return 0;
}
int Generator::set_name(string const& val){
  _name = val;
  return 0;
}
int Generator::set_busExtId(unsigned int val){
  _busExtId = val;
  return 0;
}
int Generator::set_avr(bool val){
  _avr = val;
  return 0;
}
int Generator::set_voltageSetpoint(double val){
  if (val < 0)
    return 1;
  _voltageSetpoint = val;
  return 0;
}
int Generator::set_pgen(double val){
  _pgen = val;
  return 0;
}
int Generator::set_qgen(double val){
  _qgen = val;
  return 0;
}
int Generator::set_pmin(double val){
//  if (val > _pmax)
//    return 1;
  _pmin = val;
  return 0;
}
int Generator::set_pmax(double val){
//  if (val < _pmin)
//    return 1;
  _pmax = val;
  return 0;
}
int Generator::set_qmin(double val){
//  if (val > _qmax)
//    return 1;
  _qmin = val;
  return 0;
}
int Generator::set_qmax(double val){
//  if (val < _qmin)
//    return 1;
  _qmax = val;
  return 0;
}
int Generator::set_status(bool val){
  _status = val;
  return 0;
}

int Generator::set_model(int val){
  _model = val;
  return 0;
}
int Generator::set_fss(double val){
  if (val < 0)
    return 1;
  _fss = val;
  return 0;
}

int Generator::set_xl(double val){
  if (val < 0)
    return 1;
  _xl = val;
  return 0;
}
int Generator::set_ra(double val){
  if (val < 0)
    return 1;
  _ra = val;
  return 0;
}
int Generator::set_xd(double val){
  if (val < 0)
    return 1;
  _xd = val;
  return 0;
}
int Generator::set_xd_1(double val){
  if (val < 0)
    return 1;
  _xd_1 = val;
  return 0;
}
int Generator::set_xd_2(double val){
  if (val < 0)
    return 1;
  _xd_2 = val;
  return 0;
}
int Generator::set_Td0_1(double val){
  if (val < 0)
    return 1;
  _Td0_1 = val;
  return 0;
}
int Generator::set_Td0_2(double val){
  if (val < 0)
    return 1;
  _Td0_2 = val;
  return 0;
}
int Generator::set_xq(double val){
  if (val < 0)
    return 1;
  _xq = val;
  return 0;
}
int Generator::set_xq_1(double val){
  if (val < 0)
    return 1;
  _xq_1 = val;
  return 0;
}
int Generator::set_xq_2(double val){
  if (val < 0)
    return 1;
  _xq_2 = val;
  return 0;
}
int Generator::set_Tq0_1(double val){
  if (val < 0)
    return 1;
  _Tq0_1 = val;
  return 0;
}
int Generator::set_Tq0_2(double val){
  if (val < 0)
    return 1;
  _Tq0_2 = val;
  return 0;
}
int Generator::set_M(double val){
  if (val < 0)
    return 1;
  _M = val;
  return 0;
}
int Generator::set_D(double val){
  _D = val;
  return 0;
}

int Generator::set_Ess(double val){
  _Ess = val;
  return 0;
}
int Generator::set_deltass(double val){
  _deltass = val;
  return 0;
}

int Generator::set_Uss(complex<double> const& val){
  _Uss = val;
  return 0;
}

