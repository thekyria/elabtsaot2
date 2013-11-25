
#include "communicationeditor.h"
using namespace elabtsaot;

#include "emulator.h"
#include "guiauxiliary.h"

#include <QGroupBox>
#include <QGridLayout>
#include <QListWidget>
#include <QPushButton>
#include <QIcon>
#include <QToolBar>
#include <QAction>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QTableWidget>
#include <QList>
#include <QHeaderView>
#include <QComboBox>

//#include <iostream>
using std::cout;
using std::endl;
//#include <vector>
using std::vector;
//#include <map>
using std::map;
#include <limits>
using std::numeric_limits;

CommunicationEditor::CommunicationEditor( Emulator* emu, QWidget* parent ) :
    QSplitter(Qt::Vertical, parent), _emu(emu){

  devicesBox = new QGroupBox("Connected Devices",this);
  this->addWidget( devicesBox );

  devicesLay = new QGridLayout(devicesBox);
  devicesBox->setLayout(devicesLay);

  devicesList = new QListWidget(devicesBox);
  devicesLay->addWidget( devicesList, 0,0, 3, 1);

  QPushButton* devicesUpdateBut = new QPushButton( QIcon(":/images/update.png"),
                                                   "Update list", devicesBox);
  devicesLay->addWidget( devicesUpdateBut, 0, 1);
  connect( devicesUpdateBut, SIGNAL(clicked()),
           this, SLOT(devicesUpdateSlot()) );

  QPushButton* devicesViewBut = new QPushButton( QIcon(),
                                                 "View device", devicesBox);
  devicesLay->addWidget( devicesViewBut, 1, 1);
  connect( devicesViewBut, SIGNAL(clicked()),
           this, SLOT(devicesViewSlot()) );

  QPushButton* devicesTestBut = new QPushButton( QIcon(),
                                                 "Test device", devicesBox);
  devicesLay->addWidget( devicesTestBut, 2, 1);
  connect( devicesTestBut, SIGNAL(clicked()),
           this, SLOT(devicesTestSlot()) );

  // ---------------------------------------------------------------------------

  slicesBox = new QGroupBox("Slices to use", this);
  this->addWidget( slicesBox );

  slicesLay = new QGridLayout(slicesBox);
  slicesBox->setLayout(slicesLay);

  slicesList = new QListWidget(slicesBox);
  slicesLay->addWidget( slicesList, 0,0, 4,1);

  QPushButton* slicesSetBut = new QPushButton( QIcon(),
                                            "Set number of slices", slicesBox );
  slicesLay->addWidget( slicesSetBut, 0,1 );
  connect( slicesSetBut, SIGNAL(clicked()),
           this, SLOT(slicesSetSlot()) );

  QPushButton* slicesAssignBut = new QPushButton( QIcon(),
                                         "Assign slices to devices", slicesBox);
  slicesLay->addWidget( slicesAssignBut, 1,1 );
  connect( slicesAssignBut, SIGNAL(clicked()),
           this, SLOT(slicesAssignSlot()) );

  QPushButton* slicesClearAssignBut = new QPushButton( QIcon(),
                                               "Clear assignement", slicesBox );
  slicesLay->addWidget( slicesClearAssignBut, 2,1 );
  connect( slicesClearAssignBut, SIGNAL(clicked()),
           this, SLOT(slicesClearAssignSlot()) );

  QPushButton* slicesValidateBut = new QPushButton( QIcon() ,
                                            "Validate assignement", slicesBox );
  slicesLay->addWidget( slicesValidateBut, 3,1 );
  connect( slicesValidateBut, SIGNAL(clicked()),
           this, SLOT(slicesValidateSlot()) );

  this->addWidget( new QFrame() ); // Space frame

  // ---------------------------------------------------------------------------
  // ----- Toolbar -----
  QToolBar* communicationEditorToolbar = new QToolBar( this );
  this->addWidget( communicationEditorToolbar );

  // Update devices and slices button
  QAction* updateDevicesAndSlicesAct = new QAction( QIcon(":/images/update.png"),
                       "Update devices and slices", communicationEditorToolbar);
  communicationEditorToolbar->addAction( updateDevicesAndSlicesAct );
  connect( updateDevicesAndSlicesAct, SIGNAL(triggered()),
           this, SLOT(updateDevicesAndSlicesSlot()) );

  // Auto-assign slices to devices button
  QAction* autoAssignSlicesToDevicesAct = new QAction( QIcon(":/images/execute.png"),
                   "Auto assign slices to devices", communicationEditorToolbar);
  communicationEditorToolbar->addAction( autoAssignSlicesToDevicesAct );
  connect( autoAssignSlicesToDevicesAct, SIGNAL(triggered()),
           this, SLOT(autoAssignSlicesToDevicesSlot()) );

}

void CommunicationEditor::devicesUpdateSlot(){

  int ans = _emu->initializeUSB();
  if ( ans ){
    cout << "Updating the usb communication failed with code " << ans << endl;
  } else {
    cout << "USB communication successfully updated."  << endl;
  }

  _updateDevicesList();
  emit usbChanged();
  slicesClearAssignSlot();

  return;
}

void CommunicationEditor::devicesViewSlot(){

  if ( devicesList->currentRow() >= 0 ){
    USBDevice dev = _emu->getUSBDevices().at( devicesList->currentRow() );
    _deviceViewDialog( dev );
  }

  return;
}

void CommunicationEditor::devicesTestSlot(){

  return;
}

void CommunicationEditor::slicesSetSlot(){
  int sliceCount = _emu->getHwSliceCount();
  int ans = guiauxiliary::askInt("Set slice count", sliceCount, 0,4);
  if (ans) return;

  // ----- Update backend components -----
  ans = _emu->setSliceCount(sliceCount,EMU_OPTYPE_TD);
  if (ans){
    cout << "Updating the slice count failed with code " << ans << endl;
    return;
  }
  cout << "Slice count updated successfully!" << endl;

  // ----- Update frontend components -----
  emit emuChanged(true); // true to denote topological change in the emu
  _updateSlicesList(); // Update slicesList
}

void CommunicationEditor::slicesAssignSlot(){
  int sliceId = slicesList->currentRow();
  if ( sliceId < 0 )
    // No slice selected; nothing to do
    return;

  int devId(-1);
  int deviceCount = static_cast<int>(_emu->getUSBDevices().size());
  int ans = guiauxiliary::askInt("Assign slice to device",devId,-1,deviceCount-1);
  if (ans) return;

  ans = _emu->assignSliceToDevice( sliceId, devId );
  if ( ans ){
    cout << "Mapping slice " << sliceId << " to device " << devId << " ";
    cout << "failed with code " << ans << endl;
    return;
  } else {
    cout << "Slice " <<sliceId<< " successfully mapped to device "<<devId<<endl;
    _updateSlicesList();
  }
}

void CommunicationEditor::slicesClearAssignSlot(){

  // Update _emu->sliceDeviceMap
  int ans = _emu->clearSliceDeviceMap();
  if ( ans )
    cout << "Clear slice<->device mapping failed with code " << ans << endl;
  else
    _updateSlicesList();

  return;
}

void CommunicationEditor::slicesValidateSlot(){

  // validateSliceDeviceAssignement() validates the assignment and precalibrates
  // the voltage references of the slices based on hardcoded values from premade
  // tests
  int ans = _emu->validateSliceDeviceAssignement();
  if ( ans )
    cout << "Slice <-> Devices assignement validation failed with code: "
         << ans << endl;
  else
    cout << "Slice <-> Devices assignement validated successfully! " << endl;

  // Update fitter editor (directly bound to emulator) - to incorporate Vref
  // precalibration results
  emit emuChanged(false);

  return;
}

void CommunicationEditor::updateDevicesAndSlicesSlot(){
  devicesUpdateSlot();

  int ans = _emu->setSliceCount(_emu->getUSBDevicesCount(),EMU_OPTYPE_TD);
  if ( ans ){
    cout << "Updating the slice count failed with code " << ans << endl;
    return;
  } else {
    cout << "Slice count updated successfully!" << endl;
  }

  emit emuChanged(true); // update frontend components
  _updateSlicesList();   // update slicesList
}

void CommunicationEditor::autoAssignSlicesToDevicesSlot(){
  int ans = _emu->autoAssignSlicesToDevices();
  if ( ans )
    cout << "Auto-assignment of slices to devices failed with code " << ans << endl;
  else
    cout << "Slices succesfully auto-assigned to devices!" << endl;

  emit emuChanged(true); // update frontend components
  _updateDevicesList();
  _updateSlicesList();   // update slicesList
}

void CommunicationEditor::_updateDevicesList(){

  devicesList->clear();
  vector<USBDevice> devices = _emu->getUSBDevices();
  for ( size_t k = 0 ; k != devices.size() ; ++k ){
    QString itemLabel = QString("%1: %2 <%3> at USB address %4")
          .arg(k)
          .arg(QString::fromStdString(devices[k].deviceName))
          .arg(QString::fromStdWString(devices[k].product))
          .arg(devices[k].USBAddress);
    new QListWidgetItem(itemLabel, devicesList);
  }

  return;
}

void CommunicationEditor::_updateSlicesList(){

  map<size_t,int> sliceDeviceMap = _emu->sliceDeviceMap();
  slicesList->clear();
  for ( size_t k = 0; k != _emu->getHwSliceCount(); ++k ){
    int deviceId = sliceDeviceMap.find(k)->second;
    QString txt = QString("Slice %1 mapped to device %2").arg(k).arg(deviceId);
    new QListWidgetItem( txt, slicesList );
  }

  return;
}

int CommunicationEditor::_deviceViewDialog( USBDevice const& dev ){

  QDialog* dialog = new QDialog();
  dialog->setWindowTitle( "View device dialog" );

  // Main vertical layout
  QVBoxLayout* layMain = new QVBoxLayout();
  dialog->setLayout( layMain );

  QHBoxLayout* deviceNameLay = new QHBoxLayout();
  layMain->addLayout( deviceNameLay );
  QLabel* deviceNameLabel = new QLabel("Device name", dialog);
  deviceNameLay->addWidget( deviceNameLabel );
  QLineEdit* deviceNameVal = new QLineEdit(dialog);
  deviceNameLay->addWidget( deviceNameVal );
  deviceNameVal->setReadOnly( true );
  deviceNameVal->setText( QString::fromStdString( dev.deviceName ) );

  QHBoxLayout* friendlyNameLay = new QHBoxLayout();
  layMain->addLayout( friendlyNameLay );
  QLabel* friendlyNameLabel = new QLabel("Friendly name", dialog);
  friendlyNameLay->addWidget( friendlyNameLabel );
  QLineEdit* friendlyNameVal = new QLineEdit(dialog);
  friendlyNameLay->addWidget( friendlyNameVal );
  friendlyNameVal->setReadOnly( true );
  friendlyNameVal->setText( QString::fromStdString( dev.friendlyName ) );

  QHBoxLayout* isHighSpeedLay = new QHBoxLayout();
  layMain->addLayout( isHighSpeedLay );
  QLabel* isHighSpeedLabel = new QLabel("Is high speed", dialog);
  isHighSpeedLay->addWidget( isHighSpeedLabel );
  QCheckBox* isHighSpeedVal = new QCheckBox(dialog);
  isHighSpeedLay->addWidget( isHighSpeedVal );
  isHighSpeedVal->setEnabled( false );
  isHighSpeedVal->setChecked( dev.isHighSpeed );

  QHBoxLayout* USBVersionLay = new QHBoxLayout();
  layMain->addLayout( USBVersionLay );
  QLabel* USBVersionLabel = new QLabel("USB Version", dialog);
  USBVersionLay->addWidget( USBVersionLabel );
  QSpinBox* USBVersionVal = new QSpinBox(dialog);
  USBVersionLay->addWidget( USBVersionVal );
  USBVersionVal->setRange( numeric_limits<int>::min(), numeric_limits<int>::max() );
  USBVersionVal->setReadOnly( true );
  USBVersionVal->setValue( dev.USBVersion );

  QHBoxLayout* deviceClassLay = new QHBoxLayout();
  layMain->addLayout( deviceClassLay );
  QLabel* deviceClassLabel = new QLabel("Device class label", dialog);
  deviceClassLay->addWidget( deviceClassLabel );
  QSpinBox* deviceClassVal = new QSpinBox(dialog);
  deviceClassLay->addWidget( deviceClassVal );
  deviceClassVal->setRange( numeric_limits<int>::min(), numeric_limits<int>::max() );
  deviceClassVal->setReadOnly( true );
  deviceClassVal->setValue( dev.deviceClass );

  QHBoxLayout* deviceProtocolLay = new QHBoxLayout();
  layMain->addLayout( deviceProtocolLay );
  QLabel* deviceProtocolLabel = new QLabel("Device protocol label", dialog);
  deviceProtocolLay->addWidget( deviceProtocolLabel );
  QSpinBox* deviceProtocolVal = new QSpinBox(dialog);
  deviceProtocolLay->addWidget( deviceProtocolVal );
  deviceProtocolVal->setRange( numeric_limits<int>::min(), numeric_limits<int>::max() );
  deviceProtocolVal->setReadOnly( true );
  deviceProtocolVal->setValue( dev.deviceProtocol );

  QHBoxLayout* isOpenLay = new QHBoxLayout();
  layMain->addLayout( isOpenLay );
  QLabel* isOpenLabel = new QLabel("Is open", dialog);
  isOpenLay->addWidget( isOpenLabel );
  QCheckBox* isOpenVal = new QCheckBox(dialog);
  isOpenLay->addWidget( isOpenVal );
  isOpenVal->setEnabled( false );
  isOpenVal->setChecked( dev.isOpen );

  QHBoxLayout* manufacturerLay = new QHBoxLayout();
  layMain->addLayout( manufacturerLay );
  QLabel* manufacturerLabel = new QLabel("Manufacturer label", dialog);
  manufacturerLay->addWidget( manufacturerLabel );
  QLineEdit* manufacturerVal = new QLineEdit(dialog);
  manufacturerLay->addWidget( manufacturerVal );
  manufacturerVal->setReadOnly( true );
  manufacturerVal->setText( QString::fromStdWString( dev.manufacturer ) );

  QHBoxLayout* maxPowerLay = new QHBoxLayout();
  layMain->addLayout( maxPowerLay );
  QLabel* maxPowerLabel = new QLabel("Max power label", dialog);
  maxPowerLay->addWidget( maxPowerLabel );
  QSpinBox* maxPowerVal = new QSpinBox(dialog);
  maxPowerLay->addWidget( maxPowerVal );
  maxPowerVal->setRange( numeric_limits<int>::min(), numeric_limits<int>::max() );
  maxPowerVal->setReadOnly( true );
  maxPowerVal->setValue( dev.maxPower );

  QHBoxLayout* powerStateLay = new QHBoxLayout();
  layMain->addLayout( powerStateLay );
  QLabel* powerStateLabel = new QLabel("Power state label", dialog);
  powerStateLay->addWidget( powerStateLabel );
  QSpinBox* powerStateVal = new QSpinBox(dialog);
  powerStateLay->addWidget( powerStateVal );
  powerStateVal->setRange( numeric_limits<int>::min(), numeric_limits<int>::max() );
  powerStateVal->setReadOnly( true );
  powerStateVal->setValue( dev.powerState );

  QHBoxLayout* productLay = new QHBoxLayout();
  layMain->addLayout( productLay );
  QLabel* productLabel = new QLabel("Product label", dialog);
  productLay->addWidget( productLabel );
  QLineEdit* productVal = new QLineEdit(dialog);
  productLay->addWidget( productVal );
  productVal->setReadOnly( true );
  productVal->setText( QString::fromStdWString( dev.product ) );

  QHBoxLayout* productIdLay = new QHBoxLayout();
  layMain->addLayout( productIdLay );
  QLabel* productIdLabel = new QLabel("Product ID label", dialog);
  productIdLay->addWidget( productIdLabel );
  QSpinBox* productIdVal = new QSpinBox(dialog);
  productIdLay->addWidget( productIdVal );
  productIdVal->setRange( numeric_limits<int>::min(), numeric_limits<int>::max() );
  productIdVal->setReadOnly( true );
  productIdVal->setValue( dev.productId );

  QHBoxLayout* serialNumberLay = new QHBoxLayout();
  layMain->addLayout( serialNumberLay );
  QLabel* serialNumberLabel = new QLabel("Serial number label", dialog);
  serialNumberLay->addWidget( serialNumberLabel );
  QLineEdit* serialNumberVal = new QLineEdit(dialog);
  serialNumberLay->addWidget( serialNumberVal );
  serialNumberVal->setReadOnly( true );
  serialNumberVal->setText( QString::fromStdWString( dev.serialNumber ) );

  QHBoxLayout* languageIdLay = new QHBoxLayout();
  layMain->addLayout( languageIdLay );
  QLabel* languageIdLabel = new QLabel("Language ID label", dialog);
  languageIdLay->addWidget( languageIdLabel );
  QSpinBox* languageIdVal = new QSpinBox(dialog);
  languageIdLay->addWidget( languageIdVal );
  languageIdVal->setRange( numeric_limits<int>::min(), numeric_limits<int>::max() );
  languageIdVal->setReadOnly( true );
  languageIdVal->setValue( dev.languageId );

  QHBoxLayout* USBAddressLay = new QHBoxLayout();
  layMain->addLayout( USBAddressLay );
  QLabel* USBAddressLabel = new QLabel("USB address label", dialog);
  USBAddressLay->addWidget( USBAddressLabel );
  QSpinBox* USBAddressVal = new QSpinBox(dialog);
  USBAddressLay->addWidget( USBAddressVal );
  USBAddressVal->setRange( numeric_limits<int>::min(), numeric_limits<int>::max() );
  USBAddressVal->setReadOnly( true );
  USBAddressVal->setValue( dev.USBAddress );

  QHBoxLayout* USBControlerDriverLay = new QHBoxLayout();
  layMain->addLayout( USBControlerDriverLay );
  QLabel* USBControlerDriverLabel = new QLabel("USB controller driver label", dialog);
  USBControlerDriverLay->addWidget( USBControlerDriverLabel );
  QSpinBox* USBControlerDriverVal = new QSpinBox(dialog);
  USBControlerDriverLay->addWidget( USBControlerDriverVal );
  USBControlerDriverVal->setRange( numeric_limits<int>::min(), numeric_limits<int>::max() );
  USBControlerDriverVal->setReadOnly( true );
  USBControlerDriverVal->setValue( dev.USBControlerDriver );

  QHBoxLayout* vendorIdLay = new QHBoxLayout();
  layMain->addLayout( vendorIdLay );
  QLabel* vendorIdLabel = new QLabel("Vendor ID label", dialog);
  vendorIdLay->addWidget( vendorIdLabel );
  QSpinBox* vendorIdVal = new QSpinBox(dialog);
  vendorIdLay->addWidget( vendorIdVal );
  vendorIdVal->setRange( numeric_limits<int>::min(), numeric_limits<int>::max() );
  vendorIdVal->setReadOnly( true );
  vendorIdVal->setValue( dev.vendorId );

  // Endpoints display
  QLabel* epLabel = new QLabel("Endpoints");
  layMain->addWidget( epLabel );
  QTableWidget* epTable = new QTableWidget(dialog);
  layMain->addWidget( epTable );
  epTable->setRowCount( dev.endpoints.size() );
  epTable->setColumnCount( 7 );
  QList<QString> epLabelList;
  epLabelList.push_back( QString("#") );
  epLabelList.push_back( QString("Address") );
  epLabelList.push_back( QString("Attributes") );
  epLabelList.push_back( QString("Is in") );
  epLabelList.push_back( QString("Max packet size") );
  epLabelList.push_back( QString("Last err code") );
  epLabelList.push_back( QString("Timeout") );
  QStringList* epLabels = new QStringList( epLabelList );
  epTable->setHorizontalHeaderLabels( *epLabels );
  epTable->verticalHeader()->hide();
  epTable->setSelectionMode( QAbstractItemView::SingleSelection );
  epTable->setEditTriggers( QAbstractItemView::DoubleClicked );
  epTable->setSelectionBehavior( QAbstractItemView::SelectItems );
  for (size_t k = 0; k != dev.endpoints.size(); ++k ){

    QTableWidgetItem* epIdItem;
    epIdItem = new QTableWidgetItem( QString::number(k) );
    epIdItem->setFlags( Qt::ItemIsSelectable );
    epTable->setItem( k , 0, epIdItem);

    QTableWidgetItem* epAddressItem;
    epAddressItem = new QTableWidgetItem( QString::number(dev.endpoints[k].address) );
    epAddressItem->setFlags( Qt::ItemIsSelectable );
    epTable->setItem( k , 1, epAddressItem);

    QTableWidgetItem* epAttributesItem;
    epAttributesItem = new QTableWidgetItem( QString::number(dev.endpoints[k].attributes) );
    epAttributesItem->setFlags( Qt::ItemIsSelectable );
    epTable->setItem( k , 2, epAttributesItem);

    QTableWidgetItem* epIsInItem;
    epIsInItem = new QTableWidgetItem();
//    epIsInItem = new QTableWidgetItem( QString::number(dev.endpoints[k].isIn) );
    epIsInItem->setCheckState( dev.endpoints[k].isIn ? Qt::Checked : Qt::Unchecked );
    epIsInItem->setFlags( Qt::ItemIsSelectable );
    epTable->setItem( k , 3, epIsInItem);

    QTableWidgetItem* epMaxPacketSizeItem;
    epMaxPacketSizeItem = new QTableWidgetItem( QString::number(dev.endpoints[k].maxPacketSize) );
    epMaxPacketSizeItem->setFlags( Qt::ItemIsSelectable );
    epTable->setItem( k , 4, epMaxPacketSizeItem);

    QTableWidgetItem* epLastErrorCodeItem;
    epLastErrorCodeItem = new QTableWidgetItem( QString::number(dev.endpoints[k].lastErrorCode) );
    epLastErrorCodeItem->setFlags( Qt::ItemIsSelectable );
    epTable->setItem( k , 5, epLastErrorCodeItem);

    QTableWidgetItem* epTimeoutItem;
    epTimeoutItem = new QTableWidgetItem( QString::number(dev.endpoints[k].timeout) );
//    epTimeoutItem->setFlags( Qt::ItemIsEditable );
    epTable->setItem( k , 6, epTimeoutItem);
  }
  epTable->resizeColumnsToContents();

  // Alternative display for the endpoints
//  QGroupBox* epBox = new QGroupBox("Device endpoints", dialog);
//  layMain->addWidget( epBox );
//  QVBoxLayout* epLay = new QVBoxLayout();
//  epBox->setLayout( epLay );

//  for (size_t k = 0; k != dev.endpoints.size(); ++k ){
//    QLabel* epLabel = new QLabel( QString("Endpoint %1").arg(k), epBox );
//    epLay->addWidget( epLabel );

//    QHBoxLayout* epAddressLay = new QHBoxLayout();
//    epLay->addLayout( epAddressLay );
//    QLabel* epAddressLabel = new QLabel("Address", epBox);
//    epAddressLay->addWidget( epAddressLabel );
//    QSpinBox* epAddressVal = new QSpinBox(epBox);
//    epAddressLay->addWidget( epAddressVal );
//    epAddressVal->setReadOnly( true );
//    epAddressVal->setValue( dev.endpoints[k].address );

//    QHBoxLayout* epAttributesLay = new QHBoxLayout();
//    epLay->addLayout( epAttributesLay );
//    QLabel* epAttributesLabel = new QLabel("Attributes", epBox);
//    epAttributesLay->addWidget( epAttributesLabel );
//    QSpinBox* epAttributesVal = new QSpinBox(epBox);
//    epAttributesLay->addWidget( epAttributesVal );
//    epAttributesVal->setReadOnly( true );
//    epAttributesVal->setValue( dev.endpoints[k].attributes );

//    QHBoxLayout* epIsInLay = new QHBoxLayout();
//    epLay->addLayout( epIsInLay );
//    QLabel* epIsInLabel = new QLabel("In in", epBox);
//    epIsInLay->addWidget( epIsInLabel );
//    QCheckBox* epIsInVal = new QCheckBox(epBox);
//    epIsInLay->addWidget( epIsInVal );
//    epIsInVal->setEnabled( true );
//    epIsInVal->setChecked( dev.endpoints[k].isIn );

//    QHBoxLayout* epMaxPacketSizeLay = new QHBoxLayout();
//    epLay->addLayout( epMaxPacketSizeLay );
//    QLabel* epMaxPacketSizeLabel = new QLabel("Max packet size", epBox);
//    epMaxPacketSizeLay->addWidget( epMaxPacketSizeLabel );
//    QSpinBox* epMaxPacketSizeVal = new QSpinBox(epBox);
//    epMaxPacketSizeLay->addWidget( epMaxPacketSizeVal );
//    epMaxPacketSizeVal->setReadOnly( true );
//    epMaxPacketSizeVal->setValue( dev.endpoints[k].maxPacketSize );

//    QHBoxLayout* epLastErrorCodeLay = new QHBoxLayout();
//    epLay->addLayout( epLastErrorCodeLay );
//    QLabel* epLastErrorCodeLabel = new QLabel("Last error code", epBox);
//    epLastErrorCodeLay->addWidget( epLastErrorCodeLabel );
//    QSpinBox* epLastErrorCodeVal = new QSpinBox(epBox);
//    epLastErrorCodeLay->addWidget( epLastErrorCodeVal );
//    epLastErrorCodeVal->setReadOnly( true );
//    epLastErrorCodeVal->setValue( dev.endpoints[k].lastErrorCode );

//    QHBoxLayout* epTimeoutLay = new QHBoxLayout();
//    epLay->addLayout( epTimeoutLay );
//    QLabel* epTimeoutLabel = new QLabel("Timeout", epBox);
//    epTimeoutLay->addWidget( epTimeoutLabel );
//    QSpinBox* epTimeoutVal = new QSpinBox(epBox);
//    epTimeoutLay->addWidget( epTimeoutVal );
//    epTimeoutVal->setValue( dev.endpoints[k].timeout );
//  }

  // Horizonal button layout
  QHBoxLayout* layButtons = new QHBoxLayout();
  layMain->addLayout( layButtons );
  // Ok button
  QPushButton* ok = new QPushButton("Ok");
  layButtons->addWidget( ok );
  dialog->connect( ok , SIGNAL(clicked()), dialog, SLOT(accept()) );
  // Cancel button
  QPushButton* cancel = new QPushButton("Cancel");
  layButtons->addWidget( cancel );
  dialog->connect( cancel, SIGNAL(clicked()), dialog, SLOT(reject()) );

  // Excecute dialog
  if ( dialog->exec() ){

    // Find out whether timeout values have been changed
    for ( size_t k = 0; k != dev.endpoints.size(); ++k ){
      bool ok;
      int newTimeoutVal = epTable->item( k, 6)->text().toInt(&ok);
      if ( !ok || (newTimeoutVal < 0) ){
        cout << "Timeout value for endpoint " << k;
        cout << " set to invalid value!" << endl;
        continue;
      }

      if( static_cast<unsigned long>(newTimeoutVal) == dev.endpoints[k].timeout)
        // No change in timeout value
        continue;

      // TODO: the following doesn't work
      // Timeout value has been changed by the user
//      int ans = 0;
//      ans |= emu->usb()->set_endpoint_timeout( devId, k,
//                            static_cast<unsigned long>(newTimeoutVal) );
//      if ( ans ){
//        cout << "Changing timeout value for device " <<devId<< ", endpoint "<<k;
//        cout << " failed with code " << ans << endl;
//      } else {
//        cout << "Timeout value for device " << devId << ", endpoint " << k;
//        cout << " successfully changed to " << newTimeoutVal << endl;
//      }
    }
    return 0;
  } else{
    // Dialog cancelled or not executed correctly
    return 1;
  }

  return 0;
}
