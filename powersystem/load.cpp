
#include "load.h"
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

Load::Load() :
    _extId(0), _name("defload"), _busExtId(0),
    _pdemand(0), _qdemand(0),
    _v_exp_a(0), _v_exp_b(0), _k_p_f(0), _k_q_f(0), // Corresponding to pload
    _status(true),
    _Uss(complex<double>(1,0)) {}

Load::Load(unsigned int extId,
           string const& name,
           unsigned int busExtId,
           double pdemand,
           double qdemand,
           bool status) :
  _extId(extId), _name(name), _busExtId(busExtId),
  _pdemand(pdemand), _qdemand(qdemand),
  _v_exp_a(0), _v_exp_b(0), _k_p_f(0), _k_q_f(0), // Corresponding to pload
  _status(status),
  _Uss(complex<double>(1,0)) {}

string Load::serialize() const{
  stringstream ss;
  ss << _extId << " ";
  ss << "_name:" << _name << ":_name ";
  ss << _busExtId << " ";
  ss << _pdemand << " ";
  ss << _qdemand << " ";
  ss << _v_exp_a << " ";
  ss << _v_exp_b << " ";
  ss << _k_p_f << " ";
  ss << _k_q_f << " ";
  ss << _status << " ";
  ss << _Uss << " ";
  return ss.str();
}

void Load::display() const{
  cout << "Load " << _extId << " (" << _name << ") ";
  cout << "at bus " << _busExtId << endl;
  cout << "\t" << _pdemand;
  cout << "\t" << _qdemand;
  cout << "\t" << _status << endl;
}

unsigned int Load::extId() const{ return _extId; }
string Load::name() const{ return _name; }
unsigned int Load::busExtId() const{ return _busExtId; }
unsigned int Load::type() const{
  if ( _v_exp_a == 0 && _v_exp_b == 0 )
    return LOADTYPE_CONSTP;
  if ( _v_exp_a == 1 && _v_exp_b == 1 )
    return LOADTYPE_CONSTI;
  if ( _v_exp_a == 2 && _v_exp_b == 2 )
    return LOADTYPE_CONSTZ;
  return LOADTYPE_OTHER;
}
double Load::pdemand() const{ return _pdemand; }
double Load::qdemand() const{ return _qdemand; }
double Load::v_exp_a() const{ return _v_exp_a; }
double Load::v_exp_b() const{ return _v_exp_b; }
double Load::k_p_f() const{ return _k_p_f; }
double Load::k_q_f() const{ return _k_q_f; }
bool Load::status() const{ return _status; }
complex<double> Load::Uss() const{ return _Uss; }

void Load::set_extId(unsigned int val){ _extId = val; }
void Load::set_name(string const& val){ _name = val; }
void Load::set_type(unsigned int val){
  if (val == LOADTYPE_CONSTP){
    _v_exp_a = 0;
    _v_exp_b = 0;
  } else if (val == LOADTYPE_CONSTI){
    _v_exp_a = 1;
    _v_exp_b = 1;
  } else if (val == LOADTYPE_CONSTZ){
    _v_exp_a = 2;
    _v_exp_b = 2;
  }
}
void Load::set_busExtId(unsigned int val){ _busExtId = val; }
void Load::set_pdemand(double val){ _pdemand = val; }
void Load::set_qdemand(double val){ _qdemand = val; }
void Load::set_v_exp_a(double val){ _v_exp_a = val; }
void Load::set_v_exp_b(double val){ _v_exp_b = val; }
void Load::set_k_p_f(double val){ _k_p_f = val; }
void Load::set_k_q_f(double val){ _k_q_f = val; }
void Load::set_status(bool val){ _status = val; }
void Load::set_Uss(complex<double> const& val){ _Uss = val; }
