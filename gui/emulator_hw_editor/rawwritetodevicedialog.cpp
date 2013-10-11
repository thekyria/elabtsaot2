
#include "rawwritetodevicedialog.h"
using namespace elabtsaot;

#include "auxiliary.h"

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QPushButton>
#include <QScrollBar>
#include <QFormLayout>
#include <QSplitter>
#include <QDoubleSpinBox>
#include <QLineEdit>

//#include <vector>
using std::vector;
//#include <limits>
//using std::numeric_limits;

RawWriteToDeviceDialog::RawWriteToDeviceDialog( size_t deviceCount,
                                                size_t& devId,
                                                unsigned int& startAddress,
                                                vector<uint32_t>& wrdata,
                                                QWidget* parent ) :
  QObject(parent),
  _deviceCount(deviceCount),
  _devId(devId),
  _startAddress(startAddress),
  _wrdata(wrdata){}

int RawWriteToDeviceDialog::exec(){

  // Cannot open dialog when there are no devices to write to
  if ( _deviceCount <= 0 )
    return 1;

  // Dialog itself
  QDialog* dialog = new QDialog();
  dialog->setWindowTitle( "Raw write to device" );
  QVBoxLayout* mainLay = new QVBoxLayout();
  dialog->setLayout( mainLay );

  QSpinBox* devIdForm = new QSpinBox( dialog );
  devIdForm->setRange( 0, _deviceCount-1 );
  startAddressForm = new QSpinBox( dialog );
  startAddressForm->setRange( 0, 32768 );
  QSpinBox* wordCountForm = new QSpinBox( dialog );
  wordCountForm->setRange( 0, 32768 );
  wordCountForm->findChild<QLineEdit*>()->setReadOnly(true);
  dialog->connect( wordCountForm, SIGNAL(valueChanged(int)),
                   this, SLOT(wordCountChanged(int)) );
  QFormLayout* formLay = new QFormLayout();
  mainLay->addLayout( formLay );
  formLay->addRow( "Device:", devIdForm );
  formLay->addRow( "Start address:", startAddressForm );
  formLay->addRow( "Word count:", wordCountForm );

  QFrame* wrdataFrame = new QFrame();
  mainLay->addWidget( wrdataFrame );
//  QScrollBar* wrdataSB = new QScrollBar( Qt::Vertical, wrdataFrame );
  wrdataLay = new QFormLayout();
  wrdataFrame->setLayout( wrdataLay );

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
    _devId = devIdForm->value();
    _startAddress = startAddressForm->value();
    _wrdata.clear();
    for ( size_t k = 0 ; k != wrdataForm.size() ; ++k )
      _wrdata.push_back( static_cast<uint32_t>(wrdataForm[k]->value()) );

    return 0;

  } else {
    // Dialog failed
    return 2;
  }
  return 0;
}

void RawWriteToDeviceDialog::wordCountChanged( int val ){

  // Clear wrdataForm QSpinBoxes
  wrdataForm.clear();
  // Take all widgets from wrdataLay QFormLayout
  auxiliary::clearQLayout( wrdataLay );

  // Rebuild wrdataForm and wrdataLay
  for ( int k = 0 ; k != val ; ++k ){
    QDoubleSpinBox* temp = new QDoubleSpinBox();
    // The following for a QSpinBox (int)
//    temp->setRange( 0, 2147483647 ); // 0b01111111111111111111111111111111
//    temp->setRange( 0, 4294967295 ); // 0b11111111111111111111111111111111
    // The following for a QDoubleSpinBox (double)
    temp->setRange( 0., 4294967295. );
    temp->setDecimals( 0 );
    temp->setSingleStep( 1 );
    wrdataForm.push_back( temp );
    wrdataLay->addRow( QString("[%1]").arg(startAddressForm->value()+k),
                       wrdataForm[k] );
  }

  return;
}
