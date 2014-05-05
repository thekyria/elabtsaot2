
#include "emulatorbranchdialog.h"
using namespace elabtsaot;

#include <QDialog>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QString>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QPushButton>

EmulatorBranchDialog::EmulatorBranchDialog(EmulatorBranch* embr, QWidget* parent) :
    QObject(parent), _embr(embr), _embrLocal(*embr){}

int EmulatorBranchDialog::exec(){

  // Initialize dialog
  QDialog* dialog = new QDialog();
  dialog->setWindowTitle("Node dialog");
  QVBoxLayout* mainLay = new QVBoxLayout();
  dialog->setLayout(mainLay);
  QFormLayout* fLay = new QFormLayout();
  mainLay->addLayout(fLay);

  // Short circuit switch
  swScForm = new QCheckBox("swSc");
  swScForm->setChecked(_embrLocal.sw_sc());
  fLay->addRow(QString("Short circuit switch"), swScForm);

  // Mid-grounding switch
  swMidForm = new QCheckBox("midSc");
  swMidForm->setChecked(_embrLocal.sw_mid());
  fLay->addRow(QString("Mid-grounding switch"), swMidForm);

  // Near potentiometer switch
  potNearSwForm = new QCheckBox("potNearSw");
  potNearSwForm->setChecked(_embrLocal.pot_near_sw());
  fLay->addRow(QString("Near potentiometer switch"), potNearSwForm);
  // Near potentiometer tap setting
  potNearTapForm = new QSpinBox();
  potNearTapForm->setRange(0,_embrLocal.pot_near_tap_max());
  potNearTapForm->setSingleStep(1);
  potNearTapForm->setValue(_embrLocal.pot_near_tap());
  fLay->addRow((QString("Near potentiometer tap setting"), potNearTapForm));
  connect(potNearTapForm, SIGNAL(valueChanged(int)), this, SLOT(nearPotSlot(int)));
  // Near potentiometer resistor
  potNearRForm = new QDoubleSpinBox();
  potNearRForm->setRange(_embrLocal.pot_near_getRMin(),_embrLocal.pot_near_getRMax());
  potNearRForm->setDecimals(2);
  potNearRForm->setValue(_embrLocal.pot_near_r());
  potNearRForm->setEnabled(false);
  fLay->addRow(QString("Near potentiometer resistor"), potNearRForm);

  // Far potentiometer switch
  potFarSwForm = new QCheckBox("potFarSw");
  potFarSwForm->setChecked(_embrLocal.pot_far_sw());
  fLay->addRow(QString("Far potentiometer switch"), potFarSwForm);
  // Far potentiometer tap setting
  potFarTapForm = new QSpinBox();
  potFarTapForm->setRange(0,_embrLocal.pot_far_tap_max());
  potFarTapForm->setSingleStep(1);
  potFarTapForm->setValue(_embrLocal.pot_far_tap());
  fLay->addRow((QString("Far potentiometer tap setting"), potFarTapForm));
  connect(potFarTapForm, SIGNAL(valueChanged(int)), this, SLOT(farPotSlot(int)));
  // Far potentiometer resistor
  potFarRForm = new QDoubleSpinBox();
  potFarRForm->setRange(_embrLocal.pot_far_getRMin(),_embrLocal.pot_far_getRMax());
  potFarRForm->setDecimals(2);
  potFarRForm->setValue(_embrLocal.pot_far_r());
  potFarRForm->setEnabled(false);
  fLay->addRow(QString("Far potentiometer resistor"), potFarRForm);

  // Buttons
  QHBoxLayout* buttonLay = new QHBoxLayout();
  mainLay->addLayout(buttonLay);
  QPushButton* okb = new QPushButton("OK");
  buttonLay->addWidget(okb);
  connect(okb, SIGNAL(clicked()), dialog, SLOT(accept()));
  QPushButton* cancelb = new QPushButton("Cancel");
  buttonLay->addWidget(cancelb);
  connect(cancelb, SIGNAL(clicked()), dialog, SLOT(reject()));

  // Execute dialog
  int ans = dialog->exec();
  // Dialog not executed properly
  if (!ans) return ans;

  // Update _embrLocal values
  _embrLocal.set_sw_sc(swScForm->isChecked());
  _embrLocal.set_sw_mid(swMidForm->isChecked());
  _embrLocal.set_pot_near_sw(potNearSwForm->isChecked());
  _embrLocal.set_pot_near_tap(potNearTapForm->value());
  _embrLocal.set_pot_far_sw(potFarSwForm->isChecked());
  _embrLocal.set_pot_far_tap(potFarTapForm->value());
  // Update the pointed embr
  *_embr = _embrLocal;

  return 0; // executed properly
}

void EmulatorBranchDialog::nearPotSlot(int val){
  _embrLocal.set_pot_near_tap(val);
  potNearRForm->setValue(_embrLocal.pot_near_r());
}

void EmulatorBranchDialog::farPotSlot(int val){
  _embrLocal.set_pot_far_tap(val);
  potFarRForm->setValue(_embrLocal.pot_far_r());
}
