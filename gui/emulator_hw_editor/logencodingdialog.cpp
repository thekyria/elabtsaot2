
#include "logencodingdialog.h"
using namespace elabtsaot;

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QSpinBox>

//#include <string>
using std::string;
//#include <map>
using std::map;

LogEncodingDialog::LogEncodingDialog( size_t sliceCount,
                                      size_t& sliceId,
                                      int& target,
                                      string& mode,
                                      string& fname ):
    _sliceCount(sliceCount),
    _sliceId(sliceId),
    _target(target),
    _mode(mode),
    _fname(fname){

  _targetOptions[LogEncodingDialogTarget_toConsole] = "To console";
  _targetOptions[LogEncodingDialogTarget_toFile] = "To file";

  _modeOptions[LogEncodingDialogMode_none] = "none";
  _modeOptions[LogEncodingDialogMode_dec] = "dec";
  _modeOptions[LogEncodingDialogMode_hex] = "hex";
  _modeOptions[LogEncodingDialogMode_both] = "both";
}

int LogEncodingDialog::exec(){

  QDialog* dialog = new QDialog();
  dialog->setWindowTitle( "Log encoding" );
  QVBoxLayout* mainLay = new QVBoxLayout();
  dialog->setLayout( mainLay );

  QHBoxLayout* sliceLay = new QHBoxLayout();
  mainLay->addLayout( sliceLay );
  QLabel* sliceLabel = new QLabel("Slice id");
  sliceLay->addWidget( sliceLabel );
  QSpinBox* sliceForm = new QSpinBox();
  sliceLay->addWidget( sliceForm );
  sliceForm->setRange( 0, _sliceCount-1 );

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

  if ( dialog->exec() ){
    // Dialog executed properly
    _sliceId = sliceForm->value();
    _target = targetCombo->itemData( targetCombo->currentIndex() ).toInt();
//   _mode=_modeOptions[modeCombo->itemData(modeCombo->currentIndex()).toInt()];
    _mode = modeCombo->itemText( modeCombo->currentIndex() ).toStdString();
    _fname = fnameLine->text().toStdString();

    return 0;

  } else {
    // Dialog failed
    return 1;
  }
  return 0;
}

void LogEncodingDialog::fnameDialog(){

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

void LogEncodingDialog::targetChanged(int targetId){

  if ( targetCombo->itemData( targetId ).toInt()
       == LogEncodingDialogTarget_toFile )
    fnameBut->setEnabled( true );
  else
    fnameBut->setEnabled( false );

  return;
}
