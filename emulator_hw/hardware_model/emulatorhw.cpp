
#include "emulatorhw.h"
using namespace elabtsaot;

//#include <vector>
using std::vector;
#include <limits>
using std::numeric_limits;

int EmulatorHw::init(size_t sliceCount){

  // init slices
  sliceSet.clear();
  for (size_t k = 0 ; k != sliceCount ; ++k )
    sliceSet.push_back( Slice() );

  int exitCode = 0;
  for ( size_t k = 0 ; k != sliceSet.size() ; ++k )
    exitCode |= sliceSet[k].reset(k, true);
  return exitCode;
}

int EmulatorHw::reset( bool complete ){
  // init slices
  int ans = 0;
  for ( unsigned k = 0 ; k != sliceSet.size() ; ++k )
    ans |= sliceSet[k].reset(complete);
  return ans;
}

double EmulatorHw::getMinMaxAchievableR() const{
  double minMaxR = numeric_limits<double>::max();
  for ( unsigned k = 0 ; k != sliceSet.size() ; ++k ){
    if ( sliceSet[k].ana.getMinMaxAchievableR() < minMaxR )
      minMaxR = sliceSet[k].ana.getMinMaxAchievableR();
  }
  return minMaxR;
}

size_t EmulatorHw::getEmbrCount() const{
  size_t ans = 0;
  for ( size_t k = 0 ; k != sliceSet.size() ; ++k )
    ans += sliceSet[k].ana.getEmbrCount();
  return ans;
}

void EmulatorHw::set_got_gain(double val){
  for (size_t k=0; k!=sliceSet.size(); ++k)
    sliceSet[k].ana.ADCGain = val;
}
void EmulatorHw::set_got_offset(double val){
  for (size_t k=0; k!=sliceSet.size(); ++k)
    sliceSet[k].ana.ADCOffset = val;
}

int EmulatorHw::set_real_voltage_ref_out(double val){
  for ( size_t k = 0 ; k != sliceSet.size() ; ++k )
    if ( int ans = sliceSet[k].ana.real_voltage_ref.set_out(val) )
      return k | (ans << 3);
  return 0;
}
int EmulatorHw::set_real_voltage_ref_tap(unsigned int tap){
  for ( size_t k = 0 ; k != sliceSet.size() ; ++k )
    if ( int ans = sliceSet[k].ana.real_voltage_ref.set_tap(tap) )
      return k | (ans << 3);
  return 0;
}
int EmulatorHw::set_imag_voltage_ref_out(double val){
  for ( size_t k = 0 ; k != sliceSet.size() ; ++k )
    if ( int ans = sliceSet[k].ana.imag_voltage_ref.set_out(val) )
      return k | (ans << 3);
  return 0;
}
int EmulatorHw::set_imag_voltage_ref_tap(unsigned int tap){
  for ( size_t k = 0 ; k != sliceSet.size() ; ++k )
    if ( int ans = sliceSet[k].ana.imag_voltage_ref.set_tap(tap) )
      return k | (ans << 3);
  return 0;
}
