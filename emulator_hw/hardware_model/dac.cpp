
#include "dac.h"
using namespace elabtsaot;

#include "auxiliary.h"

#include <cmath> // double pow(double,double)

DAC::DAC( unsigned int tap,
          unsigned int resolution, double out_min, double out_max ) :
    _resolution(resolution),
    _out_min(out_min), _out_min_original(out_min),
    _out_max(out_max), _out_max_original(out_max),
    _tap_max(pow(2,resolution)-1) {
  set_tap( tap ); // sets _tap, _out
  _tap_original = _tap;
  _out_original = _out;
}

int DAC::reset( bool tapwise, bool complete){

  int ans;
  if ( tapwise )
    ans = set_tap( _tap_original );
  else
    ans = set_out( _out_original );

  if ( complete ){
    _out_min = _out_min_original;
    _out_max = _out_max_original;
  }

  return ans;
}

// Setters
int DAC::set_out( double out, double* p_mismatch, bool force ){

  // Out value asked for is below the minimum DAC achievable value
  if ( out < _out_min ){
    if ( force ){
      set_tap( 0 );
      if ( p_mismatch )
        *p_mismatch = _out - out;
      return 0;
    } else {
      return 1;
    }
  }

  // Out value asked for is above the maximum DAC achievable value
  double out_max = _out_min + (_out_max-_out_min)*static_cast<double>(_tap_max)
                                              / static_cast<double>(_tap_max+1);
  if ( out > out_max ){
    if ( force ){
      set_tap( _tap_max );
      if ( p_mismatch )
        *p_mismatch = _out - out;
    } else {
      return 2;
    }
  }

  // Out value asked for is within the limits of the DAC
  double tapd = (out - _out_min)/(_out_max - _out_min)
                 * static_cast<double>(_tap_max+1) ;
  unsigned int tap = static_cast<unsigned int>(auxiliary::round(tapd));
  int ans = set_tap( tap );
  if ( ans )
    return 1;
  if ( p_mismatch )
    *p_mismatch = _out - out;

  return 0;
}

int DAC::set_tap( unsigned int val ){
  if ( val > _tap_max )
    return 1;
  _tap = val;
  _out = _out_min + (_out_max - _out_min) *   static_cast<double>(_tap)
                                            / static_cast<double>(_tap_max+1);
  return 0;
}

int DAC::set_out_min( double out_min, bool updateTap ){
  _out_min = out_min;
  if ( updateTap ){
    return set_out( _out );
  } else {
    return set_tap( _tap );
  }
  return 0;
}

int DAC::set_out_max( double out_max, bool updateTap ){
  _out_max = out_max;
  if ( updateTap ){
    return set_out( _out );
  } else {
    return set_tap( _tap );
  }
  return 0;
}

// Getters
int DAC::set_out_min( double val, bool updateTap );
int DAC::set_out_max( double val, bool updateTap );
double DAC::out() const{ return _out; }
unsigned int DAC::tap() const{ return _tap; }
unsigned int DAC::resolution() const{ return _resolution; }
double DAC::out_min() const{ return _out_min; }
double DAC::out_min_original() const{ return _out_min_original; }
double DAC::out_max() const{ return _out_max; }
double DAC::out_max_original() const{ return _out_max_original; }
unsigned int DAC::tap_max() const{ return _tap_max; }
