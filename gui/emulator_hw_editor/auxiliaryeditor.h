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
class MoteurFengtian;

class AuxiliaryEditor : public QSplitter{

  Q_OBJECT

 public:

  AuxiliaryEditor(Emulator* emu, TDEmulator* tde_hwe, MoteurFengtian* sse_fen, QWidget* parent);

 private slots:

  void ratioZSlot(double val);
  void ratioVSlot(double val);
  void maxIpuSlot(double val);
  void autoRatioZTDSlot();
  void defaultRatiosTDSlot();
  void defaultRatiosDCPFSlot();
  void getMinMaxResistorSlot() const;
  void buildGSlicesSlot() const;

  void rawReadFromDeviceSlot();
  void rawWriteToDeviceSlot();

  void encodePowersystemGPFSlot();
  void encodePowersystemDCPFSlot();
  void encodePowersystemTDSlot();
  void writeEncodingSlot();
  void logPowersystemEncodingSlot();
  void logGotEncodingSlot();
  void importEncodingSlot();

  void hardResetPressedSlot();
  void endCalibrationSlot();
  void resetEmulationSlot();
  void validateSliceAssignementSlot();
  void validateMappingSlot();
  void validateFittingPFSlot();
  void validateFittingTDSlot();
  void getEmulatorHwStateSlot();
  void getEmulatorHwCalStateSlot();

  // MoteurFengtian operations
  void resetGPFSlot();

// signals:
//  void mmdChanged();
//  void emuChanged(bool);

 private:

  void _updtGlobals();

  Emulator* _emu;
  TDEmulator* _tde_hwe; // used for _tde_hwe->resetEmulation() in resetEmulationSlot()
  MoteurFengtian* _sse_fen; // used for _sse_fen->resetGPF() in resetGPFSlot()

  QDoubleSpinBox* ratioZForm;
  QDoubleSpinBox* ratioVForm;
  QDoubleSpinBox* ratioIForm;
  QDoubleSpinBox* maxIpuForm;
};

} // end of namespace elabtsaot

#endif // AUXILIARYEDITOR_H
