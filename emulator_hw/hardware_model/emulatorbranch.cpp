
#include "emulatorbranch.h"
using namespace elabtsaot;

EmulatorBranch::EmulatorBranch():
    _sw_sc(false), _sw_mid(false), _pot_near(), _pot_far() {}

int EmulatorBranch::reset( bool complete ){

  _sw_sc = false;
  _sw_mid = false;

  int ans = 0;
  ans |= _pot_near.reset( complete );
  ans |= _pot_far.reset( complete );

  return ans;
}

// --- "getters" ---
bool EmulatorBranch::sw_sc() const{ return _sw_sc; }
bool EmulatorBranch::sw_mid() const{ return _sw_mid; }

double EmulatorBranch::pot_near_rab() const{ return _pot_near.rab(); }
double EmulatorBranch::pot_near_rw() const{ return _pot_near.rw(); }
double EmulatorBranch::pot_near_r() const{ return _pot_near.r(); }
double EmulatorBranch::pot_near_getRMax() const{ return _pot_near.getRMax(); }
unsigned int EmulatorBranch::pot_near_tap() const{ return _pot_near.tap(); }
unsigned int EmulatorBranch::pot_near_tap_max() const{ return _pot_near.tap_max(); }
bool EmulatorBranch::pot_near_sw() const{ return _pot_near.sw(); }
bool EmulatorBranch::pot_near_swA() const{ return _pot_near.swA(); }

double EmulatorBranch::pot_far_rab() const{ return _pot_far.rab(); }
double EmulatorBranch::pot_far_rw() const{ return _pot_far.rw(); }
double EmulatorBranch::pot_far_r() const{ return _pot_far.r(); }
double EmulatorBranch::pot_far_getRMax() const{ return _pot_far.getRMax(); }
unsigned int EmulatorBranch::pot_far_tap() const{ return _pot_far.tap(); }
unsigned int EmulatorBranch::pot_far_tap_max() const{ return _pot_far.tap_max(); }
bool EmulatorBranch::pot_far_sw() const{ return _pot_far.sw(); }
bool EmulatorBranch::pot_far_swA() const{ return _pot_far.swA(); }

// --- setters ---
void EmulatorBranch::set_sw_sc(bool val){ _sw_sc = val; }
void EmulatorBranch::set_sw_mid(bool val){ _sw_mid = val; }

int EmulatorBranch::set_pot_near_rab( double val, bool updateTap){ return _pot_near.set_rab( val, updateTap ); }
int EmulatorBranch::set_pot_near_rw( double val, bool updateTap){ return _pot_near.set_rw( val, updateTap ); }
int EmulatorBranch::set_pot_near_r(double pot_near_r, double* p_r_mismatch ){ return _pot_near.set_r(pot_near_r, p_r_mismatch); }
int EmulatorBranch::set_pot_near_tap( unsigned int val ){ return _pot_near.set_tap(val); }
void EmulatorBranch::set_pot_near_sw(bool val){ _pot_near.set_sw(val); }
int EmulatorBranch::set_pot_near_swA(bool val, bool updateTap){ return _pot_near.set_swA(val, updateTap); }

int EmulatorBranch::set_pot_far_rab( double val, bool updateTap){ return _pot_far.set_rab( val, updateTap ); }
int EmulatorBranch::set_pot_far_rw( double val, bool updateTap){ return _pot_far.set_rw( val, updateTap ); }
int EmulatorBranch::set_pot_far_r(double pot_far, double* p_rMismatch_far){ return _pot_far.set_r(pot_far, p_rMismatch_far); }
int EmulatorBranch::set_pot_far_tap( unsigned int val ){ return _pot_far.set_tap(val); }
void EmulatorBranch::set_pot_far_sw(bool val){ _pot_far.set_sw(val); }
int EmulatorBranch::set_pot_far_swA(bool val, bool updateTap){ return _pot_far.set_swA(val, updateTap); }
