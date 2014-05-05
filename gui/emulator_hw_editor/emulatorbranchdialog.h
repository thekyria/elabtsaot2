/*!
\file emulatorbranchdialog.h

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef EMULATORBRANCHDIALOG_H
#define EMULATORBRANCHDIALOG_H

#include "emulatorbranch.h"

#include <QObject>
class QCheckBox;
class QSpinBox;
class QDoubleSpinBox;

namespace elabtsaot{

class EmulatorBranchDialog : public QObject {

  Q_OBJECT

 public:

  EmulatorBranchDialog(EmulatorBranch* embr, QWidget* parent=0);
  int exec();

 private slots:

  void nearPotSlot(int val);
  void farPotSlot(int val);

 private:

  EmulatorBranch* _embr;
  EmulatorBranch _embrLocal;

  QCheckBox* swScForm;
  QCheckBox* swMidForm;
  QCheckBox* potNearSwForm;
  QSpinBox* potNearTapForm;
  QDoubleSpinBox* potNearRForm;
  QCheckBox* potFarSwForm;
  QSpinBox* potFarTapForm;
  QDoubleSpinBox* potFarRForm;

};

} // end of namespace elabtsaot

#endif // EMULATORBRANCHDIALOG_H
