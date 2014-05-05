
#include "nodedialog.h"
using namespace elabtsaot;

#include "slicewidget.h"

#include "atom.h"

#include <QDialog>
#include <QString>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>

NodeDialog::NodeDialog(Atom* a, bool real, QWidget* parent) :
    QObject(parent), _a(a), _real(real){}

int NodeDialog::exec(){

  QDialog* dialog = new QDialog();
  dialog->setWindowTitle("Node dialog");
  QVBoxLayout* mainLay = new QVBoxLayout();
  dialog->setLayout(mainLay);

  // ----- Image -----
  QImage* img = new QImage(":/images/node_model.png");
  QLabel* labelImg = new QLabel();
  labelImg->setPixmap( QPixmap::fromImage(*img) );
  mainLay->addWidget( labelImg );

  QHBoxLayout* bottomLay = new QHBoxLayout();
  mainLay->addLayout( bottomLay );

  // ----- Switches status -----
  QVBoxLayout* switchesLay = new QVBoxLayout();
  bottomLay->addLayout( switchesLay );

  sw_voltage = new QCheckBox("Voltage switch");
  sw_voltage->setChecked( _real ? _a->node.real_sw_voltage : _a->node.imag_sw_voltage );
  switchesLay->addWidget( sw_voltage );

  sw_current = new QCheckBox("Current switch");
  sw_current->setChecked( _real ? _a->node.real_sw_current : _a->node.imag_sw_current );
  switchesLay->addWidget( sw_current );

  sw_current_shunt = new QCheckBox("Current shunt switch");
  sw_current_shunt->setChecked( _real ? _a->node.real_sw_current_shunt : _a->node.imag_sw_current_shunt );
  switchesLay->addWidget( sw_current_shunt );

  sw_resistance = new QCheckBox("Resistance switch");
  sw_resistance->setChecked( _real ? _a->node.real_sw_resistance : _a->node.imag_sw_resistance );
  switchesLay->addWidget( sw_resistance );

  // ----- Current source -----
  QVBoxLayout* currentLay = new QVBoxLayout();
  bottomLay->addLayout( currentLay );

  QHBoxLayout* currentTapLay = new QHBoxLayout();
  currentLay->addLayout( currentTapLay );
  QLabel* currentTapLabel = new QLabel("Current source pot tap");
  currentTapLay->addWidget( currentTapLabel );
  pot_current_tap = new QSpinBox();
  pot_current_tap->setRange( 0, 256 );
  pot_current_tap->setValue( _a->node.pot_current_tap(_real) );
  currentTapLay->addWidget( pot_current_tap );
  connect( pot_current_tap, SIGNAL(valueChanged(int)),
           this, SLOT(currentPotSlot(int)) );

  QHBoxLayout* currentResLay = new QHBoxLayout();
  currentLay->addLayout( currentResLay );
  QLabel* currentResLabel = new QLabel("Current source pot resistance");
  currentResLay->addWidget( currentResLabel );
  pot_current_r = new QDoubleSpinBox();
  pot_current_r->setMaximum(1000000.);
  pot_current_r->setEnabled(false);
  pot_current_r->setUpdatesEnabled(true);
  pot_current_r->setValue( _a->node.pot_current_r(_real) );
  currentResLay->addWidget( pot_current_r );

  QHBoxLayout* currentRabLay = new QHBoxLayout();
  currentLay->addLayout( currentRabLay );
  QLabel* currentRabLabel = new QLabel("Current source pot rab");
  currentRabLay->addWidget( currentRabLabel );
  pot_current_rab = new QDoubleSpinBox();
  pot_current_rab->setRange( 0.0, 1000000.0 );
  pot_current_rab->setEnabled(false);
  pot_current_rab->setUpdatesEnabled(true);
  pot_current_rab->setValue( _a->node.pot_current_rab(_real) );
  currentRabLay->addWidget( pot_current_rab );

  pot_current_sw = new QCheckBox("Current source pot switch");
  pot_current_sw->setChecked( _a->node.pot_current_sw(_real) );
  currentLay->addWidget( pot_current_sw );

  // ----- Resistance to ground -----
  QVBoxLayout* groundLay = new QVBoxLayout();
  bottomLay->addLayout( groundLay );

  QHBoxLayout* groundTapLay = new QHBoxLayout();
  groundLay->addLayout( groundTapLay );
  QLabel* groundTapLabel = new QLabel("Ground pot tap");
  groundTapLay->addWidget( groundTapLabel );
  pot_resistance_tap = new QSpinBox();
  pot_resistance_tap->setRange( 0, 256 );
  pot_resistance_tap->setValue( _a->node.pot_resistance_tap(_real) );
  groundTapLay->addWidget( pot_resistance_tap );
  connect( pot_resistance_tap, SIGNAL(valueChanged(int)),
           this, SLOT(groundPotSlot(int)) );

  QHBoxLayout* groundResLay = new QHBoxLayout();
  groundLay->addLayout( groundResLay );
  QLabel* groundResLabel = new QLabel("Ground pot resistance");
  groundResLay->addWidget( groundResLabel );
  pot_resistance_r = new QDoubleSpinBox();
  pot_resistance_r->setMaximum(1000000.0);
  pot_resistance_r->setEnabled(false);
  pot_resistance_r->setUpdatesEnabled(true);
  pot_resistance_r->setValue( _a->node.pot_resistance_r(_real) );
  groundResLay->addWidget( pot_resistance_r );

  QHBoxLayout* groundRabLay = new QHBoxLayout();
  groundLay->addLayout( groundRabLay );
  QLabel* groundRabLabel = new QLabel("Ground pot lable");
  groundRabLay->addWidget( groundRabLabel );
  pot_resistance_rab = new QDoubleSpinBox();
  pot_resistance_rab->setRange( 0.0 , 1000000.0 );
  pot_resistance_rab->setEnabled( false );
  pot_resistance_rab->setUpdatesEnabled( true );
  pot_resistance_rab->setValue( _a->node.pot_resistance_rab(_real) );
  groundRabLay->addWidget( pot_resistance_rab );

  pot_resistance_sw = new QCheckBox("Ground pot switch");
  pot_resistance_sw->setChecked( _a->node.pot_resistance_sw(_real) );
  groundLay->addWidget( pot_resistance_sw );

  // ----- Buttons -----
  QVBoxLayout* buttonsLay = new QVBoxLayout();
  bottomLay->addLayout( buttonsLay );

  QPushButton* ok = new QPushButton("Ok");
  buttonsLay->addWidget( ok );
  connect( ok, SIGNAL(clicked()), dialog, SLOT(accept()) );

  QPushButton* cancel = new QPushButton("Cancel");
  buttonsLay->addWidget( cancel );
  connect( cancel, SIGNAL(clicked()), dialog, SLOT(reject()));

  if (dialog->exec()){
    // Dialog executed properly
    if (_real){
      _a->node.real_sw_voltage = sw_voltage->isChecked();
      _a->node.real_sw_current = sw_current->isChecked();
      _a->node.real_sw_current_shunt = sw_current_shunt->isChecked();
      _a->node.real_sw_resistance = sw_resistance->isChecked();
    } else {
      _a->node.imag_sw_voltage = sw_voltage->isChecked();
      _a->node.imag_sw_current = sw_current->isChecked();
      _a->node.imag_sw_current_shunt = sw_current_shunt->isChecked();
      _a->node.imag_sw_resistance = sw_resistance->isChecked();
    }

    _a->node.set_pot_current_tap( pot_current_tap->value(), _real );
    _a->node.set_pot_current_sw( pot_current_sw->isChecked(), _real );

    _a->node.set_pot_resistance_tap( pot_resistance_tap->value(), _real );
    _a->node.set_pot_resistance_sw( pot_resistance_sw->isChecked(), _real );
    return 0;
  } else {
    // Dialog not executed properly
    return 1;
  }
  return 0;
}

void NodeDialog::currentPotSlot( int val ){

  // save old tap val for restoring later
  int oldTap = _a->node.pot_current_tap(_real);

  // change tap val in order to get resistor val of the pot
  if ( _a->node.set_pot_current_tap(val,_real) )
    // problem changing tap val!
    return;
  double resVal = _a->node.pot_current_r(_real);
  pot_current_r->setValue( resVal );

  // restore old tap val - actual tap val will be updated only if the dialog is
  // properly excecuted
  _a->node.set_pot_current_tap(oldTap,_real);

  return;
}

void NodeDialog::groundPotSlot( int val ){

  // save old tap val for restoring later
  int oldTap = _a->node.pot_resistance_tap(_real);

  // change tap val in order to get resistor val of the pot
  if ( _a->node.set_pot_resistance_tap(val,_real) )
    // problem changing tap val!
    return;
  double resVal = _a->node.pot_resistance_r(_real);
  pot_resistance_r->setValue( resVal );

  // restore old tap val - actual tap val will be updated only if the dialog is
  // properly excecuted
  _a->node.set_pot_resistance_tap(oldTap,_real);

  return;
}
