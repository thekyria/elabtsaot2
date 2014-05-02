
#include "calibrationeditor.h"
using namespace elabtsaot;

#include "logger.h"
#include "emulator.h"
#include "importerexporter.h"
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_item.h>
#include <qwt_series_data.h>
#include <qwt_legend.h>
#include <qwt_plot_picker.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_zoomer.h>
#include <qwt_point_data.h>

#include <QSpinBox>
#include <QToolBar>
#include <QByteArray>
#include <QAction>
#include <QComboBox>
#include <QLabel>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDialog>
#include <QPushButton>
#include <QVector>
#include <QRadioButton>
#include <QProgressBar>
#include <QPen>
#include <QtAlgorithms>
#include <QGroupBox>
#include <QDir>


#include <cstdlib>
#include <cmath>
#include <iostream>
using std::cout;
using std::endl;
using std::ios;
using std::ostream;
using std::string;
#include <iomanip>
using std::vector;
#include <stdio.h>
#include <direct.h>

#define CONSTRES 2200.0

CalibrationEditor::CalibrationEditor(Emulator* emu, Logger* log, QWidget* parent) :
    QSplitter(Qt::Vertical, parent), _emu(emu), _log(log), _cal_emuhw(new EmulatorHw()) {

  setMinimumSize(650,650);
  setChildrenCollapsible(false);

  QFont labfont( "Helvetica",10);
  labfont.setWeight(75);
  int defaultpointsize;
  defaultpointsize=labfont.pointSize();

  //Defining Layout, three main columns
  QWidget *calibresults = new QWidget(this);
  addWidget(calibresults);

  // ----- CalibrationEditor toolbar -----
  QToolBar *calibrationEditorToolbar = new QToolBar("Calibration editor toolbar",this);
  addWidget(calibrationEditorToolbar);
  calibrationEditorToolbar->setFixedHeight(30);

  QAction *startCalibrationAct = new QAction( QIcon(":/images/startcal.png"),
                                              "Start calibration", this );
  calibrationEditorToolbar->addAction(startCalibrationAct);
  connect( startCalibrationAct, SIGNAL(triggered()),
           this, SLOT(startCalibrationSlot()));

  QAction *calibrationSetterAct = new QAction( QIcon(":/images/setter.png"),
                                               "Set calibration values to emulator", this );
  calibrationEditorToolbar->addAction(calibrationSetterAct);
  connect( calibrationSetterAct, SIGNAL(triggered()),
           this, SLOT(calibrationSetterSlot()) );

  QAction* endCalibrationModeAct = new QAction( QIcon(":/images/end.png"),
                                                "End calibration mode", this );
  calibrationEditorToolbar->addAction( endCalibrationModeAct );
  connect( endCalibrationModeAct, SIGNAL(triggered()),
           this, SLOT(endCalibrationModeSlot()) );

  QAction *resetCalibrationAct = new QAction( QIcon(":/images/reset.png"),
                                              "Reset calibration", this );
  calibrationEditorToolbar->addAction(resetCalibrationAct);
  connect( resetCalibrationAct, SIGNAL(triggered()), this, SLOT(hardReset()) );

  calibrationEditorToolbar->addSeparator(); // -----

  displayCurvesAct = new QAction( QIcon(":/images/calcurv.png"),
                                  "Display curve", this );
  calibrationEditorToolbar->addAction(displayCurvesAct);
  connect( displayCurvesAct, SIGNAL(triggered()),
           this, SLOT(displayCurvesSlot()) );
  displayCurvesAct->setEnabled(false);

  displayCalibrationDataAct = new QAction( QIcon(":/images/list.png"),
                                           "Display all the calibration data", this );
  calibrationEditorToolbar->addAction(displayCalibrationDataAct);
  connect( displayCalibrationDataAct, SIGNAL(triggered()),
           this, SLOT(displayCalibrationDataSlot()) );
  displayCalibrationDataAct->setEnabled(false);

  setOptionsAct = new QAction( QIcon(":/images/debugconf.png"),
                               "Set calibration debug options", this );
  calibrationEditorToolbar->addAction(setOptionsAct);
  connect( setOptionsAct, SIGNAL(triggered()),
           this, SLOT(setOptionsSlot()) );
  setOptionsAct->setEnabled(false);

  checkCellsAct = new QAction (QIcon(":/images/checkcells.png"),
                               "Check working cells",this);
  calibrationEditorToolbar->addAction(checkCellsAct);
  connect( checkCellsAct, SIGNAL(triggered()),
           this, SLOT(checkCellSlot()) );
  checkCellsAct->setEnabled(false);

  potTestAct = new QAction (QIcon(":/images/pottest.png"),
                            "Check the min and max achived values from potensiomenters",this);
  calibrationEditorToolbar->addAction(potTestAct);
  connect( potTestAct, SIGNAL(triggered()),
           this, SLOT(potTestSlot()) );
  potTestAct->setEnabled(false);

  potTestErrorAct = new QAction (QIcon(":/images/pottesterror.png"),
                                 "Check the relative error of potensiomenters",this);
  calibrationEditorToolbar->addAction(potTestErrorAct);
  connect( potTestErrorAct, SIGNAL(triggered()),
           this, SLOT(potTestErrorSlot()) );
  potTestErrorAct->setEnabled(false);

  calibrationEditorToolbar->addSeparator(); // -----

  QAction *calibrationExportAct = new QAction (QIcon(":/images/export.png"),
                                               "Export calibration values",this);
  calibrationEditorToolbar->addAction(calibrationExportAct);
  connect( calibrationExportAct, SIGNAL(triggered()),
           this, SLOT(calibrationExportSlot()) );

  QAction *calibrationImportAct = new QAction (QIcon(":/images/import.png"),
                                               "Import calibration values",this);
  calibrationEditorToolbar->addAction(calibrationImportAct);
  connect( calibrationImportAct, SIGNAL(triggered()),
           this, SLOT(calibrationImportSlot()) );


  QHBoxLayout *mainlayout = new QHBoxLayout;
  calibresults->setLayout(mainlayout);

  QHBoxLayout *row = new QHBoxLayout;
  QVBoxLayout *col1 = new QVBoxLayout;
  QVBoxLayout *col2 = new QVBoxLayout;

  QLabel *chklabel=new QLabel;
  chklabel->setText("Calibration Tests");
  labfont.setPointSize(11);
  chklabel->setFont(labfont);
  labfont.setPointSize(defaultpointsize);

  //Defining the check boxes with the labels each time for each main column
  chk0 = new QCheckBox(this);
  chk1 = new QCheckBox(this);
  chk2 = new QCheckBox(this);
  chk3 = new QCheckBox(this);
  chk4 = new QCheckBox(this);
  chk5 = new QCheckBox(this);
  chk6 = new QCheckBox(this);
  chk7 = new QCheckBox(this);
  chk8 = new QCheckBox(this);
  chk9 = new QCheckBox(this);
  chk10 = new QCheckBox(this);
  chk11 = new QCheckBox(this);
  QCheckBox *chkall = new QCheckBox(this);
  chk0->setText("ADC offset");
  chk1->setText("DAC/ADC");
  chk2->setText("Conversion Resistor");
  chk3->setText("Internal Resistor");
  chk4->setText("P0CHIP1-2");
  chk5->setText("P1CHIP1-2");
  chk6->setText("P2CHIP1-2");
  chk7->setText("P3CHIP1-2");
  chk8->setText("P0P2CHIP3");
  chk9->setText("P1P3CHIP3");
  chk10->setText("P0P2EXT");
  chk11->setText("P1P3EXT");
  chkall->setText("Calibrate ALL");
  chkall->setFont(labfont);
  chk0->setEnabled(false);
  chk1->setEnabled(false);
  chk2->setEnabled(false);
  chk0->setChecked(true);
  chk1->setChecked(true);
  chk2->setChecked(true);
  chk3->setChecked(true);
  connect(chkall,SIGNAL(toggled(bool)),chk3,SLOT(setChecked(bool)));
  connect(chkall,SIGNAL(toggled(bool)),chk4,SLOT(setChecked(bool)));
  connect(chkall,SIGNAL(toggled(bool)),chk5,SLOT(setChecked(bool)));
  connect(chkall,SIGNAL(toggled(bool)),chk6,SLOT(setChecked(bool)));
  connect(chkall,SIGNAL(toggled(bool)),chk7,SLOT(setChecked(bool)));
  connect(chkall,SIGNAL(toggled(bool)),chk8,SLOT(setChecked(bool)));
  connect(chkall,SIGNAL(toggled(bool)),chk9,SLOT(setChecked(bool)));
  connect(chkall,SIGNAL(toggled(bool)),chk10,SLOT(setChecked(bool)));
  connect(chkall,SIGNAL(toggled(bool)),chk11,SLOT(setChecked(bool)));

  chkall->setChecked(true);

  QVBoxLayout *chkcol1 = new QVBoxLayout;
  QVBoxLayout *chkcol2 = new QVBoxLayout;
  QHBoxLayout *chkrow = new QHBoxLayout;

  col1->addSpacing(1);
  col1->addWidget(chklabel);
  col1->addSpacing(1);
  chkcol1->addWidget(chk0);
  chkcol1->addWidget(chk1);
  chkcol1->addWidget(chk2);
  chkcol1->addWidget(chk3);
  chkcol1->addWidget(chk4);
  chkcol1->addWidget(chk5);
  chkcol2->addWidget(chk6);
  chkcol2->addWidget(chk7);
  chkcol2->addWidget(chk8);
  chkcol2->addWidget(chk9);
  chkcol2->addWidget(chk10);
  chkcol2->addWidget(chk11);
  chkrow->addLayout(chkcol1);
  chkrow->addLayout(chkcol2);
  col1->addLayout(chkrow);
  col1->addSpacing(2);
  col1->addWidget(chkall);
  col1->addSpacing(3);


  QLabel *devIdLab= new QLabel("Devices to calibrate");
  labfont.setPointSize(11);
  devIdLab->setFont(labfont);
  labfont.setPointSize(defaultpointsize);
  QCheckBox *devid0;
  QCheckBox *devid1;
  QCheckBox *devid2;
  QCheckBox *devid3;
  devid0 = new QCheckBox ("Device 0");
  devid1 = new QCheckBox ("Device 1");
  devid2 = new QCheckBox ("Device 2");
  devid3 = new QCheckBox ("Device 3");
  devid0->setDisabled(true);
  devid1->setDisabled(true);
  devid2->setDisabled(true);
  devid3->setDisabled(true);
  devid0->setChecked(true);
  devices_set = new QVector<QCheckBox*>;
  devices_set->append(devid0);
  devices_set->append(devid1);
  devices_set->append(devid2);
  devices_set->append(devid3);


  QGridLayout *devidl = new QGridLayout;
  devidl->addWidget(devid0,0,0);
  devidl->addWidget(devid1,1,0);
  devidl->addWidget(devid2,0,1);
  devidl->addWidget(devid3,1,1);
  devidl->setVerticalSpacing(3);
  col1->addWidget(devIdLab);
  col1->addLayout(devidl);
  col1->addSpacing(2);


  QLabel *ramptitle=new QLabel;
  ramptitle->setText("Parameters");
  labfont.setPointSize(11);
  ramptitle->setFont(labfont);
  labfont.setPointSize(defaultpointsize);
  autocheckopt = new QCheckBox("Check for default values: ");
  autocheckopt->setChecked(true);
  col1->addWidget(ramptitle);
  col1->addWidget(autocheckopt);

  QLabel *first_value_label;
  QLabel *second_value_label;
  QLabel *num_of_values;
  first_value_label = new QLabel;
  first_value_label->setText("First tested: ");
  second_value_label = new QLabel;
  second_value_label->setText("Second tested: ");
  num_of_values = new QLabel;
  num_of_values->setText("Total number of values: ");
  first_value_combobox = new QSpinBox(this);
  second_value_combobox = new QSpinBox(this);
  num_of_values_combobox = new QSpinBox(this);
  num_of_values_combobox->setMinimum(0);
  num_of_values_combobox->setValue(20);
  num_of_values_combobox->setMaximum(100);
  first_value_combobox->setMaximum(4096);
  first_value_combobox->setMinimum(0);
  second_value_combobox->setMaximum(4096);
  second_value_combobox->setMinimum(0);
  first_value_combobox->clear();
  second_value_combobox->clear();
  num_of_values_combobox->setEnabled(false);
  first_value_combobox->setEnabled(false);
  second_value_combobox->setEnabled(false);
  QLabel *intervallabel = new QLabel("Interval time of update: ");
  interval = new QSpinBox(this);
  interval->setMaximum(99999);
  interval->setMinimum(200);
  interval->setValue(30000);

  QSpinBox *referencereal0 = new QSpinBox(this);
  QSpinBox *referenceimag0 = new QSpinBox(this);

  QSpinBox *referencereal1 = new QSpinBox(this);
  QSpinBox *referenceimag1 = new QSpinBox(this);

  QSpinBox *referencereal2 = new QSpinBox(this);
  QSpinBox *referenceimag2 = new QSpinBox(this);

  QSpinBox *referencereal3 = new QSpinBox(this);
  QSpinBox *referenceimag3 = new QSpinBox(this);

  referencereal0->setMaximum(99999);
  referencereal0->setValue(DAC_DEF_TAP);
  referencereal0->setFixedWidth(50);

  referenceimag0->setMaximum(99999);
  referenceimag0->setValue(DAC_DEF_TAP);
  referenceimag0->setFixedWidth(50);

  referencereal1->setMaximum(99999);
  referencereal1->setValue(DAC_DEF_TAP);
  referencereal1->setFixedWidth(50);

  referenceimag1->setMaximum(99999);
  referenceimag1->setValue(DAC_DEF_TAP);
  referenceimag1->setFixedWidth(50);

  referencereal2->setMaximum(99999);
  referencereal2->setValue(DAC_DEF_TAP);
  referencereal2->setFixedWidth(50);

  referenceimag2->setMaximum(99999);
  referenceimag2->setValue(DAC_DEF_TAP);
  referenceimag2->setFixedWidth(50);

  referencereal3->setMaximum(99999);
  referencereal3->setValue(DAC_DEF_TAP);
  referencereal3->setFixedWidth(50);

  referenceimag3->setMaximum(99999);
  referenceimag3->setValue(DAC_DEF_TAP);
  referenceimag3->setFixedWidth(50);

  References_Real = new QVector<QSpinBox*>;
  References_Imag = new QVector<QSpinBox*>;

  References_Real->append(referencereal0);
  References_Real->append(referencereal1);
  References_Real->append(referencereal2);
  References_Real->append(referencereal3);

  References_Imag->append(referenceimag0);
  References_Imag->append(referenceimag1);
  References_Imag->append(referenceimag2);
  References_Imag->append(referenceimag3);


  QLabel *refreallabel = new QLabel("Reference V real: ");
  QLabel *refimaglabel = new QLabel("Reference V imag: ");
  labfont.setPointSize(10);
  refreallabel->setFont(labfont);
  refimaglabel->setFont(labfont);
  labfont.setPointSize(defaultpointsize);

  QGridLayout *paraml = new QGridLayout;
  paraml->addWidget(first_value_label,0,0);
  paraml->addWidget(first_value_combobox,0,1);
  paraml->addWidget(second_value_label,1,0);
  paraml->addWidget(second_value_combobox,1,1);
  paraml->addWidget(num_of_values,2,0);
  paraml->addWidget(num_of_values_combobox,2,1);
  paraml->addWidget(intervallabel,3,0);
  paraml->addWidget(interval,3,1);

  QLabel *dev0_label = new QLabel("Device 0");
  QLabel *dev1_label = new QLabel("Device 1");
  QLabel *dev2_label = new QLabel("Device 2");
  QLabel *dev3_label = new QLabel("Device 3");
  devices_set_labels = new QVector<QLabel*>;
  devices_set_labels->append(dev0_label);
  devices_set_labels->append(dev1_label);
  devices_set_labels->append(dev2_label);
  devices_set_labels->append(dev3_label);
  labfont.setPointSize(9);
  dev0_label->setFont(labfont);
  dev1_label->setFont(labfont);
  dev2_label->setFont(labfont);
  dev3_label->setFont(labfont);
  labfont.setPointSize(defaultpointsize);

  QGridLayout *paraml_2 = new QGridLayout;
  paraml_2->setAlignment(Qt::AlignCenter);
  paraml_2->addWidget(refreallabel,0,1,1,1);
  paraml_2->addWidget(dev0_label,1,0);
  paraml_2->addWidget(dev1_label,2,0);
  paraml_2->addWidget(dev2_label,3,0);
  paraml_2->addWidget(dev3_label,4,0);
  paraml_2->addWidget(References_Real->at(0),1,1);
  paraml_2->addWidget(References_Real->at(1),2,1);
  paraml_2->addWidget(References_Real->at(2),3,1);
  paraml_2->addWidget(References_Real->at(3),4,1);

  paraml_2->addWidget(refimaglabel,0,3,1,1);
  paraml_2->addWidget(References_Imag->at(0),1,3);
  paraml_2->addWidget(References_Imag->at(1),2,3);
  paraml_2->addWidget(References_Imag->at(2),3,3);
  paraml_2->addWidget(References_Imag->at(3),4,3);


  col1->addLayout(paraml);
  col1->addSpacing(3);
  col1->addLayout(paraml_2);
  col1->addSpacing(10);

  init();

  connect(autocheckopt,SIGNAL(toggled(bool)),num_of_values_combobox,SLOT(setDisabled(bool)));
  connect(autocheckopt,SIGNAL(toggled(bool)),first_value_combobox,SLOT(setDisabled(bool)));
  connect(autocheckopt,SIGNAL(toggled(bool)),second_value_combobox,SLOT(setDisabled(bool)));


  plot = new QwtPlot();
  QFont axFont("Times", 10);
  QwtText axText;
  axText.setFont(axFont);
  plot->setTitle( QwtText( "Debug Plot" ) );
  plot->setAxisTitle(QwtPlot::xBottom, axText);
  plot->insertLegend( new QwtLegend() );
  plot->setAxisScale(QwtPlot::xBottom, 0.0,3);
  plot->setAxisScale(QwtPlot::yLeft,0.0,3.0);
  plot->setAxisAutoScale(QwtPlot::xBottom,true);
  plot->setAxisAutoScale(QwtPlot::yLeft,true);
  col2->addWidget(plot);

  QwtPlotPicker* picker = new QwtPlotPicker(plot->canvas());
  picker->setTrackerMode(QwtPicker::AlwaysOn);
  QwtPlotMagnifier* magnifier = new  QwtPlotMagnifier(plot->canvas());
  magnifier->setMouseButton(Qt::RightButton,Qt::ControlModifier);
  magnifier->setMouseButton(Qt::RightButton,Qt::ControlModifier);
  QwtPlotPanner* panner = new QwtPlotPanner(plot->canvas());
  panner->setMouseButton(Qt::LeftButton,Qt::ControlModifier);
  panner->setCursor(Qt::ClosedHandCursor);
  QwtPlotZoomer* zoomer = new QwtPlotZoomer(plot->canvas(),true);
  zoomer->setEnabled(true);


  double offsetvalue;
  double gainvalue;
  double res1;
  double res2;
  double res3;
  double res4;
  offsetvalue=0;
  gainvalue=0;
  res1=0.00 ;
  res2=0.00;
  res3=0.00;
  res4=0.00;
  offsetlabel = new QLabel;
  //  offsetlabel->setText(QString ("Offset: %0").arg(offsetvalue));
  //  offsetlabel->setFont(labfont);

  gainlabel = new QLabel;
  //  gainlabel->setText(QString ("Gain: %0").arg(gainvalue));
  //  gainlabel->setFont(labfont);

  resistorlabel = new QLabel;
  //  resistorlabel->setText(QString ("Resistor(0|25|50|75|100kohm) values: %0 | %1 | %2 | %3 | %4").arg(0).arg(0).arg(0).arg(0));
  //  resistorlabel->setFont(labfont);


  QHBoxLayout *gainoffsetlayout = new QHBoxLayout;
  gainoffsetlayout->addSpacing(30);
  gainoffsetlayout->addWidget(offsetlabel);
  gainoffsetlayout->addWidget(gainlabel);
  gainoffsetlayout->addWidget(resistorlabel);
  gainoffsetlayout->setStretch(1,30);
  gainoffsetlayout->setStretch(2,30);
  gainoffsetlayout->setStretchFactor(resistorlabel,300);
  gainoffsetlayout->setAlignment(offsetlabel,Qt::AlignLeft);
  gainoffsetlayout->setAlignment(gainlabel,Qt::AlignLeft);
  gainoffsetlayout->setAlignment(resistorlabel,Qt::AlignLeft);
  col2->addLayout(gainoffsetlayout);
  row->addLayout(col1);
  row->addLayout(col2);

  mainlayout->addLayout(row);

  //Initialize the calibration storage vectors
  //The first dimensions if for the real and imag part of emulator
  for(int i=0;i<2;++i){
    _calibrationNameDataNew.append(QVector<QString>());
    _calibrationOffsetDataNew.append(QVector<double>());
    _calibrationGainDataNew.append(QVector<double>());
    _calibrationIdNew.append(QVector<int>());
    _calibrationRabNew.append(QVector<double>());
    _calibrationRwNew.append(QVector<double>());
    _rawResultsNew.append(QVector<QVector <double> >());
    _lsqResultsNew.append(QVector<QVector <double> >());
  }

  //Options flag initialize
  _options.resize(10);
  _options[0]='6';
  _options[1]='6';
  _options[2]='6';
  _options[3]='6';
  _options[4]='6';
  _options[5]='6';

  //Resistor code setter
  _resCode.append(49);
  _resCode.append(126);
  _resCode.append(203);
  _resCode.append(49);
  _resCode.append(126);
  _resCode.append(203);
  //First and second value setter

  _DACCodeFirst.append(1229);
  _DACCodeFirst.append(819);
  _DACCodeFirst.append(656);
  _DACCodeFirst.append(819);
  _DACCodeFirst.append(1229);
  _DACCodeFirst.append(1229);
  _DACCodeSecond.append(2866);
  _DACCodeSecond.append(3276);
  _DACCodeSecond.append(3439);
  _DACCodeSecond.append(3276);
  _DACCodeSecond.append(2866);
  _DACCodeSecond.append(2866);
}

CalibrationEditor::~CalibrationEditor(){ delete _cal_emuhw; }

int CalibrationEditor::init(){
  // Update local emulator according to _emu->emuhw()
  _cal_emuhw->init( _emu->getHwSliceCount() );

  _master_store.clear();
  //Initialize label
  devices_set_labels->at(0)->setText("Device 0");
  devices_set_labels->at(1)->setText("Device 1");
  devices_set_labels->at(2)->setText("Device 2");
  devices_set_labels->at(3)->setText("Device 3");
  devices_set->at(0)->setText("Device 0");
  devices_set->at(1)->setText("Device 1");
  devices_set->at(2)->setText("Device 2");
  devices_set->at(3)->setText("Device 3");

  // Update devices gui options
  for (int i=0;i<4;++i){
    devices_set->at(i)->setChecked(false);
    References_Real->at(i)->setDisabled(true);
    References_Imag->at(i)->setDisabled(true);
  }
  for (size_t i=0;i<_emu->getUSBDevicesCount();++i){
    devices_set->at(i)->setChecked(true);
    devices_set->at(i)->setText(QString::fromStdWString(_emu->getUSBDevices().at(i).product));
    devices_set_labels->at(i)->setText(QString::fromStdWString(_emu->getUSBDevices().at(i).product));
    References_Real->at(i)->setEnabled(true);
    References_Imag->at(i)->setEnabled(true);
    //Initialize the storing of all active devices
    deviceCalibrationData* dd = new deviceCalibrationData;
    dd->devId = i;
    dd->deviceName = QString::fromStdWString(_emu->getUSBDevices().at(i).product);
    _master_store.append(dd);
  }
  displayCurvesAct->setEnabled(false);
  displayCalibrationDataAct->setEnabled(false);
  setOptionsAct->setEnabled(false);
  checkCellsAct->setEnabled(false);
  potTestAct->setEnabled(false);
  potTestErrorAct->setEnabled(false);
  return 0;
}

void CalibrationEditor::updt(){
  for (size_t i(0); i<_cal_emuhw->sliceSet.size(); ++i){
    int devId = _emu->sliceDeviceMap(i);
    if (devId<0) continue;
    References_Real->at(devId)->setValue(_emu->emuhw()->sliceSet[i].ana.real_voltage_ref_tap());
    References_Imag->at(devId)->setValue(_emu->emuhw()->sliceSet[i].ana.imag_voltage_ref_tap());
  }
}

int CalibrationEditor::exportFile(QString filename){
  return io::exportCalibrationValues(filename.toStdString(), this);
}

int CalibrationEditor::importFile(QString filename){
  return io::importCalibrationValues(filename.toStdString(), this);
}

Emulator const* CalibrationEditor::emu() const{ return dynamic_cast<Emulator const*>(_emu); }

void CalibrationEditor::hardReset(){
  cout<<"Reseting devices"<<endl;
  init();
  cout<<"Resetting calibration"<<endl<<endl;
  CalibrationEditor::_softReset();
  plot->detachItems(QwtPlotItem::Rtti_PlotItem);
  plot->setTitle(QwtText( "Debug Plot" ));
  plot->replot();
}

void CalibrationEditor::startCalibrationSlot(){

  int ans = _emu->validateSliceDeviceAssignement();
  if ( ans )
    cout << "Cannot validate Slice <-> Devices assignement, try to assign all slices to devices first" << endl;
  else{
    cout << "Slice <-> Devices assignement validated, proceeding to calibration! " << endl;
    displayCurvesAct->setEnabled(true);
    displayCalibrationDataAct->setEnabled(true);
    setOptionsAct->setEnabled(true);
    checkCellsAct->setEnabled(true);
    potTestAct->setEnabled(true);
    potTestErrorAct->setEnabled(true);
    vector<uint32_t> tempvector;
    for ( size_t i = 0 ; i < _cal_emuhw->sliceSet.size() ; ++i ){
      int devId = _emu->sliceDeviceMap(i);
      cout<<endl<<endl<<"Calibration of device "<<devId<<endl;
      //Reseting before test
      tempvector.clear();
      tempvector.push_back(2222);
      _emu->usbWrite( devId, 286,tempvector);
      if (chk0->isChecked()){
        cout<<"Running ADC offset calibration..."<<endl;
        int ans = _ADCOffsetConverterCalibration(devId);
        if (ans==1){
          cout<<"Emulator board "<<_emu->getUSBDevices().at(devId).deviceName<<" failed to respond, please do a hardware reset"<<endl;
          _softReset();
          return;
        }

      }
      _log->notifyProgress(5);
      if (chk1->isChecked()){
        cout<<"Running DAC calibration..."<<endl;
        int ans = _convertersCalibration(devId);
        if (ans==1){
          cout<<"Emulator board "<<_emu->getUSBDevices().at(devId).deviceName<<" failed to respond, please do a hardware reset"<<endl;
          _softReset();
          return;
        }

      }
      _log->notifyProgress(10);

      if (chk2->isChecked()){
        cout<<"Running Conversion resistor calibration..."<<endl;
        int ans = _conversionResistorCalibrationNew(devId);
        if (_options[3]!='6'){
          cout<<"Running debug Conversion resistor calibration..."<<endl;
          ans = _conversionResistorCalibrationNew(devId);
        }

        if (ans==3){
          cout<<endl;
          cout<<"Wrong results returned by board,reset and plug-unplug board "<<devId<<endl<<endl;
          cout<<"If not corrected, check soldering points of board and connectors"<<endl;
          _log->notifyProgress(100);
          _softReset();
          return;
        }
        else if (ans==1){
          cout<<"Emulator board "<<_emu->getUSBDevices().at(devId).deviceName<<" failed to respond, please do a hardware reset"<<endl;
          _softReset();
          return;
        }
      }
      _log->notifyProgress(20);

      if (chk3->isChecked()){
        cout<<"Running Internal resistor calibration..."<<endl;
        int ans =_gridResistorCalibrationNew(devId,0);
        if (ans==3)
          cout<<"I noticed some misbehaving Internal resistors, I will continue, be careful where you map. Run 'check cells' function"<<endl;
      }
      _log->notifyProgress(30);

      if (chk4->isChecked()){
        cout<<"Running P0Chip1-2 resistor calibration..."<<endl;
        int ans =_gridResistorCalibrationNew(devId,1);
        if (ans==3)
          cout<<"I noticed some misbehaving P0Chip1-2 resistors, I will continue, be careful where you map. Run 'check cells' function"<<endl;
      }
      _log->notifyProgress(40);

      if (chk5->isChecked()){
        cout<<"Running P1Chip1-2 resistor calibration..."<<endl;
        int ans =_gridResistorCalibrationNew(devId,2);
        if (ans==3)
          cout<<"I noticed some misbehaving P1Chip1-2 resistors, I will continue, be careful where you map. Run 'check cells' function"<<endl;
      }
      _log->notifyProgress(50);

      if (chk6->isChecked()){
        cout<<"Running P2Chip1-2 resistor calibration..."<<endl;
        int ans = _gridResistorCalibrationNew(devId,3);
        if (ans==3)
          cout<<"I noticed some misbehaving P2Chip1-2 resistors, I will continue, be careful where you map. Run 'check cells' function"<<endl;
      }
      _log->notifyProgress(60);

      if (chk7->isChecked()){
        cout<<"Running P3Chip1-2 resistor calibration..."<<endl;
        int ans =_gridResistorCalibrationNew(devId,4);
        if (ans==3)
          cout<<"I noticed some misbehaving P3Chip1-2 resistors, I will continue, be careful where you map. Run 'check cells' function"<<endl;
      }
      _log->notifyProgress(70);

      if (chk8->isChecked()){
        cout<<"Running P0P2Chip3 resistor calibration..."<<endl;
        int ans =_gridResistorCalibrationNew(devId,5);
        if (ans==3)
          cout<<"I noticed some misbehaving P0P2Chip3 resistors, I will continue, be careful where you map. Run 'check cells' function"<<endl;
      }
      _log->notifyProgress(80);

      if (chk9->isChecked()){
        cout<<"Running P1P3Chip3 resistor calibration..."<<endl;
        int ans =_gridResistorCalibrationNew(devId,6);
        if (ans==3)
          cout<<"I noticed some misbehaving P1P3Chip3 resistors, I will continue, be careful where you map. Run 'check cells' function"<<endl;
      }
      _log->notifyProgress(90);

      if (chk10->isChecked()){
        cout<<"Running P0P2EXT resistor calibration..."<<endl;
        int ans =_gridResistorCalibrationNew(devId,7);
        if (ans==3)
          cout<<"I noticed some misbehaving P0P2EXT resistors, I will continue, be careful where you map. Run 'check cells' function"<<endl;
      }
      _log->notifyProgress(95);

      if (chk11->isChecked()){
        cout<<"Running P1P3EXT resistor calibration..."<<endl;
        int ans =_gridResistorCalibrationNew(devId,8);
        if (ans==3)
          cout<<"I noticed some misbehaving P1P3EXT resistors, I will continue, be careful where you map. Run 'check cells' function"<<endl;
      }

      //Saving device data
      _master_store.at(i)->nameDataNew=_calibrationNameDataNew;
      _master_store.at(i)->offsetDataNew=_calibrationOffsetDataNew;
      _master_store.at(i)->gainDataNew=_calibrationGainDataNew;
      _master_store.at(i)->idNew=_calibrationIdNew;
      _master_store.at(i)->rabNew=_calibrationRabNew;
      _master_store.at(i)->rwNew=_calibrationRwNew;
      _master_store.at(i)->rawResultsNew=_rawResultsNew;
      _master_store.at(i)->lsqResultsNew=_lsqResultsNew;
      _softReset();//erasing the temp vector of device
      _log->notifyProgress(100);
    }
  }

}

void CalibrationEditor::calibrationSetterSlot(){

  if ( _cal_emuhw->sliceSet.size() == 0 ){ cout << "Slice set empty" << endl; return; }

  // Parse raw calibration values (currently in containers) into local _cal_emuhw copy
  cout << "Parsing raw calibration results (sliceId) ... ";
  for (size_t sliceindex(0); sliceindex<_cal_emuhw->sliceSet.size() ; ++sliceindex){
    cout << sliceindex << " ";
    int ans = _parseRawResults(sliceindex);
    if (ans){
      cout << endl;
      cout << "Parsing raw calibration results for sliceId" << sliceindex << " ";
      cout << "failed with code " << ans << endl;
    }
  }
  cout << endl;

  // Verify sizes
  // TODO: slice count, as well as atom count for each slice

  // Calibrate of the real EmulatorHw
  cout << "Copying calibration results to global emulator ... ";
  for (size_t k=0; k!=_emu->emuhw()->sliceSet.size(); ++k ){
    cout << k << " ";
    _emu->emuhw()->sliceSet[k].ana.calibrate(_cal_emuhw->sliceSet[k].ana);
  }
  cout << endl;
}

void CalibrationEditor::endCalibrationModeSlot(){
  int ans = _emu->endCalibrationMode();
  if (ans)
    cout << "Ending the emulator calibration mode failed with code " << ans << endl;
  else
    cout << "Ending the emulator calibration mode was successful" << endl;
  return;
}

void CalibrationEditor:: displayCurvesSlot(){
  if(_emu->getUSBDevicesCount()==0){
    cout<<"No device connected, connect at least one to display it's calibration data"<<endl;
    return;}
  _displayCurveNew();
  return;
}

void CalibrationEditor::displayCalibrationDataSlot(){
  if(_emu->getUSBDevicesCount()==0){
    cout<<"No device connected, connect at least one to display it's calibration data"<<endl;
    return;}
  QDialog predialog;
  predialog.setWindowTitle("Select values for displaying");
  QVBoxLayout predialogl;
  predialog.setLayout(&predialogl);

  QRadioButton *rop1 = new QRadioButton("Real values");
  rop1->setChecked(true);
  QRadioButton *rop2 = new QRadioButton("Imaginary values");
  QVBoxLayout *ropl=new QVBoxLayout;
  ropl->addWidget(rop1);
  ropl->addWidget(rop2);

  QCheckBox *op0=new QCheckBox("ADC offset");
  QCheckBox *op1=new QCheckBox("ADC/DAC gain and offset");
  QCheckBox *op2=new QCheckBox("Mesured resistor values");
  QCheckBox *op4=new QCheckBox("Rwiper of resistors");
  QCheckBox *op5=new QCheckBox("Rab of resistors");

  QSpinBox *dev = new QSpinBox;
  dev->setMinimum(0);
  dev->setMaximum(_emu->getUSBDevicesCount()-1);
  QLabel *devlabel = new QLabel;
  devlabel->setText("Device data to display");
  QHBoxLayout *devlayout = new QHBoxLayout;
  devlayout->addWidget(devlabel);
  devlayout->addWidget(dev);
  QVBoxLayout *opl = new QVBoxLayout;
  opl->addWidget(op0);
  opl->addWidget(op1);
  opl->addWidget(op2);
  opl->addWidget(op4);
  opl->addWidget(op5);
  opl->addLayout(devlayout);

  QGroupBox realimagbox("Type of values",&predialog);
  realimagbox.setLayout(ropl);
  QGroupBox optionbox("Options",&predialog);
  optionbox.setLayout(opl);

  QHBoxLayout layoutButtons;
  QPushButton ok("Ok");
  QPushButton cancel("Cancel");
  predialog.connect(&ok, SIGNAL(clicked()), &predialog, SLOT(accept()));
  predialog.connect(&cancel, SIGNAL(clicked()), &predialog, SLOT(reject()));
  layoutButtons.addWidget( &ok );
  layoutButtons.addWidget( &cancel );
  predialogl.addWidget(&realimagbox);
  predialogl.addWidget(&optionbox);
  predialogl.addLayout(&layoutButtons);

  QVector<bool> dialogoptions(6,false);
  if (predialog.exec()){
    if (rop1->isChecked())
      dialogoptions[0]=true;

    if (op0->isChecked())
      dialogoptions[1]=true;
    if (op1->isChecked())
      dialogoptions[2]=true;
    if (op2->isChecked())
      dialogoptions[3]=true;
    if (op4->isChecked())
      dialogoptions[4]=true;
    if (op5->isChecked())
      dialogoptions[5]=true;
    _displayCalibrationData(dialogoptions,dev->value());

  }

}

void CalibrationEditor::setOptionsSlot(){

  QDialog dialog;
  dialog.setWindowTitle("Debuging settings");
  dialog.setMinimumSize(400,200);
  QGridLayout dialoglayout;
  dialog.setLayout(&dialoglayout);
  QLabel voltagelabel(QString("Choose one of these for diplaying the measured voltage for the specific resistor value test"));
  QLabel label1(QString("Voltage of %0 kOhm ramp: ").arg(Potentiometer::r_from_tap(_resCode[0])/1000,0,'f',2));
  QCheckBox check1;
  QLabel label2(QString("Voltage of %0 kOhm ramp: ").arg(Potentiometer::r_from_tap(_resCode[1])/1000,0,'f',2));
  QCheckBox check2;
  QLabel label3(QString("Voltage of %0 kOhm ramp: ").arg(Potentiometer::r_from_tap(_resCode[2])/1000,0,'f',2));
  QCheckBox check3;
  QLabel resistorlabel(QString("Choose one of these for diplaying the measured resistor values for the specific resistor value"));
  QLabel label4(QString("%0 kOhm res: ").arg(Potentiometer::r_from_tap(_resCode[0])/1000,0,'f',2));
  QCheckBox check4;
  QLabel label5(QString("%0 kOhm res: ").arg(Potentiometer::r_from_tap(_resCode[1])/1000,0,'f',2));
  QCheckBox check5;
  QLabel label6(QString("%0 kOhm res: ").arg(Potentiometer::r_from_tap(_resCode[2])/1000,0,'f',2));
  QCheckBox check6;
  QLabel label7(QString("Chose if you want all the nodes in one graph: "));
  QCheckBox check7;
  QLabel label8("Rerun conversion res test, to check accuracy of calibration: ");
  QCheckBox check8;
  QLabel label8s("Overide convertion r/i tap value(-1=default): ");
  QSpinBox spin8;
  spin8.setMinimum(-1);
  spin8.setValue(-1);
  spin8.setMaximum(256);
  spin8.setMaximumWidth(100);
  QLabel label9("Overide ADC offset: ");
  QCheckBox check9;
  QHBoxLayout layoutButtons;
  QPushButton ok("Ok");
  QPushButton cancel("Cancel");
  dialog.connect(&ok, SIGNAL(clicked()), &dialog, SLOT(accept()));
  dialog.connect(&cancel, SIGNAL(clicked()), &dialog, SLOT(reject()));
  layoutButtons.addWidget( &ok );
  layoutButtons.addWidget( &cancel );
  dialoglayout.addWidget(&voltagelabel,0,0,0,2);
  dialoglayout.addWidget(&label1,1,0);
  dialoglayout.addWidget(&label2,2,0);
  dialoglayout.addWidget(&label3,3,0);
  //----------------
  dialoglayout.addWidget(&resistorlabel,4,0,0,2);
  dialoglayout.addWidget(&label4,5,0);
  dialoglayout.addWidget(&label5,6,0);
  dialoglayout.addWidget(&label6,7,0);
  //----------------
  dialoglayout.addWidget(&label7,8,0);
  //----------------
  dialoglayout.addWidget(&label8,10,0);
  dialoglayout.addWidget(&label8s,11,0);
  //----------------
  dialoglayout.addWidget(&label9,13,0);
  //----------------
  dialoglayout.addWidget(&check1,1,1);
  dialoglayout.addWidget(&check2,2,1);
  dialoglayout.addWidget(&check3,3,1);
  //----------------
  dialoglayout.addWidget(&check4,5,1);
  dialoglayout.addWidget(&check5,6,1);
  dialoglayout.addWidget(&check6,7,1);
  //----------------
  dialoglayout.addWidget(&check7,8,1);
  //----------------
  dialoglayout.addWidget(&check8,10,1);
  dialoglayout.addWidget(&spin8,11,1);
  //----------------
  dialoglayout.addWidget(&check9,13,1);
  //----------------
  dialoglayout.addLayout(&layoutButtons,14,0,1,2);
  //6=default
  _options[0]='6';
  _options[1]='6';
  _options[2]='6';
  _options[3]='6';
  _options[4]='6';
  _options[5]='6';
  if( dialog.exec() ){
    if (check1.isChecked()){
      cout<<QString("Curve voltage of %0 kOhm ramp: ").arg(Potentiometer::r_from_tap(_resCode[0])/1000,0,'f',2).toStdString()<<endl;
      _options[0]='0';}
    if (check2.isChecked()){
      cout<<QString("Curve voltage of %0 kOhm ramp: ").arg(Potentiometer::r_from_tap(_resCode[1])/1000,0,'f',2).toStdString()<<endl;
      _options[0]='1';}
    if (check3.isChecked()){
      cout<<QString("Curve voltage of %0 kOhm ramp: ").arg(Potentiometer::r_from_tap(_resCode[2])/1000,0,'f',2).toStdString()<<endl;
      _options[0]='2';}
    if (check4.isChecked()){
      cout<<QString("Curve %0 kOhm res: ").arg(Potentiometer::r_from_tap(_resCode[0])/1000,0,'f',2).toStdString()<<endl;
      _options[1]='0';}
    if (check5.isChecked()){
      cout<<QString("Curve %0 kOhm res: ").arg(Potentiometer::r_from_tap(_resCode[1])/1000,0,'f',2).toStdString()<<endl;
      _options[1]='1';}
    if (check6.isChecked()){
      cout<<QString("Curve %0 kOhm res: ").arg(Potentiometer::r_from_tap(_resCode[2])/1000,0,'f',2).toStdString()<<endl;
      _options[1]='2';}
    if (check7.isChecked()){
      cout<<QString("Curve all the nodes: ").toStdString()<<endl;
      _options[2]='0';}
    if (check8.isChecked()){
      cout<<"Rerunning conversion res test "<<endl;
      _options[3]='0';}
    if (spin8.value()!=-1){
      cout<<"Overinding tap value for convertion resistor in the grid resistors test"<<endl;
      _options[5]='0';
      _conversionResCodeReal=spin8.value();
      _conversionResCodeImag=spin8.value();
      cout<<"The new code is: "<<_conversionResCodeReal<<endl;
    }
    if (check9.isChecked()){
      cout<<"Overiding ADC offset: "<<endl;
      _options[4]='0';}


  }
}

void CalibrationEditor::checkCellSlot(){
  for (size_t devId=0;devId<_emu->getUSBDevicesCount();++devId){
    cout<<endl<<"Cell check of device "<<devId<<endl;
    //Reinitialize
    QVector <QVector<QString> > calibrationnamedatanew = _master_store.at(devId)->nameDataNew;
    QVector <QVector<double> > calibrationoffsetdatanew = _master_store.at(devId)->offsetDataNew;
    QVector <QVector<double> > calibrationgaindatanew = _master_store.at(devId)->gainDataNew;
    QVector <QVector<int> > calibrationidnew = _master_store.at(devId)->idNew;
    QVector <QVector<double> > calibrationrabnew = _master_store.at(devId)->rabNew;
    QVector <QVector<double> > calibrationrwnew = _master_store.at(devId)->rwNew;
    QVector <QVector <QVector<double> > > rawresultsnew = _master_store.at(devId)->rawResultsNew;
    QVector <QVector <QVector<double> > > lsqresultsnew = _master_store.at(devId)->lsqResultsNew;

    if (calibrationnamedatanew.size()==0){
      cout<<"None test was run"<<endl;
      return;
    }
    if (calibrationnamedatanew[0].size()<254){
      cout<<"Run all calibration tests first"<<endl;
      return;
    }
    cout<<endl;
    cout<<"~~~~~~Running test to find fully working cell~~~~~~"<<endl<<endl;
    QVector<int> realfunctionalcells;
    QVector<int> imagfunctionalcells;
    for (int cell=0;cell<24;++cell){//It doesnt test the EXT res
      if(calibrationrabnew[0][cell]<11000&&calibrationrabnew[0][cell]>9000&&!std::isnan(calibrationrabnew[0][cell]))
        if(calibrationrabnew[0][cell+24]<11000&&calibrationrabnew[0][cell+24]>9000&&!std::isnan(calibrationrabnew[0][cell+24]))
          if(calibrationrabnew[0][cell+48]<11000&&calibrationrabnew[0][cell+48]>9000&&!std::isnan(calibrationrabnew[0][cell+48]))
            if(calibrationrabnew[0][cell+72]<11000&&calibrationrabnew[0][cell+72]>9000&&!std::isnan(calibrationrabnew[0][cell+72]))
              if(calibrationrabnew[0][cell+96]<11000&&calibrationrabnew[0][cell+96]>9000&&!std::isnan(calibrationrabnew[0][cell+96]))
                if(calibrationrabnew[0][cell+120]<11000&&calibrationrabnew[0][cell+120]>9000&&!std::isnan(calibrationrabnew[0][cell+120]))
                  if(calibrationrabnew[0][cell+144]<11000&&calibrationrabnew[0][cell+144]>9000&&!std::isnan(calibrationrabnew[0][cell+144]))
                    if(calibrationrabnew[0][cell+168]<11000&&calibrationrabnew[0][cell+168]>9000&&!std::isnan(calibrationrabnew[0][cell+168]))
                      realfunctionalcells.append(cell);

    }
    for (int cell=0;cell<24;++cell){//It doesnt test the EXT res
      if(calibrationrabnew[1][cell]<11000&&calibrationrabnew[1][cell]>9000&&!std::isnan(calibrationrabnew[1][cell]))
        if(calibrationrabnew[1][cell+24]<11000&&calibrationrabnew[1][cell+24]>9000&&!std::isnan(calibrationrabnew[1][cell+24]))
          if(calibrationrabnew[1][cell+48]<11000&&calibrationrabnew[1][cell+48]>9000&&!std::isnan(calibrationrabnew[1][cell+48]))
            if(calibrationrabnew[1][cell+72]<11000&&calibrationrabnew[1][cell+72]>9000&&!std::isnan(calibrationrabnew[1][cell+72]))
              if(calibrationrabnew[1][cell+96]<11000&&calibrationrabnew[1][cell+96]>9000&&!std::isnan(calibrationrabnew[1][cell+96]))
                if(calibrationrabnew[1][cell+120]<11000&&calibrationrabnew[1][cell+120]>9000&&!std::isnan(calibrationrabnew[1][cell+120]))
                  if(calibrationrabnew[1][cell+144]<11000&&calibrationrabnew[1][cell+144]>9000&&!std::isnan(calibrationrabnew[1][cell+144]))
                    if(calibrationrabnew[1][cell+168]<11000&&calibrationrabnew[1][cell+168]>9000&&!std::isnan(calibrationrabnew[1][cell+168]))
                      imagfunctionalcells.append(cell);

    }
    cout<<"Fully fuctioning real cells: "<<endl;
    for (int i=0;i<realfunctionalcells.size();++i)
      cout<<"  "<<realfunctionalcells[i]+1;
    cout<<endl<<endl;
    cout<<"Fully fuctioning imag cells: "<<endl;
    for (int i=0;i<imagfunctionalcells.size();++i)
      cout<<"  "<<imagfunctionalcells[i]+1;
    cout<<endl<<endl;
    //Failed tests
    //Real
    QVector<QVector<int> > realfailedcells;
    for (int i=0;i!=8;++i)//Initialize vector
      realfailedcells.append(QVector<int> ());

    for (int cell=0;cell<24;++cell){//It doesnt test the EXT res
      for(int testid=0;testid<8;testid++)
        if(calibrationrabnew[0][cell+(testid*24)]>11000||calibrationrabnew[0][cell+(testid*24)]<9000||std::isnan(calibrationrabnew[0][cell+(testid*24)]))
          realfailedcells[testid].append(cell);
    }
    //Failed tests
    //Imag
    QVector<QVector<int> > imagfailedcells;
    for (int i=0;i!=8;++i)//Initialize vector
      imagfailedcells.append(QVector<int> ());

    for (int cell=0;cell<24;++cell){//It doesnt test the EXT res
      for(int testid=0;testid<8;testid++)
        if(calibrationrabnew[1][cell+(testid*24)]>11000||calibrationrabnew[1][cell+(testid*24)]<9000||std::isnan(calibrationrabnew[1][cell+(testid*24)]))
          imagfailedcells[testid].append(cell);
    }

    cout<<"Cells failing Conversion resistor real test: "<<endl;
    for (int i=0;i<realfailedcells[0].size();++i)
      cout<<"  "<<realfailedcells[0][i]+1;
    cout<<endl;

    cout<<"Cells failing Conversion resistor imag test: "<<endl;
    for (int i=0;i<imagfailedcells[0].size();++i)
      cout<<"  "<<imagfailedcells[0][i]+1;
    cout<<endl<<endl;

    cout<<"Cells failing Internal resistor real test: "<<endl;
    for (int i=0;i<realfailedcells[1].size();++i)
      cout<<"  "<<realfailedcells[1][i]+1;
    cout<<endl;

    cout<<"Cells failing Internal resistor imag test: "<<endl;
    for (int i=0;i<imagfailedcells[1].size();++i)
      cout<<"  "<<imagfailedcells[1][i]+1;
    cout<<endl<<endl;

    cout<<"Cells failing P0Chip1-2 resistor real test: "<<endl;
    for (int i=0;i<realfailedcells[2].size();++i)
      cout<<"  "<<realfailedcells[2][i]+1;
    cout<<endl;

    cout<<"Cells failing P0Chip1-2 resistor imag test: "<<endl;
    for (int i=0;i<imagfailedcells[2].size();++i)
      cout<<"  "<<imagfailedcells[2][i]+1;
    cout<<endl<<endl;



    cout<<"Cells failing P1Chip1-2 resistor real test: "<<endl;
    for (int i=0;i<realfailedcells[3].size();++i)
      cout<<"  "<<realfailedcells[3][i]+1;
    cout<<endl;

    cout<<"Cells failing P1Chip1-2 resistor imag test: "<<endl;
    for (int i=0;i<imagfailedcells[3].size();++i)
      cout<<"  "<<imagfailedcells[3][i]+1;
    cout<<endl<<endl;

    cout<<"Cells failing P2Chip1-2 resistor real test: "<<endl;
    for (int i=0;i<realfailedcells[4].size();++i)
      cout<<"  "<<realfailedcells[4][i]+1;
    cout<<endl;

    cout<<"Cells failing P2Chip1-2 resistor imag test: "<<endl;
    for (int i=0;i<imagfailedcells[4].size();++i)
      cout<<"  "<<imagfailedcells[4][i]+1;
    cout<<endl<<endl;

    cout<<"Cells failing P3Chip1-2 resistor real test: "<<endl;
    for (int i=0;i<realfailedcells[5].size();++i)
      cout<<"  "<<realfailedcells[5][i]+1;
    cout<<endl;

    cout<<"Cells failing P3Chip1-2 resistor imag test: "<<endl;
    for (int i=0;i<imagfailedcells[5].size();++i)
      cout<<"  "<<imagfailedcells[5][i]+1;
    cout<<endl<<endl;

    cout<<"Cells failing P0P2Chip3 resistor real test: "<<endl;
    for (int i=0;i<realfailedcells[6].size();++i)
      cout<<"  "<<realfailedcells[6][i]+1;
    cout<<endl;

    cout<<"Cells failing P0P2Chip3 resistor imag test: "<<endl;
    for (int i=0;i<imagfailedcells[6].size();++i)
      cout<<"  "<<imagfailedcells[6][i]+1;
    cout<<endl<<endl;

    cout<<"Cells failing P1P3Chip3 resistor real test: "<<endl;
    for (int i=0;i<realfailedcells[7].size();++i)
      cout<<"  "<<realfailedcells[7][i]+1;
    cout<<endl;

    cout<<"Cells failing P1P3Chip3 resistor imag test: "<<endl;
    for (int i=0;i<imagfailedcells[7].size();++i)
      cout<<"  "<<imagfailedcells[7][i]+1;
    cout<<endl<<endl;
  }

}


void CalibrationEditor::potTestSlot(){
  for (size_t devId=0;devId<_emu->getUSBDevicesCount();++devId){
    cout<<endl<<"Potentiometer test of dev "<<devId<<endl;
    //Reinitialize
    QVector <QVector<QString> > calibrationnamedatanew = _master_store.at(devId)->nameDataNew;
    QVector <QVector<double> > calibrationrabnew = _master_store.at(devId)->rabNew;
    QVector <QVector<double> > calibrationrwnew = _master_store.at(devId)->rwNew;
    if (calibrationnamedatanew.size()==0){
      cout<<"None test was run"<<endl;
      return;
    }
    cout<<"New calibration"<<endl;
    cout<<"----Real----"<<endl;
    for (int i=0;i<calibrationrabnew[0].size();i++){
      cout << calibrationnamedatanew[0][i+48].toStdString()
          << " min value(ohm): "<<calibrationrwnew[0][i]
             << " max value(kohm): "<<calibrationrabnew[0][i]/1000<<endl;
    }
    cout<<endl;
    cout<<endl;
    cout<<"----Imag----"<<endl;
    for (int i=0;i<calibrationrabnew[0].size();i++){
      cout << calibrationnamedatanew[1][i+48].toStdString()
          << " min value(ohm): "<<calibrationrwnew[1][i]
             << " max value(kohm): "<<calibrationrabnew[1][i]/1000<<endl;
    }
  }
}

void CalibrationEditor::potTestErrorSlot(){
  for (size_t devId=0;devId<_emu->getUSBDevicesCount();++devId){
    cout<<endl<<"Potentiometer error of dev "<<devId<<endl;
    //Reinitialize
    QVector <QVector<QString> > calibrationnamedatanew = _master_store.at(devId)->nameDataNew;
    QVector <QVector<double> > calibrationrabnew = _master_store.at(devId)->rabNew;
    QVector <QVector<double> > calibrationrwnew = _master_store.at(devId)->rwNew;
    if (calibrationnamedatanew.size()==0){
      cout<<"None test was run"<<endl;
      return;
    }
    //Calculate the relative error
    //Relative error at rw
    double tempres;
    double sumtempres=0;
    for (int k=0;k!=calibrationrwnew[0].size();++k){
      //Real
      cout<<calibrationnamedatanew[0][k+48].toStdString()<<" relative error at rw: ";
      tempres=(calibrationrwnew[0][k]-Potentiometer::r_from_tap(0))/calibrationrwnew[0][k]*100;
      sumtempres=sumtempres+abs(tempres);
      cout<<tempres<<"%"<<endl;
    }
    sumtempres=sumtempres/calibrationrwnew[0].size();
    cout<<endl;
    cout<<"The avarage relative error of real Rwiper is: "<<sumtempres<<"%"<<endl;
    cout<<endl<<endl;

    sumtempres=0;
    for (int k=0;k!=calibrationrwnew[1].size();++k){
      //imag
      cout<<calibrationnamedatanew[1][k+48].toStdString()<<" relative error at rw: ";
      tempres=(calibrationrwnew[1][k]-Potentiometer::r_from_tap(0))/calibrationrwnew[1][k]*100;
      sumtempres=sumtempres+abs(tempres);
      cout<<tempres<<"%"<<endl;
    }
    sumtempres=sumtempres/calibrationrwnew[1].size();
    cout<<endl;
    cout<<"The avarage relative error of imag Rwiper is: "<<sumtempres<<"%"<<endl;
    cout<<endl<<endl<<endl;

    //Relative error at rab
    sumtempres=0;
    for (int k=0;k!=calibrationrabnew[0].size();++k){
      //Real
      cout<<calibrationnamedatanew[0][k+48].toStdString()<<" relative error at rab: ";
      tempres=(calibrationrabnew[0][k]-Potentiometer::r_from_tap(256))/calibrationrabnew[0][k]*100;
      sumtempres=sumtempres+abs(tempres);
      cout<<tempres<<"%"<<endl;
    }
    sumtempres=sumtempres/calibrationrabnew[0].size();
    cout<<endl;
    cout<<"The avarage relative error of real Rab is: "<<sumtempres<<"%"<<endl;
    cout<<endl<<endl;

    sumtempres=0;
    for (int k=0;k!=calibrationrabnew[1].size();++k){
      //imag
      cout<<calibrationnamedatanew[1][k+48].toStdString()<<" relative error at rab: ";
      tempres=(calibrationrabnew[1][k]-Potentiometer::r_from_tap(256))/calibrationrabnew[1][k]*100;
      sumtempres=sumtempres+abs(tempres);
      cout<<tempres<<"%"<<endl;
    }
    sumtempres=sumtempres/calibrationrabnew[1].size();
    cout<<endl;
    cout<<"The avarage relative error of imag Rab is: "<<sumtempres<<"%"<<endl;
  }
}

void CalibrationEditor::calibrationExportSlot(){
  // Define the file name
  QDir currentdir;
  currentdir.cdUp();
  QString filename = QDir::toNativeSeparators(currentdir.path()).append("\\calibrationfile.xml");
  int ans = exportFile(filename);
  if (!ans)
    cout << "Exported successfully" << endl;
  else
    cout << "Export failed with code " << ans << endl;
}

void CalibrationEditor::calibrationImportSlot(){
  // Define the file name
  QDir currentdir;
  currentdir.cdUp();
  QString filename = QDir::toNativeSeparators(currentdir.path()).append("\\calibrationfile.xml");
  int ans = importFile(filename);

  if (!ans){
    cout << endl;
    cout << "Import finished for the corresponding devices" <<endl;
    cout << "Press the set calibration button to set the values in emulator" << endl;
  } else {
    cout << "Import failed with code " << ans << endl;
  }
}


int CalibrationEditor::_ADCOffsetConverterCalibration(int devId){
  vector<uint32_t> tempvector;
  //Reset Prior the test
  tempvector.clear();
  tempvector.push_back(2222);
  _emu->usbWrite( devId, 286,tempvector);

  //Setting the configuration file of calibration
  _confVector.clear();
  _confVector.append(static_cast<uint32_t>(0));
  _confVector.insert(_confVector.size(),3,static_cast<uint32_t>(0));
  _confVector.append(static_cast<uint32_t>(786432));
  _confVector.insert(_confVector.size(),8,static_cast<uint32_t>(0));
  _confVector.append(static_cast<uint32_t>(206703681));
  _confVector.append(static_cast<uint32_t>(414524679));
  _confVector.append(static_cast<uint32_t>(622345677));
  _confVector.append(static_cast<uint32_t>(4051392147u));
  _confVector.append(static_cast<uint32_t>(1));
  _confVector.insert(_confVector.size(),2,static_cast<uint32_t>(0));
  _confVector.append(static_cast<uint32_t>(1));
  _confVector.insert(_confVector.size(),11,static_cast<uint32_t>(0));
  _confVector.append(static_cast<uint32_t>(3221225472u));
  _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(References_Real->at(devId)->value()));
  _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(References_Imag->at(devId)->value()));
  _confVector.insert(_confVector.size(),23,static_cast<uint32_t>(696969));
  _confVector.append(static_cast<uint32_t>(3229616128u));
  _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(3221225772u));
  _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(3221225472u+interval->value()));
  _confVector.insert(_confVector.size(),104,static_cast<uint32_t>(696969));
  _confVector.insert(_confVector.size(),52,static_cast<uint32_t>(2290649224u));
  //Switches
  _confVector.insert(_confVector.size(),12,static_cast<uint32_t>(4009750271u));
  _confVector.append(static_cast<uint32_t>(4294967295u));
  _confVector.insert(_confVector.size(),3,static_cast<uint32_t>(0));
  //Nios code
  _confVector.append(static_cast<uint32_t>(3221225472u));
  //Initial gian offset...dont care here
  _confVector.insert(_confVector.size(),24,static_cast<uint32_t>(10487040));
  _confVector.insert(_confVector.size(),24,static_cast<uint32_t>(8390656));
  //Steps
  _confVector.append(static_cast<uint32_t>(15));
  //First and second code...dont care
  _confVector.append(static_cast<uint32_t>(170));
  _confVector.append(static_cast<uint32_t>(858));
  //Nios code  +3 for ADCoffset test
  _confVector.append(static_cast<uint32_t>(3221225475u));
  _confVector.append(static_cast<uint32_t>(0));
  _confVector.append(static_cast<uint32_t>(0));
  _confVector.append(static_cast<uint32_t>(100));
  _confVector.append(static_cast<uint32_t>(1));

  tempvector=_confVector.toStdVector();
  //Write Calibration Configuration to FPGA
  _emu->usbWrite( devId, 0,tempvector);
  vector<uint32_t> data;
  //Setting the start of the calibration
  tempvector.clear();
  tempvector.push_back(1111);
  _emu->usbWrite( devId, 286,tempvector);

  //Read the configuration details for confirmation
  long int watchdog_timer=0;
  while (1){
    ++watchdog_timer;
    _emu->usbRead( devId, 287,1,data);
    if (data.at(0)==4444)
      break;
    if (watchdog_timer>30)
      return 1;//Return 1 if the board doesnt not respont to commands of calibration
  }



  //Read the results from FPGA memory,first and last possition is empty
  _emu->usbRead( devId, 290,385,data);


  //Initialize the decoding vectors
  QVector<QVector<double> > decodedresultsreal;
  QVector<QVector<double> > decodedresultsimag;
  for (int i=0;i<24;++i){
    decodedresultsreal.append(QVector<double>());
    decodedresultsimag.append(QVector<double>());
  }
  //Take groups of 15 uint32_t values(24 nodes) and convert them to double
  QVector<uint32_t> tempnodedata;
  QVector<uint32_t> tempnodesdata = QVector<uint32_t>::fromStdVector(data);
  for (int i=0;i<24;++i){
    tempnodedata.clear();
    tempnodedata=tempnodesdata.mid(1+i*16,15);
    _resultsHandling(tempnodedata,i,&decodedresultsreal,&decodedresultsimag,15);
  }

  double meanreal;
  double meanimag;
  for (int i=0;i<24;++i){
    //Store the raw results before calculating gain/offset
    _rawResultsNew[0].append(decodedresultsreal[i]);
    _rawResultsNew[1].append(decodedresultsimag[i]);

    meanreal=0;
    meanimag=0;
    //Real and imag part
    for (int k=0;k<15;++k){
      meanreal=meanreal+decodedresultsreal[i][k];
      meanimag=meanimag+decodedresultsimag[i][k];
    }
    meanreal=meanreal/15;
    meanimag=meanimag/15;
    //Now we have the mean value for both the real and imag
    //Store first the real part
    _calibrationNameDataNew[0].push_back(QString("ADC offset real of node %0").arg(i+1));
    _calibrationIdNew[0].push_back(11);

    double offsetvalue;
    offsetvalue=2.5-meanreal;


    _calibrationOffsetDataNew[0].push_back(offsetvalue);
    _calibrationGainDataNew[0].push_back(0);


    //Then the imag part
    _calibrationNameDataNew[1].push_back(QString("ADC offset imag of node %0").arg(i+1));
    _calibrationIdNew[1].push_back(11);

    offsetvalue=2.5-meanimag;

    _calibrationOffsetDataNew[1].push_back(offsetvalue);
    _calibrationGainDataNew[1].push_back(0);
  }
  return 0; //Succeed
}

int CalibrationEditor::_convertersCalibration(int devId){
  vector<uint32_t> tempvector;
  //Reset Prior the test
  tempvector.clear();
  tempvector.push_back(2222);
  _emu->usbWrite( devId, 286,tempvector);

  //Setting the configuration file of calibration
  _confVector.clear();
  _confVector.append(static_cast<uint32_t>(0));
  _confVector.insert(_confVector.size(),3,static_cast<uint32_t>(0));
  _confVector.append(static_cast<uint32_t>(786432));
  _confVector.insert(_confVector.size(),8,static_cast<uint32_t>(0));
  _confVector.append(static_cast<uint32_t>(206703681));
  _confVector.append(static_cast<uint32_t>(414524679));
  _confVector.append(static_cast<uint32_t>(622345677));
  _confVector.append(static_cast<uint32_t>(4051392147u));
  _confVector.append(static_cast<uint32_t>(1));
  _confVector.insert(_confVector.size(),2,static_cast<uint32_t>(0));
  _confVector.append(static_cast<uint32_t>(1));
  _confVector.insert(_confVector.size(),11,static_cast<uint32_t>(0));
  _confVector.append(static_cast<uint32_t>(3221225472u));
  _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(References_Real->at(devId)->value()));
  _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(References_Imag->at(devId)->value()));
  _confVector.insert(_confVector.size(),23,static_cast<uint32_t>(696969));
  _confVector.append(static_cast<uint32_t>(3229616128u));
  _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(3221225772u));
  _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(3221225472u+interval->value()));
  _confVector.insert(_confVector.size(),104,static_cast<uint32_t>(696969));
  _confVector.insert(_confVector.size(),52,static_cast<uint32_t>(2290649224u));
  //Switches
  _confVector.insert(_confVector.size(),12,static_cast<uint32_t>(2583665151u));
  _confVector.append(static_cast<uint32_t>(4294967295u));
  _confVector.insert(_confVector.size(),3,static_cast<uint32_t>(0));
  //Nios code
  _confVector.append(static_cast<uint32_t>(3221225472u));
  _confVector.insert(_confVector.size(),24,static_cast<uint32_t>(10487040));
  //_confVector.insert(_confVector.size(),24,static_cast<uint32_t>(8390656));
  //I am using ADC test's offsets
  QVector<uint32_t> encodedinput;
  _offsetGainHandling(&encodedinput,3);//insert ADC offset, 3 for ADC offset from store
  for(int i=0;i<24;++i){
    _confVector.append(encodedinput[i]);
  }
  _confVector.append(static_cast<uint32_t>(15));
  _confVector.append(static_cast<uint32_t>(170));
  _confVector.append(static_cast<uint32_t>(858));
  _confVector.append(static_cast<uint32_t>(3221225472u));
  _confVector.append(static_cast<uint32_t>(0));
  _confVector.append(static_cast<uint32_t>(0));
  _confVector.append(static_cast<uint32_t>(100));
  _confVector.append(static_cast<uint32_t>(1));

  tempvector=_confVector.toStdVector();
  //Write Calibration Configuration to FPGA
  _emu->usbWrite( devId, 0,tempvector);
  vector<uint32_t> data;
  //Setting the start of the calibration
  tempvector.clear();
  tempvector.push_back(1111);
  _emu->usbWrite( devId, 286,tempvector);

  //Read the configuration details for confirmation
  long int watchdog_timer=0;
  while (1){
    ++watchdog_timer;
    _emu->usbRead( devId, 287,1,data);
    if (data.at(0)==4444)
      break;
    if (watchdog_timer>30)
      return 1;//Return 1 if the board doesnt not respont to commands of calibration
  }



  //Read the results from FPGA memory,first and last possition is empty
  _emu->usbRead( devId, 290,385,data);


  //Initialize the decoding vectors
  QVector<QVector<double> > decodedresultsreal;
  QVector<QVector<double> > decodedresultsimag;
  for (int i=0;i<24;++i){
    decodedresultsreal.append(QVector<double>());
    decodedresultsimag.append(QVector<double>());
  }
  //Take groups of 15 uint32_t values(24 nodes) and convert them to double
  QVector<uint32_t> tempnodedata;
  QVector<uint32_t> tempnodesdata = QVector<uint32_t>::fromStdVector(data);
  for (int i=0;i<24;++i){
    tempnodedata.clear();
    tempnodedata=tempnodesdata.mid(1+i*16,15);
    _resultsHandling(tempnodedata,i,&decodedresultsreal,&decodedresultsimag,15);
  }
  //Expected data vector construction
  QVector<double> normdata;
  QVector<double> normxdata;
  double tempvoltage;
  //Parameters
  double startvoltage;
  double voltagestep;
  startvoltage=(double)858/4096*5;
  voltagestep=(double)170/4096*5;
  for (int i=0;i<15;++i)
    normxdata.append(i);
  for (int i=0;i<15;++i){
    tempvoltage=startvoltage+i*voltagestep;
    normdata.append(tempvoltage);
  }

  //For each of 24 groups calculate the least squares of the 15 values
  //and the offset/gain compare to Expected data
  double alpha;
  double beta;
  //X axis values, for vramp is 1 ..to.. 15 int
  QVector<double> xdata;
  for (int i=0;i<15;++i)
    xdata.push_back(i);
  QVector<double> lsqdata;
  QVector<double> lsqxdata;
  for (int i=0;i<24;++i){
    //Store the raw results before calculating gain/offset

    _rawResultsNew[0].append(decodedresultsreal[i]);
    _rawResultsNew[1].append(decodedresultsimag[i]);

    //First for the real part
    alpha=0.0;

    beta=0.0;
    lsqxdata.clear();
    lsqdata.clear();
    _leastSquares(decodedresultsreal[i],xdata,&alpha,&beta);
    for (int k=0;k<15;++k)
      lsqxdata.push_back(k);
    for (int k=0;k<15;++k){
      lsqdata.push_back(alpha*lsqxdata.at(k)+beta);
    }
    //At this point we have both the Expected and the least squares line,
    //so we can calculate the gain an offset
    _calibrationNameDataNew[0].push_back(QString("DAC/ADC real of node %0").arg(i+1));
    _calibrationIdNew[0].push_back(10);
    double offsetvalue;
    double gainvalue;
    offsetvalue=normdata[7]-lsqdata[7];
    gainvalue=(normdata[7]-normdata[8]) / (lsqdata[7]-lsqdata[8]);
    _calibrationOffsetDataNew[0].push_back(offsetvalue);
    _calibrationGainDataNew[0].push_back(gainvalue);
    _lsqResultsNew[0].push_back(lsqdata);
    //Then for the imag part
    alpha=0.0;
    beta=0.0;
    lsqxdata.clear();
    lsqdata.clear();
    _leastSquares(decodedresultsimag[i],xdata,&alpha,&beta);
    for (int k=0;k<15;++k)
      lsqxdata.push_back(k);
    for (int k=0;k<15;++k)
      lsqdata.push_back(alpha*lsqxdata.at(k)+beta);
    //At this point we have both the Expected and the least squares line,
    //so we can calculate the gain an offset
    _calibrationNameDataNew[1].push_back(QString("DAC/ADC imag of node %0").arg(i+1));
    _calibrationIdNew[1].push_back(10);
    offsetvalue=normdata[7]-lsqdata[7];
    gainvalue=(normdata[7]-normdata[8]) / (lsqdata[7]-lsqdata[8]);

    _calibrationOffsetDataNew[1].push_back(offsetvalue);
    _calibrationGainDataNew[1].push_back(gainvalue);
    _lsqResultsNew[1].push_back(lsqdata);


  }

  return 0; //Succeed
}


int CalibrationEditor::_conversionResistorCalibrationNew(int devId){
  vector<uint32_t> tempvector;
  //Reset Prior the test
  tempvector.clear();
  tempvector.push_back(2222);
  _emu->usbWrite( devId, 286,tempvector);
  _confVector.clear();
  _confVector.append(static_cast<uint32_t>(0));
  _confVector.insert(_confVector.size(),3,static_cast<uint32_t>(0));
  _confVector.append(static_cast<uint32_t>(786432));
  _confVector.insert(_confVector.size(),8,static_cast<uint32_t>(0));
  _confVector.append(static_cast<uint32_t>(206703681));
  _confVector.append(static_cast<uint32_t>(414524679));
  _confVector.append(static_cast<uint32_t>(622345677));
  _confVector.append(static_cast<uint32_t>(4051392147u));
  _confVector.append(static_cast<uint32_t>(1));
  _confVector.insert(_confVector.size(),2,static_cast<uint32_t>(0));
  _confVector.append(static_cast<uint32_t>(1));
  _confVector.insert(_confVector.size(),11,static_cast<uint32_t>(0));
  _confVector.append(static_cast<uint32_t>(3221225472u));
  _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(References_Real->at(devId)->value()));
  _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(References_Imag->at(devId)->value()));
  _confVector.insert(_confVector.size(),23,static_cast<uint32_t>(696969));
  _confVector.append(static_cast<uint32_t>(3229616128u));
  _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(3221225772u));
  _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(3221225472u+interval->value()));
  ///Below the POT values
  //Resistor values
  int tempresP3;
  int tempresP1;
  for (int i=0;i<24;++i){
    _confVector.insert(_confVector.size(),3,static_cast<uint32_t>(100<<24|100<<16|100<<8|100));
    if (_options[3]=='6')
      _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(_resCode[1]<<24|0<<16|_resCode[1]<<8|0));//default code 126
    else{//In this it will enter only on debug mode
      if (_options[3]=='0'){//First run with the default test
        _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(_resCode[1]<<24|0<<16|_resCode[1]<<8|0));//default code 126
      }
      else if (_options[3]=='1'){//Second run with the previous corrected values
        tempresP3=Potentiometer::tap_from_r(Potentiometer::r_from_tap(_resCode[1]),_calibrationRabNew[0][i],_calibrationRwNew[0][i]);
        tempresP1=Potentiometer::tap_from_r(Potentiometer::r_from_tap(_resCode[1]),_calibrationRabNew[1][i],_calibrationRwNew[1][i]);
        _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(tempresP3<<24|0<<16|tempresP1<<8|0));//corrected code
      }
    }
  }
  _confVector.insert(_confVector.size(),8,static_cast<uint32_t>(696969));
  //TCON values
  for(int i=0;i<24;++i){
    _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(2290649224u));
    _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(3099101320u));//4177037448u for switch of pot closed
  }
  _confVector.insert(_confVector.size(),4,static_cast<uint32_t>(2290649224u));
  //Switches values
  _confVector.insert(_confVector.size(),12,static_cast<uint32_t>(4009750271u));
  _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(4294967295u));
  //Other values
  _confVector.insert(_confVector.size(),3,static_cast<uint32_t>(0));
  _confVector.append(static_cast<uint32_t>(3221225472u));
  //Corrected Gain-offset
  QVector<uint32_t> encodedinput;
  _offsetGainHandling(&encodedinput,0);//insert adc/dac gain/offset, 0 for gain
  for(int i=0;i<24;++i){
    _confVector.append(encodedinput[i]);
  }
  _offsetGainHandling(&encodedinput,1);//insert adc/dac gain/offset, 1 for offset
  for(int i=0;i<24;++i){
    _confVector.append(encodedinput[i]);
  }
  if(autocheckopt->isChecked()){//Running the default new hardcoded version
    _confVector.append(static_cast<uint32_t>(20));
    _confVector.append(static_cast<uint32_t>(0));
    _confVector.append(static_cast<uint32_t>(0));
    _confVector.append(static_cast<uint32_t>(3221225475u));//NIOS + 3 for default options
  }
  else if(!autocheckopt->isChecked()){ //new ,user inputed parameters
    _confVector.append(static_cast<uint32_t>(num_of_values_combobox->value()));
    _confVector.append(static_cast<uint32_t>(second_value_combobox->value()));
    _confVector.append(static_cast<uint32_t>(first_value_combobox->value()));
    _confVector.append(static_cast<uint32_t>(3221225474u));//NIOS for accepting user defined ramp
  }
  _confVector.append(static_cast<uint32_t>(0));
  _confVector.append(static_cast<uint32_t>(0));
  _confVector.append(static_cast<uint32_t>(101));
  _confVector.append(static_cast<uint32_t>(1));

  tempvector=_confVector.toStdVector();
  //Write Calibration Configuration to FPGA
  _emu->usbWrite( devId, 0,tempvector);
  vector<uint32_t> data;
  //Setting the start of the calibration
  tempvector.clear();
  tempvector.push_back(1111);
  _emu->usbWrite( devId, 286,tempvector);
  long int watchdog_timer=0;
  while (1){
    ++watchdog_timer;
    _emu->usbRead( devId, 287,1,data);
    if (data.at(0)==4444)
      break;
    if (watchdog_timer>30)
      return 1;//Return 1 if the board doesnt not respont to commands of calibration
  }


  //Read the results from FPGA memory,first and last possition is empty
  vector<uint32_t> data1, data2, data3, data4;
  _emu->usbRead( devId,  290 , 400 , data1 );
  _emu->usbRead( devId,  690 , 400 , data2 );
  _emu->usbRead( devId, 1090 , 400 , data3 );
  _emu->usbRead( devId, 1490 ,  312 , data4 );
  data.clear();
  data.insert(data.end(), data1.begin(), data1.end());
  data.insert(data.end(), data2.begin(), data2.end());
  data.insert(data.end(), data3.begin(), data3.end());
  data.insert(data.end(), data4.begin(), data4.end());

  //Parameters
  QVector<double> firstvoltage;
  QVector<double> secondvoltage;

  if (autocheckopt->isChecked()){

    firstvoltage.append((double)_DACCodeFirst[0]/4096*5);
    secondvoltage.append((double)_DACCodeSecond[0]/4096*5);


    firstvoltage.append((double)_DACCodeFirst[1]/4096*5);
    secondvoltage.append((double)_DACCodeSecond[1]/4096*5);



    firstvoltage.append((double)_DACCodeFirst[2]/4096*5);
    secondvoltage.append((double)_DACCodeSecond[2]/4096*5);

  }
  else { //user defined values

    firstvoltage.append((double)first_value_combobox->value()/4096*5);
    secondvoltage.append((double)second_value_combobox->value()/4096*5);



    firstvoltage.append((double)first_value_combobox->value()/4096*5);
    secondvoltage.append((double)second_value_combobox->value()/4096*5);


    firstvoltage.append((double)first_value_combobox->value()/4096*5);
    secondvoltage.append((double)second_value_combobox->value()/4096*5);
  }

  //Create the ideal resistors ramp
  QVector<double> idealresistorramp;
  QVector<double> xidealresistorramp;
  xidealresistorramp.append(_resCode[0]);
  xidealresistorramp.append(_resCode[1]);
  xidealresistorramp.append(_resCode[2]);
  idealresistorramp.append(Potentiometer::r_from_tap(_resCode[0]));
  idealresistorramp.append(Potentiometer::r_from_tap(_resCode[1]));
  idealresistorramp.append(Potentiometer::r_from_tap(_resCode[2]));
  _P3ResNew.clear();
  _P1ResNew.clear();
  //Initialize the resistor vectors,2rows(real,imag)x24nodes=48inside vectors
  for (int i=0;i<24;++i){
    _P3ResNew.append(QVector<double>(3));
    _P1ResNew.append(QVector<double>(3));
  }
  QVector<uint32_t> tempnodedata;
  QVector<uint32_t> tempnodesdata = QVector<uint32_t>::fromStdVector(data);
  double alpha;
  double beta;
  //Initialize the decoding vectors
  QVector<QVector<double> > decodedresultsreal;
  QVector<QVector<double> > decodedresultsimag;

  for (int i=0;i<24;++i){
    decodedresultsreal.append(QVector<double>());
    decodedresultsimag.append(QVector<double>());
  }

  //Data handling
  for (int j=0;j<3;++j){
    //Take groups of 20 uint32_t values(24 nodes) and convert them to double
    decodedresultsreal.clear();
    decodedresultsimag.clear();
    for (int i=0;i<24;++i){
      decodedresultsreal.append(QVector<double>());
      decodedresultsimag.append(QVector<double>());
    }
    for (int i=0;i<24;++i){
      tempnodedata.clear();
      tempnodedata=tempnodesdata.mid(1+i*21+j*504,20);
      _resultsHandling(tempnodedata,i,&decodedresultsreal,&decodedresultsimag,20);

      if (_options[0]=='0'){
        if (j==0){//store only the 0.9kohm values
          _rawResultsNew[0].append(decodedresultsreal[i]);
          _rawResultsNew[1].append(decodedresultsimag[i]);
        }}
      if (_options[0]=='1'){
        if (j==1){//store only the 1kohm values
          _rawResultsNew[0].append(decodedresultsreal[i]);
          _rawResultsNew[1].append(decodedresultsimag[i]);
        }}
      if (_options[0]=='2'){
        if (j==2){//store only the 1.1kohm values
          _rawResultsNew[0].append(decodedresultsreal[i]);
          _rawResultsNew[1].append(decodedresultsimag[i]);
        }}

    }

    //Save in decodedresults the measured resistance
    //Take the mean value
    double firstmeanreal;
    double secondmeanreal;
    double firstmeanimag;
    double secondmeanimag;


    double Itemp;

    //For each of 24 groups calculate the least squares of the 20 values
    for (int i=0;i<24;++i){
      firstmeanreal=0.0;
      secondmeanreal=0.0;
      firstmeanimag=0.0;
      secondmeanimag=0.0;
      //First value
      for (int k=0;k<10;++k){
        //real
        Itemp=(decodedresultsreal.at(i).at(k)-2.5)/CONSTRES;
        decodedresultsreal[i][k]=(firstvoltage[j]-2.5)/Itemp;
        firstmeanreal=firstmeanreal+decodedresultsreal[i][k];
        //imag
        Itemp=(decodedresultsimag.at(i).at(k)-2.5)/CONSTRES;
        decodedresultsimag[i][k]=(firstvoltage[j]-2.5)/Itemp;
        firstmeanimag=firstmeanimag+decodedresultsimag[i][k];
      }
      firstmeanreal=firstmeanreal/10;
      firstmeanimag=firstmeanimag/10;
      //Second value
      for (int k=10;k<20;++k){
        //real
        Itemp=(decodedresultsreal.at(i).at(k)-2.5)/CONSTRES;
        decodedresultsreal[i][k]=(secondvoltage[j]-2.5)/Itemp;
        secondmeanreal=secondmeanreal+decodedresultsreal[i][k];
        //imag
        Itemp=(decodedresultsimag.at(i).at(k)-2.5)/CONSTRES;
        decodedresultsimag[i][k]=(secondvoltage[j]-2.5)/Itemp;
        secondmeanimag=secondmeanimag+decodedresultsimag[i][k];
      }
      secondmeanreal=secondmeanreal/10;
      secondmeanimag=secondmeanimag/10;


      _P3ResNew[i][j]=(firstmeanreal+secondmeanreal)/2;
      _P1ResNew[i][j]=(firstmeanimag+secondmeanimag)/2;

      if (_options[1]=='0'){
        if (j==0){//store only the 0.9kohm values
          _rawResultsNew[0].append(decodedresultsreal[i]);
          _rawResultsNew[1].append(decodedresultsimag[i]);
        }}
      if (_options[1]=='1'){
        if (j==1){//store only the 1kohm values
          _rawResultsNew[0].append(decodedresultsreal[i]);
          _rawResultsNew[1].append(decodedresultsimag[i]);
        }}
      if (_options[1]=='2'){
        if (j==2){//store only the 1.1kohm values
          _rawResultsNew[0].append(decodedresultsreal[i]);
          _rawResultsNew[1].append(decodedresultsimag[i]);
        }}

    }
  }
  if (_options[0]=='6'&&_options[1]=='6'){
    for (int i=0;i<24;++i)
    {
      _rawResultsNew[0].append(_P3ResNew[i]);
      _rawResultsNew[1].append(_P1ResNew[i]);

    }
  }


  QVector<double> xdata2;
  xdata2.push_back(_resCode[0]);
  xdata2.push_back(_resCode[1]);
  xdata2.push_back(_resCode[2]);

  QVector<double> tempresreal;
  QVector<double> tempresimag;
  QVector<double> lsqdata;
  QVector<double> lsqxdata;
  for (int i=0;i<24;++i){
    tempresreal.clear();
    tempresimag.clear();
    for (int j=0;j<3;++j){
      tempresreal.append(_P3ResNew.at(i).at(j));
      tempresimag.append(_P1ResNew.at(i).at(j));
    }
    //First for the real part
    alpha=0.0;
    beta=0.0;
    _leastSquares(tempresreal,xdata2,&alpha,&beta);
    lsqxdata.clear();
    lsqdata.clear();
    //    for (int k=23;k<30;k=k+3)
    //      lsqxdata.push_back(k);
    //    for (int k=23;k<30;k=k+3)
    //      lsqdata.push_back(alpha*k+beta);
    for (int k=0;k<257;k=k+1)
      lsqxdata.push_back(k);
    for (int k=0;k<257;k=k+1)
      lsqdata.push_back(alpha*k+beta);
    //At this point we have both the ideal and the least squares line,
    //so we can calculate the gain an offset
    if (_options[3]=='6')
      _calibrationNameDataNew[0].push_back(QString("Conversion resistor real of node %0").arg(i+1));
    else{
      if (_options[3]=='0')
        _calibrationNameDataNew[0].push_back(QString("Conversion resistor real of node %0").arg(i+1));
      else if(_options[3]=='1'){
        _calibrationNameDataNew[0].push_back(QString("Debug Conversion resistor real of node %0").arg(i+1));
      }
    }
    _calibrationIdNew[0].push_back(9);
    double offsetvalue;
    double gainvalue;
    offsetvalue=idealresistorramp[0]-lsqdata[_resCode[0]];
    gainvalue=(idealresistorramp[1]-idealresistorramp[0])/(xidealresistorramp[1]-xidealresistorramp[0]) - (lsqdata[_resCode[1]]-lsqdata[_resCode[0]])/(lsqxdata[_resCode[1]]-lsqxdata[_resCode[0]]);
    _calibrationOffsetDataNew[0].push_back(offsetvalue);
    _calibrationGainDataNew[0].push_back(gainvalue);
    _calibrationRwNew[0].push_back(beta);//0kohm equivalent
    _calibrationRabNew[0].push_back(alpha*256+beta-beta);//10kohm equivalent   //remove the measurement of rw from max value to have the rab
    _lsqResultsNew[0].push_back(lsqdata);
    //Then for the imag part
    alpha=0.0;
    beta=0.0;
    _leastSquares(tempresimag,xdata2,&alpha,&beta);
    lsqxdata.clear();
    lsqdata.clear();
    //    for (int k=23;k<30;k=k+3)
    //      lsqxdata.push_back(k);
    //    for (int k=23;k<30;k=k+3)
    //      lsqdata.push_back(alpha*k+beta);
    for (int k=0;k<257;k=k+1)
      lsqxdata.push_back(k);
    for (int k=0;k<257;k=k+1)
      lsqdata.push_back(alpha*k+beta);
    //At this point we have both the ideal and the least squares line,
    //so we can calculate the gain an offset
    if (_options[3]=='6')
      _calibrationNameDataNew[1].push_back(QString("Conversion resistor imag of node %0").arg(i+1));
    else
      if (_options[3]=='0')
        _calibrationNameDataNew[1].push_back(QString("Conversion resistor imag of node %0").arg(i+1));
      else if(_options[3]=='1'){
        _calibrationNameDataNew[1].push_back(QString("Debug Conversion resistor imag of node %0").arg(i+1));
      }
    _calibrationIdNew[1].push_back(9);
    offsetvalue=idealresistorramp[0]-lsqdata[_resCode[0]];
    gainvalue=(idealresistorramp[1]-idealresistorramp[0])/(xidealresistorramp[1]-xidealresistorramp[0]) - (lsqdata[_resCode[1]]-lsqdata[_resCode[0]])/(lsqxdata[_resCode[1]]-lsqxdata[_resCode[0]]);
    _calibrationOffsetDataNew[1].push_back(offsetvalue);
    _calibrationGainDataNew[1].push_back(gainvalue);
    _calibrationRwNew[1].push_back(beta);//0kohm equivalent
    _calibrationRabNew[1].push_back(alpha*256+beta-beta);//10kohm equivalent, we remove the rw value
    _lsqResultsNew[1].push_back(lsqdata);
  }

  bool error_found=0;
  for(int i=0;i<24;++i){
    if(_calibrationRabNew[0][i]<9000 || _calibrationRabNew[0][i]>11000 || std::isnan(_calibrationRabNew[0][i])){
      error_found=1;
      cout<<"Conversion resistor (real): "<<i+1<<" of device: "<< devId <<" exhibits a problem"<<endl;
    }
    if(_calibrationRabNew[1][i]<9000 || _calibrationRabNew[1][i]>11000 || std::isnan(_calibrationRabNew[1][i])){
      error_found=1;
      cout<<"Conversion resistor (imag): "<<i+1<<" of device: "<< devId <<" exhibits a problem"<<endl;
    }
  }

  if ( error_found ){
    return 3;//The results returned is wrong or zero
  }
  else {
    if (_options[3]=='0')//getting ready for second run of debug test
      _options[3]='1';
    else if (_options[3]=='1')//reseting the code for the next device
      _options[3]='0';
    return 0; //Succeed
  }
}

int CalibrationEditor::_gridResistorCalibrationNew(int devId, int testid ){
  vector<uint32_t> tempvector;
  //Reset Prior the test
  tempvector.clear();
  tempvector.push_back(2222);
  _emu->usbWrite( devId, 286,tempvector);

  //Need to set the test 7 test 8 of the ext resitor manually because there the data is inputed as one 32bit word
  int defaultresistor;
  defaultresistor=_resCode[4];
  //Making the vectors with the tap for the grid resistor test
  vector<int> convertionrestapreal(24);
  vector<int> convertionrestapimag(24);
  if (_options[5]!='6'){//Overriding...
    for (size_t i=0;i<24;++i){
      convertionrestapreal[i]=_conversionResCodeReal;
      convertionrestapimag[i]=_conversionResCodeImag;
    }
    cout<<"Overrided values of tap"<<endl;
  }
  else{
    for (size_t i=0;i<24;++i){
      convertionrestapreal[i]=Potentiometer::tap_from_r(Potentiometer::r_from_tap(_resCode[2]),_calibrationRabNew[0][i],_calibrationRwNew[0][i]);//rescode[2] because we want the 8kohm
      convertionrestapimag[i]=Potentiometer::tap_from_r(Potentiometer::r_from_tap(_resCode[2]),_calibrationRabNew[1][i],_calibrationRwNew[1][i]);
    }
  }

  _confVector.clear();
  _confVector.append(static_cast<uint32_t>(0));
  _confVector.insert(_confVector.size(),3,static_cast<uint32_t>(0));
  _confVector.append(static_cast<uint32_t>(786432));
  _confVector.insert(_confVector.size(),8,static_cast<uint32_t>(0));
  _confVector.append(static_cast<uint32_t>(206703681));
  _confVector.append(static_cast<uint32_t>(414524679));
  _confVector.append(static_cast<uint32_t>(622345677));
  _confVector.append(static_cast<uint32_t>(4051392147u));
  _confVector.append(static_cast<uint32_t>(1));
  _confVector.insert(_confVector.size(),2,static_cast<uint32_t>(0));
  _confVector.append(static_cast<uint32_t>(1));
  _confVector.insert(_confVector.size(),11,static_cast<uint32_t>(0));
  _confVector.append(static_cast<uint32_t>(3221225472u));
  _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(References_Real->at(devId)->value()));
  _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(References_Imag->at(devId)->value()));
  _confVector.insert(_confVector.size(),23,static_cast<uint32_t>(696969));
  _confVector.append(static_cast<uint32_t>(3229616128u));
  _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(3221225772u));
  _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(3221225472u+interval->value()));
  ///Below the POT values
  //Resistor values
  uint32_t tempuint0;
  uint32_t tempuint1;
  uint32_t tempuint2;
  uint32_t tempuint3;
  //The vector dependent in the type of resistor start from here...
  if (testid==0){//Internal resistor test
    for (int i=0;i<24;++i){
      _confVector.insert(_confVector.size(),3,static_cast<uint32_t>(1684300900u));
      tempuint3=static_cast<uint32_t>(convertionrestapreal[i]);
      tempuint2=static_cast<uint32_t>(defaultresistor);
      tempuint1=static_cast<uint32_t>(convertionrestapimag[i]);
      tempuint0=static_cast<uint32_t>(defaultresistor);
      //Swifting the 8bit values to the appropriate place
      //tempint0 no swift,tempint1 8bit left swift,tempint2 16 bit swift,tempint3 24bit swift
      tempuint3=tempuint3<<24;
      tempuint2=tempuint2<<16;
      tempuint1=tempuint1<<8;
      _confVector.append(static_cast<uint32_t>(tempuint0 | tempuint1 | tempuint2 | tempuint3));
    }
    _confVector.insert(_confVector.size(),8,static_cast<uint32_t>(100<<24|100<<16|100<<8|100));
    //TCON values
    for(int i=0;i<24;++i){
      _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(2290649224u));
      _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(3149629576u));//4294936712u old
    }
    _confVector.insert(_confVector.size(),4,static_cast<uint32_t>(2290649224u));
    //Switches values
    _confVector.insert(_confVector.size(),12,static_cast<uint32_t>(1442797055u));
    _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(4294967295u));
  }
  else if (testid==1){//P0CHIP1-2 test
    for (int i=0;i<24;++i){
      _confVector.insert(_confVector.size(),2,static_cast<uint32_t>(100<<24|100<<16|100<<8|defaultresistor));
      _confVector.append(static_cast<uint32_t>(1684300900u));
      tempuint3=static_cast<uint32_t>(convertionrestapreal[i]);
      tempuint2=static_cast<uint32_t>(100);
      tempuint1=static_cast<uint32_t>(convertionrestapimag[i]);
      tempuint0=static_cast<uint32_t>(100);
      //Swifting the 8bit values to the appropriate place
      //tempint0 no swift,tempint1 8bit left swift,tempint2 16 bit swift,tempint3 24bit swift
      tempuint3=tempuint3<<24;
      tempuint2=tempuint2<<16;
      tempuint1=tempuint1<<8;
      _confVector.append(static_cast<uint32_t>(tempuint0 | tempuint1 | tempuint2 | tempuint3));
    }
    _confVector.insert(_confVector.size(),8,static_cast<uint32_t>(100<<24|100<<16|100<<8|100));
    //TCON values
    for(int i=0;i<24;++i){
      _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(2290845835u));//2291107983u old value
      _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(3099101320u));//4177037448u old value
    }
    _confVector.insert(_confVector.size(),4,static_cast<uint32_t>(2290649224u));
    //Switches values
    _confVector.append(static_cast<uint32_t>(4282375484u));//set for the first default node 1
    _confVector.insert(_confVector.size(),11,static_cast<uint32_t>(4282384191u));//the rest closed for the nodes
    _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(4294967295u));

  }
  else if (testid==2){//P1CHIP1-2 test
    for (int i=0;i<24;++i){
      _confVector.insert(_confVector.size(),2,static_cast<uint32_t>(100<<24|100<<16|defaultresistor<<8|100));
      _confVector.append(static_cast<uint32_t>(1684300900u));
      tempuint3=static_cast<uint32_t>(convertionrestapreal[i]);
      tempuint2=static_cast<uint32_t>(100);
      tempuint1=static_cast<uint32_t>(convertionrestapimag[i]);
      tempuint0=static_cast<uint32_t>(100);
      //Swifting the 8bit values to the appropriate place
      //tempint0 no swift,tempint1 8bit left swift,tempint2 16 bit swift,tempint3 24bit swift
      tempuint3=tempuint3<<24;
      tempuint2=tempuint2<<16;
      tempuint1=tempuint1<<8;
      _confVector.append(static_cast<uint32_t>(tempuint0 | tempuint1 | tempuint2 | tempuint3));
    }
    _confVector.insert(_confVector.size(),8,static_cast<uint32_t>(100<<24|100<<16|100<<8|100));
    //TCON values
    for(int i=0;i<24;++i){
      _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(2293795000u));//old value 2297989368u
      _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(3099101320u));//old value 4177037448u
    }
    _confVector.insert(_confVector.size(),4,static_cast<uint32_t>(2290649224u));
    //Switches values
    _confVector.insert(_confVector.size(),12,static_cast<uint32_t>(3711884606u));
    _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(4294967295u));
  }
  else if (testid==3){//P2CHIP1-2 test
    for (int i=0;i<24;++i){
      _confVector.insert(_confVector.size(),2,static_cast<uint32_t>(100<<24|defaultresistor<<16|100<<8|100));
      _confVector.append(static_cast<uint32_t>(1684300900u));
      tempuint3=static_cast<uint32_t>(convertionrestapreal[i]);
      tempuint2=static_cast<uint32_t>(100);
      tempuint1=static_cast<uint32_t>(convertionrestapimag[i]);
      tempuint0=static_cast<uint32_t>(100);
      //Swifting the 8bit values to the appropriate place
      //tempint0 no swift,tempint1 8bit left swift,tempint2 16 bit swift,tempint3 24bit swift
      tempuint3=tempuint3<<24;
      tempuint2=tempuint2<<16;
      tempuint1=tempuint1<<8;
      _confVector.append(static_cast<uint32_t>(tempuint0 | tempuint1 | tempuint2 | tempuint3));
    }
    _confVector.insert(_confVector.size(),8,static_cast<uint32_t>(100<<24|100<<16|100<<8|100));
    //TCON values
    for(int i=0;i<24;++i){
      _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(2340981640u));//old value 2408091528u
      _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(3099101320u));//old value 4177037448u
    }
    _confVector.insert(_confVector.size(),4,static_cast<uint32_t>(2290649224u));
    //Switches values
    _confVector.insert(_confVector.size(),12,static_cast<uint32_t>(3711425847u));
    _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(4294967295u));
  }
  else if (testid==4){//P3CHIP1-2 test
    for (int i=0;i<24;++i){
      _confVector.insert(_confVector.size(),2,static_cast<uint32_t>((defaultresistor<<24)|(100<16)|(100<<8)|(100)));
      _confVector.append(static_cast<uint32_t>(1684300900u));
      tempuint3=static_cast<uint32_t>(convertionrestapreal[i]);
      tempuint2=static_cast<uint32_t>(100);
      tempuint1=static_cast<uint32_t>(convertionrestapimag[i]);
      tempuint0=static_cast<uint32_t>(100);
      //Swifting the 8bit values to the appropriate place
      //tempint0 no swift,tempint1 8bit left swift,tempint2 16 bit swift,tempint3 24bit swift
      tempuint3=tempuint3<<24;
      tempuint2=tempuint2<<16;
      tempuint1=tempuint1<<8;
      _confVector.append(static_cast<uint32_t>(tempuint0 | tempuint1 | tempuint2 | tempuint3));
    }
    _confVector.insert(_confVector.size(),8,static_cast<uint32_t>(100<<24|100<<16|100<<8|100));
    //TCON values
    for(int i=0;i<24;++i){
      _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(3095967880u));//4169726088u old value
      _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(3099101320u));//4177037448u old value
    }
    _confVector.insert(_confVector.size(),4,static_cast<uint32_t>(2290649224u));
    //Switches values
    _confVector.append(static_cast<uint32_t>(4282375475u));//set for the first default node 1
    _confVector.insert(_confVector.size(),11,static_cast<uint32_t>(4282384191u));//the rest closed for the nodes
    _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(4294967295u));
  }
  else if (testid==5){//P0P2CHIP3 test
    for (int i=0;i<24;++i){
      _confVector.insert(_confVector.size(),2,static_cast<uint32_t>(1684300900u));
      tempuint0=static_cast<uint32_t>(defaultresistor);
      tempuint1=static_cast<uint32_t>(100);
      tempuint2=static_cast<uint32_t>(defaultresistor);
      tempuint3=static_cast<uint32_t>(100);
      tempuint3=tempuint3<<24;
      tempuint2=tempuint2<<16;
      tempuint1=tempuint1<<8;
      _confVector.append(static_cast<uint32_t>(tempuint0 | tempuint1 | tempuint2 | tempuint3));
      tempuint3=static_cast<uint32_t>(convertionrestapreal[i]);
      tempuint2=static_cast<uint32_t>(100);
      tempuint1=static_cast<uint32_t>(convertionrestapimag[i]);
      tempuint0=static_cast<uint32_t>(100);
      //Swifting the 8bit values to the appropriate place
      //tempint0 no swift,tempint1 8bit left swift,tempint2 16 bit swift,tempint3 24bit swift
      tempuint3=tempuint3<<24;
      tempuint2=tempuint2<<16;
      tempuint1=tempuint1<<8;
      _confVector.append(static_cast<uint32_t>(tempuint0 | tempuint1 | tempuint2 | tempuint3));
    }
    _confVector.insert(_confVector.size(),8,static_cast<uint32_t>(100<<24|100<<16|100<<8|100));
    //TCON values
    for(int i=0;i<24;++i){
      _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(2290649224u));
      _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(3099102091u));//old value 4177039247u
    }
    _confVector.insert(_confVector.size(),4,static_cast<uint32_t>(2290649224u));
    //Switches values
    _confVector.insert(_confVector.size(),12,static_cast<uint32_t>(3709852959u));
    _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(4294967295u));
  }
  else if (testid==6){//P1P3CHIP3 test
    for (int i=0;i<24;++i){
      _confVector.insert(_confVector.size(),2,static_cast<uint32_t>(1684300900u));
      tempuint0=static_cast<uint32_t>(defaultresistor);
      tempuint1=static_cast<uint32_t>(100);
      tempuint2=static_cast<uint32_t>(defaultresistor);
      tempuint3=static_cast<uint32_t>(100);
      tempuint3=tempuint3<<16;
      tempuint2=tempuint2<<24;
      tempuint1=tempuint1;
      tempuint0=tempuint0<<8;
      _confVector.append(static_cast<uint32_t>(tempuint0 | tempuint1 | tempuint2 | tempuint3));
      tempuint3=static_cast<uint32_t>(convertionrestapreal[i]);
      tempuint2=static_cast<uint32_t>(100);
      tempuint1=static_cast<uint32_t>(convertionrestapimag[i]);
      tempuint0=static_cast<uint32_t>(100);
      //Swifting the 8bit values to the appropriate place
      //tempint0 no swift,tempint1 8bit left swift,tempint2 16 bit swift,tempint3 24bit swift
      tempuint3=tempuint3<<24;
      tempuint2=tempuint2<<16;
      tempuint1=tempuint1<<8;
      _confVector.append(static_cast<uint32_t>(tempuint0 | tempuint1 | tempuint2 | tempuint3));
    }
    _confVector.insert(_confVector.size(),8,static_cast<uint32_t>(100<<24|100<<16|100<<8|100));
    //TCON values
    for(int i=0;i<24;++i){
      _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(2290649224u));
      _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(3099113656u));//old value 4177066232u
    }
    _confVector.insert(_confVector.size(),4,static_cast<uint32_t>(2290649224u));
    //Switches values
    _confVector.append(static_cast<uint32_t>(4282375439u));//set for the first default node 1
    _confVector.insert(_confVector.size(),11,static_cast<uint32_t>(4282384191u));//the rest closed for the nodes
    _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(4294967295u));
  }
  else if (testid==7){//P0P2EXT test
    for (int i=0;i<24;++i){//no need to set all the 24 nodes,but just in case
      _confVector.insert(_confVector.size(),3,static_cast<uint32_t>(1684300900u));
      tempuint3=static_cast<uint32_t>(convertionrestapreal[i]);
      tempuint2=static_cast<uint32_t>(100);
      tempuint1=static_cast<uint32_t>(convertionrestapimag[i]);
      tempuint0=static_cast<uint32_t>(100);
      //Swifting the 8bit values to the appropriate place
      //tempint0 no swift,tempint1 8bit left swift,tempint2 16 bit swift,tempint3 24bit swift
      tempuint3=tempuint3<<24;
      tempuint2=tempuint2<<16;
      tempuint1=tempuint1<<8;
      _confVector.append(static_cast<uint32_t>(tempuint0 | tempuint1 | tempuint2 | tempuint3));
    }
    //Now setting the ext res
    _confVector.append(static_cast<uint32_t>(100<<24|defaultresistor<<16|100<<8|defaultresistor));
    _confVector.append(static_cast<uint32_t>(100<<24|defaultresistor<<16|100<<8|defaultresistor));
    _confVector.append(static_cast<uint32_t>(1684300900u));
    _confVector.append(static_cast<uint32_t>(100<<24|defaultresistor<<16|100<<8|defaultresistor));
    _confVector.append(static_cast<uint32_t>(100<<24|defaultresistor<<16|100<<8|defaultresistor));
    _confVector.append(static_cast<uint32_t>(100<<24|defaultresistor<<16|100<<8|defaultresistor));
    _confVector.append(static_cast<uint32_t>(100<<24|defaultresistor<<16|100<<8|defaultresistor));
    _confVector.append(static_cast<uint32_t>(100<<24|defaultresistor<<16|100<<8|defaultresistor));
    //TCON values
    for(int i=0;i<5;++i){//not for all the nodes
      _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(2290649224u));
      _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(3099101320u));//old value 4177037448u
    }
    _confVector.insert(_confVector.size(),3,static_cast<uint32_t>(2290649224u));
    _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(3099101320u));//old value 4177037448u
    _confVector.insert(_confVector.size(),11,static_cast<uint32_t>(2290649224u));
    _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(3099101320u));//old value 4177037448u
    _confVector.insert(_confVector.size(),22,static_cast<uint32_t>(2290649224u));
    _confVector.append(static_cast<uint32_t>(2341178251u));//old value 2408550287u
    _confVector.append(static_cast<uint32_t>(2341177480u));//old value 2408548488u
    _confVector.append(static_cast<uint32_t>(2341178251u));//old value 2408550287u
    _confVector.append(static_cast<uint32_t>(2341178251u));//old value 2408550287u
    //Switches values
    _confVector.insert(_confVector.size(),2,static_cast<uint32_t>(3711950143u));
    _confVector.insert(_confVector.size(),2,static_cast<uint32_t>(4282375487u));
    _confVector.insert(_confVector.size(),2,static_cast<uint32_t>(4282384191u));
    _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(4282375487u));
    _confVector.insert(_confVector.size(),5,static_cast<uint32_t>(4282384191u));
    _confVector.append(static_cast<uint32_t>(1431654645u));
  }
  else if (testid==8){//P1P3EXT test
    for (int i=0;i<24;++i){//no need to set all the 24 nodes,but just in case
      _confVector.insert(_confVector.size(),3,static_cast<uint32_t>(1684300900u));
      tempuint3=static_cast<uint32_t>(convertionrestapreal[i]);
      tempuint2=static_cast<uint32_t>(100);
      tempuint1=static_cast<uint32_t>(convertionrestapimag[i]);
      tempuint0=static_cast<uint32_t>(100);
      //Swifting the 8bit values to the appropriate place
      //tempint0 no swift,tempint1 8bit left swift,tempint2 16 bit swift,tempint3 24bit swift
      tempuint3=tempuint3<<24;
      tempuint2=tempuint2<<16;
      tempuint1=tempuint1<<8;
      _confVector.append(static_cast<uint32_t>(tempuint0 | tempuint1 | tempuint2 | tempuint3));
    }
    //Now setting the ext res
    _confVector.append(static_cast<uint32_t>(defaultresistor<<24|100<<16|defaultresistor<<8|100));
    _confVector.append(static_cast<uint32_t>(defaultresistor<<24|100<<16|defaultresistor<<8|100));
    _confVector.append(static_cast<uint32_t>(1684300900u));
    _confVector.append(static_cast<uint32_t>(defaultresistor<<24|100<<16|defaultresistor<<8|100));
    _confVector.append(static_cast<uint32_t>(defaultresistor<<24|100<<16|defaultresistor<<8|100));
    _confVector.append(static_cast<uint32_t>(defaultresistor<<24|100<<16|defaultresistor<<8|100));
    _confVector.append(static_cast<uint32_t>(defaultresistor<<24|100<<16|defaultresistor<<8|100));
    _confVector.append(static_cast<uint32_t>(defaultresistor<<24|100<<16|defaultresistor<<8|100));
    //TCON values
    for(int i=0;i<5;++i){//not for all the nodes
      _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(2290649224u));
      _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(3099101320u));//old value 4177037448u
    }
    _confVector.insert(_confVector.size(),3,static_cast<uint32_t>(2290649224u));
    _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(3099101320u));//old value 4177037448u
    _confVector.insert(_confVector.size(),11,static_cast<uint32_t>(2290649224u));
    _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(3099101320u));//old value 4177037448u
    _confVector.insert(_confVector.size(),22,static_cast<uint32_t>(2290649224u));
    _confVector.append(static_cast<uint32_t>(3099113656u));//old value 4177066232u
    _confVector.append(static_cast<uint32_t>(3099101320u));//old value 4177037448u
    _confVector.append(static_cast<uint32_t>(3099113656u));//old value 4177066232u
    _confVector.append(static_cast<uint32_t>(3099113656u));//old value 4177066232u
    //Switches values
    _confVector.insert(_confVector.size(),2,static_cast<uint32_t>(3711950143u));
    _confVector.insert(_confVector.size(),2,static_cast<uint32_t>(4282375487u));
    _confVector.insert(_confVector.size(),2,static_cast<uint32_t>(4282384191u));
    _confVector.insert(_confVector.size(),1,static_cast<uint32_t>(4282375487u));
    _confVector.insert(_confVector.size(),4,static_cast<uint32_t>(4282384191u));
    _confVector.append(static_cast<uint32_t>(1520));
  }
  //...to here

  //Other values
  _confVector.insert(_confVector.size(),3,static_cast<uint32_t>(0));
  _confVector.append(static_cast<uint32_t>(3221225472u));

  //Corrected Gain
  QVector<uint32_t> encodedinput;
  _offsetGainHandling(&encodedinput,0);//0 test for adc/dac gain/offset, 0 for gain
  for(int i=0;i<24;++i){
    _confVector.append(encodedinput[i]);
  }
  _offsetGainHandling(&encodedinput,1);//0 test for adc/dac gain/offset, 0 for offset
  for(int i=0;i<24;++i){
    _confVector.append(encodedinput[i]);
  }
  if (autocheckopt->isChecked()){//Running the default new hardcoded version
    _confVector.append(static_cast<uint32_t>(20));
    _confVector.append(static_cast<uint32_t>(0));
    _confVector.append(static_cast<uint32_t>(0));
    _confVector.append(static_cast<uint32_t>(3221225475u));//NIOS + 1 for default options
  }
  else { //Old, user inputed parameters
    _confVector.append(static_cast<uint32_t>(num_of_values_combobox->value()));
    _confVector.append(static_cast<uint32_t>(second_value_combobox->value()));
    _confVector.append(static_cast<uint32_t>(first_value_combobox->value()));
    _confVector.append(static_cast<uint32_t>(3221225474u));//NIOS for accepting user defined ramp
  }

  _confVector.append(static_cast<uint32_t>(0));
  _confVector.append(static_cast<uint32_t>(0));
  _confVector.append(static_cast<uint32_t>(102));
  _confVector.append(static_cast<uint32_t>(1));


  //Setting the start of the calibration
  tempvector=_confVector.toStdVector();
  //Write Calibration Configuration to FPGA
  _emu->usbWrite( devId, 0,tempvector);
  vector<uint32_t> data;


  int intcount=1;//internal counter of the internal tests in resistor, set to first node
  if (testid!=1&&testid!=4&&testid!=6){//not P0CHIP1-2 test and P3Chip1-2 and P1P3CHIP3, old way of testing
    tempvector.clear();
    tempvector.push_back(1111);
    _emu->usbWrite( devId, 286,tempvector);
    long int watchdog_timer=0;
    while (1){
      ++watchdog_timer;
      _emu->usbRead( devId, 287,1,data);
      if (data.at(0)==4444)
        break;
      if (watchdog_timer>30)
        return 1;//Return 1 if the board doesnt not respont to commands of calibration
    }
  }
  else{
    tempvector.clear();
    tempvector.push_back(1200+intcount);
    _emu->usbWrite( devId, 286,tempvector);
    long int watchdog_timer=0;
    while (1){
      ++watchdog_timer;
      _emu->usbRead( devId, 287,1,data);
      if (data.at(0)==4444)
        break;
      if (watchdog_timer>30)
        return 1;//Return 1 if the board doesnt not respont to commands of calibration
    }
    tempvector.clear();
    tempvector.push_back(2222);
    _emu->usbWrite( devId, 286,tempvector);

    intcount++;
    int switchgroup=1;//1-12
    while (intcount<25){
      //Setting the 2nd to 24th node switches mode
      tempvector.clear();
      for (int c=0;c<12;c++){
        tempvector.push_back(static_cast<uint32_t>(4282384191u));//open all the node switches
      }
      if (testid==1){//P0Chip1-2
        if(intcount%2==0)//if the is an even number of node then imput this
          tempvector[switchgroup-1]=static_cast<uint32_t>(3711762239u);//close switch  for the corrent node ,the -1 is because of the vector location
        else
          tempvector[switchgroup-1]=static_cast<uint32_t>(4282375484u);//close switch  for the corrent node,the -1 is because of the vector location
      }
      else if(testid==4){//P3Chip1-2
        if(intcount%2==0)
          tempvector[switchgroup-1]=static_cast<uint32_t>(3711172415u);
        else
          tempvector[switchgroup-1]=static_cast<uint32_t>(4282375475u);
      }
      else if(testid==6){//P1P3Chip3
        if(intcount%2==0)
          tempvector[switchgroup-1]=static_cast<uint32_t>(3708813119u);
        else
          tempvector[switchgroup-1]=static_cast<uint32_t>(4282375439u);
      }

      _emu->usbWrite( devId, 217,tempvector);
      tempvector.clear();
      tempvector.push_back(1200+intcount);
      _emu->usbWrite( devId, 286,tempvector);
      long int watchdog_timer=0;
      while (1){
        ++watchdog_timer;
        _emu->usbRead( devId, 287,1,data);
        if (data.at(0)==4444)
          break;
        if (watchdog_timer>30)
          return 1;//Return 1 if the board doesnt not respont to commands of calibration
      }

      tempvector.clear();
      tempvector.push_back(2222);
      _emu->usbWrite( devId, 286,tempvector);
      intcount++;
      if ((intcount % 2 !=0)&&(intcount<25))//increase every two tests
        switchgroup++;
    }
  }


  //Read the results from FPGA memory,first and last possition is empty
  vector<uint32_t> data1, data2, data3, data4;
  _emu->usbRead( devId,  290 , 400 , data1 );
  _emu->usbRead( devId,  690 , 400 , data2 );
  _emu->usbRead( devId, 1090 , 400 , data3 );
  _emu->usbRead( devId, 1490 ,  312 , data4 );
  data.clear();
  data.insert(data.end(), data1.begin(), data1.end());
  data.insert(data.end(), data2.begin(), data2.end());
  data.insert(data.end(), data3.begin(), data3.end());
  data.insert(data.end(), data4.begin(), data4.end());

  //Parameters
  QVector<double> firstvoltage;
  QVector<double> secondvoltage;

  if (autocheckopt->isChecked()){

    firstvoltage.append((double)_DACCodeFirst[3]/4096*5);
    secondvoltage.append((double)_DACCodeSecond[3]/4096*5);


    firstvoltage.append((double)_DACCodeFirst[4]/4096*5);
    secondvoltage.append((double)_DACCodeSecond[4]/4096*5);



    firstvoltage.append((double)_DACCodeFirst[5]/4096*5);
    secondvoltage.append((double)_DACCodeSecond[5]/4096*5);

  }
  else { //user defined values

    firstvoltage.append((double)first_value_combobox->value()/4096*5);
    secondvoltage.append((double)second_value_combobox->value()/4096*5);



    firstvoltage.append((double)first_value_combobox->value()/4096*5);
    secondvoltage.append((double)second_value_combobox->value()/4096*5);


    firstvoltage.append((double)first_value_combobox->value()/4096*5);
    secondvoltage.append((double)second_value_combobox->value()/4096*5);
  }

  //Create the ideal resistors ramp
  QVector<double> idealresistorramp;
  QVector<double> xidealresistorramp;
  xidealresistorramp.append(_resCode[3]);
  xidealresistorramp.append(_resCode[4]);
  xidealresistorramp.append(_resCode[5]);
  idealresistorramp.append(Potentiometer::r_from_tap(_resCode[3]));
  idealresistorramp.append(Potentiometer::r_from_tap(_resCode[4]));
  idealresistorramp.append(Potentiometer::r_from_tap(_resCode[5]));

  QVector<QVector<double> > P0res;
  QVector<QVector<double> > P2res;
  //Initialize the resistor vectors,2rows(real,imag)x24nodes=48inside vectors
  for (int i=0;i<24;++i){
    P0res.append(QVector<double>(3));
    P2res.append(QVector<double>(3));
  }
  QVector<uint32_t> tempnodedata;
  QVector<uint32_t> tempnodesdata = QVector<uint32_t>::fromStdVector(data);
  double alpha;
  double beta;
  //Initialize the decoding vectors
  QVector<QVector<double> > decodedresultsreal;
  QVector<QVector<double> > decodedresultsimag;

  for (int i=0;i<24;++i){
    decodedresultsreal.append(QVector<double>());
    decodedresultsimag.append(QVector<double>());
  }

  //Data handling
  for (int j=0;j<3;++j){
    //Take groups of 15 uint32_t values(24 nodes) and convert them to double
    decodedresultsreal.clear();
    decodedresultsimag.clear();
    for (int i=0;i<24;++i){
      decodedresultsreal.append(QVector<double>());
      decodedresultsimag.append(QVector<double>());
    }
    for (int i=0;i<24;++i){
      tempnodedata.clear();
      tempnodedata=tempnodesdata.mid(1+i*21+j*504,20);
      _resultsHandling(tempnodedata,i,&decodedresultsreal,&decodedresultsimag,20);

      if (_options[0]=='0'){
        if (j==0){//store only the 5kohm values
          _rawResultsNew[0].append(decodedresultsreal[i]);
          _rawResultsNew[1].append(decodedresultsimag[i]);
        }}
      if (_options[0]=='1'){
        if (j==1){//store only the 6kohm values
          _rawResultsNew[0].append(decodedresultsreal[i]);
          _rawResultsNew[1].append(decodedresultsimag[i]);
        }}
      if (_options[0]=='2'){
        if (j==2){//store only the 7kohm values
          _rawResultsNew[0].append(decodedresultsreal[i]);
          _rawResultsNew[1].append(decodedresultsimag[i]);
        }}

    }

    //Save in decodedresults the measured resistance
    //Take the mean value
    double firstmeanreal=0;
    double secondmeanreal=0;
    double firstmeanimag=0;
    double secondmeanimag=0;
    double Itemp;
    unsigned int rescodet=0;

    //For each of 24 groups calculate the least squares of the 15 values
    for (int i=0;i<24;++i){
      firstmeanreal=0;
      secondmeanreal=0;
      firstmeanimag=0;
      secondmeanimag=0;
      //First value
      double voltage;
      for (int k=0;k<10;++k){
        //real

        rescodet=convertionrestapreal[i];

        voltage=decodedresultsreal[i][k];
        if(_options[3]=='6') // If we are running with the default options
          Itemp=(firstvoltage[j]-2.5)/((_lsqResultsNew[0][i+24][rescodet]));//The first 24 is the adc
        else //a conversion resistor test was rerun, and we use its onwn raw measured results istead the ones of leastsquares
          Itemp=(firstvoltage[j]-2.5)/((_rawResultsNew[0][i+72][2]));
        decodedresultsreal[i][k]=(decodedresultsreal[i][k]-2.5)/Itemp;
        firstmeanreal=firstmeanreal+decodedresultsreal[i][k];

        //imag
        rescodet=convertionrestapimag[i];
        if(_options[3]=='6')
          Itemp=(firstvoltage[j]-2.5)/((_lsqResultsNew[1][i+24][rescodet]));
        else
          Itemp=(firstvoltage[j]-2.5)/((_rawResultsNew[1][i+72][2]));
        decodedresultsimag[i][k]=(decodedresultsimag[i][k]-2.5)/Itemp;
        firstmeanimag=firstmeanimag+decodedresultsimag[i][k];
      }
      firstmeanreal=firstmeanreal/10;
      firstmeanimag=firstmeanimag/10;

      //Second value
      for (int k=10;k<20;++k){
        //real
        voltage=decodedresultsreal[i][k];
        rescodet=convertionrestapreal[i];
        if(_options[3]=='6')
          Itemp=(secondvoltage[j]-2.5)/((_lsqResultsNew[0][i+24][rescodet]));
        else
          Itemp=(secondvoltage[j]-2.5)/((_rawResultsNew[0][i+72][2]));
        decodedresultsreal[i][k]=(decodedresultsreal[i][k]-2.5)/Itemp;
        secondmeanreal=secondmeanreal+decodedresultsreal[i][k];

        //imag
        rescodet=convertionrestapimag[i];
        if(_options[3]=='6')
          Itemp=(secondvoltage[j]-2.5)/((_lsqResultsNew[1][i+24][rescodet]));
        else
          Itemp=(secondvoltage[j]-2.5)/((_rawResultsNew[1][i+72][2]));
        decodedresultsimag[i][k]=(decodedresultsimag[i][k]-2.5)/Itemp;
        secondmeanimag=secondmeanimag+decodedresultsimag[i][k];
      }
      secondmeanreal=secondmeanreal/10;
      secondmeanimag=secondmeanimag/10;

      P0res[i][j]=(firstmeanreal+secondmeanreal)/2;
      P2res[i][j]=(firstmeanimag+secondmeanimag)/2;
      //cout<<P0res[i][j]<<endl;
      //cout<<"code  "<<rescodet<<endl;
      if (_options[1]=='0'){
        if (j==0){//store only the 5kohm values
          _rawResultsNew[0].append(decodedresultsreal[i]);
          _rawResultsNew[1].append(decodedresultsimag[i]);
        }}
      if (_options[1]=='1'){
        if (j==1){//store only the 6kohm values
          _rawResultsNew[0].append(decodedresultsreal[i]);
          _rawResultsNew[1].append(decodedresultsimag[i]);
        }}
      if (_options[1]=='2'){
        if (j==2){//store only the 7kohm values
          _rawResultsNew[0].append(decodedresultsreal[i]);
          _rawResultsNew[1].append(decodedresultsimag[i]);
        }}
    }

  }
  if (_options[0]=='6'&&_options[1]=='6'){
    for (int i=0;i<24;++i)
    {
      _rawResultsNew[0].append(P0res[i]);
      _rawResultsNew[1].append(P2res[i]);
    }
  }


  QVector<double> xdata2;
  xdata2.push_back(_resCode[3]);
  xdata2.push_back(_resCode[4]);
  xdata2.push_back(_resCode[5]);
  QVector<double> tempresreal;
  QVector<double> tempresimag;
  QVector<double> lsqdata;
  QVector<double> lsqxdata;
  for (int i=0;i<24;++i){
    tempresreal.clear();
    tempresimag.clear();
    for (int j=0;j<3;++j){
      tempresreal.append(P0res.at(i).at(j));
      tempresimag.append(P2res.at(i).at(j));
    }
    //First for the real part
    alpha=0.0;
    beta=0.0;
    _leastSquares(tempresreal,xdata2,&alpha,&beta);
    lsqxdata.clear();
    lsqdata.clear();
    //    for (int k=127;k<180;k=k+26)
    //      lsqxdata.push_back(k);
    //    for (int k=127;k<180;k=k+26)
    //      lsqdata.push_back(alpha*k+beta);
    for (int k=0;k<257;k=k+1)
      lsqxdata.push_back(k);
    for (int k=0;k<257;k=k+1)
      lsqdata.push_back(alpha*k+beta);
    //At this point we have both the ideal and the least squares line,
    //so we can calculate the gain an offset
    //Bypass the unused nodes for the EXT res
    if ((testid==7||testid==8)&&i!=0&&i!=1&&i!=2&&i!=3&&i!=4&&i!=6&&i!=12)
      continue;

    if (testid==0)
      _calibrationNameDataNew[0].push_back(QString("Internal resistor real of node %0").arg(i+1));
    else if (testid==1)
      _calibrationNameDataNew[0].push_back(QString("P0CHIP1-2 resistor real of node %0").arg(i+1));
    else if (testid==2)
      _calibrationNameDataNew[0].push_back(QString("P1CHIP1-2 resistor real of node %0").arg(i+1));
    else if (testid==3)
      _calibrationNameDataNew[0].push_back(QString("P2CHIP1-2 resistor real of node %0").arg(i+1));
    else if (testid==4)
      _calibrationNameDataNew[0].push_back(QString("P3CHIP1-2 resistor real of node %0").arg(i+1));
    else if (testid==5)
      _calibrationNameDataNew[0].push_back(QString("P0P2CHIP3 resistor real of node %0").arg(i+1));
    else if (testid==6)
      _calibrationNameDataNew[0].push_back(QString("P1P3CHIP3 resistor real of node %0").arg(i+1));
    else if (testid==7)
      _calibrationNameDataNew[0].push_back(QString("P0P2EXT resistor real of node %0").arg(i+1));
    else if (testid==8)
      _calibrationNameDataNew[0].push_back(QString("P1P3EXT resistor real of node %0").arg(i+1));
    _calibrationIdNew[0].push_back(testid);
    double offsetvalue;
    double gainvalue;
    offsetvalue=idealresistorramp[0]-lsqdata[_resCode[3]];
    gainvalue=(idealresistorramp[1]-idealresistorramp[0])/(xidealresistorramp[1]-xidealresistorramp[0]) - (lsqdata[_resCode[4]]-lsqdata[_resCode[3]])/(lsqxdata[_resCode[4]]-lsqxdata[_resCode[3]]);
    _calibrationOffsetDataNew[0].push_back(offsetvalue);
    _calibrationGainDataNew[0].push_back(gainvalue);
    _calibrationRwNew[0].push_back(beta);//0kohm equivalent
    _calibrationRabNew[0].push_back(alpha*256+beta-beta);//10kohm equivalent
    _lsqResultsNew[0].push_back(lsqdata);
    //Then for the imag part
    alpha=0.0;
    beta=0.0;
    _leastSquares(tempresimag,xdata2,&alpha,&beta);
    lsqxdata.clear();
    lsqdata.clear();
    //    for (int k=127;k<180;k=k+26)
    //      lsqxdata.push_back(k);
    //    for (int k=127;k<180;k=k+26)
    //      lsqdata.push_back(alpha*k+beta);
    for (int k=0;k<257;k=k+1)
      lsqxdata.push_back(k);
    for (int k=0;k<257;k=k+1)
      lsqdata.push_back(alpha*k+beta);
    //At this point we have both the ideal and the least squares line,
    //so we can calculate the gain an offset
    if (testid==0)
      _calibrationNameDataNew[1].push_back(QString("Internal resistor imag of node %0").arg(i+1));
    else if (testid==1)
      _calibrationNameDataNew[1].push_back(QString("P0CHIP1-2 resistor imag of node %0").arg(i+1));
    else if (testid==2)
      _calibrationNameDataNew[1].push_back(QString("P1CHIP1-2 resistor imag of node %0").arg(i+1));
    else if (testid==3)
      _calibrationNameDataNew[1].push_back(QString("P2CHIP1-2 resistor imag of node %0").arg(i+1));
    else if (testid==4)
      _calibrationNameDataNew[1].push_back(QString("P3CHIP1-2 resistor imag of node %0").arg(i+1));
    else if (testid==5)
      _calibrationNameDataNew[1].push_back(QString("P0P2CHIP3 resistor imag of node %0").arg(i+1));
    else if (testid==6)
      _calibrationNameDataNew[1].push_back(QString("P1P3CHIP3 resistor imag of node %0").arg(i+1));
    else if (testid==7)
      _calibrationNameDataNew[1].push_back(QString("P0P2EXT resistor imag of node %0").arg(i+1));
    else if (testid==8)
      _calibrationNameDataNew[1].push_back(QString("P1P3EXT resistor imag of node %0").arg(i+1));

    _calibrationIdNew[1].push_back(testid);
    offsetvalue=idealresistorramp[0]-lsqdata[_resCode[3]];
    gainvalue=(idealresistorramp[1]-idealresistorramp[0])/(xidealresistorramp[1]-xidealresistorramp[0]) - (lsqdata[_resCode[4]]-lsqdata[_resCode[3]])/(lsqxdata[_resCode[4]]-lsqxdata[_resCode[3]]);
    _calibrationOffsetDataNew[1].push_back(offsetvalue);
    _calibrationGainDataNew[1].push_back(gainvalue);
    _calibrationRwNew[1].push_back(beta);//0kohm equivalent
    _calibrationRabNew[1].push_back(alpha*256+beta-beta);//10kohm equivalent
    _lsqResultsNew[1].push_back(lsqdata);
  }

  bool error_found=0;
  for(int i=0;i<24;++i){//+((testid+1)*24) in order to access the next results in the huge vector with the data of the next resistors depending on the test
    if ((testid==7||testid==8))
      continue;
    if(_calibrationRabNew[0][i+((testid+1)*24)]<9000 || _calibrationRabNew[0][i+((testid+1)*24)]>11000 || std::isnan(_calibrationRabNew[0][i+((testid+1)*24)])){
      error_found=1;
    }
    if(_calibrationRabNew[1][i+((testid+1)*24)]<9000 || _calibrationRabNew[1][i+((testid+1)*24)]>11000 || std::isnan(_calibrationRabNew[1][i+((testid+1)*24)])){
      error_found=1;
    }
  }
  if ( error_found ){
    return 3;//The results returned is wrong or zero
  }

  return 0; //Succeed

}


void CalibrationEditor::_displayCurveNew(){
  QDialog predialog;
  predialog.setWindowTitle("Real or Imaginary values");
  QVBoxLayout predoptions;
  predialog.setLayout(&predoptions);
  QLabel optitle("Choose type");
  QRadioButton op1("Real values");
  op1.setChecked(true);
  QRadioButton op2("Imaginary values");
  predoptions.addWidget(&optitle);
  predoptions.addWidget(&op1);
  predoptions.addWidget(&op2);
  QSpinBox *dev = new QSpinBox;
  dev->setMinimum(0);
  dev->setMaximum(_emu->getUSBDevicesCount()-1);
  QLabel *devlabel = new QLabel;
  devlabel->setText("Device data to display");
  QHBoxLayout *devlayout = new QHBoxLayout;
  devlayout->addWidget(devlabel);
  devlayout->addWidget(dev);
  predoptions.addLayout(devlayout);
  QHBoxLayout layoutButtons;
  QPushButton ok("Ok");
  QPushButton cancel("Cancel");
  predialog.connect(&ok, SIGNAL(clicked()), &predialog, SLOT(accept()));
  predialog.connect(&cancel, SIGNAL(clicked()), &predialog, SLOT(reject()));
  layoutButtons.addWidget( &ok );
  layoutButtons.addWidget( &cancel );
  predoptions.addLayout( &layoutButtons );

  if( predialog.exec() ){
    cout<<endl;
    //Reinitialize with the correct for the selected device
    QVector <QVector<QString> > calibrationnamedatanew = _master_store.at(dev->value())->nameDataNew;
    QVector <QVector<double> > calibrationoffsetdatanew = _master_store.at(dev->value())->offsetDataNew;
    QVector <QVector<double> > calibrationgaindatanew = _master_store.at(dev->value())->gainDataNew;
    QVector <QVector<int> > calibrationidnew = _master_store.at(dev->value())->idNew;
    QVector <QVector<double> > calibrationrabnew = _master_store.at(dev->value())->rabNew;
    QVector <QVector<double> > calibrationrwnew = _master_store.at(dev->value())->rwNew;
    QVector <QVector <QVector<double> > > rawresultsnew = _master_store.at(dev->value())->rawResultsNew;
    QVector <QVector <QVector<double> > > lsqresultsnew = _master_store.at(dev->value())->lsqResultsNew;
    if (calibrationnamedatanew.size()==0)
      return;
    cout<<"Curves of device "<<dev->value()<<endl;
    QDialog dialog;
    dialog.setWindowTitle("Choose debug curve to display");
    QVBoxLayout layoutMain;
    QLabel curvtitle;
    QHBoxLayout curvlayout;
    QHBoxLayout layoutButtons;
    dialog.setLayout(&layoutMain);
    int selection;
    int realorimag;
    QComboBox curvs;
    if (op1.isChecked()){
      realorimag=0;
      curvtitle.setText("Choose debug curve of real part to display");
      curvs.addItem("None Selected");
      curvs.addItems(calibrationnamedatanew.at(0).toList());
    }
    else if (op2.isChecked()){
      realorimag=1;
      curvtitle.setText("Choose debug curve of imaginary part to display");
      curvs.addItem("None Selected");
      curvs.addItems(calibrationnamedatanew.at(1).toList());
    }
    curvlayout.addWidget(&curvtitle);
    curvlayout.addWidget(&curvs);
    layoutMain.addLayout(&curvlayout);
    QPushButton ok("Ok");
    QPushButton cancel("Cancel");
    dialog.connect(&ok, SIGNAL(clicked()), &dialog, SLOT(accept()));
    dialog.connect(&cancel, SIGNAL(clicked()), &dialog, SLOT(reject()));
    layoutButtons.addWidget( &ok );
    layoutButtons.addWidget( &cancel );
    layoutMain.addLayout( &layoutButtons );

    if( dialog.exec() ){
      if (curvs.currentIndex()==0){//none selected
        cout<<"None selected"<<endl;
        return;}
      selection=curvs.currentIndex()-1;
      cout<<"Curving finnished..."<<endl;
      cout<<calibrationnamedatanew[realorimag][selection].toStdString()<<endl;

      QVector<double> normxdata;
      QVector<double> normdata;
      QVector<double> lsqxdata;
      QVector<double> lsqdata;
      QVector<double> temprawresultsnew;
      QVector<double> temprawxresults;
      plot->detachItems(QwtPlotItem::Rtti_PlotItem);
      plot->setAxisAutoScale(QwtPlot::xBottom,true);
      plot->setAxisAutoScale(QwtPlot::yLeft,true);
      plot->repaint();
      plot->replot();
      int id;
      id=calibrationidnew.at(realorimag).at(selection);
      if (id==11){//ADC offset
        for (int i=0;i<15;++i){
          temprawxresults.append(i);
        }
        temprawresultsnew=rawresultsnew[realorimag][selection];
      }
      else if(id==10){//DAC ADC data
        double tempvoltage;
        //Parameters
        double startvoltage;
        double voltagestep;
        startvoltage=(double)858/4096*5;
        voltagestep=(double)170/4096*5;
        for (int i=0;i<15;++i){
          normxdata.append(i);
          lsqxdata.append(i);
          temprawxresults.append(i);
          tempvoltage=startvoltage+i*voltagestep;
          normdata.append(tempvoltage);
        }
        lsqdata=lsqresultsnew[realorimag][selection-24];//ADC offset doesnt have lsq so 24 less
        temprawresultsnew=rawresultsnew[realorimag][selection];
      }
      else if (id==9||id==0||id==1||id==2||id==3
               ||id==4||id==5||id==6||id==7||id==8){//rest types of data (resistors)

        if (id==9){
          lsqdata.clear();
          for (int k=0;k<3;k++){
            normxdata.append(_resCode[k]);
            lsqxdata.append(_resCode[k]);
            temprawxresults.append(_resCode[k]);
            lsqdata.append(lsqresultsnew[realorimag][selection-24][_resCode[k]]);
          }
          normdata.append(Potentiometer::r_from_tap(_resCode[0]));
          normdata.append(Potentiometer::r_from_tap(_resCode[1]));
          normdata.append(Potentiometer::r_from_tap(_resCode[2]));
          ;
        }
        else {
          lsqdata.clear();
          for (int k=3;k<6;k++){
            normxdata.append(_resCode[k]);
            lsqxdata.append(_resCode[k]);
            temprawxresults.append(_resCode[k]);
            lsqdata.append(lsqresultsnew[realorimag][selection-24][_resCode[k]]);
          }
          normdata.append(Potentiometer::r_from_tap(_resCode[3]));
          normdata.append(Potentiometer::r_from_tap(_resCode[4]));
          normdata.append(Potentiometer::r_from_tap(_resCode[5]));
        }

        temprawresultsnew=rawresultsnew[realorimag][selection];

      }

      if (_options[0]!='6'||_options[1]!='6'){
        temprawxresults.clear();
        for (int i=0;i<20;++i){
          temprawxresults.append(i);
        }
        if (_options[2]!='6'){

          for(int i=rawresultsnew[realorimag].size()-24;i<rawresultsnew[realorimag].size()  ;i++){
            temprawresultsnew=rawresultsnew[realorimag][i];
            QwtPlotCurve *curve = new QwtPlotCurve;
            QwtPointArrayData *res1 = new QwtPointArrayData(temprawxresults,temprawresultsnew);
            curve->setData(res1);
            curve->setTitle(calibrationnamedatanew[realorimag][i]);
            curve->setPen(QPen(Qt::darkMagenta,1.5));
            curve->attach(plot);
          }

        }
        if (_options[2]=='6'){
          temprawresultsnew=rawresultsnew[realorimag][selection];
          QwtPlotCurve *curve = new QwtPlotCurve;
          QwtPointArrayData *res1 = new QwtPointArrayData(temprawxresults,temprawresultsnew);
          curve->setData(res1);
          curve->setTitle(calibrationnamedatanew[realorimag][selection]);
          curve->setPen(QPen(Qt::darkMagenta,1.5));
          curve->attach(plot);
          plot->setTitle(calibrationnamedatanew[realorimag][selection]);
        }


        plot->replot();

        return;}

      //Set title of plot
      plot->setTitle(calibrationnamedatanew[realorimag][selection]);

      //The expected curve
      QwtPointArrayData *res1;
      QwtPlotCurve *curve = new QwtPlotCurve;
      if(id!=11){
        res1 = new QwtPointArrayData(normxdata,normdata);
        curve->setData(res1);
        curve->setTitle("Expected");
        curve->setPen(QPen(Qt::red,1.5));
        curve->attach(plot);}

      //The least squares plot curve
      if(id!=11){
        res1 = new QwtPointArrayData( lsqxdata,lsqdata);
        QwtPlotCurve *curve2 = new QwtPlotCurve;
        curve2->setData(res1);
        curve2->setTitle("Least Squares");
        curve2->setPen(QPen(Qt::blue,1.5));
        curve2->attach(plot);}

      //The raw results curve
      res1 = new QwtPointArrayData( temprawxresults,temprawresultsnew);
      QwtPlotCurve *curve3 = new QwtPlotCurve;
      curve3->setData(res1);
      curve3->setTitle("Raw Data");
      curve3->setPen(QPen(Qt::darkMagenta,1.5));
      curve3->attach(plot);
      plot->replot();
      if (id==11){
        offsetlabel->setText(QString ("Offset: %0").arg(calibrationoffsetdatanew[realorimag][selection]));
        gainlabel->setText(QString ("Gain: %0").arg(calibrationgaindatanew[realorimag][selection]));
        resistorlabel->clear();
      }
      else if (id==10){
        offsetlabel->setText(QString ("Offset: %0").arg(calibrationoffsetdatanew[realorimag][selection]));
        gainlabel->setText(QString ("Gain: %0").arg(calibrationgaindatanew[realorimag][selection]));
        resistorlabel->clear();
      }

      else{
        offsetlabel->setText(QString ("Offset: %0").arg(calibrationoffsetdatanew[realorimag][selection]));
        gainlabel->setText(QString ("Gain: %0").arg(calibrationgaindatanew[realorimag][selection]));
        if (id==9){resistorlabel->setText(QString ("Resistor(0|%0|%1|%2|10kohm) values: %3 | %4 | %5 | %6 | %7")
                                          .arg(Potentiometer::r_from_tap(_resCode[0])/1000,0,'f',2)
              .arg(Potentiometer::r_from_tap(_resCode[1])/1000,0,'f',2)
              .arg(Potentiometer::r_from_tap(_resCode[2])/1000,0,'f',2)
              .arg(calibrationrwnew[realorimag][selection-48])
              .arg(lsqresultsnew[realorimag][selection-24][_resCode[0]]).arg(lsqresultsnew[realorimag][selection-24][_resCode[1]])
              .arg(lsqresultsnew[realorimag][selection-24][_resCode[2]]).arg(calibrationrabnew[realorimag][selection-48]));}
        else {resistorlabel->setText(QString ("Resistor(0|%0|%1|%2|10kohm) values: %3 | %4 | %5 | %6 | %7")
                                     .arg(Potentiometer::r_from_tap(_resCode[3])/1000,0,'f',2)
              .arg(Potentiometer::r_from_tap(_resCode[4])/1000,0,'f',2)
              .arg(Potentiometer::r_from_tap(_resCode[5])/1000,0,'f',2)
              .arg(calibrationrwnew[realorimag][selection-48])
              .arg(lsqresultsnew[realorimag][selection-24][_resCode[3]]).arg(lsqresultsnew[realorimag][selection-24][_resCode[4]])
              .arg(lsqresultsnew[realorimag][selection-24][_resCode[5]]).arg(calibrationrabnew[realorimag][selection-48]));}

      }
    }
  }
}


int CalibrationEditor::_parseRawResults( size_t sliceindex ){

  int devId = _emu->sliceDeviceMap(sliceindex);
  if (devId<0) return 2;

  //Reinitialize with the correct for the selected device corresponding to the selected slice
  QVector <QVector<QString> > calibrationnamedatanew = _master_store.at(devId)->nameDataNew;
  QVector <QVector<double> > calibrationoffsetdatanew = _master_store.at(devId)->offsetDataNew;
  QVector <QVector<double> > calibrationgaindatanew = _master_store.at(devId)->gainDataNew;
  QVector <QVector<double> > calibrationrabnew = _master_store.at(devId)->rabNew;
  QVector <QVector<double> > calibrationrwnew = _master_store.at(devId)->rwNew;
  if (calibrationnamedatanew.size()==0){
    cout<<"None test was run for this device"<<endl;
    return 3;
  }
  if (calibrationnamedatanew[0].size()!=254)
    return 1;
  //We have one slice for now
  Slice *sl = &_cal_emuhw->sliceSet[sliceindex];
  int i=0;
  Atom *at;
  size_t ver, hor;
  sl->ana.size(ver,hor);
  bool updateTap(false);
  for (size_t v = 0 ; v != ver ; ++v){
    for (size_t h = 0 ; h != hor ; ++h){
      at=&sl->ana._atomSet[v][h];
      //ADC
      at->node.set_adc_offset_corr(calibrationoffsetdatanew[0][i+24],true);// +24 because the first 24 is the adc offset not the DAC/ADC test
      at->node.set_adc_offset_corr(calibrationoffsetdatanew[1][i+24],false);// +24 because the first 24 is the adc offset not the DAC/ADC test
      at->node.set_adc_gain_corr(calibrationgaindatanew[0][i+24],true);// +24 because the first 24 is the adc offset not the DAC/ADC test
      at->node.set_adc_gain_corr(calibrationgaindatanew[1][i+24],false);// +24 because the first 24 is the adc offset not the DAC/ADC test
      //Convertion Resistor
      at->node.set_real_pot_current_rab(calibrationrabnew[0][i],updateTap);
      at->node.set_imag_pot_current_rab(calibrationrabnew[1][i],updateTap);
      at->node.set_real_pot_current_rw(calibrationrwnew[0][i],updateTap);
      at->node.set_imag_pot_current_rw(calibrationrwnew[1][i],updateTap);
      //Internal resistor
      at->node.set_real_pot_resistance_rab(calibrationrabnew[0][i+24],updateTap);
      at->node.set_imag_pot_resistance_rab(calibrationrabnew[1][i+24],updateTap);
      at->node.set_real_pot_resistance_rw(calibrationrwnew[0][i+24],updateTap);
      at->node.set_imag_pot_resistance_rw(calibrationrwnew[1][i+24],updateTap);

      //P0Chip1-2 and P1Chip1-2
      at->set_embr_real_pot_far_rab(EMBRPOS_R,calibrationrabnew[0][i+48],updateTap);
      at->set_embr_imag_pot_far_rab(EMBRPOS_R,calibrationrabnew[1][i+48],updateTap);
      at->set_embr_real_pot_near_rab(EMBRPOS_R,calibrationrabnew[0][i+72],updateTap);
      at->set_embr_imag_pot_near_rab(EMBRPOS_R,calibrationrabnew[1][i+72],updateTap);

      at->set_embr_real_pot_far_rw(EMBRPOS_R,calibrationrwnew[0][i+48],updateTap);
      at->set_embr_imag_pot_far_rw(EMBRPOS_R,calibrationrwnew[1][i+48],updateTap);
      at->set_embr_real_pot_near_rw(EMBRPOS_R,calibrationrwnew[0][i+72],updateTap);
      at->set_embr_imag_pot_near_rw(EMBRPOS_R,calibrationrwnew[1][i+72],updateTap);

      //P2Chip1-2 and P3Chip1-2
      at->set_embr_real_pot_near_rab(EMBRPOS_U,calibrationrabnew[0][i+96],updateTap);
      at->set_embr_imag_pot_near_rab(EMBRPOS_U,calibrationrabnew[1][i+96],updateTap);
      at->set_embr_real_pot_far_rab(EMBRPOS_U,calibrationrabnew[0][i+120],updateTap);
      at->set_embr_imag_pot_far_rab(EMBRPOS_U,calibrationrabnew[1][i+120],updateTap);

      at->set_embr_real_pot_near_rw(EMBRPOS_U,calibrationrwnew[0][i+96],updateTap);
      at->set_embr_imag_pot_near_rw(EMBRPOS_U,calibrationrwnew[1][i+96],updateTap);
      at->set_embr_real_pot_far_rw(EMBRPOS_U,calibrationrwnew[0][i+120],updateTap);
      at->set_embr_imag_pot_far_rw(EMBRPOS_U,calibrationrwnew[1][i+120],updateTap);

      //P0P2Chip3 and P1P3Chip3
      at->set_embr_real_pot_near_rab(EMBRPOS_UR,calibrationrabnew[0][i+144],updateTap);
      at->set_embr_imag_pot_near_rab(EMBRPOS_UR,calibrationrabnew[1][i+144],updateTap);
      at->set_embr_real_pot_far_rab(EMBRPOS_UR,calibrationrabnew[0][i+168],updateTap);
      at->set_embr_imag_pot_far_rab(EMBRPOS_UR,calibrationrabnew[1][i+168],updateTap);

      at->set_embr_real_pot_near_rw(EMBRPOS_UR,calibrationrwnew[0][i+144],updateTap);
      at->set_embr_imag_pot_near_rw(EMBRPOS_UR,calibrationrwnew[1][i+144],updateTap);
      at->set_embr_real_pot_far_rw(EMBRPOS_UR,calibrationrwnew[0][i+168],updateTap);
      at->set_embr_imag_pot_far_rw(EMBRPOS_UR,calibrationrwnew[1][i+168],updateTap);

      //P0P2EXT and P1P3EXT
      if (i==1||i==2||i==3||i==4){
        at->set_embr_real_pot_near_rab(EMBRPOS_D,calibrationrabnew[0][i+192],updateTap);
        at->set_embr_imag_pot_near_rab(EMBRPOS_D,calibrationrabnew[1][i+192],updateTap);
        at->set_embr_real_pot_far_rab(EMBRPOS_D,calibrationrabnew[0][i+199],updateTap);
        at->set_embr_imag_pot_far_rab(EMBRPOS_D,calibrationrabnew[1][i+199],updateTap);

        at->set_embr_real_pot_near_rw(EMBRPOS_D,calibrationrwnew[0][i+192],updateTap);
        at->set_embr_imag_pot_near_rw(EMBRPOS_D,calibrationrwnew[1][i+192],updateTap);
        at->set_embr_real_pot_far_rw(EMBRPOS_D,calibrationrwnew[0][i+199],updateTap);
        at->set_embr_imag_pot_far_rw(EMBRPOS_D,calibrationrwnew[1][i+199],updateTap);
      }
      if (i==0){
        at->set_embr_real_pot_near_rab(EMBRPOS_L,calibrationrabnew[0][i+192],updateTap);
        at->set_embr_imag_pot_near_rab(EMBRPOS_L,calibrationrabnew[1][i+192],updateTap);
        at->set_embr_real_pot_far_rab(EMBRPOS_L,calibrationrabnew[0][i+199],updateTap);
        at->set_embr_imag_pot_far_rab(EMBRPOS_L,calibrationrabnew[1][i+199],updateTap);

        at->set_embr_real_pot_near_rw(EMBRPOS_L,calibrationrwnew[0][i+192],updateTap);
        at->set_embr_imag_pot_near_rw(EMBRPOS_L,calibrationrwnew[1][i+192],updateTap);
        at->set_embr_real_pot_far_rw(EMBRPOS_L,calibrationrwnew[0][i+199],updateTap);
        at->set_embr_imag_pot_far_rw(EMBRPOS_L,calibrationrwnew[1][i+199],updateTap);
      }
      if (i==6){//Have to take right position because EXT doesnt have 24 nodes
        at->set_embr_real_pot_near_rab(EMBRPOS_L,calibrationrabnew[0][5+192],updateTap);
        at->set_embr_imag_pot_near_rab(EMBRPOS_L,calibrationrabnew[1][5+192],updateTap);
        at->set_embr_real_pot_far_rab(EMBRPOS_L,calibrationrabnew[0][5+199],updateTap);
        at->set_embr_imag_pot_far_rab(EMBRPOS_L,calibrationrabnew[1][5+199],updateTap);

        at->set_embr_real_pot_near_rw(EMBRPOS_L,calibrationrwnew[0][5+192],updateTap);
        at->set_embr_imag_pot_near_rw(EMBRPOS_L,calibrationrwnew[1][5+192],updateTap);
        at->set_embr_real_pot_far_rw(EMBRPOS_L,calibrationrwnew[0][5+199],updateTap);
        at->set_embr_imag_pot_far_rw(EMBRPOS_L,calibrationrwnew[1][5+199],updateTap);
      }
      if (i==12){//Have to take right position because EXT doesnt have 24 nodes
        at->set_embr_real_pot_near_rab(EMBRPOS_L,calibrationrabnew[0][6+192],updateTap);
        at->set_embr_imag_pot_near_rab(EMBRPOS_L,calibrationrabnew[1][6+192],updateTap);
        at->set_embr_real_pot_far_rab(EMBRPOS_L,calibrationrabnew[0][6+199],updateTap);
        at->set_embr_imag_pot_far_rab(EMBRPOS_L,calibrationrabnew[1][6+199],updateTap);

        at->set_embr_real_pot_near_rw(EMBRPOS_L,calibrationrwnew[0][6+192],updateTap);
        at->set_embr_imag_pot_near_rw(EMBRPOS_L,calibrationrwnew[1][6+192],updateTap);
        at->set_embr_real_pot_far_rw(EMBRPOS_L,calibrationrwnew[0][6+199],updateTap);
        at->set_embr_imag_pot_far_rw(EMBRPOS_L,calibrationrwnew[1][6+199],updateTap);
      }
      ++i;
    }
  }
  return 0;
}


void CalibrationEditor::_displayCalibrationData(QVector<bool> dialogoptions, int devId){
  cout<<endl<<endl;
  //Reinitialize with the correct for the selected device
  QVector <QVector<QString> > calibrationnamedatanew = _master_store.at(devId)->nameDataNew;
  QVector <QVector<double> > calibrationoffsetdatanew = _master_store.at(devId)->offsetDataNew;
  QVector <QVector<double> > calibrationgaindatanew = _master_store.at(devId)->gainDataNew;
  QVector <QVector<int> > calibrationidnew = _master_store.at(devId)->idNew;
  QVector <QVector<double> > calibrationrabnew = _master_store.at(devId)->rabNew;
  QVector <QVector<double> > calibrationrwnew = _master_store.at(devId)->rwNew;
  QVector <QVector <QVector<double> > > rawresultsnew = _master_store.at(devId)->rawResultsNew;
  QVector <QVector <QVector<double> > > lsqresultsnew = _master_store.at(devId)->lsqResultsNew;
  if (calibrationnamedatanew.size()==0)
    return;

  cout<<"Calibration type results for device "<<devId<<endl;

  cout<<"---------------------------"<<endl;
  cout<<"------Real-----"<<endl;
  cout<<"Titles storage real: "<<calibrationnamedatanew[0].size()<<endl;
  cout<<"Offset storage real: "<<calibrationoffsetdatanew[0].size()<<endl;
  cout<<"Gain storage real: "<<calibrationgaindatanew[0].size()<<endl;
  cout<<"Id storage real: "<<calibrationidnew[0].size()<<endl;
  cout<<"rab storage real: "<<calibrationrabnew[0].size()<<endl;
  cout<<"Lowtap storage real: "<<calibrationrwnew[0].size()<<endl;
  cout<<"Least squares real: "<<lsqresultsnew[0].size()<<endl;
  cout<<"Raw results real: "<<rawresultsnew[0].size()<<endl;
  cout<<endl;
  cout<<"------Imag-----"<<endl;
  cout<<"Titles storage imag: "<<calibrationnamedatanew[1].size()<<endl;
  cout<<"Offset storage imag: "<<calibrationoffsetdatanew[1].size()<<endl;
  cout<<"Gain storage imag: "<<calibrationgaindatanew[1].size()<<endl;
  cout<<"Id storage imag: "<<calibrationidnew[1].size()<<endl;
  cout<<"rab storage imag: "<<calibrationrabnew[1].size()<<endl;
  cout<<"Lowtap storage imag: "<<calibrationrwnew[0].size()<<endl;
  cout<<"Least squares imag: "<<lsqresultsnew[1].size()<<endl;
  cout<<"Raw results imag: "<<rawresultsnew[1].size()<<endl;
  cout<<endl<<endl;
  _log->notifyProgress(10);
  if (calibrationnamedatanew[0].size()==0){
    cout<<"Empty...Run all calibration tests first"<<endl;
    return;
  }
  if (dialogoptions[0]==true&&dialogoptions[1]==true){
    for (int i=0;i<24;++i)
      cout<<"Offset real of ADC of ramp of node "<<i+1<<" is "<<calibrationoffsetdatanew[0][i]<<endl;
    cout<<endl<<endl;
  }
  else if (dialogoptions[0]==false&&dialogoptions[1]==true){
    for (int i=0;i<24;++i)
      cout<<"Offset imag of ADC of ramp of node "<<i+1<<" is "<<calibrationoffsetdatanew[1][i]<<endl;
    cout<<endl<<endl;

  }

  if (dialogoptions[0]==true&&dialogoptions[2]==true){
    for (int i=0;i<24;++i)
      cout<<"Offset real of ADC/DAC test of node "<<i+1<<" is "<<calibrationoffsetdatanew[0][i+24]<<endl;
    cout<<endl<<endl;
    for (int i=0;i<24;++i)
      cout<<"Gain real of ADC/DAC test of node "<<i+1<<" is "<<calibrationgaindatanew[0][i+24]<<endl;
    cout<<endl<<endl;
  }
  else if (dialogoptions[0]==false&&dialogoptions[2]==true){
    for (int i=0;i<24;++i)
      cout<<"Offset imag of ADC/DAC test of node "<<i+1<<" is "<<calibrationoffsetdatanew[1][i+24]<<endl;
    cout<<endl<<endl;
    for (int i=0;i<24;++i)
      cout<<"Gain imag of ADC/DAC test of node "<<i+1<<" is "<<calibrationgaindatanew[1][i+24]<<endl;
    cout<<endl<<endl;
  }
  //  if (dialogoptions[0]==true&&dialogoptions[2]==true){
  //    cout<<"Offset"<<endl;
  //    for (int i=0;i<24;++i)
  //      cout<<calibrationoffsetdatanew[0][i+24]<<endl;
  //    cout<<endl<<"Gain"<<endl;
  //    for (int i=0;i<24;++i)
  //      cout<<calibrationgaindatanew[0][i+24]<<endl;
  //    cout<<endl;
  //  }


  if (calibrationnamedatanew[0].size()<72){
    _log->notifyProgress(100);
    return;
  }
  _log->notifyProgress(30);
  if (dialogoptions[0]==true&&dialogoptions[3]==true){//The individual three values of resistor is displayed by this
    cout<<"--------Real--------"<<endl;
    for (int i=0;i<24;++i){
      cout<<"Conversion res real resistor of node "<<i+1<<" values: "<<lsqresultsnew[0][i+24][_resCode[0]]<<" ||  "<<lsqresultsnew[0][i+24][_resCode[1]]<<" ||  "<<lsqresultsnew[0][i+24][_resCode[2]]<<endl;
    }
  }
  else if (dialogoptions[0]==false&&dialogoptions[3]==true){
    cout<<"--------Imag--------"<<endl;
    for (int i=0;i<24;++i){
      cout<<"Conversion res imag resistor of node "<<i+1<<" values: "<<lsqresultsnew[1][i+24][_resCode[0]]<<" ||  "<<lsqresultsnew[1][i+24][_resCode[1]]<<" ||  "<<lsqresultsnew[1][i+24][_resCode[2]]<<endl;
    }
  }

  if (dialogoptions[0]==true&&dialogoptions[3]==true){

    for (int i=48;i<calibrationnamedatanew[0].size()-24;++i){
      cout<<calibrationnamedatanew[0][i+24].toStdString()<<" values: "<<lsqresultsnew[0][i][_resCode[3]]<<" ||  "<<lsqresultsnew[0][i][_resCode[4]]<<" ||  "<<lsqresultsnew[0][i][_resCode[5]]<<endl;
    }
    cout<<endl<<endl;
  }
  else if (dialogoptions[0]==false&&dialogoptions[3]==true){

    for (int i=48;i<calibrationnamedatanew[1].size()-24;++i){
      cout<<calibrationnamedatanew[1][i+24].toStdString()<<" values: "<<lsqresultsnew[1][i][_resCode[3]]<<" ||  "<<lsqresultsnew[1][i][_resCode[4]]<<" ||  "<<lsqresultsnew[1][i][_resCode[5]]<<endl;
    }
    cout<<endl<<endl;
  }
  _log->notifyProgress(50);
  if (dialogoptions[0]==true&&dialogoptions[5]==true){
    cout<<"   Rab values   "<<endl<<endl;
    cout<<"--------Real--------"<<endl;
    for (int p=0;p<calibrationrabnew[0].size();++p){
      cout<<calibrationnamedatanew[0][p+48].toStdString()<<"  calibrated rab= "<<calibrationrabnew[0][p]<<endl;
    }
    cout<<endl;
  }
  else if (dialogoptions[0]==false&&dialogoptions[5]==true){
    cout<<"   Rab values   "<<endl<<endl;
    cout<<"--------Imag--------"<<endl;
    for (int p=0;p<calibrationrabnew[1].size();++p){
      cout<<calibrationnamedatanew[1][p+48].toStdString()<<"  calibrated rab= "<<calibrationrabnew[1][p]<<endl;
    }
    cout<<endl;
  }
  _log->notifyProgress(60);
  if (dialogoptions[0]==true&&dialogoptions[4]==true){
    cout<<"   Lowtap values   "<<endl<<endl;
    cout<<"--------Real--------"<<endl;
    for (int p=0;p<calibrationrwnew[0].size();++p){
      cout<<calibrationnamedatanew[0][p+48].toStdString()<<"  calibrated rw= "<<calibrationrwnew[0][p]<<endl;
    }
    cout<<endl;
  }
  else if (dialogoptions[0]==false&&dialogoptions[4]==true){
    cout<<"   Lowtap values   "<<endl<<endl;
    cout<<"--------Imag--------"<<endl;
    for (int p=0;p<calibrationrwnew[1].size();++p){
      cout<<calibrationnamedatanew[1][p+48].toStdString()<<"  calibrated rw= "<<calibrationrwnew[1][p]<<endl;
    }
    cout<<endl;
  }
  _log->notifyProgress(100);
}

void CalibrationEditor::_leastSquares(QVector<double> const& data,
                                      QVector<double> const& xdata,
                                      double *alpha,
                                      double *beta){
  //Calculate the least square
  int n = data.size();
  double bot;
  int i;
  double top;
  double xbar;
  double ybar;
  xbar = 0.0;
  ybar = 0.0;
  for ( i = 0; i < n; i++ ){
    xbar = xbar + xdata[i];
    ybar = ybar + data[i];
  }
  xbar = xbar / ( double ) n;
  ybar = ybar / ( double ) n;
  top = 0.0;
  bot = 0.0;
  for ( i = 0; i < n; i++ ){
    top = top + ( xdata[i] - xbar ) * ( data[i] - ybar );
    bot = bot + ( xdata[i] - xbar ) * ( xdata[i] - xbar );
  }
  *alpha = top / bot;
  *beta = ybar - (*alpha) * xbar;
  //cout<< "Least Squares line is y = " << *alpha << " * x + " << *beta <<endl;
}



void CalibrationEditor::_resultsHandling(QVector<uint32_t> const& nodedata,
                                         int node,
                                         QVector<QVector<double> > *decodedresultsreal,
                                         QVector<QVector<double> > *decodedresultsimag,
                                         int numofSamples){
  //We add 2.5V because the results are in pu -2 +2

  //Takes 20 uint values and converts them to double
  //Results handling
  //Vector a contains signed 0x0000Q2.14 format numbers
  unsigned int bit16 = (1<<15);
  unsigned int signExpansionL = ~( (1<<16) - 1 );
  int32_t LSB;
  int32_t MSB;
  double temp_double;
  //It takes the node 1 real and imag bianry values and
  //creates 2 vectors with the double values for each
  //decodedresults[n] is the results size2vector of n node with real and imag
  for (int k = 0 ; k < numofSamples ; ++k ){
    LSB = static_cast<int32_t>( nodedata[k] & 0x0000FFFF );
    MSB = static_cast<int32_t>( nodedata[k] & 0xFFFF0000 );

    if ( LSB & bit16 ) // get sign
      LSB |= signExpansionL;
    temp_double = static_cast<double>(LSB) / (1<<14);
    (*decodedresultsreal)[node].append(temp_double+2.5);//real
    temp_double = static_cast<double>(MSB) / (1<<30);
    (*decodedresultsimag)[node].append(temp_double+2.5);//imag
  }
}

void CalibrationEditor::_offsetGainHandling( QVector<uint32_t>*encodedinput,
                                             int gainoroffset ){

  //Caution manually overriding option 4, to avoid taking double offset
  _options[4]='0';
  //typeoftest is an offset(multiple of 24) in the vector of
  //data results that point to the correct test,DAC,Internalresistor etc
  encodedinput->clear();
  double tempreal;
  double tempimag;
  uint32_t uintreal;
  uint32_t uintimag;
  uint32_t combined;
  //Convert the doubles to unint32 words
  for (int i=0;i<24;++i){
    if (gainoroffset==0){//0 for gain
      if (_options[4]!='6')
        tempreal=_calibrationGainDataNew.at(0).at(i+24);//0 for real and +24 because the first 24 is the adc offset not the DAC/ADC test
      else
        tempreal=_calibrationGainDataNew.at(0).at(i+24)+_calibrationGainDataNew.at(0).at(i);
      tempreal=1.25*(tempreal);//1.25 is the starting gain,now we have the absolut value to enter in the fpga memory
      tempreal=tempreal*(1<<10);
      uintreal=static_cast<uint32_t>(tempreal);
      if (_options[4]!='6')
        tempimag=_calibrationGainDataNew.at(1).at(i+24);//1 for imag and +24 because the first 24 is the adc offset not the DAC/ADC test
      else
        tempimag=_calibrationGainDataNew.at(1).at(i+24)+_calibrationGainDataNew.at(1).at(i);
      tempimag=1.25*(tempimag);//1.25 is the starting gain,now we have the absolut value to enter in the fpga memory
      tempimag=tempimag*(1<<10);
      uintimag=static_cast<uint32_t>(tempimag);
      uintimag=uintimag<<13;
      combined=uintimag | uintreal;

      encodedinput->push_back(combined);
    }
    else if (gainoroffset==1){//1 for offset, the offset accepts the exact 12bit value of the Vramp
      if (_options[4]!='6')
        tempreal=_calibrationOffsetDataNew.at(0).at(i+24);//0 for real and +24 because the first 24 is the adc offset not the DAC/ADC test
      else
        tempreal=_calibrationOffsetDataNew.at(0).at(i+24)+_calibrationOffsetDataNew.at(0).at(i);
      tempreal=2.5+tempreal;
      tempreal=(tempreal*4096)/5;//convert the double to the 12bit value of the ramp
      uintreal=static_cast<uint32_t>(tempreal);
      if (_options[4]!='6')
        tempimag=_calibrationOffsetDataNew.at(1).at(i+24);//1 for imag and +24 because the first 24 is the adc offset not the DAC/ADC test
      else
        tempimag=_calibrationOffsetDataNew.at(1).at(i+24)+_calibrationOffsetDataNew.at(1).at(i);
      tempimag=2.5+tempimag;
      tempimag=tempimag/5.0*4096;//convert the double to the 12bit value of the ramp
      uintimag=static_cast<uint32_t>(tempimag);
      uintimag=uintimag<<12;
      combined=uintimag | uintreal;
      encodedinput->push_back(combined);
    }
    else if (gainoroffset==3){//3 for acquiring ADC offsets, the offset accepts the exact 12bit value of the Vramp
      tempreal=_calibrationOffsetDataNew.at(0).at(i);
      tempreal=2.5+tempreal;
      tempreal=(tempreal*4096)/5;//convert the double to the 12bit value of the ramp
      uintreal=static_cast<uint32_t>(tempreal);
      tempimag=_calibrationOffsetDataNew.at(1).at(i);
      tempimag=2.5+tempimag;
      tempimag=tempimag/5.0*4096;//convert the double to the 12bit value of the ramp
      uintimag=static_cast<uint32_t>(tempimag);
      uintimag=uintimag<<12;
      combined=uintimag | uintreal;
      encodedinput->push_back(combined);
    }
  }

}

void CalibrationEditor::_softReset(){

  _calibrationNameDataNew.clear();
  _calibrationOffsetDataNew.clear();
  _calibrationGainDataNew.clear();
  _calibrationRabNew.clear();
  _calibrationRwNew.clear();
  _calibrationIdNew.clear();
  _rawResultsNew.clear();
  _lsqResultsNew.clear();
  _P3ResNew.clear();
  _P1ResNew.clear();
  gainlabel->clear();
  offsetlabel->clear();
  resistorlabel->clear();
  //Initialize the calibration storage vectors
  //The first dimensions if for the real and imag part of emulator
  for(int i=0;i<2;++i){
    _calibrationNameDataNew.append(QVector<QString>());
    _calibrationOffsetDataNew.append(QVector<double>());
    _calibrationGainDataNew.append(QVector<double>());
    _calibrationIdNew.append(QVector<int>());
    _calibrationRabNew.append(QVector <double>());
    _calibrationRwNew.append(QVector<double>());
    _rawResultsNew.append(QVector<QVector <double> >());
    _lsqResultsNew.append(QVector<QVector <double> >());
  }
  _log->notifyProgress(0);
  //Resetting
  vector<uint32_t> tempvector;
  tempvector.clear();
  tempvector.push_back(2222);
  for (size_t i=0;i<_emu->getUSBDevicesCount();i++)
    _emu->usbWrite( i, 286,tempvector);
}
