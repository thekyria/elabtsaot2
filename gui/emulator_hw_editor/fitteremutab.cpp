
#include "fitteremutab.h"
using namespace elabtsaot;

#include "emulator.h"

#include <QGroupBox>
#include <QFormLayout>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QToolBox>
#include <QPushButton>

FitterEmuTab::FitterEmuTab( Emulator* emu, QWidget* parent) :
    QSplitter(Qt::Horizontal, parent), _emu(emu){

  // Global emulator parameters box
  _initGlobalParamsBox(); // initializes globalParamsBox and handles connects
  globalParamsBox->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Preferred);
//  globalParamsBox->setMaximumWidth( globalParamsBox->minimumSizeHint().width() );

  // Left-side toolbox
  QToolBox* toolbox = new QToolBox( this );
  toolbox->setSizePolicy( QSizePolicy::Minimum,QSizePolicy::Preferred );
  addWidget( toolbox );
  toolbox->addItem( globalParamsBox, QString("Emu global params") );

  int width = 0;
  for ( int k = 0 ; k != toolbox->count() ; ++k )
    if ( toolbox->widget(k)->minimumSizeHint().width() > width )
      width = toolbox->widget(k)->minimumSizeHint().width();
  toolbox->setMaximumWidth( width );

  // Right-side placeholder frame
  QFrame* frm = new QFrame( this );
  addWidget( frm );

  // Update view
  updt();
}

void FitterEmuTab::updt(){
  ratioZForm->setValue( _emu->ratioZ() );
  ratioVForm->setValue( _emu->ratioV() );
  ratioIForm->setValue( _emu->ratioI() );
}

void FitterEmuTab::ratioZSlot(double val){
  _emu->set_ratioZ( val );
  ratioIForm->setValue( _emu->ratioI() );
}

void FitterEmuTab::ratioVSlot(double val){
  _emu->set_ratioV( val );
  ratioIForm->setValue( _emu->ratioI() );
}

void FitterEmuTab::_initGlobalParamsBox(){

  globalParamsBox = new QGroupBox("Global emulator parameters", this);
  QFormLayout* globalParamsLay = new QFormLayout(globalParamsBox);
  globalParamsBox->setLayout( globalParamsLay );

  // Ratio Z
  QLabel* ratioZLabel = new QLabel("Ratio Z");
  ratioZForm = new QDoubleSpinBox();
  ratioZForm->setDecimals( 0 );
  ratioZForm->setRange( 0,1000000 );
  ratioZForm->setSingleStep( 1000 );
  globalParamsLay->addRow( ratioZLabel, ratioZForm);

  // Ratio V
  QLabel* ratioVLabel = new QLabel("Ratio V");
  ratioVForm = new QDoubleSpinBox();
  ratioVForm->setDecimals( 2) ;
  ratioVForm->setMinimum( 0.0 );
  ratioVForm->setSingleStep( 0.01 );
  globalParamsLay->addRow( ratioVLabel, ratioVForm );

  // Ratio I
  QLabel* ratioILabel = new QLabel("Ratio I");
  ratioIForm = new QDoubleSpinBox();
  ratioIForm->setReadOnly( true );
  ratioIForm->setDecimals( 7 );
//  ratioIForm->setMinimum( 0 );
//  ratioIForm->setSingleStep( 0.0000001 );
  globalParamsLay->addRow( ratioILabel, ratioIForm );


  // ----------------- Connect signals -----------------
  connect( ratioZForm, SIGNAL(valueChanged(double)),
           this, SLOT(ratioZSlot(double)) );
  connect( ratioVForm, SIGNAL(valueChanged(double)),
           this, SLOT(ratioVSlot(double)) );
}
