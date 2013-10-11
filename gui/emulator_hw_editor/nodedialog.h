/*!
\file nodedialog.h

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef NODEDIALOG_H
#define NODEDIALOG_H

#include <QObject>
class QCheckBox;
class QSpinBox;
class QDoubleSpinBox;

namespace elabtsaot{

class Atom;

class NodeDialog : public QObject {

  Q_OBJECT

 public:

  NodeDialog( Atom* a, bool real, QWidget* parent = 0);
  int exec();

 public slots:

  void currentPotSlot( int val );
  void groundPotSlot( int val );

 private:

  Atom* _a;
  bool _real;

  // Layout variables
  QCheckBox* sw_voltage;
  QCheckBox* sw_current;
  QCheckBox* sw_current_shunt;
  QCheckBox* sw_resistance;

  QSpinBox* pot_current_tap;
  QDoubleSpinBox* pot_current_r;
  QDoubleSpinBox* pot_current_rab;
  QCheckBox* pot_current_sw;

  QSpinBox* pot_resistance_tap;
  QDoubleSpinBox* pot_resistance_r;
  QDoubleSpinBox* pot_resistance_rab;
  QCheckBox* pot_resistance_sw;

};

} // end of namespace elabtsaot

#endif // NODEDIALOG_H
