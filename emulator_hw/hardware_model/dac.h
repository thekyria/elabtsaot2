/*!
\file dac.h

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef DAC_H
#define DAC_H

#define DAC_DEF_RESOLUTION 16
#define DAC_DEF_OUTMIN 0.0
#define DAC_DEF_OUTMAX 5.0
#define DAC_DEF_TAP (1<<(DAC_DEF_RESOLUTION-1))

namespace elabtsaot{

class DAC{

 public:

  DAC(unsigned int tap, unsigned int resolution, double out_min, double out_max);
  int reset( bool tapwise, bool complete);

  // Setters
  int set_out(double val, double* p_mismatch = 0, bool force = true);
  int set_tap(unsigned int val);
  int set_out_min(double val, bool updateTap);
  int set_out_max(double val, bool updateTap);

  // Getters
  double out() const;
  unsigned int tap() const;
  unsigned int resolution() const;
  double out_min() const;
  double out_min_original() const;
  double out_max() const;
  double out_max_original() const;
  unsigned int tap_max() const;

 private:

  double _out;
  double _out_original;
  unsigned int _tap;
  unsigned int _tap_original;
  unsigned int _resolution;
  double _out_min;
  double _out_min_original;
  double _out_max;
  double _out_max_original;
  unsigned int _tap_max;

};

} // end of namespace elabtsaot

#endif // DAC_H
