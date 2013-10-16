
#include "branch.h"
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

#ifndef VERBOSE_PWS_BRN
#define VERBOSE_PWS_BRN 0
#endif //VERBOSE_PWS_BRN

Branch::Branch(unsigned int extId,
               unsigned int fromBusExtId,
               unsigned int toBusExtId,
               string const& name,
               double r,
               double x,
               double b,
               bool status) :
    _extId(extId), _fromBusExtId(fromBusExtId), _toBusExtId(toBusExtId),
    _name(name),
    _r(r), _x(x), _b_from(b/2), _b_to(b/2), _g_from(0), _g_to(0), _c_series_x(0),
    _Xratio(0), _Xratio_tap(0), _Xratio_base(0),
    _Xratio_tap_min(0), _Xratio_tap_max(0), _Xratio_tap_step(0),
    _Xshift(0), _Xshift_tap(0), _Xshift_base(0),
    _Xshift_tap_min(0), _Xshift_tap_max(0), _Xshift_tap_step(0),
    _status(status),
    _ifrom(complex<double>(0,0)), _ito(complex<double>(0,0)),
    _sfrom(complex<double>(0,0)), _sto(complex<double>(0,0)){}

string Branch::serialize() const{

  stringstream ss;

  ss << _extId << " ";
  ss << _fromBusExtId << " ";
  ss << _toBusExtId << " ";
  ss << "_name:" << _name << ":_name ";
  ss << _r << " ";
  ss << _x << " ";
  ss << _b_from << " ";
  ss << _b_to << " ";
  ss << _g_from << " ";
  ss << _g_to << " ";
  ss << _c_series_x << " ";
  ss << _Xratio << " ";
  ss << _Xratio_tap << " ";
  ss << _Xratio_base << " ";
  ss << _Xratio_tap_min << " ";
  ss << _Xratio_tap_max << " ";
  ss << _Xratio_tap_step << " ";
  ss << _Xshift << " ";
  ss << _Xshift_tap << " ";
  ss << _Xshift_base << " ";
  ss << _Xshift_tap_min << " ";
  ss << _Xshift_tap_max << " ";
  ss << _Xshift_tap_step << " ";
  ss << _status << " ";
  ss << _ifrom << " ";
  ss << _ito << " ";
  ss << _sfrom << " ";
  ss << _sto << " ";

  return ss.str();
}

void Branch::display() const{
  cout << "Branch " << _extId << " (" << _name << ") ";
  cout << "from bus " << _fromBusExtId << " ";
  cout << "to bus " << _toBusExtId << ":" << endl;
  cout << _r << " ";
  cout << _x << " ";
  cout << this->b() << " ";

  cout << _Xratio << " ";
  cout << _Xshift << " ";

  cout << _status << " ";

//  cout << _ifrom << " ";
//  cout << _ito << " ";
//  cout << _sfrom << " ";
//  cout << _sto << " ";

  cout << endl;
}

int Branch::Xparams_validate() const{
  // Validate Xratio parameters
  if ( _Xratio_base <= 0 )
    return 10;
  if ( _Xratio_tap_step <= 0 )
    return 11;
  if ( (_Xratio_base+_Xratio_tap_min*_Xratio_tap_step) <= 0 )
    // Minimum reachable _Xratio value cannot be non-positive
    return 14;

  // Validate Xshift parameters
  if ( _Xshift_tap_step <= 0 )
    return 20;

  return 0;
}

int Branch::Xparams_update(){
  int exitCode = 0;

  if ( (exitCode = Xparams_validate()) ){
#if VERBOSE_PWS_BRN
    cout << "Transformer parameters could not be validated!" << endl;
    cout << "Xparams_validate() exited with code: " << exitCode << endl;
#endif // VERBOSE_PWS_BRN
    return 1;
  }
  _Xratio = _Xratio_base + _Xratio_tap * _Xratio_tap_step;
  _Xshift = _Xshift_base + _Xshift_tap * _Xshift_tap_step;

  return 0;
}

int Branch::Xratio_tap_change(bool up){
  // Assert that branch is a transformer (and is not a simple line)
  if ( _Xratio == 0 )
    return 1;

  // Check that transformer ratio tap related variables are properly initialized
  if ( _Xratio_base <= 0 )
    return 20;
  if ( _Xratio_tap_step <= 0 )
    return 21;
  if ( (_Xratio_base+_Xratio_tap_min*_Xratio_tap_step) <= 0 )
    // Minimum reachable _Xratio value cannot be non-positive
    return 24;

  if ( up ){
    // Step up change requested
    if ( _Xratio_tap < _Xratio_tap_max )
      ++_Xratio_tap;
    else
      // Transformer already at its upper tap limit
      return 4;
  } else{
    // Step down change requested
    if ( _Xratio_tap > _Xratio_tap_min)
      --_Xratio_tap;
    else
      // Transformer already at its lower tap limit
      return 5;
  }
  _Xratio = _Xratio_base + _Xratio_tap * _Xratio_tap_step;

  return 0;
}

int Branch::Xratio_tap_change(int val){
  // Assert that branch is a transformer (and is not a simple line)
  if ( _Xratio == 0 )
    return 1;

  // Check that transformer ratio tap related variables are properly initialized
  if ( _Xratio_base <= 0 )
    return 20;
  if ( _Xratio_tap_step <= 0 )
    return 21;
  if ( (_Xratio_base+_Xratio_tap_min*_Xratio_tap_step) <= 0 )
    // Minimum reachable _Xratio value cannot be non-positive
    return 24;

  if ( val >= 0 ){
    // Step up change requested
    if ( (_Xratio_tap+val) <= _Xratio_tap_max )
      _Xratio_tap += val;
    else
      // Transformer would go above its upper ratio tap limit
      return 4;
  } else{ // ( val < 0 )
    // Step down change requested
    if ( (_Xratio_tap+val) >= _Xratio_tap_min)
      _Xratio_tap += val; // addition '+=' since val is negative!
    else
      // Transformer would go below its lower ratio tap limit
      return 5;
  }
  _Xratio = _Xratio_base + _Xratio_tap * _Xratio_tap_step;

  return 0;
}

int Branch::Xshift_tap_change(bool up){
  // Assert that branch is a transformer (and is not a simple line)
  if ( _Xratio == 0 )
    return 1;

  // Check that transformer shift tap related variables are properly initialized
  if ( _Xshift_tap_step <= 0 )
    return 21;

  if ( up ){
    // Step up change requested
    if ( _Xshift_tap < _Xshift_tap_max )
      ++_Xshift_tap;
    else
      // Transformer already at its upper shift tap limit
      return 4;
  } else{
    // Step down change requested
    if ( _Xshift_tap > _Xshift_tap_min)
      --_Xshift_tap;
    else
      // Transformer already at its lower shift tap limit
      return 5;
  }
  _Xshift = _Xshift_base + _Xshift_tap * _Xshift_tap_step;

  return 0;
}

int Branch::Xshift_tap_change(int val){
  // Assert that branch is a transformer (and is not a simple line)
  if ( _Xratio == 0 )
    return 1;

  // Check that transformer shift tap related variables are properly initialized
  if ( _Xshift_tap_step <= 0 )
    return 21;

  if ( val >= 0 ){
    // Step up change requested
    if ( (_Xshift_tap+val) <= _Xshift_tap_max )
      _Xshift_tap += val;
    else
      // Transformer would go above its upper shift tap limit
      return 4;
  } else{
    // Step down change requested
    if ( (_Xshift_tap+val) >= _Xshift_tap_min)
      _Xshift_tap += val;
    else
      // Transformer would go below its lower shift tap limit
      return 5;
  }
  _Xshift = _Xshift_base + _Xshift_tap * _Xshift_tap_step;

  return 0;
}

// --- getters ---
unsigned int Branch::extId() const{ return _extId; }
unsigned int Branch::fromBusExtId() const{ return _fromBusExtId; }
unsigned int Branch::toBusExtId() const{ return _toBusExtId; }
string Branch::name() const{ return _name; }
double Branch::r() const{ return _r; }
double Branch::x() const{ return _x; }
double Branch::b() const{ return _b_from+_b_to; }
double Branch::b_from() const{ return _b_from; }
double Branch::b_to() const{ return _b_to; }
double Branch::g() const{ return _g_from+_g_to; }
double Branch::g_from() const{ return _g_from; }
double Branch::g_to() const{ return _g_to; }
double Branch::c_series_x() const{ return _c_series_x; }

double Branch::Xratio() const{ return _Xratio; }
int Branch::Xratio_tap() const{ return _Xratio_tap; }
double Branch::Xratio_base() const{ return _Xratio_base; }
int Branch::Xratio_tap_min() const{ return _Xratio_tap_min; }
int Branch::Xratio_tap_max() const{ return _Xratio_tap_max; }
double Branch::Xratio_tap_step() const{ return _Xratio_tap_step; }
double Branch::Xshift() const{ return _Xshift; }
int Branch::Xshift_tap() const{ return _Xshift_tap; }
double Branch::Xshift_base() const{ return _Xshift_base; }
int Branch::Xshift_tap_min() const{ return _Xshift_tap_min; }
int Branch::Xshift_tap_max() const{ return _Xshift_tap_max; }
double Branch::Xshift_tap_step() const{ return _Xshift_tap_step; }

bool Branch::status() const{ return _status; }

complex<double> Branch::ifrom() const{ return _ifrom; }
complex<double> Branch::ito() const{ return _ito; }
complex<double> Branch::sfrom() const{ return _sfrom; }
complex<double> Branch::sto() const{ return _sto; }

// --- setters ---
void Branch::set_extId(unsigned int val){ _extId = val; }
void Branch::set_fromBusExtId(unsigned int val){ _fromBusExtId = val; }
void Branch::set_toBusExtId(unsigned int val){ _toBusExtId = val; }
void Branch::set_name(string const& val){ _name = val; }
void Branch::set_r(double val){ _r = val; }
void Branch::set_x(double val){ _x = val; }
void Branch::set_b(double val){ _b_from = val/2; _b_to = val/2; }
void Branch::set_b_from(double val){ _b_from = val; }
void Branch::set_b_to(double val){ _b_to = val; }
void Branch::set_g(double val){ _g_from = val/2; _g_to = val/2; }
void Branch::set_g_from(double val){ _g_from = val; }
void Branch::set_g_to(double val){ _g_to = val; }
void Branch::set_c_series_x(double val){ _c_series_x = val; }

void Branch::set_Xratio(double val){ _Xratio = val; }
int Branch::set_Xratio_tap(int val){
  if ( val < _Xratio_tap_min )
    return 1;
  if ( val > _Xratio_tap_max )
    return 2;
  _Xratio_tap = val;
  return 0;
}
int Branch::set_Xratio_base(double val){
  if ( val <= 0 )
    return 1;
  _Xratio_base = val;
  return 0;
}
void Branch::set_Xratio_tap_min(int val){ _Xratio_tap_min = val; }
void Branch::set_Xratio_tap_max(int val){ _Xratio_tap_max = val; }
int Branch::set_Xratio_tap_step(double val){
  if ( val <= 0 )
    return 1;
  _Xratio_tap_step = val;
  return 0;
}
void Branch::set_Xshift(double val){ _Xshift = val; }
int Branch::set_Xshift_tap(int val){
  if ( val < _Xshift_tap_min )
    return 1;
  if ( val > _Xshift_tap_max )
    return 2;
  _Xshift_tap = val;
  return 0;
}
int Branch::set_Xshift_base(double val){
  _Xshift_base = val;
  return 0;
}
void Branch::set_Xshift_tap_min(int val){ _Xshift_tap_min = val; }
void Branch::set_Xshift_tap_max(int val){ _Xshift_tap_max = val; }
int Branch::set_Xshift_tap_step(double val){
  if ( val <= 0 )
    return 1;
  _Xshift_tap_step = val;
  return 0;
}

void Branch::set_status(bool val){ _status = val; }

void Branch::set_ifrom(complex<double> const& val){ _ifrom = val; }
void Branch::set_ito(complex<double> const& val){ _ito = val; }
void Branch::set_sfrom(complex<double> const& val){ _sfrom = val;}
void Branch::set_sto(complex<double> const& val){ _sto = val; }
