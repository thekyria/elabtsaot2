
#include "resultsrenamedialog.h"
using namespace elabtsaot;

#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

//#include <string>
using std::string;

ResultsRenameDialog::ResultsRenameDialog( string& newTitle,
                                          QWidget* parent ) :
    QObject(parent), _newTitle(newTitle) {}

int ResultsRenameDialog::exec(){

  // Dialog itself
  QDialog* dialog = new QDialog();
  dialog->setWindowTitle( "Rename td results" );
  QVBoxLayout* mainLay = new QVBoxLayout();
  dialog->setLayout( mainLay );

  // Old title
  QHBoxLayout* oldTitleLay = new QHBoxLayout();
  mainLay->addLayout( oldTitleLay );
  QLabel* oldTitleLabel = new QLabel( "Old Title", dialog );
  oldTitleLay->addWidget( oldTitleLabel );
  QLineEdit* oldTitleForm = new QLineEdit( QString::fromStdString( _newTitle ),
                                           dialog);
  oldTitleLay->addWidget( oldTitleForm );
  oldTitleForm->setEnabled( false );

  // Old title
  QHBoxLayout* newTitleLay = new QHBoxLayout();
  mainLay->addLayout( newTitleLay );
  QLabel* newTitleLabel = new QLabel( "New Title", dialog );
  newTitleLay->addWidget( newTitleLabel );
  QLineEdit* newTitleForm = new QLineEdit( QString::fromStdString( _newTitle ),
                                           dialog);
  newTitleLay->addWidget( newTitleForm );
//  newTitleForm->setEnabled( true );

  // Buttons
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
    _newTitle = newTitleForm->text().toStdString();
    return 0;

  } else {
    // Dialog failed
    return 2;
  }
  return 0;
}
