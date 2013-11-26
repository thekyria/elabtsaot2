
#include "node.h"
using namespace elabtsaot;

#include <iostream>
using std::cout;
using std::endl;

Node::Node() :
    _real_adc_offset_corr(0), _real_adc_gain_corr(1),
    _real_sw_voltage(false),
    _real_sw_current(false),
    _real_sw_current_shunt(false),
    _real_sw_resistance(false),
    _real_pot_current(),
    _real_pot_resistance(),
    _imag_adc_offset_corr(0), _imag_adc_gain_corr(1),
    _imag_sw_voltage(false),
    _imag_sw_current(false),
    _imag_sw_current_shunt(false),
    _imag_sw_resistance(false),
    _imag_pot_current(),
    _imag_pot_resistance() {}

int Node::display() const{
  cout << "Real part" << endl;
  cout << "sw_V=" << _real_sw_voltage << " ";
  cout << "sw_I=" << _real_sw_current << " ";
  cout << "sw_I_sh=" << _real_sw_current_shunt << " ";
  cout << "sw_R=" << _real_sw_resistance << " ";
  cout << "R_I=" << _real_pot_current.r() << " ";
  cout << "R_R=" << _real_pot_resistance.r() << " ";
  cout << endl;

  cout << "Imag part" << endl;
  cout << "sw_V=" << _imag_sw_voltage << " ";
  cout << "sw_I=" << _imag_sw_current << " ";
  cout << "sw_I_sh=" << _imag_sw_current_shunt << " ";
  cout << "sw_R=" << _imag_sw_resistance << " ";
  cout << "R_I=" << _imag_pot_current.r() << " ";
  cout << "R_R=" << _imag_pot_resistance.r() << " ";
  cout << endl;

  return 0;
}

int Node::reset( bool complete ){
  int exitFlag = 0;

  _real_sw_voltage = false;
  _real_sw_current = false;
  _real_sw_current_shunt = false;
  _real_sw_resistance = false;
  exitFlag |= _real_pot_current.reset(complete);
  exitFlag |= _real_pot_resistance.reset(complete);

  _imag_sw_voltage = false;
  _imag_sw_current = false;
  _imag_sw_current_shunt = false;
  _imag_sw_resistance = false;
  exitFlag |= _imag_pot_current.reset(complete);
  exitFlag |= _imag_pot_resistance.reset(complete);

  if ( complete ){
    _real_adc_offset_corr = 0;
    _real_adc_gain_corr = 1;
    _imag_adc_offset_corr = 0;
    _imag_adc_gain_corr = 1;
  }

  return exitFlag;
}

// --- "getters" ---
double Node::real_adc_offset_corr() const{ return _real_adc_offset_corr; }
double Node::real_adc_gain_corr() const{ return _real_adc_gain_corr; }
bool Node::real_sw_voltage() const{ return _real_sw_voltage; }
bool Node::real_sw_current() const{ return _real_sw_current; }
bool Node::real_sw_current_shunt() const{ return _real_sw_current_shunt; }
bool Node::real_sw_resistance() const{ return _real_sw_resistance;}
double Node::real_pot_current_rab() const{ return _real_pot_current.rab(); }
double Node::real_pot_current_rw() const{ return _real_pot_current.rw(); }
double Node::real_pot_current_r() const{ return _real_pot_current.r(); }
unsigned int Node::real_pot_current_tap() const{return _real_pot_current.tap();}
bool Node::real_pot_current_sw() const{ return _real_pot_current.sw(); }
bool Node::real_pot_current_swA() const{ return _real_pot_current.swA(); }
double Node::real_pot_current_getRMax() const{ return _real_pot_current.getRMax(); }
double Node::real_pot_resistance_rab() const{ return _real_pot_resistance.rab(); }
double Node::real_pot_resistance_rw() const{ return _real_pot_resistance.rw(); }
double Node::real_pot_resistance_r() const{ return _real_pot_resistance.r(); }
unsigned int Node::real_pot_resistance_tap() const{ return _real_pot_resistance.tap(); }
bool Node::real_pot_resistance_sw() const{ return _real_pot_resistance.sw(); }
bool Node::real_pot_resistance_swA() const{ return _real_pot_resistance.swA(); }
double Node::real_pot_resistance_getRMax() const{ return _real_pot_resistance.getRMax(); }

double Node::imag_adc_offset_corr() const{ return _imag_adc_offset_corr; }
double Node::imag_adc_gain_corr() const{ return _imag_adc_gain_corr; }
bool Node::imag_sw_voltage() const{ return _imag_sw_voltage; }
bool Node::imag_sw_current() const{ return _imag_sw_current; }
bool Node::imag_sw_current_shunt() const{ return _imag_sw_current_shunt; }
bool Node::imag_sw_resistance() const{ return _imag_sw_resistance;}
double Node::imag_pot_current_rab() const{ return _imag_pot_current.rab(); }
double Node::imag_pot_current_rw() const{ return _imag_pot_current.rw(); }
double Node::imag_pot_current_r() const{ return _imag_pot_current.r(); }
unsigned int Node::imag_pot_current_tap() const{return _imag_pot_current.tap();}
bool Node::imag_pot_current_sw() const{ return _imag_pot_current.sw(); }
bool Node::imag_pot_current_swA() const{ return _imag_pot_current.swA(); }
double Node::imag_pot_current_getRMax() const{ return _imag_pot_current.getRMax(); }
double Node::imag_pot_resistance_rab() const{ return _imag_pot_resistance.rab(); }
double Node::imag_pot_resistance_rw() const{ return _imag_pot_resistance.rw(); }
double Node::imag_pot_resistance_r() const{ return _imag_pot_resistance.r(); }
unsigned int Node::imag_pot_resistance_tap() const{ return _imag_pot_resistance.tap(); }
bool Node::imag_pot_resistance_sw() const{ return _imag_pot_resistance.sw(); }
bool Node::imag_pot_resistance_swA() const{ return _imag_pot_resistance.swA(); }
double Node::imag_pot_resistance_getRMax() const{ return _imag_pot_resistance.getRMax(); }

// --- "setters" ---
void Node::set_real_adc_offset_corr( double val ){ _real_adc_offset_corr = val; }
void Node::set_real_adc_gain_corr( double val ){ _real_adc_gain_corr = val; }
void Node::set_real_sw_voltage(bool val){ _real_sw_voltage = val; }
void Node::set_real_sw_current(bool val){ _real_sw_current = val; }
void Node::set_real_sw_current_shunt(bool val){ _real_sw_current_shunt = val; }
void Node::set_real_sw_resistance(bool val){ _real_sw_resistance = val; }
int Node::set_real_pot_current_rab(double val, bool updateTap){ return _real_pot_current.set_rab( val, updateTap ); }
int Node::set_real_pot_current_rw(double val, bool updateTap){ return _real_pot_current.set_rw( val, updateTap ); }
int Node::set_real_pot_current_r(double val){ double rMismatch; return _real_pot_current.set_r(val, &rMismatch); }
int Node::set_real_pot_current_tap(unsigned int val){ return _real_pot_current.set_tap(val); }
void Node::set_real_pot_current_sw(bool val){ _real_pot_current.set_sw(val); }
int Node::set_real_pot_current_swA(bool val, bool updateTap){ return _real_pot_current.set_swA(val, updateTap); }
int Node::set_real_pot_resistance_rab(double val, bool updateTap){ return _real_pot_resistance.set_rab( val, updateTap ); }
int Node::set_real_pot_resistance_rw(double val, bool updateTap){ return _real_pot_resistance.set_rw( val, updateTap ); }
int Node::set_real_pot_resistance_r(double val){ double rMismatch; return _real_pot_resistance.set_r(val, &rMismatch); }
int Node::set_real_pot_resistance_tap(unsigned int val){ return _real_pot_resistance.set_tap(val); }
void Node::set_real_pot_resistance_sw(bool val){ _real_pot_resistance.set_sw(val); }
int Node::set_real_pot_resistance_swA(bool val, bool updateTap){ return _real_pot_resistance.set_swA(val, updateTap); }

void Node::set_imag_adc_offset_corr( double val ){ _imag_adc_offset_corr = val; }
void Node::set_imag_adc_gain_corr( double val ){ _imag_adc_gain_corr = val; }
void Node::set_imag_sw_voltage(bool val){ _imag_sw_voltage = val; }
void Node::set_imag_sw_current(bool val){ _imag_sw_current = val; }
void Node::set_imag_sw_current_shunt(bool val){ _imag_sw_current_shunt = val; }
void Node::set_imag_sw_resistance(bool val){ _imag_sw_resistance = val; }
int Node::set_imag_pot_current_rab(double val, bool updateTap){ return _imag_pot_current.set_rab( val, updateTap ); }
int Node::set_imag_pot_current_rw(double val, bool updateTap){ return _imag_pot_current.set_rw( val, updateTap ); }
int Node::set_imag_pot_current_r(double val){ double rMismatch; return _imag_pot_current.set_r(val, &rMismatch); }
int Node::set_imag_pot_current_tap(unsigned int val){ return _imag_pot_current.set_tap(val); }
void Node::set_imag_pot_current_sw(bool val){ _imag_pot_current.set_sw(val); }
int Node::set_imag_pot_current_swA(bool val, bool updateTap){ return _imag_pot_current.set_swA(val, updateTap); }
int Node::set_imag_pot_resistance_rab(double val, bool updateTap){ return _imag_pot_resistance.set_rab( val, updateTap ); }
int Node::set_imag_pot_resistance_rw(double val, bool updateTap){ return _imag_pot_resistance.set_rw( val, updateTap ); }
int Node::set_imag_pot_resistance_r(double val){ double rMismatch; return _imag_pot_resistance.set_r(val, &rMismatch); }
int Node::set_imag_pot_resistance_tap(unsigned int val){ return _imag_pot_resistance.set_tap(val); }
void Node::set_imag_pot_resistance_sw(bool val){ _imag_pot_resistance.set_sw(val); }
int Node::set_imag_pot_resistance_swA(bool val, bool updateTap){ return _imag_pot_resistance.set_swA(val, updateTap); }
