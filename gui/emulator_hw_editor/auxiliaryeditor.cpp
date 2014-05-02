
#include "auxiliaryeditor.h"
using namespace elabtsaot;

#include "auxiliary.h"
#include "encoder.h"
#include "emulator.h"
#include "tdemulator.h"
#include "moteurfengtian.h"
#include "importerexporter.h"

#include "logencodingdialog.h"
#include "rawreadfromdevicedialog.h"
#include "rawwritetodevicedialog.h"
#include "guiauxiliary.h"

#include <QFrame>
#include <QToolBar>
#include <QAction>
#include <QIcon>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QSpinBox>
#include <QPushButton>
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>

//#include <iostream>
using std::cout;
using std::endl;
//#include <vector>
using std::vector;
//#include <string>
using std::string;

AuxiliaryEditor::AuxiliaryEditor(Emulator* emu, TDEmulator* tde_hwe, MoteurFengtian* sse_fen, QWidget* parent) :
  QSplitter(Qt::Vertical, parent), _emu(emu), _tde_hwe(tde_hwe), _sse_fen(sse_fen) {

  // ---------------------------------------------------------------------------
  // ----- Global parameters group box -----
  QGroupBox* globalParamsBox = new QGroupBox("Global slice parameters", this);
  QFormLayout* globalParamsLay = new QFormLayout(globalParamsBox);
  globalParamsBox->setLayout( globalParamsLay );

  // RatioZ
  QLabel* ratioZLabel = new QLabel("RatioZ");
  ratioZForm = new QDoubleSpinBox();
  ratioZForm->setDecimals(0);
  ratioZForm->setSingleStep(1);
  ratioZForm->setRange(1., 100000.);
//  ratioZForm->setValue( _emu->ratioZ() );
  globalParamsLay->addRow( ratioZLabel, ratioZForm );

  // RatioV
  QLabel* ratioVLabel = new QLabel("RatioV");
  ratioVForm = new QDoubleSpinBox();
  ratioVForm->setDecimals(3);
  ratioVForm->setSingleStep(0.010);
  ratioVForm->setRange(0.100, 15.000);
//  ratioVForm->setValue( _emu->ratioV() );
  globalParamsLay->addRow( ratioVLabel, ratioVForm );

  // RatioI
  QLabel* ratioILabel = new QLabel("RatioI");
  ratioIForm = new QDoubleSpinBox();
  ratioIForm->setDecimals(6);
  ratioIForm->setSingleStep(0.000010);
//  ratioIForm->setValue( _emu->ratioI() );
  ratioIForm->setRange(0.000001,0.05);
  ratioIForm->setEnabled(false); // to prevent user editing this field
  globalParamsLay->addRow( ratioILabel, ratioIForm );

  // Maximum I [pu]
  QLabel* maxIpuLabel = new QLabel("Max I [pu]");
  maxIpuForm = new QDoubleSpinBox();
  //  ratioIForm->setDecimals(0);
  //  ratioIForm->setSingleStep(1);
//  maxIpuForm->setValue( _emu->maxIpu() );
  globalParamsLay->addRow( maxIpuLabel, maxIpuForm );

  // Update values
  _updtGlobals();

  // Auto ratioZ
  QLabel* autoRatioZLabel = new QLabel("Auto-scale RatioZ");
  QPushButton* autoRatioZBut = new QPushButton("Auto-scale RatioZ");
  globalParamsLay->addRow(autoRatioZLabel, autoRatioZBut);

  // Default ratios
  QLabel* defaultRatiosTDLabel = new QLabel("Default Ratios (TD)");
  QPushButton* defaultRatiosTDBut = new QPushButton("Default Ratios (TD)");
  globalParamsLay->addRow(defaultRatiosTDLabel, defaultRatiosTDBut);

  QLabel* defaultRatiosDCPFLabel = new QLabel("Default Ratios (DCPF)");
  QPushButton* defaultRatiosDCPFBut = new QPushButton("Default Ratios (DCPF)");
  globalParamsLay->addRow(defaultRatiosDCPFLabel, defaultRatiosDCPFBut);

  QLabel* getMaxRLabel = new QLabel("Get max R");
  QPushButton* getMaxRBut = new QPushButton("Get max R");
  globalParamsLay->addRow(getMaxRLabel, getMaxRBut);

  QLabel* buildGSlicesLabel = new QLabel("Build G slices");
  QPushButton* buildGSlicesBut= new QPushButton("Build G slices");
  globalParamsLay->addRow(buildGSlicesLabel,buildGSlicesBut);

  // ----------------- Connect signals -----------------
  connect( ratioZForm, SIGNAL(valueChanged(double)), this, SLOT(ratioZSlot(double)) );
  connect( ratioVForm, SIGNAL(valueChanged(double)), this, SLOT(ratioVSlot(double)) );
  connect( maxIpuForm, SIGNAL(valueChanged(double)), this, SLOT(maxIpuSlot(double)) );
  connect( autoRatioZBut, SIGNAL(clicked()), this, SLOT(autoRatioZTDSlot()) );
  connect( defaultRatiosTDBut, SIGNAL(clicked()), this, SLOT(defaultRatiosTDSlot()) );
  connect( defaultRatiosDCPFBut, SIGNAL(clicked()), this, SLOT(defaultRatiosDCPFSlot()) );
  connect( getMaxRBut, SIGNAL(clicked()), this, SLOT(getMinMaxResistorSlot()) );
  connect( buildGSlicesBut, SIGNAL(clicked()), this, SLOT(buildGSlicesSlot()) );

  // ---------------------------------------------------------------------------
  // ----- Raw usb operations group box -----
  QGroupBox* rawUsbBox = new QGroupBox("Raw USB operations", this);
  QFormLayout* rawUsbLay = new QFormLayout(rawUsbBox);
  rawUsbBox->setLayout(rawUsbLay);

  // Explicit raw read from device
  QLabel* rawReadLabel = new QLabel("Raw read from device");
  QPushButton* rawReadBut = new QPushButton("Raw read from device");
  rawUsbLay->addRow(rawReadLabel,rawReadBut);

  // Explicit raw write to device
  QLabel* rawWriteLabel = new QLabel("Raw write from device");
  QPushButton* rawWriteBut = new QPushButton("Raw write from device");
  rawUsbLay->addRow(rawWriteLabel,rawWriteBut);

  // ----------------- Connect signals -----------------
  connect(rawReadBut, SIGNAL(clicked()), this, SLOT(rawReadFromDeviceSlot()));
  connect(rawWriteBut, SIGNAL(clicked()), this, SLOT(rawWriteToDeviceSlot()));


  // ---------------------------------------------------------------------------
  // ----- Encoding operations group box -----
  QGroupBox* encodingBox = new QGroupBox("Encoding operations", this);
  QFormLayout* encodingLay = new QFormLayout(encodingBox);
  encodingBox->setLayout(encodingLay);

  // Encode (GPF) powersystem
  QLabel* encodePowersystemGPFLabel = new QLabel("Encode powersystem (GPF)");
  QPushButton* encodePowersystemGPFBut = new QPushButton("Encode powersystem (GPF)");
  encodingLay->addRow(encodePowersystemGPFLabel,encodePowersystemGPFBut);

  // Encode (DCPF) powersystem
  QLabel* encodePowersystemDCPFLabel = new QLabel("Encode powersystem (DCPF)");
  QPushButton* encodePowersystemDCPFBut = new QPushButton("Encode powersystem (DCPF)");
  encodingLay->addRow(encodePowersystemDCPFLabel,encodePowersystemDCPFBut);

  // Encode (TD) powersystem
  QLabel* encodePowersystemTDLabel = new QLabel("Encode (TD) powersystem");
  QPushButton* encodePowersystemTDBut = new QPushButton("Encode powersystem");
  encodingLay->addRow(encodePowersystemTDLabel,encodePowersystemTDBut);

  // Write encoding
  QLabel* writeEncodingLabel = new QLabel("Write encoding");
  QPushButton* writeEncodingBut = new QPushButton("Write encoding");
  encodingLay->addRow(writeEncodingLabel,writeEncodingBut);

  // Log Emulator::_encoding
  QLabel* logEncodingLabel = new QLabel("Log powersystem encoding");
  QPushButton* logEncodingBut = new QPushButton("Log powersystem encoding");
  encodingLay->addRow(logEncodingLabel,logEncodingBut);

  // Log (calibrated) got encoding
  QLabel* logGotEncodingLabel = new QLabel("Log GOT encoding");
  QPushButton* logGotEncodingBut = new QPushButton("Log GOT encoding");
  encodingLay->addRow(logGotEncodingLabel,logGotEncodingBut);

  // Import encoding
  QLabel* importEncodingLabel = new QLabel("Import encoding");
  QPushButton* importEncodingBut = new QPushButton("Import encoding");
  encodingLay->addRow(importEncodingLabel,importEncodingBut);

  // ----------------- Connect signals -----------------
  connect(encodePowersystemGPFBut, SIGNAL(clicked()), this, SLOT(encodePowersystemGPFSlot()));
  connect(encodePowersystemDCPFBut, SIGNAL(clicked()), this, SLOT(encodePowersystemDCPFSlot()));
  connect(encodePowersystemTDBut, SIGNAL(clicked()), this, SLOT(encodePowersystemTDSlot()));
  connect(writeEncodingBut, SIGNAL(clicked()), this, SLOT(writeEncodingSlot()));
  connect(logEncodingBut, SIGNAL(clicked()), this, SLOT(logPowersystemEncodingSlot()));
  connect(logGotEncodingBut, SIGNAL(clicked()), this, SLOT(logGotEncodingSlot()));
  connect(importEncodingBut, SIGNAL(clicked()), this, SLOT(importEncodingSlot()));

  // ---------------------------------------------------------------------------
  // ----- GPF group box -----
  QGroupBox* gpfBox = new QGroupBox("MoteurFengtian operations", this);
  QFormLayout* gpfLay = new QFormLayout(gpfBox);
  gpfBox->setLayout(gpfLay);

  // Reset GPF
  QLabel* resetGPFLabel = new QLabel("Reset GPF");
  QPushButton* resetGPFBut = new QPushButton("Reset GPF");
  gpfLay->addRow(resetGPFLabel,resetGPFBut);

  // ----------------- Connect signals -----------------
  connect(resetGPFBut, SIGNAL(clicked()), this, SLOT(resetGPFSlot()));

  // ---------------------------------------------------------------------------
  // ----- Toolbar -----
  QToolBar* auxiliaryEditorToolbar = new QToolBar("Emulator raw toolbar", this);
  this->addWidget( auxiliaryEditorToolbar );
  auxiliaryEditorToolbar->setFixedHeight(30);
  auxiliaryEditorToolbar->setToolTip("Please use with extreme caution!");

  // Hard-reset pressed button
  QAction* hardResetPressedAct = new QAction( QIcon(":/images/reset_red.png"),
                                              "Hard reset pressed",
                                              auxiliaryEditorToolbar );
  auxiliaryEditorToolbar->addAction( hardResetPressedAct );
  connect( hardResetPressedAct, SIGNAL(triggered()),
           this, SLOT(hardResetPressedSlot()) );

  // End calibration mode button
  QAction* endCalibrationAct = new QAction( QIcon(":/images/event.png"),
                                            "End calibration mode",
                                            auxiliaryEditorToolbar );
  auxiliaryEditorToolbar->addAction( endCalibrationAct );
  connect( endCalibrationAct, SIGNAL(triggered()),
           this, SLOT(endCalibrationSlot()) );

  // Reset emulation
  QAction* resetEmulationAct = new QAction( QIcon(":/images/event.png"),
                                            "Reset emulation",
                                            auxiliaryEditorToolbar );
  auxiliaryEditorToolbar->addAction( resetEmulationAct );
  connect( resetEmulationAct, SIGNAL(triggered()),
           this, SLOT(resetEmulationSlot()) );

  // Validate slice device assignement
  QAction* validateSliceAssignementAct = new QAction( QIcon(":/images/event.png"),
                                          "Validate slice device assignement",
                                          auxiliaryEditorToolbar );
  auxiliaryEditorToolbar->addAction( validateSliceAssignementAct );
  connect ( validateSliceAssignementAct, SIGNAL( triggered()),
            this, SLOT(validateSliceAssignementSlot()) );

  // Validate mapping
  QAction* validateMappingAct = new QAction( QIcon(":/images/event.png"),
                                             "Validate mapping",
                                             auxiliaryEditorToolbar );
  auxiliaryEditorToolbar->addAction( validateMappingAct );
  connect( validateMappingAct, SIGNAL(triggered()),
           this, SLOT( validateMappingSlot() ));

  // Validate fitting (PF)
  QAction* validateFittingPFAct = new QAction(QIcon(":/images/event.png"),
                                              "Validate fitting",
                                              auxiliaryEditorToolbar);
  auxiliaryEditorToolbar->addAction(validateFittingPFAct);
  connect(validateFittingPFAct, SIGNAL(triggered()), this, SLOT(validateFittingPFSlot()));

  // Validate fitting (TD)
  QAction* validateFittingTDAct = new QAction(QIcon(":/images/event.png"),
                                              "Validate fitting",
                                              auxiliaryEditorToolbar);
  auxiliaryEditorToolbar->addAction(validateFittingTDAct);
  connect(validateFittingTDAct, SIGNAL(triggered()), this, SLOT(validateFittingTDSlot()));

  auxiliaryEditorToolbar->addSeparator(); // -----

  // Get Emulator state
  QAction* getEmulatorHwStateAct = new QAction( QIcon(":/images/event.png"),
                                                "Get Emulator state",
                                                auxiliaryEditorToolbar);
  auxiliaryEditorToolbar->addAction( getEmulatorHwStateAct );
  connect( getEmulatorHwStateAct, SIGNAL(triggered()),
           this, SLOT(getEmulatorHwStateSlot()) );

  // Get Emulator calibration state
  QAction* getEmulatorHwCalStateAct = new QAction( QIcon(":/images/event.png"),
                                          "Get Emulator calibration state",
                                          auxiliaryEditorToolbar);
  auxiliaryEditorToolbar->addAction( getEmulatorHwCalStateAct );
  connect( getEmulatorHwCalStateAct, SIGNAL(triggered()),
           this, SLOT(getEmulatorHwCalStateSlot()) );
}


void AuxiliaryEditor::ratioZSlot(double val){
  _emu->set_ratioZ(val);
  ratioIForm->setValue(_emu->ratioI());
}
void AuxiliaryEditor::ratioVSlot(double val){
  _emu->set_ratioV(val);
  ratioIForm->setValue(_emu->ratioI());
}
void AuxiliaryEditor::maxIpuSlot(double val){
  _emu->set_maxIpu(val);
  ratioIForm->setValue(_emu->ratioI());
}
void AuxiliaryEditor::autoRatioZTDSlot(){
  _emu->autoRatioZ(EMU_OPTYPE_TD);
  _updtGlobals();
}
void AuxiliaryEditor::defaultRatiosTDSlot(){
  _emu->defaultRatios(EMU_OPTYPE_TD);
  _updtGlobals();
}
void AuxiliaryEditor::defaultRatiosDCPFSlot(){
  _emu->defaultRatios(EMU_OPTYPE_DCPF);
  _updtGlobals();
}

void AuxiliaryEditor::getMinMaxResistorSlot() const{
  cout << "Maximum achievable R: " << _emu->getMaxR() << endl;
}

void AuxiliaryEditor::buildGSlicesSlot() const{
  size_t sliceCount = _emu->getHwSliceCount();
  ublas::matrix<double,ublas::column_major> G; // temp container
  for (size_t k(0); k!=sliceCount; ++k)
    _emu->emuhw()->sliceSet[k].ana.buildG(G,true); // debug-prints results
}

void AuxiliaryEditor::rawReadFromDeviceSlot(){

  size_t devId, wordCount;
  unsigned int startAddress;
  int target;
  string mode, fname;
  RawReadFromDeviceDialog dialog( _emu->getUSBDevicesCount(),
                                  devId, startAddress, wordCount,
                                  target, mode, fname,
                                  this );
  int ans = dialog.exec();
  if ( ans ) return;

  vector<uint32_t> rddata;
  ans = _emu->usbRead( devId, startAddress, wordCount, rddata );
  if ( ans ){
    cout << "Reading device " << devId << ": "
         << wordCount << " words starting at address " << startAddress
         << " failed with code " << ans << endl;
    return;
  }
  cout << "Reading device " << devId << ": "
       << wordCount << " words starting at address " << startAddress
       << " was successful!" << endl;
//  for ( size_t k = 0; k != rddata.size(); ++k )
//    cout<<"dev "<<devId<<"$  data["<<k+startAddress<<"]: "<< rddata[k]<<endl;

  switch ( target ){
  case RawReadFromDeviceDialogTarget_toConsole:
    auxiliary::log_vector( rddata, startAddress, mode, cout );
    break;
  case RawReadFromDeviceDialogTarget_toFile:
    auxiliary::log_vector( rddata, startAddress, mode, fname );
  default:
    // Do nothing!
    break;
  }


  return;
}

void AuxiliaryEditor::rawWriteToDeviceSlot(){

  size_t devId;
  unsigned int startAddress;
  vector<uint32_t> wrdata;
  RawWriteToDeviceDialog dialog( _emu->getUSBDevicesCount(),
                                 devId, startAddress, wrdata );
  int ans = dialog.exec();
  if ( ans ) return;

  ans = _emu->usbWrite( devId, startAddress, wrdata );
  if ( ans )
    cout << "Writing to device " << devId
         << ": " << wrdata.size()
         << " words starting at address " << startAddress
         << " failed with code " << ans << endl;
  else
    cout << "Writing to device " << devId
         << ": " << wrdata.size()
         << " words starting at address " << startAddress
         << " was successful!" << endl;

  return;
}

void AuxiliaryEditor::encodePowersystemGPFSlot(){
  int ans = _emu->encodePowersystem(EMU_OPTYPE_GPF);
  if ( ans ) cout << "Encode (GPF) powersyetem failed with code " << ans << endl;
  else cout << "Encode powersyetem (GPF) was successful!" << endl;
  return;
}

void AuxiliaryEditor::encodePowersystemDCPFSlot(){
  int ans = _emu->encodePowersystem(EMU_OPTYPE_DCPF);
  if ( ans ) cout << "Encode (DCPF) powersyetem failed with code " << ans << endl;
  else cout << "Encode powersyetem (DCPF) was successful!" << endl;
  return;
}

void AuxiliaryEditor::encodePowersystemTDSlot(){
  int ans = _emu->encodePowersystem(EMU_OPTYPE_TD);
  if ( ans ) cout << "Encode (TD) powersyetem failed with code " << ans << endl;
  else cout << "Encode powersyetem (TD) was successful!" << endl;
  return;
}

void AuxiliaryEditor::writeEncodingSlot(){
  int ans = _emu->writeEncoding(false,true);
  if ( ans ) cout << "Write encoding failed with code " << ans << endl;
  else cout << "Write encoding was successful!" << endl;
  return;
}

void AuxiliaryEditor::logPowersystemEncodingSlot(){
  // Get slice count, abort if zero
  size_t sliceCount = _emu->getHwSliceCount();
  if (sliceCount==0) return;
  // Show options dialog
  size_t sliceId;
  int target;
  string mode, fname;
  LogEncodingDialog dialog( sliceCount, sliceId, target, mode, fname);
  int ans = dialog.exec();
  if (ans) return;
  // Log according to options
  if (sliceId>=_emu->encoding.size())
    cout << "No encoding for sliceId: " << sliceId << endl;
  else
  switch ( target ){
  case LogEncodingDialogTarget_toConsole:
    auxiliary::log_vector( _emu->encoding.at(sliceId), 1, mode, cout );
    break;
  case LogEncodingDialogTarget_toFile:
    auxiliary::log_vector( _emu->encoding.at(sliceId), 1, mode, fname );
    break;
  }
}

void AuxiliaryEditor::logGotEncodingSlot(){
  // Get slice count, abort if zero
  size_t sliceCount = _emu->getHwSliceCount();
  if (sliceCount==0) return;
  // Show options dialog
  size_t sliceId;
  int target;
  string mode, fname;
  LogEncodingDialog dialog( sliceCount, sliceId, target, mode, fname);
  int ans = dialog.exec();
  if (ans) return;
  // Get atomsGot encoding that was asked for
  vector<uint32_t> got_conf;
  Slice const& sl(_emu->emuhw()->sliceSet[sliceId]);
  encoder::detail::encode_atomsGot(sl,got_conf);
  // Log according to options
  switch ( target ){
  case LogEncodingDialogTarget_toConsole:
    auxiliary::log_vector( got_conf, 1, mode, cout );
    break;
  case LogEncodingDialogTarget_toFile:
    auxiliary::log_vector( got_conf, 1, mode, fname );
    break;
  }
}

void AuxiliaryEditor::importEncodingSlot(){
  int sliceId(-1);
//  int ans = guiauxiliary::askInt("Select slice",sliceId,-1,_emu->encoding.size()-1);
  int ans = guiauxiliary::askInt("Select slice",sliceId,-1,_emu->getHwSliceCount()-1);
  if (sliceId<0) return;
  QString filename = guiauxiliary::askFileName(QString("tep"),true);
  ans = io::importEncoding(filename.toStdString(), sliceId , *_emu, true);
  if (ans) cout << "Import encoding failed with code " << ans << endl;
  else cout << "Sucessfully imported encoding!" << endl;
}

void AuxiliaryEditor::hardResetPressedSlot(){
  _emu->hardResetPressed();
  cout << "Emulator structure notified for hard reset key press!" << endl;
}

void AuxiliaryEditor::endCalibrationSlot(){
  int ans = _emu->endCalibrationMode();
  if ( ans ) cout << "Ending calibration mode failed with code " << ans << endl;
  else cout << "Successfully ended calibration mode!" << endl;
}

void AuxiliaryEditor::resetEmulationSlot(){
  int ans = _tde_hwe->resetEmulation(true);
  if ( ans ) cout << "Reset emulation failed with code " << ans << endl;
  else cout << "Successfully reset the emulation!" << endl;
}

void AuxiliaryEditor::validateSliceAssignementSlot(){
  int ans = _emu->validateSliceDeviceAssignement();
  if ( ans ) cout << "Slice-device assignement validation failed with code " <<ans<<endl;
  else cout << "Slice-device assignement validation was successful!" << endl;
}

void AuxiliaryEditor::validateMappingSlot(){
  int ans = _emu->validateMapping();
  if ( ans ) cout << "Mapping validation failed with code " << ans << endl;
  else cout << "Mapping validation was successful!" << endl;
}

void AuxiliaryEditor::validateFittingPFSlot(){
  int ans = _emu->validateFitting(EMU_OPTYPE_GPF);
  if ( ans ) cout << "Validate fitting (PF) failed with code " << ans << endl;
  else cout << "Validate fitting was successful!" << endl;
}

void AuxiliaryEditor::validateFittingTDSlot(){
  int ans = _emu->validateFitting(EMU_OPTYPE_TD);
  if ( ans ) cout << "Validate fitting (TD) failed with code " << ans << endl;
  else cout << "Validate fitting was successful!" << endl;
}

void AuxiliaryEditor::getEmulatorHwStateSlot(){
  cout << "Emulator state: " << _emu->state() << endl;
}

void AuxiliaryEditor::getEmulatorHwCalStateSlot(){
  cout << "Emulator calibration state: " << _emu->state_calibration()<<endl;
}


void AuxiliaryEditor::resetGPFSlot(){
  int ans = _sse_fen->resetGPF();
  if (ans) cout << "Reset GPF failed with code " << ans << endl;
  else cout << "Successfully reset GPF!" << endl;
}

void AuxiliaryEditor::_updtGlobals(){
  ratioZForm->setValue( _emu->ratioZ() );
  ratioVForm->setValue( _emu->ratioV() );
//  ratioIForm->setValue( _emu->ratioI() );
  maxIpuForm->setValue( _emu->maxIpu() );
}
