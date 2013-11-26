/*!
\file calibrationeditor.h

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.
\author Lilis Georgios, georgios.lilis at epfl dot ch, Elab
EPFL
*/

#ifndef CALIBRATIONEDITOR_H
#define CALIBRATIONEDITOR_H

#include <QVector>
#include <QSplitter>
class QLabel;
class QwtPlot;
class QCheckBox;
class QSpinBox;
class QByteArray;
class QAction;

#include <stdint.h>

namespace elabtsaot{

class Logger;
class EmulatorHw;
class Emulator;
class USBFPGAInterface;
struct deviceCalibrationData;

class CalibrationEditor : public QSplitter{

  Q_OBJECT

 public:

  // ------------------------- Functions -------------------------
  CalibrationEditor( Emulator* emu, Logger* log, QWidget* parent = 0);
  ~CalibrationEditor();

  int init();
  void updt();
  int exportFile(QString filename);
  int importFile(QString filename);

  Emulator const* emu() const;

 public slots:

  void hardReset();
  void startCalibrationSlot();
  void calibrationSetterSlot();
  void endCalibrationModeSlot();
  void displayCurvesSlot();
  void displayCalibrationDataSlot();
  void setOptionsSlot();
  void checkCellSlot();
  void potTestSlot();
  void potTestErrorSlot();
  void calibrationExportSlot();
  void calibrationImportSlot();

 public:

  QVector<deviceCalibrationData*> _master_store;

 private:

  // --- Calibration components functions-----
  int _ADCOffsetConverterCalibration(int devId);
  int _convertersCalibration(int devId);
  int _conversionResistorCalibrationNew(int devId);
  int _gridResistorCalibrationNew(int devId,int testid);

  // --- Other essential functions-------------
  void _displayCurveNew();
  int _parseRawResults(size_t sliceindex);
  void _displayCalibrationData(QVector<bool> dialogoptions,int devId);
  void _leastSquares(QVector<double> const& data,
                     QVector<double> const& xdata,
                     double* alpha, double* beta);
  void _resultsHandling(QVector<uint32_t> const& nodedata, int node,
                        QVector<QVector<double> > *decodedresultsreal,
                        QVector<QVector<double> > *decodedresultsimag ,int numofSamples);
  void _offsetGainHandling(QVector<uint32_t>* encodedinput, int gainoroffset);
  void _softReset();

  // ------------------------- Variables -------------------------
  Emulator* _emu;
  Logger* _log;
  EmulatorHw* _cal_emuhw;

  QVector<int> _resCode;
  QVector<int> _DACCodeFirst;
  QVector<int> _DACCodeSecond;
  QVector<uint32_t> _confVector;
  QByteArray _options;
  int _conversionResCodeReal;
  int _conversionResCodeImag;

  // 24 first positions are the ADC/DAC values
  QVector<QVector<QString> > _calibrationNameDataNew;
  QVector<QVector<double> > _calibrationOffsetDataNew;
  QVector<QVector<double> > _calibrationGainDataNew;
  QVector<QVector<int> > _calibrationIdNew;
  QVector<QVector<double> > _calibrationRabNew;
  QVector<QVector<double> > _calibrationRwNew;
  QVector<QVector<QVector<double> > > _rawResultsNew;
  QVector<QVector<QVector<double> > > _lsqResultsNew;

  //Some extra
  QVector<QVector<double> > _P3ResNew;
  QVector<QVector<double> > _P1ResNew;

  // ----- GUI widgets -----
  QCheckBox *chk0, *chk1, *chk2, *chk3;
  QCheckBox *chk4, *chk5, *chk6, *chk7;
  QCheckBox *chk8, *chk9, *chk10, *chk11;
  QCheckBox *autocheckopt;
  QVector<QCheckBox*> *devices_set;
  QVector<QLabel*> *devices_set_labels;
  QSpinBox *first_value_combobox;
  QSpinBox *second_value_combobox;
  QSpinBox *num_of_values_combobox;
  QSpinBox *interval;
  QVector<QSpinBox*> *References_Real;
  QVector<QSpinBox*> *References_Imag;
  QwtPlot *plot;
  QLabel *offsetlabel;
  QLabel *gainlabel;
  QLabel *resistorlabel;
  QAction *displayCurvesAct, *displayCalibrationDataAct, *setOptionsAct, *checkCellsAct;
  QAction *potTestAct, *potTestErrorAct;
};

struct deviceCalibrationData{
  int devId;
  QString deviceName;
  QVector<QVector<QString> > nameDataNew;
  QVector<QVector<double> > offsetDataNew;
  QVector<QVector<double> > gainDataNew;
  QVector<QVector<int> > idNew;
  QVector<QVector<double> > rabNew;
  QVector<QVector<double> > rwNew;
  QVector<QVector<QVector<double> > > rawResultsNew;
  QVector<QVector<QVector<double> > > lsqResultsNew;
};

} // end of namespace elabtsaot

#endif // CALIBRATIONEDITOR_H
