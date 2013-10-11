
#include "autogeneratescenariosdialog.h"
using namespace elabtsaot;

#include <QDialog>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QDoubleSpinBox>

#define FAULTSTART   2.0 // set to 0.0, for faster scenarios
#define FAULTSTOP    2.2 // set to 0.2, for faster scenarios

AutoGenerateScenariosDialog::
AutoGenerateScenariosDialog( double& scenarioStopTime,
                             bool& trip,
                             bool& ignoreLowZBranches,
                             double& faultLocation,
                             double& faultStart,
                             double& faultStop,
                             QWidget* parent ) :
  QObject(parent), _scenarioStopTime(scenarioStopTime),
  _trip(trip), _ignoreLowZBranches(ignoreLowZBranches),
  _faultLocation(faultLocation),
  _faultStart(faultStart),
  _faultStop(faultStop){}

int AutoGenerateScenariosDialog::exec(){

  QDialog* dialog = new QDialog();
  dialog->setWindowTitle("Auto generate scenarios");
  QVBoxLayout* mainLay = new QVBoxLayout();
  dialog->setLayout( mainLay );

  // Scenario stop time
  QHBoxLayout* sceStopLay( new QHBoxLayout() );
  mainLay->addLayout( sceStopLay );
  QLabel* sceStopLabel( new QLabel("Scenario stop [sec]") );
  sceStopLay->addWidget( sceStopLabel );
  sceStopForm = new QDoubleSpinBox(dialog);
  sceStopLay->addWidget( sceStopForm );
  sceStopForm->setDecimals( 3 );
  sceStopForm->setRange( 0.001,100.000 );
  sceStopForm->setValue( 4.000 );
  connect( sceStopForm, SIGNAL(valueChanged(double)),
           this, SLOT(sceStopChanged(double)) );

  // Trip
  QHBoxLayout* tripLay( new QHBoxLayout() );
  mainLay->addLayout( tripLay );
  QLabel* tripLabel( new QLabel("Trip", dialog) );
  tripLay->addWidget( tripLabel );
  QCheckBox* tripCheck( new QCheckBox(dialog) );
  tripLay->addWidget( tripCheck );
  tripCheck->setChecked( false );

  // Ignore low-Z branches
  QHBoxLayout* lowZLay( new QHBoxLayout() );
  mainLay->addLayout( lowZLay );
  QLabel* lowZLabel( new QLabel("Ignore low-Z branches", dialog) );
  lowZLay->addWidget( lowZLabel );
  QCheckBox* lowZCheck( new QCheckBox(dialog) );
  lowZLay->addWidget( lowZCheck );
  lowZCheck->setChecked( true );

  // Fault location
  QHBoxLayout* faultLocationLay( new QHBoxLayout() );
  mainLay->addLayout( faultLocationLay );
  QLabel* faultLocationLabel( new QLabel("Fault location", dialog) );
  faultLocationLay->addWidget( faultLocationLabel );
  QCheckBox* faultLocationRandomCheck( new QCheckBox("Random", dialog) );
  faultLocationLay->addWidget( faultLocationRandomCheck );
  faultLocationRandomCheck->setChecked(false);
  connect( faultLocationRandomCheck, SIGNAL(clicked(bool)),
           this, SLOT(randomFaultLocationChecked(bool)));
  faultLocationForm = new QDoubleSpinBox(dialog);
  faultLocationLay->addWidget( faultLocationForm );
  faultLocationForm->setDecimals( 3 );
  faultLocationForm->setRange( 0.0, 1.0 );
  faultLocationForm->setValue( 0.5 );

  // Fault start
  QHBoxLayout* faultStartLay( new QHBoxLayout() );
  mainLay->addLayout( faultStartLay );
  QLabel* faultStartLabel( new QLabel("Fault Start [sec]", dialog) );
  faultStartLay->addWidget( faultStartLabel );
  QCheckBox* faultStartRandomCheck( new QCheckBox("Random", dialog) );
  faultStartLay->addWidget( faultStartRandomCheck );
  faultStartRandomCheck->setChecked(false);
  connect( faultStartRandomCheck, SIGNAL(clicked(bool)),
           this, SLOT(randomFaultStartChecked(bool)));
  faultStartForm = new QDoubleSpinBox(dialog);
  faultStartLay->addWidget( faultStartForm );
  faultStartForm->setDecimals( 3 );
  faultStartForm->setRange( 0.000,FAULTSTOP );
  faultStartForm->setValue( FAULTSTART );

  // Fault Stop
  QHBoxLayout* faultStopLay( new QHBoxLayout() );
  mainLay->addLayout( faultStopLay );
  QLabel* faultStopLabel( new QLabel("Fault Stop [sec]", dialog) );
  faultStopLay->addWidget( faultStopLabel );
  QCheckBox* faultStopRandomCheck( new QCheckBox("Random", dialog) );
  faultStopLay->addWidget( faultStopRandomCheck );
  faultStopRandomCheck->setChecked(false);
  connect( faultStopRandomCheck, SIGNAL(clicked(bool)),
           this, SLOT(randomFaultStopChecked(bool)));
  faultStopForm = new QDoubleSpinBox(dialog);
  faultStopLay->addWidget( faultStopForm );
  faultStopForm->setDecimals( 3 );
  faultStopForm->setRange( 0.001,4.000 );
  faultStopForm->setValue( FAULTSTOP );
  connect( faultStopForm, SIGNAL(valueChanged(double)),
           this, SLOT(faultStopChanged(double)) );

  // Ok & Cancel buttons
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
    _scenarioStopTime = sceStopForm->value();
    _trip = tripCheck->isChecked();
    _ignoreLowZBranches = lowZCheck->isChecked();
    if ( faultLocationRandomCheck->isChecked() )
      _faultLocation = -1;
    else
      _faultLocation = faultLocationForm->value();
    if ( faultStartRandomCheck->isChecked() )
      _faultStart = -1;
    else
      _faultStart = faultStartForm->value();
    if ( faultStopRandomCheck->isChecked() )
      _faultStop = -1;
    else
      _faultStop = faultStopForm->value();

    return 0;

  } else {
    // Dialog failed
    return 1;
  }
  return 0;
}

void AutoGenerateScenariosDialog::sceStopChanged(double val){
  if ( faultStopForm->value() > val )
    faultStopForm->setValue(val);
  faultStopForm->setMaximum( val );
  return;
}

void AutoGenerateScenariosDialog::faultStopChanged(double val){
  if ( faultStartForm->value() > val )
    faultStartForm->setValue(val);
  faultStartForm->setMaximum( val );
}

void AutoGenerateScenariosDialog::randomFaultLocationChecked(bool checked){
  faultLocationForm->setEnabled(!checked);
  return;
}

void AutoGenerateScenariosDialog::randomFaultStartChecked(bool checked){
  faultStartForm->setEnabled(!checked);
  return;
}

void AutoGenerateScenariosDialog::randomFaultStopChecked(bool checked){
  faultStopForm->setEnabled(!checked);
  return;
}
