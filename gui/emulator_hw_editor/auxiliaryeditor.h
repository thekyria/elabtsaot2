/*!
\file auxiliaryeditor.h

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef AUXILIARYEDITOR_H
#define AUXILIARYEDITOR_H

#include <QSplitter>

class QGroupBox;
class QDoubleSpinBox;
class QPushButton;

namespace elabtsaot{

class Emulator;
class TDEmulator;

class AuxiliaryEditor : public QSplitter{

  Q_OBJECT

 public:

  AuxiliaryEditor(Emulator* emu, TDEmulator* tde_hwe, QWidget* parent);

 private slots:

  void ratioZSlot(double val);
  void ratioVSlot(double val);
  void maxIpuSlot(double val);
  void autoRatioZSlot();
  void defaultRatiosSlot();

  void hardResetPressedSlot();
  void endCalibrationSlot();
  void resetEmulationSlot();
  void validateSliceAssignementSlot();
  void validateMappingSlot();
  void validateFittingSlot();
  void encodePowersystemSlot();
  void writeEncodingSlot();
  void logPowersystemEncodingSlot();
  void getEmulatorHwStateSlot();
  void getEmulatorHwCalStateSlot();
  void rawReadFromDeviceSlot();
  void rawWriteToDeviceSlot();

// signals:
//  void mmdChanged();
//  void emuChanged(bool);

 private:

  void _updtGlobals();
  int _rawReadFromDeviceDialog( size_t& devId,
                                unsigned int& startAddress,
                                size_t& wordCount );

  Emulator* _emu;
  TDEmulator* _tde_hwe; // used for _tde_hwe->resetEmulation() in resetEmulationSlot()

  QDoubleSpinBox* ratioZForm;
  QDoubleSpinBox* ratioVForm;
  QDoubleSpinBox* ratioIForm;
  QDoubleSpinBox* maxIpuForm;
  QPushButton* autoRatioZBut;
  QPushButton* defaultRatiosBut;

};

} // end of namespace elabtsaot

#endif // AUXILIARYEDITOR_H
