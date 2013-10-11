
#include "sidepane.h"
using namespace elabtsaot;

#include <QPushButton>
#include <QFrame>
#include <QIcon>
#include <QLabel>

SidePane::SidePane(QWidget *parent) :
    QSplitter(Qt::Vertical, parent), _isHwShown(true){

  // Spacing Widget
  setFixedWidth(100);
  QWidget* space = new QWidget();
  space->setFixedWidth(100);
  space->setFixedHeight(3);
  addWidget(space);

  // ----- General buttons -----
  // Splash button
  buttonSplash = new QPushButton( QIcon(":/images/start.png"),
                                  "Start" , this );
  addWidget(buttonSplash);
  buttonSplash->setFixedWidth(100);
  buttonSplash->setFixedHeight(40);
  connect( buttonSplash, SIGNAL(clicked()),
           this, SIGNAL(buttonSplash_clicked()) );

  // Powersystem editor button
  buttonPowersystem = new QPushButton( QIcon(":/images/schematic.png"),
                                       "Powersystem", this);
  addWidget(buttonPowersystem);
  buttonPowersystem->setFixedWidth(100);
  buttonPowersystem->setFixedHeight(40);
  connect( buttonPowersystem, SIGNAL(clicked()),
           this, SIGNAL(buttonPowersystem_clicked()) );

  // SSAnalysis editor button
  buttonSSAnalysis = new QPushButton( QIcon(),
                                      "SS Analysis", this );
  addWidget(buttonSSAnalysis);
  buttonSSAnalysis->setFixedWidth(100);
  buttonSSAnalysis->setFixedHeight(40);
  connect( buttonSSAnalysis, SIGNAL(clicked()),
           this, SIGNAL(buttonSSAnalysis_clicked()) );

  // Scenarios button
  buttonScenarios = new QPushButton( QIcon(":/images/lightning.png"),
                                     "Scenarios", this );
  addWidget(buttonScenarios);
  buttonScenarios->setFixedWidth(100);
  buttonScenarios->setFixedHeight(40);
  connect( buttonScenarios, SIGNAL(clicked()),
           this, SIGNAL(buttonScenarios_clicked()) );

  // TDAnalysis button
  buttonTDAnalysis = new QPushButton( QIcon(":/images/analysis.png"),
                                      "TD Analysis", this );
  addWidget(buttonTDAnalysis);
  buttonTDAnalysis->setFixedWidth(100);
  buttonTDAnalysis->setFixedHeight(40);
  connect( buttonTDAnalysis, SIGNAL(clicked()),
           this, SIGNAL(buttonTDAnalysis_clicked()) );

  // Spacer frame
  QFrame *frm1 = new QFrame( this ); // Used as spacer
  addWidget(frm1);

  // ----- Emulator specific buttons -----
  // Communication button
  buttonCommunication = new QPushButton( QIcon(":/images/usb.png"),
                                         "Communication", this );
  addWidget( buttonCommunication );
  buttonCommunication->setFixedWidth(100);
  buttonCommunication->setFixedHeight(40);
  connect( buttonCommunication, SIGNAL(clicked()),
           this, SIGNAL(buttonCommunication_clicked()) );

  // Calibration button
  buttonCalibration = new QPushButton( QIcon(":/images/Calib.png"),
                                       "Calibration" , this );
  addWidget(buttonCalibration);
  buttonCalibration->setFixedWidth(100);
  buttonCalibration->setFixedHeight(40);
  connect( buttonCalibration, SIGNAL(clicked()),
           this, SIGNAL(buttonCalibration_clicked()) );

  // Mapper button
  buttonMapper = new QPushButton( QIcon(":/images/placer.png"),
                                  "Mapper", this );
  addWidget(buttonMapper);
  buttonMapper->setFixedWidth(100);
  buttonMapper->setFixedHeight(40);
  connect( buttonMapper, SIGNAL(clicked()),
           this, SIGNAL(buttonMapper_clicked()) );

  // Fitter button
  buttonFitter = new QPushButton( QIcon(":/images/fitter.png"),
                                 "Fitter", this );
  addWidget(buttonFitter);
  buttonFitter->setFixedWidth(100);
  buttonFitter->setFixedHeight(40);
  connect( buttonFitter, SIGNAL(clicked()),
           this, SIGNAL(buttonFitter_clicked()) );

  // Auxiliary button
  buttonAuxiliary = new QPushButton( QIcon(":/images/event.png"),
                                     "Auxiliary", this );
  addWidget(buttonAuxiliary);
  buttonAuxiliary->setFixedWidth(100);
  buttonAuxiliary->setFixedHeight(40);
  connect( buttonAuxiliary, SIGNAL(clicked()),
           this, SIGNAL(buttonAuxiliary_clicked()) );

  // Show Hw button
  buttonShowHw = new QPushButton( this );
  buttonShowHw->setIcon( QIcon(":/images/uparrow.png") );
  addWidget(buttonShowHw);
  buttonShowHw->setFlat( true );
  buttonShowHw->setFixedWidth(100);
  buttonShowHw->setFixedHeight(15);
  connect( buttonShowHw, SIGNAL(clicked()),
           this, SLOT(showHwSlot()) );
  QLabel* showHwLabel = new QLabel( "Show hardware specific views", this );
  addWidget( showHwLabel );
  showHwLabel->setWordWrap( true );
  showHwLabel->setAlignment( Qt::AlignHCenter );
  showHwLabel->setFixedWidth(100);
  showHwLabel->setMaximumHeight(25);

  showHwSlot();
}

void SidePane::showHwSlot(){

  _isHwShown = !_isHwShown;

  if ( _isHwShown ){
    buttonCommunication->show();
    buttonCalibration->show();
    buttonMapper->show();
    buttonFitter->show();
    buttonAuxiliary->show();
    buttonShowHw->setIcon( QIcon(":/images/dnarrow.png") );
//    buttonShowHw->update();
  } else { // !_isHwShown
    buttonCommunication->hide();
    buttonCalibration->hide();
    buttonMapper->hide();
    buttonFitter->hide();
    buttonAuxiliary->hide();
    buttonShowHw->setIcon( QIcon(":/images/uparrow.png") );
//    buttonShowHw->update();
  }

  return;
}
