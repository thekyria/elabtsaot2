
#include "auxiliaryeditor.h"
using namespace elabtsaot;

#include "emulator.h"
#include "tdemulator.h"
#include "logencodingdialog.h"
#include "rawreadfromdevicedialog.h"
#include "rawwritetodevicedialog.h"
#include "auxiliary.h"

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

AuxiliaryEditor::AuxiliaryEditor(Emulator* emu, TDEmulator* tde_hwe, QWidget* parent) :
  QSplitter(Qt::Vertical, parent), _emu(emu), _tde_hwe(tde_hwe) {

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
  autoRatioZBut = new QPushButton("Auto-scale RatioZ");
  globalParamsLay->addRow(autoRatioZLabel, autoRatioZBut);

  // Default ratios
  QLabel* defaultRatiosLabel = new QLabel("Default Ratios");
  defaultRatiosBut = new QPushButton("Default Ratios");
  globalParamsLay->addRow(defaultRatiosLabel, defaultRatiosBut);

  // ----------------- Connect signals -----------------
  connect( ratioZForm, SIGNAL(valueChanged(double)),
           this, SLOT(ratioZSlot(double)) );
  connect( ratioVForm, SIGNAL(valueChanged(double)),
           this, SLOT(ratioVSlot(double)) );
  connect( maxIpuForm, SIGNAL(valueChanged(double)),
           this, SLOT(maxIpuSlot(double)) );
  connect( autoRatioZBut, SIGNAL(clicked()),
           this, SLOT(autoRatioZSlot()) );
  connect( defaultRatiosBut, SIGNAL(clicked()),
           this, SLOT(defaultRatiosSlot()) );


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

  // Validate fitting
  QAction* validateFittingAct = new QAction( QIcon(":/images/event.png"),
                                             "Validate fitting",
                                             auxiliaryEditorToolbar );
  auxiliaryEditorToolbar->addAction( validateFittingAct );
  connect( validateFittingAct, SIGNAL(triggered()),
           this, SLOT( validateFittingSlot() ));

  // Encode powersystem
  QAction* encodePowersystemAct = new QAction( QIcon(":/images/event.png"),
                                               "Encode powersystem",
                                               auxiliaryEditorToolbar );
  auxiliaryEditorToolbar->addAction( encodePowersystemAct );
  connect( encodePowersystemAct, SIGNAL(triggered()),
           this, SLOT( encodePowersystemSlot() ));

  // Write encoding
  QAction* writeEncodingAct = new QAction( QIcon(":/images/event.png"),
                                           "Write encoding",
                                           auxiliaryEditorToolbar );
  auxiliaryEditorToolbar->addAction( writeEncodingAct );
  connect( encodePowersystemAct, SIGNAL(triggered()),
           this, SLOT( writeEncodingSlot()) );

  auxiliaryEditorToolbar->addSeparator(); // -----

  // Log Emulator::_encoding
  QAction* logPowersystemEncodingAct = new QAction( QIcon(":/images/event.png"),
                                               "Log powersystem encoding",
                                               auxiliaryEditorToolbar );
  auxiliaryEditorToolbar->addAction( logPowersystemEncodingAct );
  connect( logPowersystemEncodingAct, SIGNAL(triggered()),
           this, SLOT( logPowersystemEncodingSlot() ));

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

  auxiliaryEditorToolbar->addSeparator(); // -----

  // Explicit raw read from device
  QAction* rawReadFromDeviceAct = new QAction( QIcon(":/images/event.png"),
                                              "Raw read from device",
                                              auxiliaryEditorToolbar );
  auxiliaryEditorToolbar->addAction( rawReadFromDeviceAct );
  connect( rawReadFromDeviceAct, SIGNAL(triggered()),
           this, SLOT(rawReadFromDeviceSlot()) );

  // Explicit raw write to device
  QAction* rawWriteToDeviceAct = new QAction( QIcon(":/images/event.png"),
                                              "Raw write to device",
                                              auxiliaryEditorToolbar );
  auxiliaryEditorToolbar->addAction( rawWriteToDeviceAct );
  connect( rawWriteToDeviceAct, SIGNAL(triggered()),
           this, SLOT(rawWriteToDeviceSlot()) );
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
void AuxiliaryEditor::autoRatioZSlot(){
  _emu->autoRatioZ();
  _updtGlobals();
}
void AuxiliaryEditor::defaultRatiosSlot(){
  _emu->defaultRatios();
  _updtGlobals();
}

void AuxiliaryEditor::hardResetPressedSlot(){
  _emu->hardResetPressed();
  cout << "Emulator structure notified for hard reset key press!" << endl;
  return;
}

void AuxiliaryEditor::endCalibrationSlot(){
  int ans = _emu->endCalibrationMode();
  if ( ans ) cout << "Ending calibration mode failed with code " << ans << endl;
  else cout << "Successfully ended calibration mode!" << endl;
  return;
}

void AuxiliaryEditor::resetEmulationSlot(){
  int ans = _tde_hwe->resetEmulation(true);
  if ( ans ) cout << "Reset emulation failed with code " << ans << endl;
  else cout << "Successfully reset the emulation!" << endl;
  return;
}

void AuxiliaryEditor::validateSliceAssignementSlot(){
  int ans = _emu->validateSliceDeviceAssignement();
  if ( ans ) cout << "Slice-device assignement validation failed with code " <<ans<<endl;
  else cout << "Slice-device assignement validation was successful!" << endl;
  return;
}

void AuxiliaryEditor::validateMappingSlot(){
  int ans = _emu->validateMapping();
  if ( ans ) cout << "Mapping validation failed with code " << ans << endl;
  else cout << "Mapping validation was successful!" << endl;
  return;
}

void AuxiliaryEditor::validateFittingSlot(){
  int ans = _emu->validateFitting();
  if ( ans ) cout << "Validate fitting failed with code " << ans << endl;
  else cout << "Validate fitting was successful!" << endl;
  return;
}

void AuxiliaryEditor::encodePowersystemSlot(){
  int ans = _emu->encodePowersystem();
  if ( ans ) cout << "Encode powersyetem failed with code " << ans << endl;
  else cout << "Encode powersyetem was successful!" << endl;
  return;
}

void AuxiliaryEditor::writeEncodingSlot(){
  int ans = _emu->writeEncoding(false);
  if ( ans ) cout << "Write encoding failed with code " << ans << endl;
  else cout << "Write encoding was successful!" << endl;
  return;
}

void AuxiliaryEditor::logPowersystemEncodingSlot(){

  size_t sliceCount = _emu->getHwSliceCount();
  size_t sliceId;
  int target;
  string mode, fname;
  LogEncodingDialog dialog( sliceCount, sliceId, target, mode, fname);
  int ans = dialog.exec();
  if ( ans ) return;

  switch ( target ){
  case LogEncodingDialogTarget_toConsole:
    if ( _emu->encoding.size() > sliceId ){
      vector<uint32_t> sliceEncoding = _emu->encoding.at(sliceId);
      auxiliary::log_vector( sliceEncoding, 1, mode, cout );
    }
    break;

  case LogEncodingDialogTarget_toFile:
    if ( _emu->encoding.size() > sliceId ){
      vector<uint32_t> sliceEncoding = _emu->encoding.at(sliceId);
      auxiliary::log_vector( sliceEncoding, 1, mode, fname );
    }
    break;

  default:
    // Do nothing!
    break;
  }

  return;
}

void AuxiliaryEditor::getEmulatorHwStateSlot(){
  cout << "Emulator state: " << _emu->state() << endl;
  return;
}

void AuxiliaryEditor::getEmulatorHwCalStateSlot(){
  cout << "Emulator calibration state: " << _emu->state_calibration()<<endl;
  return;
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

void AuxiliaryEditor::_updtGlobals(){
  ratioZForm->setValue( _emu->ratioZ() );
  ratioVForm->setValue( _emu->ratioV() );
//  ratioIForm->setValue( _emu->ratioI() );
  maxIpuForm->setValue( _emu->maxIpu() );
  return;
}

int AuxiliaryEditor::_rawReadFromDeviceDialog( size_t& devId,
                                               unsigned int& startAddress,
                                               size_t& wordCount ){

  QDialog* dialog = new QDialog();
  dialog->setWindowTitle( "Raw read from device" );
  QVBoxLayout* mainLay = new QVBoxLayout();
  dialog->setLayout( mainLay );

  QSpinBox* devIdForm = new QSpinBox( dialog );
  devIdForm->setRange( 0, _emu->getUSBDevicesCount()-1 );
  QSpinBox* startAddressForm = new QSpinBox( dialog );
  startAddressForm->setRange( 0, 4095 );
  QSpinBox* wordCountForm = new QSpinBox( dialog );
  wordCountForm->setRange( 0, 4095 );

  QFormLayout* formLay = new QFormLayout();
  mainLay->addLayout( formLay );
  formLay->addRow( "Device:", devIdForm );
  formLay->addRow( "Start address:", startAddressForm );
  formLay->addRow( "Word count:", wordCountForm );

  QHBoxLayout* buttonLay = new QHBoxLayout();
  mainLay->addLayout( buttonLay );
  QPushButton* okb = new QPushButton("OK");
  buttonLay->addWidget( okb );
  dialog->connect( okb, SIGNAL(clicked()), dialog, SLOT(accept()) );
  QPushButton* cancelb = new QPushButton("Cancel");
  buttonLay->addWidget( cancelb );
  dialog->connect( cancelb, SIGNAL(clicked()), dialog, SLOT(reject()) );

  if ( dialog->exec() ){
    // Dialog executed properly
    devId = devIdForm->value();
    startAddress = startAddressForm->value();
    wordCount = wordCountForm->value();
    return 0;

  } else {
    // Dialog failed
    return 1;
  }
  return 0;
}
