
#include "sliceanalog.h"
using namespace elabtsaot;

//#include <vector>
using std::vector;
#include <complex>
using std::complex;
#include <limits>
using std::numeric_limits;

#define HORIZONTALNUMBEROFATOMS 6 //!< Default horizontal number of atoms
#define VERTICALNUMBEROFATOMS 4   //!< Default vertical number of atoms
#define DEFGOTGAIN 1.25           //!< Default GOT gain level [Volt]
#define DEFGOTOFFSET 2.5          //!< Default GOT offset [Volt]

SliceAnalog::SliceAnalog() :
  _atomSet(VERTICALNUMBEROFATOMS, vector<Atom>(HORIZONTALNUMBEROFATOMS,
          Atom(&_real_voltage_ref,&_imag_voltage_ref))),
  _real_voltage_ref( DAC_DEF_TAP, DAC_DEF_RESOLUTION, DAC_DEF_OUTMIN, DAC_DEF_OUTMAX ),
  _imag_voltage_ref( DAC_DEF_TAP, DAC_DEF_RESOLUTION, DAC_DEF_OUTMIN, DAC_DEF_OUTMAX ) {}

int SliceAnalog::reset(bool complete){
  int ans = 0;
  // ----- Initialize atom set -----
  // Add left side connections of the slice
  if ( _atomSet.size() > 1 )
    for ( size_t k = 0; k != _atomSet.size()-1 ; ++k)
      _atomSet[k][0].set_embr_exist( EMBRPOS_L , true );
  // Add down side connections of the slice
  if ( _atomSet[0].size() > 2)
    for ( size_t k = 1; k != _atomSet[0].size()-1 ; ++k)
      _atomSet[0][k].set_embr_exist( EMBRPOS_D , true );
  // Initialize atom members
  for ( size_t k = 0 ; k != _atomSet.size() ; ++k )
    for ( size_t m = 0 ; m != _atomSet[k].size() ; ++m )
      ans |= _atomSet[k][m].reset( complete );

  // ----- Initialize private members -----
  _got_gain = static_cast<double>(DEFGOTGAIN);
  _got_offset = static_cast<double>(DEFGOTOFFSET);
  // Perform value-wise reset to the voltage reference DACs
  // (alternative is tap-wise reset)
  ans |= _real_voltage_ref.reset( false, complete );
  ans |= _imag_voltage_ref.reset( false, complete );
  return ans;
}

void SliceAnalog::size(size_t& ver, size_t& hor) const{
  ver = _atomSet.size();
  if (_atomSet.size()==0)  hor = 0;
  else                    ver = _atomSet[0].size();
}

double SliceAnalog::getMinMaxAchievableR() const{
  double minMaxR = numeric_limits<double>::max();
  for ( unsigned k = 0 ; k != _atomSet.size() ; ++k )
    for ( unsigned m = 0 ; m != _atomSet[k].size() ; ++m )
      if ( _atomSet[k][m].getMinMaxAchievableR() < minMaxR )
        minMaxR = _atomSet[k][m].getMinMaxAchievableR();
  return minMaxR;
}

ublas::matrix<complex<double> > SliceAnalog::calculate_Y_matrix(){
  // TODO for proper emulator branches (EMBRPOS_ R, UR, U) take into account the
  // case when mid grounding switch is closed
  ublas::matrix<complex<double> > Y_hw;
  size_t n = _atomSet.size() * _atomSet[0].size();
  Y_hw.resize(n,n);
  Y_hw.clear();

  size_t k, m;
  size_t a_n = 0; // a_tom n_umber
  double temp_y = 0;

  for ( k = 0 ; k != _atomSet.size() ; ++k ){
    for ( m = 0 ; m != _atomSet[k].size() ; ++m ){

      // --- For the atom node ---
      temp_y = 0;

      // Resistance to ground switch is closed
      if ( _atomSet[k][m].node_real_sw_resistance() ){
        // Resistance to ground potentiometer internal switch is closed
        if ( _atomSet[k][m].node_real_pot_resistance_sw() ){
          temp_y += 1/ ( _atomSet[k][m].node_real_pot_resistance_r() );
        }
      }
      // Add node admittance to element (a_n,a_n)
      Y_hw(a_n,a_n) += complex<double>(0, temp_y);

      // --- For branch EMBRPOS_R ---
      temp_y = 0;
      assert( _atomSet[k][m].embr_exist(EMBRPOS_R) ); // Embr phys exists

      // Proper emulator branch (near and far end potentiometers)
      if ( _atomSet[k][m].embr_real_pot_near_sw(EMBRPOS_R) &&
           _atomSet[k][m].embr_real_pot_far_sw(EMBRPOS_R)){
        // Near and far end potentiometer internal switch is closed
        temp_y += 1/( _atomSet[k][m].embr_real_pot_near_r(EMBRPOS_R) +
                      _atomSet[k][m].embr_real_pot_far_r(EMBRPOS_R) );
      }
      // Branch short circuit switch
      if ( _atomSet[k][m].embr_real_sw_sc(EMBRPOS_R) ){
        // Branch short circuit switch is closed
        temp_y += 1.0 / 2.5; // 2.5 Ohm is the typical value of the sc switch cl
      }
      // Add emulator branch admittance ..
      // .. to element (a_n,a_n) and  ..
      Y_hw(a_n,a_n) += complex<double>(0, temp_y);
      // to element (a_n,a_n+1) unless we are at the last column of the atom set
      if ( m != (_atomSet[k].size()-1) )
        Y_hw(a_n,a_n+1) -= complex<double>(0, temp_y);

      // --- For branch EMBRPOS_UR ---
      temp_y = 0;
      assert( _atomSet[k][m].embr_exist(EMBRPOS_UR) ); // Embr phys exists

      // Proper emulator branch (near and far end potentiometers)
      if ( _atomSet[k][m].embr_real_pot_near_sw(EMBRPOS_UR) &&
           _atomSet[k][m].embr_real_pot_far_sw(EMBRPOS_UR)){
        // Near and far end potentiometer internal switch is closed
        temp_y += 1/( _atomSet[k][m].embr_real_pot_near_r(EMBRPOS_UR) +
                      _atomSet[k][m].embr_real_pot_far_r(EMBRPOS_UR) );
      }
      // Branch short circuit switch
      if ( _atomSet[k][m].embr_real_sw_sc(EMBRPOS_UR) ){
        // Branch short circuit switch is closed
        temp_y += 1.0 / 2.5; // 2.5 Ohm is the typical value of the sc sw closed
      }
      // Add emulator branch admittance ..
      // .. to element (a_n,a_n) and  ..
      Y_hw(a_n,a_n) += complex<double>(0, temp_y);
      // .. to element (a_n+1,a_n+1) unless we are at either the last column
      // or the last row of the atom set
      if ( ( k != (_atomSet.size()-1) ) && ( m != (_atomSet[k].size()-1) ) )
        Y_hw(a_n+1,a_n+1) -= complex<double>(0, temp_y);

      // --- For branch EMBRPOS_U ---
      temp_y = 0;
      assert( _atomSet[k][m].embr_exist(EMBRPOS_U) ); // Embr phys exists

      // Proper emulator branch (near and far end potentiometers)
      if ( _atomSet[k][m].embr_real_pot_near_sw(EMBRPOS_U) &&
           _atomSet[k][m].embr_real_pot_far_sw(EMBRPOS_U)){
        // Near and far end potentiometer internal switch is closed
        temp_y += 1/( _atomSet[k][m].embr_real_pot_near_r(EMBRPOS_U) +
                      _atomSet[k][m].embr_real_pot_far_r(EMBRPOS_U) );
      }
      // Branch short circuit switch
      if ( _atomSet[k][m].embr_real_sw_sc(EMBRPOS_U) ){
        // Branch short circuit switch is closed
        temp_y += 1.0 / 2.5; // 2.5 Ohm is the typical value of the sc sw closed
      }
      // Add emulator branch admittance ..
      // .. to element (a_n,a_n) and  ..
      Y_hw(a_n,a_n) += complex<double>(0, temp_y);
      // to element (a_n+1,a_n) unless we are at the last row of the atom set
      if ( k != (_atomSet.size()-1) )
        Y_hw(a_n+1,a_n) -= complex<double>(0, temp_y);

      ++a_n; // increase atom number
    }
  }

  return Y_hw;
}

void SliceAnalog::calibrate(SliceAnalog const& cal_sl){
  size_t ver, hor;
  this->size(ver,hor);
  for ( size_t m = 0 ; m != ver ; ++m ){
    for ( size_t n = 0 ; n != hor ; ++n ){
      Atom const* cal_am = cal_sl.getAtom(m,n);
      _atomSet[m][n].calibrate(*cal_am);
    } // horizontal atom loop
  } // vertical atom loop
}

int SliceAnalog::nodeCurrentSource(size_t id_ver, size_t id_hor, double seriesR, double shuntR){
  int ans = 0;
  // Connect current source
  _atomSet[id_ver][id_hor].set_node_real_sw_current(true);
  _atomSet[id_ver][id_hor].set_node_imag_sw_current(true);
  // Set series resistance
  ans |= _atomSet[id_ver][id_hor].set_node_real_pot_current_r( seriesR );
  ans |= _atomSet[id_ver][id_hor].set_node_imag_pot_current_r( seriesR );

  // Disconnect currenct voltage source, and shunt resistor
  _atomSet[id_ver][id_hor].set_node_real_sw_voltage(false);
  _atomSet[id_ver][id_hor].set_node_imag_sw_voltage(false);
  _atomSet[id_ver][id_hor].set_node_real_sw_current_shunt(false);
  _atomSet[id_ver][id_hor].set_node_imag_sw_current_shunt(false);

  // Connect resistance to ground - if needed
  if (shuntR > 0){
    _atomSet[id_ver][id_hor].set_node_real_sw_resistance(true);
    _atomSet[id_ver][id_hor].set_node_imag_sw_resistance(true);
    ans |= _atomSet[id_ver][id_hor].set_node_real_pot_resistance_r( shuntR );
    ans |= _atomSet[id_ver][id_hor].set_node_imag_pot_resistance_r( shuntR );
  } else {
    _atomSet[id_ver][id_hor].set_node_real_sw_resistance(false);
    _atomSet[id_ver][id_hor].set_node_imag_sw_resistance(false);
  }
  return ans;
}

int SliceAnalog::nodeVoltageSource(size_t id_ver, size_t id_hor, double shuntR){
  int ans = 0;
  // Connect voltage source
  _atomSet[id_ver][id_hor].set_node_real_sw_voltage(true);
  _atomSet[id_ver][id_hor].set_node_imag_sw_voltage(true);

  // Disconnect current source and current shunt resistor
  _atomSet[id_ver][id_hor].set_node_real_sw_current(false);
  _atomSet[id_ver][id_hor].set_node_imag_sw_current(false);
  _atomSet[id_ver][id_hor].set_node_real_sw_current_shunt(false);
  _atomSet[id_ver][id_hor].set_node_imag_sw_current_shunt(false);

  // Connect resistance to ground - if needed
  if (shuntR > 0){
    _atomSet[id_ver][id_hor].set_node_real_sw_resistance(true);
    _atomSet[id_ver][id_hor].set_node_imag_sw_resistance(true);
    ans |= _atomSet[id_ver][id_hor].set_node_real_pot_resistance_r( shuntR );
    ans |= _atomSet[id_ver][id_hor].set_node_imag_pot_resistance_r( shuntR );
  } else {
    _atomSet[id_ver][id_hor].set_node_real_sw_resistance(false);
    _atomSet[id_ver][id_hor].set_node_imag_sw_resistance(false);
  }
  return ans;
}

int SliceAnalog::nodeDisconnect(size_t id_ver, size_t id_hor){
  // Open all switches
  _atomSet[id_ver][id_hor].set_node_real_sw_current(false);
  _atomSet[id_ver][id_hor].set_node_imag_sw_current(false);
  _atomSet[id_ver][id_hor].set_node_real_sw_current_shunt(false);
  _atomSet[id_ver][id_hor].set_node_imag_sw_current_shunt(false);
  _atomSet[id_ver][id_hor].set_node_real_sw_voltage(false);
  _atomSet[id_ver][id_hor].set_node_imag_sw_voltage(false);
  _atomSet[id_ver][id_hor].set_node_real_sw_resistance(false);
  _atomSet[id_ver][id_hor].set_node_imag_sw_resistance(false);
  return 0;
}

int SliceAnalog::embrConnect(size_t id_ver, size_t id_hor, size_t pos,
                             double r_near, double r_far){

  // Input argument validation
  if (id_ver >= _atomSet.size())                 return 20;
  if (id_hor >= _atomSet[id_ver].size())         return 21;
  if (!_atomSet[id_ver][id_hor].embr_exist(pos)) return 22;

  int exitCode = 0;
  // Close end connection switches
  _atomSet[id_ver][id_hor].set_embr_real_pot_near_sw(pos, true);
  _atomSet[id_ver][id_hor].set_embr_imag_pot_near_sw(pos, true);
  _atomSet[id_ver][id_hor].set_embr_real_pot_far_sw(pos, true);
  _atomSet[id_ver][id_hor].set_embr_imag_pot_far_sw(pos, true);
  // Open mid gnd switches
  _atomSet[id_ver][id_hor].set_embr_real_sw_mid(pos, false);
  _atomSet[id_ver][id_hor].set_embr_imag_sw_mid(pos, false);
  // Set near/far resistors
  exitCode |= _atomSet[id_ver][id_hor].set_embr_real_pot_near_r(pos, r_near)   !=0?(1<<4):0 ;
  exitCode |= _atomSet[id_ver][id_hor].set_embr_imag_pot_near_r(pos, r_near)   !=0?(1<<5):0 ;
  exitCode |= _atomSet[id_ver][id_hor].set_embr_real_pot_far_r(pos, r_far)     !=0?(1<<6):0 ;
  exitCode |= _atomSet[id_ver][id_hor].set_embr_imag_pot_far_r(pos, r_far)     !=0?(1<<7):0 ;
  // Provision for too small values
  if ( (r_near + r_far) <= static_cast<double> (2*POTENTIOMETER_RW) ){
    // branch corresponding to a short circuit ( r_near ~== r_far ~== 0 )
    _atomSet[id_ver][id_hor].set_embr_real_sw_sc(pos, true);
    _atomSet[id_ver][id_hor].set_embr_imag_sw_sc(pos, true);
  } else {
    // branch corresponding to a normal branch
    _atomSet[id_ver][id_hor].set_embr_real_sw_sc(pos, false);
    _atomSet[id_ver][id_hor].set_embr_imag_sw_sc(pos, false);
  }
  return exitCode;
}

int SliceAnalog::embrDisconnect(size_t id_ver, size_t id_hor, size_t pos){

  // Input argument validation
  if ( id_ver >= _atomSet.size() )                    return 20;
  if ( id_hor >= _atomSet[id_ver].size() )            return 21;
  // Cannot modify an emulator hw branch that does not exist!
  if (!_atomSet[id_ver][id_hor].embr_exist(pos))      return 22;

  int exitCode = 0;
  // Open all corresponding switches
  _atomSet[id_ver][id_hor].set_embr_real_sw_sc(pos, false);
  _atomSet[id_ver][id_hor].set_embr_imag_sw_sc(pos, false);
  _atomSet[id_ver][id_hor].set_embr_real_sw_mid(pos, false);
  _atomSet[id_ver][id_hor].set_embr_imag_sw_mid(pos, false);
  _atomSet[id_ver][id_hor].set_embr_real_pot_near_sw(pos, false);
  _atomSet[id_ver][id_hor].set_embr_imag_pot_near_sw(pos, false);
  _atomSet[id_ver][id_hor].set_embr_real_pot_far_sw(pos, false);
  _atomSet[id_ver][id_hor].set_embr_imag_pot_far_sw(pos, false);
  return exitCode;
}

void SliceAnalog::set_got_gain(double val){ _got_gain = val; }
int SliceAnalog::set_got_offset(double val){
  if ( (val < 0) || (val > DAC_DEF_OUTMAX ) )
    // got_offset out of bounds [0 , DEFMAXVVOLT=5] Volt
    return 1;
  _got_offset = val;
  return 0;
}
int SliceAnalog::set_real_voltage_ref_val(double val){return _real_voltage_ref.set_out(val);}
int SliceAnalog::set_real_voltage_ref_tap(unsigned int tap){return _real_voltage_ref.set_tap(tap);}
int SliceAnalog::set_real_voltage_ref_out_min(double val,bool updateTap){return _real_voltage_ref.set_out_min(val,updateTap);}
int SliceAnalog::set_real_voltage_ref_out_max(double val,bool updateTap){return _real_voltage_ref.set_out_max(val,updateTap);}

int SliceAnalog::set_imag_voltage_ref_val(double val){return _imag_voltage_ref.set_out(val);}
int SliceAnalog::set_imag_voltage_ref_tap(unsigned int tap){return _imag_voltage_ref.set_tap(tap);}
int SliceAnalog::set_imag_voltage_ref_out_min(double val,bool updateTap){return _imag_voltage_ref.set_out_min(val,updateTap);}
int SliceAnalog::set_imag_voltage_ref_out_max(double val,bool updateTap){return _imag_voltage_ref.set_out_max(val,updateTap);}

// --- getters ---
Atom const* SliceAnalog::getAtom(size_t ver, size_t hor) const{return dynamic_cast<Atom const*>(&_atomSet[ver][hor]);}
size_t SliceAnalog::getEmbrCount() const{
  size_t ans = 0;
  for ( size_t k = 0 ; k != _atomSet.size() ; ++k )
    for ( size_t m = 0 ; m != _atomSet[k].size() ; ++m )
      ans += _atomSet[k][m].getEmbrCount();
  return ans;
}
double SliceAnalog::got_gain() const{ return _got_gain; }
double SliceAnalog::got_offset() const{ return _got_offset; }
double SliceAnalog::real_voltage_ref_val() const{ return _real_voltage_ref.out(); }
unsigned int SliceAnalog::real_voltage_ref_tap() const{ return _real_voltage_ref.tap(); }
double SliceAnalog::real_voltage_ref_val_min() const{ return _real_voltage_ref.out_min(); }
double SliceAnalog::real_voltage_ref_val_max() const{ return _real_voltage_ref.out_max(); }
unsigned int SliceAnalog::real_voltage_ref_tap_max() const{ return _real_voltage_ref.tap_max(); }
double SliceAnalog::imag_voltage_ref_val() const{ return _imag_voltage_ref.out(); }
unsigned int SliceAnalog::imag_voltage_ref_tap() const{ return _imag_voltage_ref.tap(); }
double SliceAnalog::imag_voltage_ref_val_min() const{ return _imag_voltage_ref.out_min(); }
double SliceAnalog::imag_voltage_ref_val_max() const{ return _imag_voltage_ref.out_max(); }
unsigned int SliceAnalog::imag_voltage_ref_tap_max() const{ return _imag_voltage_ref.tap_max(); }
