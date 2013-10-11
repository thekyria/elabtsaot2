
#include "addplotdialog.h"
using namespace elabtsaot;

#include "physics.h"
#include "tdresultsplotproperties.h"

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QStandardItemModel>

#include <vector>
using std::vector;

AddPlotDialog::AddPlotDialog( QString& name,
                              TDResultsPlotProperties& plotProperties,
                              bool editable,
                              QWidget* parent ) :
    QObject(parent),
    _name(name),
    _plotProperties(plotProperties),
    _editable(editable) {}

int AddPlotDialog::exec(){

  // Dialog to enter parameters of the new plot
  QDialog dialog;
  dialog.setWindowTitle("New Plot");
  QVBoxLayout layoutMain;
  dialog.setLayout(&layoutMain);

  // Plot name form
  QHBoxLayout layoutName;
  layoutMain.addLayout(&layoutName);
  QLabel labelName("Name: ");
  layoutName.addWidget(&labelName);
  QLineEdit formName( "New plot" );
  layoutName.addWidget(&formName);
  formName.setText(_name);
//  formName.setReadOnly( !_editable ); // Perhaps formName should always be editable

  // Variable type - According to TDResultsVariable
  QHBoxLayout variableLay;
  layoutMain.addLayout( &variableLay );
  QLabel variableLabel("Variable: ");
  variableLay.addWidget( &variableLabel );
  variableCombo = new QComboBox( &dialog );
  variableLay.addWidget( variableCombo );
  variableCombo->addItem( QIcon(":/images/angle.png"),
    QString::fromStdString( physics::variableToString(physics::ANGLE) ),
    physics::ANGLE );
  variableCombo->addItem( QIcon(":/images/speed.png"),
    QString::fromStdString( physics::variableToString(physics::ANGULAR_SPEED) ),
    physics::ANGULAR_SPEED );
  variableCombo->addItem( QIcon(":/images/volt.png"),
    QString::fromStdString( physics::variableToString(physics::VOLTAGE) ),
    physics::VOLTAGE );
  variableCombo->addItem( QIcon(":/images/power.png"),
    QString::fromStdString( physics::variableToString(physics::POWER) ),
    physics::POWER );
  variableCombo->addItem( QIcon(":/images/ampere.png"),
    QString::fromStdString( physics::variableToString(physics::CURRENT) ),
    physics::CURRENT );
  // Set preselected combobox values according to _plotProperties
  int varId = variableCombo->findData( _plotProperties.variable );
  variableCombo->setCurrentIndex( varId );
  variableCombo->setEnabled( _editable );
  connect( variableCombo, SIGNAL(currentIndexChanged(int)),
           this, SLOT(variableChangedSlot(int)) );

  // Description
  QHBoxLayout descLay;
  layoutMain.addLayout( &descLay );
  QLabel descLabel1("Description: ");
  descLay.addWidget( &descLabel1 );
  descVal = new QLineEdit( " " );
  descLay.addWidget( descVal );
  descVal->setReadOnly( true );
  QLabel descLabel2(" measured in ");
  descLay.addWidget( &descLabel2 );
  unitsVal = new QLineEdit( " " );
  descLay.addWidget( unitsVal );
  unitsVal->setReadOnly( true );
//  unitsVal->setText( _plotProperties.units );

  // Buttons
  QHBoxLayout layoutButtons;
  layoutMain.addLayout( &layoutButtons );
  QPushButton ok("Ok");
  layoutButtons.addWidget( &ok );
  dialog.connect(&ok, SIGNAL(clicked()), &dialog, SLOT(accept()));
  QPushButton cancel("Cancel");
  layoutButtons.addWidget( &cancel );
  dialog.connect(&cancel, SIGNAL(clicked()), &dialog, SLOT(reject()));

  // In order to update the entries of the widgets
  this->variableChangedSlot( variableCombo->currentIndex() );

  if( dialog.exec() ){
    // Dialog executed properly
    _name = formName.text(); // TODO perhaps change to descVal

    bool ok = true;
    _plotProperties.variable = variableCombo->itemData( variableCombo->currentIndex() ).toInt(&ok);
    if ( !ok ) return 2;

    _plotProperties.units = unitsVal->text().toStdString();
    return 0;

  } else {
    // Dialog cancelled or not executed correctly
    return 1;
  }

  return 0;
}

void AddPlotDialog::variableChangedSlot(int newIndex){

  bool ok = true;
  int variable = variableCombo->itemData( variableCombo->currentIndex() ).toInt(&ok);
  if ( !ok ) return;
  int unit = physics::defaultUnitOfVariable( variable );

  // Change and description and units label
  descVal->setText( QString::fromStdString( physics::variableToString(variable) ) );
  unitsVal->setText( QString::fromStdString( physics::unitToString(unit) ) );

  return;
}
