
#include "rawreadfromdevicedialog.h"
using namespace elabtsaot;

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QPushButton>
#include <QFormLayout>
#include <QFileDialog>
#include <QLineEdit>
#include <QLabel>
#include <QComboBox>

//#include <string>
using std::string;
//#include <map>
using std::map;

RawReadFromDeviceDialog::RawReadFromDeviceDialog( size_t deviceCount,
                                                  size_t& devId,
                                                  unsigned int& startAddress,
                                                  size_t& wordCount,
                                                  int& target,
                                                  string& mode,
                                                  string& fname,
                                                  QWidget* parent ) :
    QObject( parent ),
    _deviceCount(deviceCount),
    _devId(devId),
    _startAddress(startAddress),
    _wordCount(wordCount),
    _target(target),
    _mode(mode),
    _fname(fname){

  _targetOptions[RawReadFromDeviceDialogTarget_toConsole] = "To console";
  _targetOptions[RawReadFromDeviceDialogTarget_toFile] = "To file";

  _modeOptions[RawReadFromDeviceDialogMode_none] = "none";
  _modeOptions[RawReadFromDeviceDialogMode_dec] = "dec";
  _modeOptions[RawReadFromDeviceDialogMode_hex] = "hex";
  _modeOptions[RawReadFromDeviceDialogMode_both] = "both";
}

int RawReadFromDeviceDialog::exec(){

  // Dialog itself
  QDialog* dialog = new QDialog();
  dialog->setWindowTitle( "Raw read from device" );
  QVBoxLayout* mainLay = new QVBoxLayout();
  dialog->setLayout( mainLay );

  QSpinBox* devIdForm = new QSpinBox( dialog );
  devIdForm->setRange( 0, _deviceCount-1 );
  QSpinBox* startAddressForm = new QSpinBox( dialog );
  startAddressForm->setRange( 0, 32768 );
  QSpinBox* wordCountForm = new QSpinBox( dialog );
  wordCountForm->setRange( 1, 32768 );

  QFormLayout* formLay = new QFormLayout();
  mainLay->addLayout( formLay );
  formLay->addRow( "Device:", devIdForm );
  formLay->addRow( "Start address:", startAddressForm );
  formLay->addRow( "Word count:", wordCountForm );

  QHBoxLayout* targetLay = new QHBoxLayout();
  mainLay->addLayout( targetLay );
  QLabel* targetLabel = new QLabel("Log target");
  targetLay->addWidget( targetLabel );
  targetCombo = new QComboBox();
  targetLay->addWidget( targetCombo );
  for ( map<int,string>::const_iterator it = _targetOptions.begin() ;
        it != _targetOptions.end() ; ++it )
    targetCombo->addItem( QIcon(),
                          QString::fromStdString((*it).second),
                          QVariant((*it).first) );
  connect( targetCombo, SIGNAL(currentIndexChanged(int)),
           this, SLOT(targetChanged(int)) );

  QHBoxLayout* modeLay = new QHBoxLayout();
  mainLay->addLayout( modeLay );
  QLabel* modeLabel = new QLabel("Mode");
  modeLay->addWidget( modeLabel );
  QComboBox* modeCombo = new QComboBox();
  modeLay->addWidget( modeCombo );
  for ( map<int,string>::const_iterator it = _modeOptions.begin() ;
        it != _modeOptions.end() ; ++it )
    modeCombo->addItem( QIcon(),
                        QString::fromStdString((*it).second),
                        QVariant((*it).first) );
  modeCombo->setCurrentIndex(3);

  QHBoxLayout* fnameLay = new QHBoxLayout();
  mainLay->addLayout( fnameLay );
  QLabel* fnameLabel = new QLabel("Filename");
  fnameLay->addWidget( fnameLabel );
  fnameLine = new QLineEdit();
  fnameLay->addWidget( fnameLine );
  fnameLine->setReadOnly( true );
  fnameBut = new QPushButton();
  fnameLay->addWidget( fnameBut );
  connect( fnameBut, SIGNAL(clicked()), this, SLOT(fnameDialog()) );

  QHBoxLayout* buttonLay = new QHBoxLayout();
  mainLay->addLayout( buttonLay );
  QPushButton* okb = new QPushButton("OK");
  buttonLay->addWidget( okb );
  dialog->connect( okb, SIGNAL(clicked()), dialog, SLOT(accept()) );
  QPushButton* cancelb = new QPushButton("Cancel");
  buttonLay->addWidget( cancelb );
  dialog->connect( cancelb, SIGNAL(clicked()), dialog, SLOT(reject()) );

  // Execute dialog
  if ( dialog->exec() ){
    // Dialog executed properly
    _devId = devIdForm->value();
    _startAddress = startAddressForm->value();
    _wordCount = wordCountForm->value();
    _target = targetCombo->itemData( targetCombo->currentIndex() ).toInt();
    _mode = modeCombo->itemText( modeCombo->currentIndex() ).toStdString();
    _fname = fnameLine->text().toStdString();
    return 0;

  } else {
    // Dialog failed
    return 1;
  }
  return 0;
}

void RawReadFromDeviceDialog::fnameDialog(){

  QString ext("tec");
  QString caption = QString("Open %0 file to log encoding").arg(ext);
  QString filter = QString(".%0 files (*.%1)").arg(ext).arg(ext);

  // Get a filename for reading
  QString fileName = QFileDialog::getSaveFileName( 0, caption,
                                                   QDir::current().path(),
                                                   filter);
  if ( !fileName.isNull() ){
    fnameLine->setText( fileName );
  }

  return;
}

void RawReadFromDeviceDialog::targetChanged(int targetId){

  if ( targetCombo->itemData( targetId ).toInt()
       == RawReadFromDeviceDialogTarget_toFile )
    fnameBut->setEnabled( true );
  else
    fnameBut->setEnabled( false );

  return;
}
