
#include "potentiometer.h"
using namespace elabtsaot;

#include "auxiliary.h"

#include <iostream>
using std::cout;
using std::endl;
#include <cmath>
// for the round and the power operator: double pow(double,double)

Potentiometer::Potentiometer() :
    _rab(POTENTIOMETER_RAB), _rw(POTENTIOMETER_RW),
    _resolution(POTENTIOMETER_RESOLUTION),
    _tap_max(pow(2,POTENTIOMETER_RESOLUTION)),
    _sw(true), _swA(POTENTIOMETER_SWA){
  set_tap( POTENTIOMETER_DEFTAPVALUE );
}

Potentiometer::Potentiometer(double r) :
    _rab(POTENTIOMETER_RAB), _rw(POTENTIOMETER_RW),
    _resolution(POTENTIOMETER_RESOLUTION),
    _tap_max(pow(2,POTENTIOMETER_RESOLUTION)),
    _sw(true), _swA(POTENTIOMETER_SWA){
  double temp;
  set_r(r, &temp);
}

Potentiometer::Potentiometer(unsigned int tap) :
    _rab(POTENTIOMETER_RAB), _rw(POTENTIOMETER_RW),
    _resolution(POTENTIOMETER_RESOLUTION),
    _tap_max(pow(2,POTENTIOMETER_RESOLUTION)),
    _sw(true), _swA(POTENTIOMETER_SWA){
  set_tap( tap );
}

int Potentiometer::display() const{
  cout << "Internal sw: " << _sw;
  cout << " Tap: " << _tap;
  cout << " => R: " << _r << endl;

  return 0;
}

int Potentiometer::reset( bool complete ){
  _sw = false;
  if ( complete ){
    _rab = POTENTIOMETER_RAB;
    _rw = POTENTIOMETER_RW;
    _swA = POTENTIOMETER_SWA;
  }

  return set_tap( POTENTIOMETER_DEFTAPVALUE );
}

int Potentiometer::set_rab(double rab, bool updateTap){

  if ( rab <= 0 )
    // Invalid new rab value!
    return 1;

  _rab = rab;
  // If the update tap flag is set then the tap setting is updated in order to
  // retain the resistance value _r in the potentiometer. Otherwise, _r is
  // changes due to the change in _rab.
  int ans;
  if ( updateTap ){
    ans = set_r( _r, 0, true );
  } else {
    int temp_r = r_from_tap( _tap, _rab, _rw, _swA );
    if ( temp_r < 0 ){
      ans = 2;
    } else{
      _r = temp_r;
      ans = 0;
    }
  }

  return ans;
}

int Potentiometer::set_rw(double rw, bool updateTap){

  if ( rw <= 0) {
    // Invalid new rw value!
    return 1;
  }

  _rw = rw;
  // If the update tap flag is set then the tap setting is updated in order to
  // retain the resistance value _r in the potentiometer. Otherwise, _r is
  // changes due to the change in _rw.
  int ans;
  if ( updateTap ){
    ans = set_r( _r, 0, true );
  } else {
    int temp_r = r_from_tap( _tap, _rab, _rw, _swA );
    if ( temp_r < 0 ){
      ans = 2;
    } else{
      _r = temp_r;
      ans = 0;
    }
  }

  return ans;
}

int Potentiometer::set_r(double r, double* p_rMismatch, bool force){

  double rmin = _swA ? parallel_r( _rw, _rab) : _rw;
  double rmax = getRMax();
  // Check that asked for resistance is within limits
  if ( r >= rmax ){
    // If the resistance value asked for is above the maximum achievable
    // limit of the potentiometer, so check the 'force' flag.
    // If the flag is set update the tap setting of the potentiometer to match
    // the resistance asked for as much as possible - set to _tap_max
    if ( force ){
      set_tap( _tap_max );
      if ( p_rMismatch )
        *p_rMismatch = _r - r;
    }
    return 1;
  }

  if ( r <= rmin ){
    // If the resistance value asked for is below the minimum
    // achievable limit of the potentiometer, so check the 'force' flag.
    // If the flag is set update the tap setting of the potentiometer to match
    // the resistance asked for as much as possible - set to _tap_max
    if ( force ){
      set_tap( 0 );
      if ( p_rMismatch )
        *p_rMismatch = _r - r;
    }
    return 2;
  }

  // The resistance asked for is within the achievable limits of the
  // potentiometer. So calculate the respective tap setting and update
  // accordingly.
  int new_tap = tap_from_r( r, _rab, _rw, _swA );
  if ( new_tap < 0 )
    return 3;

  set_tap( static_cast<unsigned int>(new_tap) );
  if ( p_rMismatch )
    *p_rMismatch = _r - r;

  return 0;
}

int Potentiometer::set_tap(unsigned int tap){
  if (tap > _tap_max){
    // Invalid tap value!
    return 1;
  } else{
    _tap = tap;
    _r = r_from_tap( _tap, _rab, _rw, _swA );

    return 0;
  }
}

void Potentiometer::set_sw(bool status){ _sw = status; }
int Potentiometer::set_swA(bool status, bool updateTap){
  _swA = status;

  // If the update tap flag is set then the tap setting is updated in order to
  // retain the resistance value _r in the potentiometer. Otherwise, _r is
  // changes due to the change in _rw.
  int ans;
  if ( updateTap ){
    ans = set_r( _r, 0, true );
  } else {
    int temp_r = r_from_tap( _tap, _rab, _rw, _swA );
    if ( temp_r < 0 ){
      ans = 1;
    } else{
      _r = temp_r;
      ans = 0;
    }
  }

  return ans;
}

// --- getters ---
double Potentiometer::rab() const{ return _rab; }
double Potentiometer::rw() const{ return _rw; }
unsigned int Potentiometer::resolution() const{ return _resolution; }
double Potentiometer::r() const{ return _r; }
unsigned int Potentiometer::tap() const{ return _tap; }
unsigned int Potentiometer::tap_max() const{ return _tap_max; }
bool Potentiometer::sw() const{ return _sw; }
bool Potentiometer::swA() const{ return _swA; }
double Potentiometer::getRMax() const{
  double rmax = _swA ? _rab : _rw+_rab;
  return rmax;
}

// --- static calculator functions ---
double Potentiometer::parallel_r( double r1 , double r2 ){
  return r1 * r2 / (r1 + r2);
}

double Potentiometer::r_from_tap( unsigned int tap,
                                  double rab,
                                  double rw,
                                  bool swA,
                                  unsigned int resolution ) {

  unsigned int tap_max = pow(2, resolution);

  if (tap > tap_max)
    // Invalid tap value! So return a negative value;
    return -1.0;

  double r;
  if ( swA ){
    r =   ( rab * static_cast<double>(tap)/static_cast<double>(tap_max) )
        + parallel_r( rw, rab*( 1.0 - static_cast<double>(tap) /
                                      static_cast<double>(tap_max)  ) );
  } else{
    r = rw + ( rab * static_cast<double>(tap)/static_cast<double>(tap_max) );
  }
  return r;
}

int Potentiometer::tap_from_r( double r,
                               double rab,
                               double rw,
                               bool swA,
                               unsigned int resolution ){
  double n, n1, n2, tap, A, B, C, D;
  unsigned int tap_max = pow(2, resolution);

  if ( swA ){
    // Check that asked for resistance is within limits
    if ( r >= rab ){
      // If the resistance value asked for is above rab, the maximum achievable
      // limit of the potentiometer, then a negative value is returned
      return -1;
    }

    if ( r <= parallel_r( rw, rab) ){
      // If the resistance value asked for is below rw//rab, the minimum
      // achievable limit of the potentiometer then a negative value is returned
      return -2;
    }

    // The resistance asked for is within the achievable limits of the
    // potentiometer. So calculate the respective tap setting and update
    // accordingly.

    // Let: n = tap / tap_max
    // and: r = rw + n*rab + (1-n)*rab//rw
    // solve for n (trinomial): A*n^2 + B*n + C = 0
    A = rab;
    B = - r - rab;
    C = r + r*rw/rab - rw;
    D = B*B - 4.0*A*C;
    if ( D < 0 )
      // Negative determinant! Cannot find a solution
      return -3;

    n1 = (-B + sqrt(D))/(2.0*A);
    n2 = (-B - sqrt(D))/(2.0*A);

    // DEBUG
//    cout << "Two solutions of the trinomial:";
//    cout << " n1 = " << n1;
//    cout << " n2 = " << n2;
//    cout << endl;
    // It was found out that the useful root is n2; TODO: prove WHY!

    n = n2;
    if ( n >= 1 ){
      // This check is normally redundant. This case would have been caught by
      // the check: 'if ( r >= rab )'
      n = 1; // saturate n to max possible
    } else if ( n <= 0 ){
      // This check is normally redundant. This case would have been caught by
      // the check: 'if ( r <= parallel_r( rw, rab) )'
      n = 0; // saturate n to min possible
    }

  } else{ // if (!swA)
    // Check that asked for resistance is within limits
    if ( r >= rab+rw ){
      // If the resistance value asked for is above rab+rw, the maximum
      // achievable limit of the potentiometer, then a negative value is returned
      return -1;
    }

    if ( r <= rw ){
      // If the resistance value asked for is below rw, the minimum
      // achievable limit of the potentiometer then a negative value is returned
      return -2;
    }

    // The resistance asked for is within the achievable limits of the
    // potentiometer. So calculate the respective tap setting and update
    // accordingly.
    n = (r-rw)/rab;
  }

  // Solve for tap = n * tap_max
  tap = n * static_cast<double>(tap_max);
  return static_cast<unsigned int>( auxiliary::round(tap) );
}

double const Potentiometer::_rw_trend[257] = {
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5,
5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9,
10, 10, 10, 10, 10, 11, 11, 11, 11, 12, 12, 12, 12, 13, 13, 13, 14, 14, 15, 15,
15, 14, 14, 14, 13, 13, 13, 13, 13, 12, 12, 12, 12, 12, 11, 11, 11, 11, 11, 11,
10, 10, 10, 10, 10, 10, 10, 9, 9, 9, 9, 9, 9, 9, 9, 9, 8, 8, 8, 8, 8, 8, 8, 8,
8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
10, 10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12,
12, 13, 13, 14, 14, 14, 15, 15, 15, 16, 15, 15, 16, 16, 17, 17, 18, 18, 19, 20,
20, 20, 20, 21, 21, 21, 21, 22, 22, 22, 23, 23, 22, 22, 22, 22, 21, 21, 21, 21,
20, 20, 20, 20, 19, 19, 19, 19, 18, 18, 18, 18, 17, 17, 17, 17, 16, 16, 16, 16,
15, 15, 15, 15, 14, 14 };
