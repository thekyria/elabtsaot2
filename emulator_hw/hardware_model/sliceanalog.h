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
  void buildG(ublas::matrix<double,ublas::column_major>& G, bool real) const;
  void buildCd(ublas::matrix<int,ublas::column_major>& Cd) const;
  void buildGbr(ublas::vector<double>& Gbr, bool real) const;
  void buildBrStatus(ublas::vector<int>& brStatus, bool real) const;

  void calibrate(SliceAnalog const& cal_sl);

  int nodeCurrentSource(size_t id_ver, size_t id_hor, double seriesR, double shuntR);
  int nodeVoltageSource(size_t id_ver, size_t id_hor, double shuntR);
  void nodeDisconnect(size_t id_ver, size_t id_hor);
  int embrConnect(size_t id_ver, size_t id_hor, size_t pos, double r_near, double r_far);
  int embrDisconnect(size_t id_ver, size_t id_hor, size_t pos);

  Atom const* getAtom(size_t ver, size_t hor) const;
  size_t getEmbrCount() const; //!< emulator branches physically present on slice

  double ADCGain;   //!< ADC gain
  double ADCOffset; //!< ADC offset in [V] (the "zero" corresponds to this value)
  DAC real_voltage_ref; //!< Real voltage reference in [Volt]
  DAC imag_voltage_ref; //!< Imaginary voltage reference in [Volt]

 private:

  std::vector<std::vector<Atom> > _atomSet;

};

} // end of namespace elabtsaot

#endif // SLICEANALOG_H
