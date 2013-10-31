/*!
\file sliceanalog.h
\brief Definition file for class SliceAnalog

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef SLICEANALOG_H
#define SLICEANALOG_H

#include "atom.h"

#include <boost/numeric/ublas/matrix.hpp>
namespace ublas = boost::numeric::ublas;

namespace elabtsaot{

/*!  Slice analog PCB design (4x6 lattice of Atoms)
  -----------------------------------
    |/_   |/_   |/_   |/_   |/_   |/_

  _ |/_   |/_   |/_   |/_   |/_   |/_

  _ |/_   |/_   |/_   |/_   |/_   |/_

  _ |/_   |/_   |/_   |/_   |/_   |/_
          |     |     |     |
  -----------------------------------

  \sa Atom

  \warning This class is part of the emulator_hw classes that provide a software
  representation of the hardware emulator developed at ELAB. As such, changes in
  hardware are reflected in the software classes. For more information on the
  hardware of the project contact: guillaume dot lanz at epfl dot ch (hw team
  leader)

  \author thekyria
  \date Sep 2013
*/
class SliceAnalog{

  friend class FitterEditor;
  friend class CalibrationEditor;

 public:

  SliceAnalog();
  int reset(bool complete);
  void size(size_t& ver, size_t& hor) const;

  double getMinMaxAchievableR() const; //!< Get min over max achievable R of the slice
//  ublas::matrix<std::complex<double> > calculate_Y_matrix();
  void calibrate(SliceAnalog const& cal_sl);

  int nodeCurrentSource(size_t id_ver, size_t id_hor, double seriesR, double shuntR);
  int nodeVoltageSource(size_t id_ver, size_t id_hor, double shuntR);
  void nodeDisconnect(size_t id_ver, size_t id_hor);
  int embrConnect(size_t id_ver, size_t id_hor, size_t pos, double r_near, double r_far);
  int embrDisconnect(size_t id_ver, size_t id_hor, size_t pos);

  void set_got_gain(double val);
  int set_got_offset(double val);
  int set_real_voltage_ref_val(double val);
  int set_real_voltage_ref_tap(unsigned int tap);
  int set_real_voltage_ref_out_min(double val, bool updateTap);
  int set_real_voltage_ref_out_max(double val, bool updateTap);
  int set_imag_voltage_ref_val(double val);
  int set_imag_voltage_ref_tap(unsigned int val);
  int set_imag_voltage_ref_out_min(double val, bool updateTap);
  int set_imag_voltage_ref_out_max(double val, bool updateTap);

  Atom const* getAtom(size_t ver, size_t hor) const;
  size_t getEmbrCount() const; //!< emulator branches physically present on slice
  double got_gain() const;
  double got_offset() const;
  double real_voltage_ref_val() const;
  unsigned int real_voltage_ref_tap() const;
  double real_voltage_ref_val_min() const;
  double real_voltage_ref_val_max() const;
  unsigned int real_voltage_ref_tap_max() const;
  double imag_voltage_ref_val() const;
  unsigned int imag_voltage_ref_tap() const;
  double imag_voltage_ref_val_min() const;
  double imag_voltage_ref_val_max() const;
  unsigned int imag_voltage_ref_tap_max() const;


private:

  std::vector<std::vector<Atom> > _atomSet;

  double _got_gain;   //!< Gain for the voltage of the GOT module
  double _got_offset; //!< Offset to compensate the ADC voltage; in [V]

  DAC _real_voltage_ref; //!< Real voltage reference in [Volt]
  DAC _imag_voltage_ref; //!< Imaginary voltage reference in [Volt]
};

} // end of namespace elabtsaot

#endif // SLICEANALOG_H
