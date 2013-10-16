
#include "powersystemeditordialogs.h"
using namespace elabtsaot;

#include "powersystem.h"

#include <QDialog>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QRadioButton>
#include <QComboBox>
#include <QString>
#include <QButtonGroup>

int pwsEditorDialogs::slackDialog( int oldSlackBusExtId,
                                   int oldSlackGenExtId,
                                   int* newSlackGenExtId){

  QDialog* dialog = new QDialog();
  dialog->setWindowTitle("Set Slack Generator");

  // Main vertical layout
  QVBoxLayout* layMain = new QVBoxLayout();
  dialog->setLayout( layMain );
  // Information label
  unsigned int slackBusExtId = oldSlackBusExtId;
  unsigned int slackGenExtId = oldSlackGenExtId;
  QString infoString;
  infoString = QString("Current slack generator %0 at bus %1")
                .arg(slackGenExtId)
                .arg(slackBusExtId);
  QLabel* infoLabel = new QLabel( infoString );
  layMain->addWidget( infoLabel );
  // Slack selection spinbox
  QSpinBox* num = new QSpinBox();
  num->setMaximum( RAND_MAX );
  num->setValue( slackGenExtId );
  layMain->addWidget( num );
  // Horizonal button layout
  QHBoxLayout* layButtons = new QHBoxLayout();
  layMain->addLayout( layButtons );
  // Ok button
  QPushButton* ok = new QPushButton("Ok");
  layButtons->addWidget( ok );
  dialog->connect( ok , SIGNAL(clicked()), dialog, SLOT(accept()) );
  // Cancel button
  QPushButton* cancel = new QPushButton("Cancel");
  layButtons->addWidget( cancel );
  dialog->connect( cancel, SIGNAL(clicked()), dialog, SLOT(reject()) );

  // Excecute dialog
  if ( dialog->exec() ){
    // Return requested slack gen external id to the output argument
    *newSlackGenExtId = num->value();
    return 0;
  } else{
    // Dialog cancelled or not executed correctly
    return 1;
  }

  return 0;
}

int pwsEditorDialogs::busDialog(Bus *bus){
  // Main dialog
  QDialog dialog;
  dialog.setWindowTitle("Property Editor: Bus");
  QVBoxLayout layoutMain;
  dialog.setLayout(&layoutMain);

  // Properties
  // extId
  QHBoxLayout layoutExtId;
  QLabel labelExtId("Id:");
  QSpinBox formExtId;
  formExtId.setMaximum( RAND_MAX );
  formExtId.setValue( bus->extId );
  layoutExtId.addWidget( &labelExtId );
  layoutExtId.addWidget( &formExtId );
  layoutMain.addLayout( &layoutExtId );
  // name
  QHBoxLayout layoutName;
  QLabel labelName("Name:");
  QLineEdit formName;
  formName.setText( bus->name.c_str() );
  layoutName.addWidget( &labelName );
  layoutName.addWidget( &formName );
  layoutMain.addLayout( &layoutName );
  // shunt conductance
  QHBoxLayout layoutGsh;
  QLabel labelGsh("Shunt conductance [pu]:");
  QDoubleSpinBox formGsh;
  formGsh.setValue( bus->gsh );
  layoutGsh.addWidget( &labelGsh );
  layoutGsh.addWidget( &formGsh );
  layoutMain.addLayout( &layoutGsh );
  // shunt susceptance
  QHBoxLayout layoutBsh;
  QLabel labelBsh("Shunt susceptance [pu]:");
  QDoubleSpinBox formBsh;
  formBsh.setValue( bus->bsh );
  layoutBsh.addWidget( &labelBsh );
  layoutBsh.addWidget( &formBsh );
  layoutMain.addLayout( &layoutBsh );
  // base voltage
  QHBoxLayout layoutBaseKV;
  QLabel labelBaseKV("Base voltage [KV]:");
  QDoubleSpinBox formBaseKV;
  formBaseKV.setValue( bus->baseKV );
  layoutBaseKV.addWidget( &labelBaseKV );
  layoutBaseKV.addWidget( &formBaseKV );
  layoutMain.addLayout( &layoutBaseKV );

  // Buttons
  QHBoxLayout layoutButtons;
  QPushButton ok("Ok");
  QPushButton cancel("Cancel");
  dialog.connect(&ok, SIGNAL(clicked()), &dialog, SLOT(accept()));
  dialog.connect(&cancel, SIGNAL(clicked()), &dialog, SLOT(reject()));
  layoutButtons.addWidget( &ok );
  layoutButtons.addWidget( &cancel );
  layoutMain.addLayout( &layoutButtons );
  // Execute dialog
  if( dialog.exec() ){
    // Dialog executed properly
    // Store values back
    bus->extId = formExtId.value();
    bus->name = formName.text().toStdString();
    bus->gsh = formGsh.value();
    bus->bsh = formBsh.value();
    bus->baseKV = formBaseKV.value();

    return 0;

  } else{
    // Dialog not executed properly
    return 1;
  }

  return 0;
}

int pwsEditorDialogs::branchDialog(Branch *branch){

  // Main dialog
  QDialog dialog;
  dialog.setWindowTitle("Property Editor: Branch");
  QVBoxLayout layoutMain;
  dialog.setLayout(&layoutMain);
  // image
  QImage img(":/images/branch_model.png");
  QLabel labelImg;
  labelImg.setPixmap( QPixmap::fromImage(img) );
  layoutMain.addWidget( &labelImg );

  // Properties
  // extId
  QHBoxLayout layoutExtId;
  QLabel labelExtId("Id:");
  QSpinBox formExtId;
  formExtId.setMaximum( RAND_MAX );
  formExtId.setValue( branch->extId() );
  layoutExtId.addWidget( &labelExtId );
  layoutExtId.addWidget( &formExtId );
  layoutMain.addLayout( &layoutExtId );
  // name
  QHBoxLayout layoutName;
  QLabel labelName("Name:");
  QLineEdit formName;
  formName.setText( branch->name().c_str() );
  layoutName.addWidget( &labelName );
  layoutName.addWidget( &formName );
  layoutMain.addLayout( &layoutName );
  // formBus & toBus
  QHBoxLayout layoutBuses;
  QLabel labelBuses("From/To bus:");
  QSpinBox formFromBus, formToBus;
  formFromBus.setMaximum( RAND_MAX );
  formFromBus.setValue( branch->fromBusExtId() );
  formToBus.setMaximum( RAND_MAX );
  formToBus.setValue( branch->toBusExtId() );
  layoutBuses.addWidget( &labelBuses );
  layoutBuses.addWidget( &formFromBus );
  layoutBuses.addWidget( &formToBus );
  layoutMain.addLayout( &layoutBuses );
  // resistance
  QHBoxLayout layoutR;
  QLabel labelR("Resistance [pu]:");
  QDoubleSpinBox formR;
  formR.setDecimals( 5 );
  formR.setValue( branch->r() );
  layoutR.addWidget( &labelR );
  layoutR.addWidget( &formR );
  layoutMain.addLayout( &layoutR );
  // reactance
  QHBoxLayout layoutX;
  QLabel labelX("Reactance [pu]:");
  QDoubleSpinBox formX;
  formX.setDecimals( 5 );
  formX.setValue( branch->x() );
  layoutX.addWidget( &labelX );
  layoutX.addWidget( &formX );
  layoutMain.addLayout( &layoutX );
  // susceptance to ground
  QHBoxLayout layoutB;
  QLabel labelB("Ground Susceptance (near/far end) [pu]:");
  QDoubleSpinBox formBfr, formBto;
  formBfr.setDecimals( 5 );
  formBto.setDecimals( 5 );
  formBfr.setValue( branch->b_from() );
  formBto.setValue( branch->b_to() );
  layoutB.addWidget( &labelB );
  layoutB.addWidget( &formBfr );
  layoutB.addWidget( &formBto );
  layoutMain.addLayout( &layoutB );
  // conductance to ground
  QHBoxLayout layoutG;
  QLabel labelG("Ground Conductance (near/far end) [pu]:");
  QDoubleSpinBox formGfr, formGto;
  formGfr.setDecimals( 5 );
  formGto.setDecimals( 5 );
  formGfr.setValue( branch->g_from() );
  formGto.setValue( branch->g_to() );
  layoutG.addWidget( &labelG );
  layoutG.addWidget( &formGfr );
  layoutG.addWidget( &formGto );
  layoutMain.addLayout( &layoutG );
  // in-series capacitance
  QHBoxLayout layoutC;
  QLabel labelC("In-series Capacitance [pu]:");
  QDoubleSpinBox formCvalue;
  formCvalue.setDecimals( 5 );
  formCvalue.setMinimum( -formCvalue.maximum() );
  formCvalue.setValue( branch->c_series_x() );
  layoutC.addWidget( &labelC );
  layoutC.addWidget( &formCvalue );
  layoutMain.addLayout( &layoutC );
  // x ratio
  QHBoxLayout layoutXratio;
  QLabel labelXratio("Transformer ratio \n (current/tap/base/tap_min/tap_max/tap_step):");
  QDoubleSpinBox formXratio, formXratio_base, formXratio_tap_step;
  QDoubleSpinBox formXratio_tap, formXratio_tap_min, formXratio_tap_max;
  formXratio.setDecimals( 5 );
  formXratio_base.setDecimals( 5 );
  formXratio_tap_step.setDecimals( 5 );
  formXratio_tap.setDecimals( 5 );
  formXratio_tap_min.setDecimals( 5 );
  formXratio_tap_max.setDecimals( 5 );
  formXratio.setValue( branch->Xratio() );
  formXratio_tap.setValue( branch->Xratio_tap() );
  formXratio_base.setValue( branch->Xratio_base() );
  formXratio_tap_min.setValue( branch->Xratio_tap_min() );
  formXratio_tap_max.setValue( branch->Xratio_tap_max() );
  formXratio_tap_step.setValue( branch->Xratio_tap_step() );
  layoutXratio.addWidget( &labelXratio );
  layoutXratio.addWidget( &formXratio );
  layoutXratio.addWidget( &formXratio_tap );
  layoutXratio.addWidget( &formXratio_base );
  layoutXratio.addWidget( &formXratio_tap_min );
  layoutXratio.addWidget( &formXratio_tap_max );
  layoutXratio.addWidget( &formXratio_tap_step );
  layoutMain.addLayout( &layoutXratio );
  // x shift
  QHBoxLayout layoutXshift;
  QLabel labelXshift("Transformer shift \n (current/tap/base/tap_min/tap_max/tap_step):");
  QDoubleSpinBox formXshift, formXshift_base, formXshift_tap_step;
  QDoubleSpinBox formXshift_tap, formXshift_tap_min, formXshift_tap_max;
  formXshift.setDecimals( 5 );
  formXshift_base.setDecimals( 5 );
  formXshift_tap_step.setDecimals( 5 );
  formXshift_tap.setDecimals( 5 );
  formXshift_tap_min.setDecimals( 5 );
  formXshift_tap_max.setDecimals( 5 );
  formXshift.setValue( branch->Xshift() );
  formXshift_tap.setValue( branch->Xshift_tap() );
  formXshift_base.setValue( branch->Xshift_base() );
  formXshift_tap_min.setValue( branch->Xshift_tap_min() );
  formXshift_tap_max.setValue( branch->Xshift_tap_max() );
  formXshift_tap_step.setValue( branch->Xshift_tap_step() );
  layoutXshift.addWidget( &labelXshift );
  layoutXshift.addWidget( &formXshift );
  layoutXshift.addWidget( &formXshift_tap );
  layoutXshift.addWidget( &formXshift_base );
  layoutXshift.addWidget( &formXshift_tap_min );
  layoutXshift.addWidget( &formXshift_tap_max );
  layoutXshift.addWidget( &formXshift_tap_step );
  layoutMain.addLayout( &layoutXshift );
  // status
  QHBoxLayout layoutStatus;
  QLabel labelStatus("Status:");
  QRadioButton statusOn("On-line", &labelStatus);
  if( branch->status()==1 )
    statusOn.setChecked(true);
  QRadioButton statusOff("Off-line", &labelStatus);
  if( branch->status()==0 ) statusOff.setChecked(true);
  QHBoxLayout layoutSubstatus;
  layoutSubstatus.addWidget( &statusOn );
  layoutSubstatus.addWidget( &statusOff );
  layoutStatus.addWidget( &labelStatus );
  layoutStatus.addLayout( &layoutSubstatus );
  layoutMain.addLayout( &layoutStatus );
  // Buttons
  QHBoxLayout layoutButtons;
  QPushButton ok("Ok");
  QPushButton cancel("Cancel");
  dialog.connect(&ok, SIGNAL(clicked()), &dialog, SLOT(accept()));
  dialog.connect(&cancel, SIGNAL(clicked()), &dialog, SLOT(reject()));
  layoutButtons.addWidget( &ok );
  layoutButtons.addWidget( &cancel );
  layoutMain.addLayout( &layoutButtons );
  // Execute dialog
  if( dialog.exec() ){
    // Dialog executed properly
    // Store values back
    branch->set_extId( formExtId.value() );
    branch->set_name( formName.text().toStdString() );
    branch->set_fromBusExtId( formFromBus.value() );
    branch->set_toBusExtId( formToBus.value() );
    branch->set_r( formR.value() );
    branch->set_x( formX.value() );
    branch->set_b_from( formBfr.value() );
    branch->set_b_to( formBto.value() );
    branch->set_g_from( formGfr.value() );
    branch->set_g_to( formBto.value() );
    branch->set_c_series_x( formCvalue.value() );

    branch->set_Xratio( formXratio.value() );
    branch->set_Xratio_tap( formXratio_tap.value() );
    branch->set_Xratio_base( formXratio_base.value() );
    branch->set_Xratio_tap_min( formXratio_tap_min.value() );
    branch->set_Xratio_tap_max( formXratio_tap_max.value() );
    branch->set_Xratio_tap_step( formXratio_tap_step.value() );
    branch->set_Xshift( formXshift.value() );
    branch->set_Xshift_tap( formXshift_tap.value() );
    branch->set_Xshift_base( formXshift_base.value() );
    branch->set_Xshift_tap_min( formXshift_tap_min.value() );
    branch->set_Xshift_tap_max( formXshift_tap_max.value() );
    branch->set_Xshift_tap_step( formXshift_tap_step.value() );

    branch->set_status( statusOn.isChecked() );

    return 0;

  } else{
    // Dialog not executed properly
    return 1;
  }

  return 0;
}

int pwsEditorDialogs::genDialog(Generator *gen){
  // Main dialog
  QDialog dialog;
  dialog.setWindowTitle("Property Editor: Generator");
  QVBoxLayout layoutMain;
  dialog.setLayout(&layoutMain);
  // Properties
  // extId & atBus
  QHBoxLayout layoutExtIdBus;
  QLabel labelExtId("Id:");
  QSpinBox formExtId;
  formExtId.setMaximum( RAND_MAX );
  formExtId.setValue( gen->extId() );
  layoutExtIdBus.addWidget( &labelExtId );
  layoutExtIdBus.addWidget( &formExtId );
  QLabel labelBus("At bus:");
  QSpinBox formBus;
  formBus.setMaximum( RAND_MAX );
  formBus.setValue( gen->busExtId() );
  layoutExtIdBus.addWidget( &labelBus );
  layoutExtIdBus.addWidget( &formBus );
  layoutMain.addLayout( &layoutExtIdBus );
  // name
  QHBoxLayout layoutName;
  QLabel labelName("Name:");
  QLineEdit formName;
  formName.setText( gen->name().c_str() );
  layoutName.addWidget( &labelName );
  layoutName.addWidget( &formName );
  layoutMain.addLayout( &layoutName );
  // voltage setpoint
  QHBoxLayout layoutVPoint;
  QLabel labelVPoint("Voltage setpoint [pu]:");
  QDoubleSpinBox formVPoint;
  formVPoint.setDecimals( 5 );
  formVPoint.setValue( gen->voltageSetpoint() );
  layoutVPoint.addWidget( &labelVPoint );
  layoutVPoint.addWidget( &formVPoint );
  QCheckBox formAvr("AVR");
  formAvr.setChecked( gen->avr() );
  layoutVPoint.addWidget( &formAvr );
  layoutMain.addLayout( &layoutVPoint );
  // p power
  QHBoxLayout layoutPPower;
  QLabel labelPPower("Active Power (min/generated/max) [pu]:");
  QDoubleSpinBox formPMin, formPGen, formPMax;
  formPMin.setDecimals( 5 );
  formPGen.setDecimals( 5 );
  formPMax.setDecimals( 5 );
  formPMin.setValue( gen->pmin() );
  formPGen.setValue( gen->pgen() );
  formPMax.setValue( gen->pmax() );
  layoutPPower.addWidget( &labelPPower );
  layoutPPower.addWidget( &formPMin );
  layoutPPower.addWidget( &formPGen );
  layoutPPower.addWidget( &formPMax );
  layoutMain.addLayout( &layoutPPower );
  // q power
  QHBoxLayout layoutQPower;
  QLabel labelQPower("Reactive Power (min/generated/max) [pu]:");
  QDoubleSpinBox formQMin, formQGen, formQMax;
  formQMin.setDecimals( 5 );
  formQGen.setDecimals( 5 );
  formQMax.setDecimals( 5 );
  formQMin.setValue( gen->qmin() );
  formQGen.setValue( gen->qgen() );
  formQMax.setValue( gen->qmax() );
  layoutQPower.addWidget( &labelQPower );
  layoutQPower.addWidget( &formQMin );
  layoutQPower.addWidget( &formQGen );
  layoutQPower.addWidget( &formQMax );
  layoutMain.addLayout( &layoutQPower );

  // dynamic data
  QLabel labelDynamic("<br>Dynamic data<hr>");
  layoutMain.addWidget( &labelDynamic );
  // model
  QHBoxLayout layoutDyn0;
  QLabel labelDyn01("Model:");
  QSpinBox formDyn01;
  formDyn01.setValue( gen->model() );
  layoutDyn0.addWidget( &labelDyn01 );
  layoutDyn0.addWidget( &formDyn01 );
  layoutMain.addLayout( &layoutDyn0 );
  // leakage reactance, armature resistance
  QHBoxLayout layoutDyn1;
  QLabel labelDyn11("Leakage reactance [pu]:");
  QLabel labelDyn12("Armature reactance [pu]:");
  QDoubleSpinBox formDyn11, formDyn12;
  formDyn11.setDecimals( 5 );
  formDyn12.setDecimals( 5 );
  formDyn11.setValue( gen->xl() );
  formDyn12.setValue( gen->ra() );
  layoutDyn1.addWidget( &labelDyn11 );
  layoutDyn1.addWidget( &formDyn11 );
  layoutDyn1.addWidget( &labelDyn12 );
  layoutDyn1.addWidget( &formDyn12 );
  layoutMain.addLayout( &layoutDyn1 );
  // direct axis
  QLabel labelDirAxis("<br>Direct Axis:");
  layoutMain.addWidget( &labelDirAxis );
  // reactance
  QHBoxLayout layoutDyn2;
  QLabel labelDyn21("Reactance (synchronous/transient/sub-transient) [pu]:");
  QDoubleSpinBox formDyn21, formDyn22, formDyn23;
  formDyn21.setDecimals( 5 );
  formDyn22.setDecimals( 5 );
  formDyn23.setDecimals( 5 );
  formDyn21.setValue( gen->xd() );
  formDyn22.setValue( gen->xd_1() );
  formDyn23.setValue( gen->xd_2() );
  layoutDyn2.addWidget( &labelDyn21 );
  layoutDyn2.addWidget( &formDyn21 );
  layoutDyn2.addWidget( &formDyn22 );
  layoutDyn2.addWidget( &formDyn23 );
  layoutMain.addLayout( &layoutDyn2 );
  // time constants
  QHBoxLayout layoutDyn3;
  QLabel labelDyn31("Open-circuit transient/sub-transient time constant [s]:");
  QDoubleSpinBox formDyn31, formDyn32;
  formDyn31.setDecimals( 5 );
  formDyn32.setDecimals( 5 );
  formDyn31.setValue( gen->Td0_1() );
  formDyn32.setValue( gen->Td0_2() );
  layoutDyn3.addWidget( &labelDyn31 );
  layoutDyn3.addWidget( &formDyn31 );
  layoutDyn3.addWidget( &formDyn32 );
  layoutMain.addLayout( &layoutDyn3 );
  // quadrature axis
  QLabel labelQuadAxis("<br>Quadrature Axis:");
  layoutMain.addWidget( &labelQuadAxis );
  // reactance
  QHBoxLayout layoutDyn4;
  QLabel labelDyn41("Reactance (synchronous/transient/sub-transient) [pu]:");
  QDoubleSpinBox formDyn41, formDyn42, formDyn43;
  formDyn41.setDecimals( 5 );
  formDyn42.setDecimals( 5 );
  formDyn43.setDecimals( 5 );
  formDyn41.setValue( gen->xq() );
  formDyn42.setValue( gen->xq_1() );
  formDyn43.setValue( gen->xq_2() );
  layoutDyn4.addWidget( &labelDyn41 );
  layoutDyn4.addWidget( &formDyn41 );
  layoutDyn4.addWidget( &formDyn42 );
  layoutDyn4.addWidget( &formDyn43 );
  layoutMain.addLayout( &layoutDyn4 );
  // time constants
  QHBoxLayout layoutDyn5;
  QLabel labelDyn51("Transient/Sub-transient time constant [s]:");
  QDoubleSpinBox formDyn51, formDyn52;
  formDyn51.setDecimals( 5 );
  formDyn52.setDecimals( 5 );
  formDyn51.setValue( gen->Tq0_1() );
  formDyn52.setValue( gen->Tq0_2() );
  layoutDyn5.addWidget( &labelDyn51 );
  layoutDyn5.addWidget( &formDyn51 );
  layoutDyn5.addWidget( &formDyn52 );
  layoutMain.addLayout( &layoutDyn5 );
  // Mech starting t & damping coeff
  QHBoxLayout layoutDyn6;
  QLabel labelDyn61("Mechanical starting t \n (M) [kWs/kVA]:");
  QLabel labelDyn62("Damping coefficient \n d(k) [pu]:");
  QDoubleSpinBox formDyn61, formDyn62;
  formDyn61.setDecimals( 5 );
  formDyn62.setDecimals( 5 );
  formDyn61.setMaximum(10000000.0);
//  formDyn61.setSpecialValueText("Inf");
//  formDyn61.setWrapping(true);
//  if( gen->M() == formDyn61.maximum() )
//    formDyn61.setValue(0);
//  else
    formDyn61.setValue( gen->M() );
  formDyn62.setValue( gen->D() );
  layoutDyn6.addWidget( &labelDyn61 );
  layoutDyn6.addWidget( &formDyn61 );
  layoutDyn6.addWidget( &labelDyn62 );
  layoutDyn6.addWidget( &formDyn62 );
  layoutMain.addLayout( &layoutDyn6 );

  // status
  QHBoxLayout layoutStatus;
  QLabel labelStatus("Status:");
  QRadioButton statusOn("On-line", &labelStatus);
  if( gen->status()==true ) statusOn.setChecked(true);
  QRadioButton statusOff("Off-line", &labelStatus);
  if( gen->status()==false ) statusOff.setChecked(true);
  QHBoxLayout layoutSubstatus;
  layoutSubstatus.addWidget( &statusOn );
  layoutSubstatus.addWidget( &statusOff );
  layoutStatus.addWidget( &labelStatus );
  layoutStatus.addLayout( &layoutSubstatus );
  layoutMain.addLayout( &layoutStatus );
  // Buttons
  QHBoxLayout layoutButtons;
  QPushButton ok("Ok");
  QPushButton cancel("Cancel");
  dialog.connect(&ok, SIGNAL(clicked()), &dialog, SLOT(accept()));
  dialog.connect(&cancel, SIGNAL(clicked()), &dialog, SLOT(reject()));
  layoutButtons.addWidget( &ok );
  layoutButtons.addWidget( &cancel );
  layoutMain.addLayout( &layoutButtons );
  // Execute dialog
  if( dialog.exec() ){
    // Dialog executed properly
    // Store values back
    gen->set_extId( formExtId.value() );
    gen->set_name( formName.text().toStdString() );
    gen->set_busExtId( formBus.value() );
    gen->set_voltageSetpoint( formVPoint.value() );
    gen->set_avr( formAvr.isChecked() );
    gen->set_pmin( formPMin.value() );
    gen->set_pgen( formPGen.value() );
    gen->set_pmax( formPMax.value() );
    gen->set_qmin( formQMin.value() );
    gen->set_qgen( formQGen.value() );
    gen->set_qmax( formQMax.value() );
    gen->set_status( statusOn.isChecked() );
    // dynamic data
    gen->set_model( formDyn01.value() );
    gen->set_xl( formDyn11.value() );
    gen->set_ra( formDyn12.value() );
    gen->set_xd( formDyn21.value() );
    gen->set_xd_1( formDyn22.value() );
    gen->set_xd_2( formDyn23.value() );
    gen->set_Td0_1( formDyn31.value() );
    gen->set_Td0_2( formDyn32.value() );
    gen->set_xq( formDyn41.value() );
    gen->set_xq_1( formDyn42.value() );
    gen->set_xq_2( formDyn43.value() );
    gen->set_Tq0_1( formDyn51.value() );
    gen->set_Tq0_2( formDyn52.value() );
    if( formDyn61.value()==0 )
      gen->set_M( formDyn61.maximum() );
    else
      gen->set_M( formDyn61.value() );
    gen->set_D( formDyn62.value() );

    return 0;

  } else{
    // Dialog not executed properly
    return 1;
  }

  return 0;
}

int pwsEditorDialogs::loadDialog(Load *load){
  // Main dialog
  QDialog dialog;
  dialog.setWindowTitle("Property Editor: Load");
  QVBoxLayout layoutMain;
  dialog.setLayout(&layoutMain);
  // Properties
  // extId
  QHBoxLayout layoutExtId;
  QLabel labelExtId("Id:");
  QSpinBox formExtId;
  formExtId.setMaximum( RAND_MAX );
  formExtId.setValue( load->extId() );
  layoutExtId.addWidget( &labelExtId );
  layoutExtId.addWidget( &formExtId );
  layoutMain.addLayout( &layoutExtId );
  // name
  QHBoxLayout layoutName;
  QLabel labelName("Name:");
  QLineEdit formName;
  formName.setText( load->name().c_str() );
  layoutName.addWidget( &labelName );
  layoutName.addWidget( &formName );
  layoutMain.addLayout( &layoutName );
  // type
  QHBoxLayout layoutType;
  QLabel labelType("Type:");
  QVBoxLayout layoutSubtype;
  QButtonGroup typeGroup( &labelType );
  QRadioButton typeS("Constant Power", &labelType);
  typeGroup.addButton( &typeS );
  if( load->type() == LOADTYPE_CONSTP )
    typeS.setChecked(true);
  QRadioButton typeI("Constant Current", &labelType);
  typeGroup.addButton( &typeI );
  if( load->type() == LOADTYPE_CONSTI )
    typeI.setChecked(true);
  QRadioButton typeZ("Constant Impedance", &labelType);
  typeGroup.addButton( &typeZ );
  if( load->type() == LOADTYPE_CONSTZ )
    typeZ.setChecked(true);
  layoutSubtype.addWidget( &typeS );
  layoutSubtype.addWidget( &typeI );
  layoutSubtype.addWidget( &typeZ );
  layoutType.addWidget( &labelType );
  layoutType.addLayout( &layoutSubtype );
  layoutMain.addLayout( &layoutType );
  // at bus
  QHBoxLayout layoutBus;
  QLabel labelBus("At bus:");
  QSpinBox formBus;
  formBus.setMaximum( RAND_MAX );
  formBus.setValue( load->busExtId() );
  layoutBus.addWidget( &labelBus );
  layoutBus.addWidget( &formBus );
  layoutMain.addLayout( &layoutBus );
  // p power
  QHBoxLayout layoutPPower;
  layoutMain.addLayout( &layoutPPower );
  QLabel labelPPower("Active Power [pu]:");
  QDoubleSpinBox formPPower;
  formPPower.setMinimum(-10000);
  formPPower.setValue( load->pdemand() );
  layoutPPower.addWidget( &labelPPower );
  layoutPPower.addWidget( &formPPower );
  // q power
  QHBoxLayout layoutQPower;
  layoutMain.addLayout( &layoutQPower );
  QLabel labelQPower("Reactive Power [pu]:");
  QDoubleSpinBox formQPower;
  formQPower.setMinimum(-10000);
  formQPower.setValue( load->qdemand() );
  layoutQPower.addWidget( &labelQPower );
  layoutQPower.addWidget( &formQPower );
//  // p parameters
//  QHBoxLayout layoutPParam;
//  layoutMain.addLayout( &layoutPParam );
//  QLabel labelPParam("Active power parameters (k_pf, v_exp");
//  layoutPParam.addWidget( &labelPParam );
//  QDoubleSpinBox formPParamKPF;
//  formPParamKPF.setValue( load->k_p_f() );
//  layoutPParam.addWidget( &formPParamKPF );
//  QDoubleSpinBox formPParamVExp;
//  formPParamVExp.setValue( load->v_exp_a() );
//  layoutPParam.addWidget( &formPParamVExp );
//  // q parameters
//  QHBoxLayout layoutQParam;
//  layoutMain.addLayout( &layoutQParam );
//  QLabel labelQParam("Reactive power parameters (k_pf, v_exp");
//  layoutQParam.addWidget( &labelQParam );
//  QDoubleSpinBox formQParamKQF;
//  formQParamKQF.setValue( load->k_q_f() );
//  layoutQParam.addWidget( &formQParamKQF );
//  QDoubleSpinBox formQParamVExp;
//  formQParamVExp.setValue( load->v_exp_b() );
//  layoutQParam.addWidget( &formQParamVExp );
  // status
  QHBoxLayout layoutStatus;
  QLabel labelStatus("Status:");
  QRadioButton statusOn("On-line", &labelStatus);
  if( load->status()==1 ) statusOn.setChecked(true);
  QRadioButton statusOff("Off-line", &labelStatus);
  if( load->status()==0 ) statusOff.setChecked(true);
  QHBoxLayout layoutSubstatus;
  layoutSubstatus.addWidget( &statusOn );
  layoutSubstatus.addWidget( &statusOff );
  layoutStatus.addWidget( &labelStatus );
  layoutStatus.addLayout( &layoutSubstatus );
  layoutMain.addLayout( &layoutStatus );
  // Buttons
  QHBoxLayout layoutButtons;
  QPushButton ok("Ok");
  QPushButton cancel("Cancel");
  dialog.connect(&ok, SIGNAL(clicked()), &dialog, SLOT(accept()));
  dialog.connect(&cancel, SIGNAL(clicked()), &dialog, SLOT(reject()));
  layoutButtons.addWidget( &ok );
  layoutButtons.addWidget( &cancel );
  layoutMain.addLayout( &layoutButtons );
  // Execute dialog
  if( dialog.exec() ){
    // Dialog executed properly
    // Determine values to be stored based on dialog widgets
    unsigned int type;
    if( typeS.isChecked() )
      type = LOADTYPE_CONSTP;
    else if( typeI.isChecked() )
      type = LOADTYPE_CONSTI;
    else if( typeZ.isChecked() )
      type = LOADTYPE_CONSTZ;

    // Store values back
    load->set_extId( formExtId.value() );
    load->set_name( formName.text().toStdString() );
    load->set_type( type );
    load->set_busExtId( formBus.value() );
    load->set_pdemand( formPPower.value() );
    load->set_qdemand( formQPower.value() );
    load->set_status( statusOn.isChecked() );
//    // TODO: Dependent variables on load type
//    load->set_k_p_f( formPParamKPF.value() );
//    load->set_v_exp_a( formPParamVExp.value() );
//    load->set_k_q_f( formQParamKQF.value() );
//    load->set_v_exp_b( formQParamVExp.value() );

    return 0;

  } else{
    // Dialog not executed properly
    return 1;
  }
  return 0;
}
