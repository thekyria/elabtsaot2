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

class CalibrationEditor : public QSplitter{

  Q_OBJECT

 public:

  // ------------------------- Functions -------------------------
  CalibrationEditor( Emulator* emu, Logger* log, QWidget* parent = 0);
  ~CalibrationEditor();

  int init();
  int updt();
  int calexport(QString filename);
  int calimport(QString filename);
  int hardreset();
  Emulator* _emu;

  //Master Store of all the devices
  struct devicedata{
    int device_id;
    QString deviceName;
    QVector <QVector<QString> > calibrationnamedatanew;
    QVector <QVector<double> > calibrationoffsetdatanew;
    QVector <QVector<double> > calibrationgaindatanew;
    QVector <QVector<int> > calibrationidnew;
    QVector <QVector<double> > calibrationrabnew;
    QVector <QVector<double> > calibrationrwnew;
    QVector <QVector <QVector<double> > > rawresultsnew;
    QVector <QVector <QVector<double> > > lsqresultsnew;
  };
  QVector<devicedata*> _master_store;


 public slots:

  void startCalibrationSlot();
  void calibrationSetterSlot();
  void endCalibrationModeSlot();
  void displayCurvesSlot();
  void resetCalibrationSlot();
  void displayCalibrationDataSlot();
  void setOptionsSlot();
  void checkCellSlot();
  void potTestSlot();
  void potTestErrorSlot();
  void calibrationExportSlot();
  void calibrationImportSlot();

 private:

  //---Calibration components functions-----
  int _ADCOffsetConverterCalibration(int devid);
  int _convertersCalibration(int devid);
  int _conversionResistorCalibrationNew(int devid);
  int _gridResistorCalibrationNew( int devid,int testid );

  //---Other essential functions-------------
  void _displayCurveNew();
  int _calibrationSetter( size_t sliceindex );
  void _displayCalibrationData( QVector<bool> dialogoptions,int devid );
  void _leastSquares( QVector<double> const& data,
                      QVector<double> const& xdata,
                      double* alpha, double* beta);
  void _resultsHandling( QVector<uint32_t> const& nodedata, int node,
                         QVector<QVector<double> > *decodedresultsreal,
                         QVector<QVector<double> > *decodedresultsimag ,int numofSamples);
  void _offsetGainHandling( QVector<uint32_t>* encodedinput, int gainoroffset);
  void _soft_reset();
  void _hard_reset();

  // ------------------------- Variables -------------------------
  QVector <int> rescode;
  QVector <int> DACcodefirst;
  QVector <int> DACcodesecond;
  QVector <uint32_t> confvector;
  QByteArray options;
  int _convertionrescodereal;
  int _convertionrescodeimag;

  //24 first positions are the ADC/DAC values
  //NEW
  //Stores
  QVector <QVector<QString> > calibrationnamedatanew;
  QVector <QVector<double> > calibrationoffsetdatanew;
  QVector <QVector<double> > calibrationgaindatanew;
  QVector <QVector<int> > calibrationidnew;
  QVector <QVector<double> > calibrationrabnew;
  QVector <QVector<double> > calibrationrwnew;
  QVector <QVector <QVector<double> > > rawresultsnew;
  QVector <QVector <QVector<double> > > lsqresultsnew;

  //Some extra
  QVector<QVector<double> > P3resnew;
  QVector<QVector<double> > P1resnew;

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

  // -----
  Logger* _log;
  EmulatorHw* _cal_emuhw;

};

} // end of namespace elabtsaot

#endif // CALIBRATIONEDITOR_H
