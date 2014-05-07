
#include "node.h"
using namespace elabtsaot;

Node::Node() :
    real_adc_offset_corr(NODE_ADC_OFFSET_CORR_NOMINAL), real_adc_gain_corr(NODE_ADC_GAIN_CORR_NOMINAL),
    real_dac_offset_corr(NODE_DAC_OFFSET_CORR_NOMINAL), real_dac_gain_corr(NODE_DAC_GAIN_CORR_NOMINAL),
    real_sw_voltage(false),
    real_sw_current(false),
    real_sw_current_shunt(false),
    real_sw_resistance(false),
    imag_adc_offset_corr(NODE_ADC_OFFSET_CORR_NOMINAL), imag_adc_gain_corr(NODE_ADC_GAIN_CORR_NOMINAL),
    imag_dac_offset_corr(NODE_DAC_OFFSET_CORR_NOMINAL), imag_dac_gain_corr(NODE_DAC_GAIN_CORR_NOMINAL),
    imag_sw_voltage(false),
    imag_sw_current(false),
    imag_sw_current_shunt(false),
    imag_sw_resistance(false),
    _real_pot_current(),
    _real_pot_resistance(),
    _imag_pot_current(),
    _imag_pot_resistance() {}

int Node::reset( bool complete ){
  int ans = 0;

  real_sw_voltage = false;
  real_sw_current = false;
  real_sw_current_shunt = false;
  real_sw_resistance = false;
  ans |= _real_pot_current.reset(complete);
  ans |= _real_pot_resistance.reset(complete);

  imag_sw_voltage = false;
  imag_sw_current = false;
  imag_sw_current_shunt = false;
  imag_sw_resistance = false;
  ans |= _imag_pot_current.reset(complete);
  ans |= _imag_pot_resistance.reset(complete);

  if (complete){
    real_adc_offset_corr = NODE_ADC_OFFSET_CORR_NOMINAL;
    real_adc_gain_corr = NODE_ADC_GAIN_CORR_NOMINAL;
    real_dac_offset_corr = NODE_DAC_OFFSET_CORR_NOMINAL;
    real_dac_gain_corr = NODE_DAC_GAIN_CORR_NOMINAL;

    imag_adc_offset_corr = NODE_ADC_OFFSET_CORR_NOMINAL;
    imag_adc_gain_corr = NODE_ADC_GAIN_CORR_NOMINAL;
    imag_dac_offset_corr = NODE_DAC_OFFSET_CORR_NOMINAL;
    imag_dac_gain_corr = NODE_DAC_GAIN_CORR_NOMINAL;
  }

  return ans;
}

// --- "getters" ---
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
