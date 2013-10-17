
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
  formGsh.setValue( bus->Gsh );
  layoutGsh.addWidget( &labelGsh );
  layoutGsh.addWidget( &formGsh );
  layoutMain.addLayout( &layoutGsh );
  // shunt susceptance
  QHBoxLayout layoutBsh;
  QLabel labelBsh("Shunt susceptance [pu]:");
  QDoubleSpinBox formBsh;
  formBsh.setValue( bus->Bsh );
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
    bus->Gsh = formGsh.value();
    bus->Bsh = formBsh.value();
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
  formExtId.setValue( branch->extId );
  layoutExtId.addWidget( &labelExtId );
  layoutExtId.addWidget( &formExtId );
  layoutMain.addLayout( &layoutExtId );
  // status
  QHBoxLayout layoutStatus;
  QLabel labelStatus("Status:");
  QRadioButton statusOn("On-line", &labelStatus);
  if(branch->status) statusOn.setChecked(true);
  QRadioButton statusOff("Off-line", &labelStatus);
  if(!branch->status) statusOff.setChecked(true);
  QHBoxLayout layoutSubstatus;
  layoutSubstatus.addWidget( &statusOn );
  layoutSubstatus.addWidget( &statusOff );
  layoutStatus.addWidget( &labelStatus );
  layoutStatus.addLayout( &layoutSubstatus );
  layoutMain.addLayout( &layoutStatus );
  // formBus & toBus
  QHBoxLayout layoutBuses;
  QLabel labelBuses("From/To bus:");
  QSpinBox formFromBus, formToBus;
  formFromBus.setMaximum( RAND_MAX );
  formFromBus.setValue( branch->fromBusExtId );
  formToBus.setMaximum( RAND_MAX );
  formToBus.setValue( branch->toBusExtId );
  layoutBuses.addWidget( &labelBuses );
  layoutBuses.addWidget( &formFromBus );
  layoutBuses.addWidget( &formToBus );
  layoutMain.addLayout( &layoutBuses );
  // resistance
  QHBoxLayout layoutR;
  QLabel labelR("Resistance [pu]:");
  QDoubleSpinBox formR;
  formR.setDecimals( 5 );
  formR.setValue( branch->R );
  layoutR.addWidget( &labelR );
  layoutR.addWidget( &formR );
  layoutMain.addLayout( &layoutR );
  // reactance
  QHBoxLayout layoutX;
  QLabel labelX("Reactance [pu]:");
  QDoubleSpinBox formX;
  formX.setDecimals( 5 );
  formX.setValue( branch->X );
  layoutX.addWidget( &labelX );
  layoutX.addWidget( &formX );
  layoutMain.addLayout( &layoutX );
  // susceptance to ground
  QHBoxLayout layoutB;
  QLabel labelB("Ground Susceptance (near/far end) [pu]:");
  QDoubleSpinBox formBfr, formBto;
  formBfr.setDecimals( 5 );
  formBto.setDecimals( 5 );
  formBfr.setValue( branch->Bfrom );
  formBto.setValue( branch->Bto );
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
  formGfr.setValue( branch->Gfrom );
  formGto.setValue( branch->Gto );
  layoutG.addWidget( &labelG );
  layoutG.addWidget( &formGfr );
  layoutG.addWidget( &formGto );
  layoutMain.addLayout( &layoutG );
  // x ratio
  QHBoxLayout layoutXratio;
  QLabel labelXratio("Transformer ratio:");
  QDoubleSpinBox formXratio;
  formXratio.setDecimals( 5 );
  formXratio.setValue( branch->Xratio );
  layoutXratio.addWidget( &labelXratio );
  layoutXratio.addWidget( &formXratio );
  layoutMain.addLayout( &layoutXratio );
  // x shift
  QHBoxLayout layoutXshift;
  QLabel labelXshift("Transformer shift:");
  QDoubleSpinBox formXshift;
  formXshift.setDecimals( 5 );
  formXshift.setValue( branch->Xshift );
  layoutXshift.addWidget( &labelXshift );
  layoutXshift.addWidget( &formXshift );
  layoutMain.addLayout( &layoutXshift );
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
    branch->extId = formExtId.value();
    branch->status = statusOn.isChecked();
    branch->fromBusExtId = formFromBus.value();
    branch->toBusExtId = formToBus.value();
    branch->R = formR.value();
    branch->X = formX.value();
    branch->Bfrom = formBfr.value();
    branch->Bto = formBto.value();
    branch->Gfrom = formGfr.value();
    branch->Gto = formBto.value();
    branch->Xratio = formXratio.value();
    branch->Xshift = formXshift.value();

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
  QLabel labelExtId("extId:");
  QSpinBox formExtId;
  formExtId.setMaximum( RAND_MAX );
  formExtId.setValue( gen->extId );
  layoutExtIdBus.addWidget( &labelExtId );
  layoutExtIdBus.addWidget( &formExtId );
  QLabel labelBus("busExtId:");
  QSpinBox formBus;
  formBus.setMaximum( RAND_MAX );
  formBus.setValue( gen->busExtId );
  layoutExtIdBus.addWidget( &labelBus );
  layoutExtIdBus.addWidget( &formBus );
  layoutMain.addLayout( &layoutExtIdBus );
  // name
  QHBoxLayout layoutName;
  QLabel labelName("name:");
  QLineEdit formName;
  formName.setText( gen->name.c_str() );
  layoutName.addWidget( &labelName );
  layoutName.addWidget( &formName );
  layoutMain.addLayout( &layoutName );

  // dynamic data
  QLabel labelDynamic("<br>Dynamic data<hr>");
  layoutMain.addWidget( &labelDynamic );
  // model
  QHBoxLayout layoutDyn0;
  QLabel labelDyn01("model:");
  QSpinBox formDyn01;
  formDyn01.setValue(gen->model);
  layoutDyn0.addWidget( &labelDyn01 );
  layoutDyn0.addWidget( &formDyn01 );
  layoutMain.addLayout( &layoutDyn0 );
  // leakage reactance, armature resistance
  QHBoxLayout layoutDyn1;
  QLabel labelDyn11("xl [pu]:");
  QLabel labelDyn12("ra [pu]:");
  QDoubleSpinBox formDyn11, formDyn12;
  formDyn11.setDecimals( 5 );
  formDyn12.setDecimals( 5 );
  formDyn11.setValue( gen->xl );
  formDyn12.setValue( gen->ra );
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
  QLabel labelDyn21("xd/xd_1/xd_2 [pu]:");
  QDoubleSpinBox formDyn21, formDyn22, formDyn23;
  formDyn21.setDecimals(5);
  formDyn22.setDecimals(5);
  formDyn23.setDecimals(5);
  formDyn21.setValue(gen->xd );
  formDyn22.setValue(gen->xd_1);
  formDyn23.setValue(gen->xd_2);
  layoutDyn2.addWidget(&labelDyn21);
  layoutDyn2.addWidget(&formDyn21);
  layoutDyn2.addWidget(&formDyn22);
  layoutDyn2.addWidget(&formDyn23);
  layoutMain.addLayout(&layoutDyn2);
  // time constants
  QHBoxLayout layoutDyn3;
  QLabel labelDyn31("Td0_1/Td0_2 [s]:");
  QDoubleSpinBox formDyn31, formDyn32;
  formDyn31.setDecimals(5);
  formDyn32.setDecimals(5);
  formDyn31.setValue( gen->Td0_1 );
  formDyn32.setValue( gen->Td0_2 );
  layoutDyn3.addWidget( &labelDyn31 );
  layoutDyn3.addWidget( &formDyn31 );
  layoutDyn3.addWidget( &formDyn32 );
  layoutMain.addLayout( &layoutDyn3 );
  // quadrature axis
  QLabel labelQuadAxis("<br>Quadrature Axis:");
  layoutMain.addWidget( &labelQuadAxis );
  // reactance
  QHBoxLayout layoutDyn4;
  QLabel labelDyn41("xq/xq_1/xq_2 [pu]:");
  QDoubleSpinBox formDyn41, formDyn42, formDyn43;
  formDyn41.setDecimals(5);
  formDyn42.setDecimals(5);
  formDyn43.setDecimals(5);
  formDyn41.setValue( gen->xq );
  formDyn42.setValue( gen->xq_1 );
  formDyn43.setValue( gen->xq_2 );
  layoutDyn4.addWidget( &labelDyn41 );
  layoutDyn4.addWidget( &formDyn41 );
  layoutDyn4.addWidget( &formDyn42 );
  layoutDyn4.addWidget( &formDyn43 );
  layoutMain.addLayout( &layoutDyn4 );
  // time constants
  QHBoxLayout layoutDyn5;
  QLabel labelDyn51("Tq0_1/Tq0_2 [s]:");
  QDoubleSpinBox formDyn51, formDyn52;
  formDyn51.setDecimals(5);
  formDyn52.setDecimals(5);
  formDyn51.setValue(gen->Tq0_1);
  formDyn52.setValue(gen->Tq0_2);
  layoutDyn5.addWidget( &labelDyn51 );
  layoutDyn5.addWidget( &formDyn51 );
  layoutDyn5.addWidget( &formDyn52 );
  layoutMain.addLayout( &layoutDyn5 );
  // Mech starting t & damping coeff
  QHBoxLayout layoutDyn6;
  QLabel labelDyn61("M [kWs/kVA]:");
  QLabel labelDyn62("D [pu]:");
  QDoubleSpinBox formDyn61, formDyn62;
  formDyn61.setDecimals(5);
  formDyn62.setDecimals(5);
  formDyn61.setMaximum(10000000.0);
//  formDyn61.setSpecialValueText("Inf");
//  formDyn61.setWrapping(true);
//  if( gen->M() == formDyn61.maximum() )
//    formDyn61.setValue(0);
//  else
    formDyn61.setValue( gen->M );
  formDyn62.setValue( gen->D );
  layoutDyn6.addWidget( &labelDyn61 );
  layoutDyn6.addWidget( &formDyn61 );
  layoutDyn6.addWidget( &labelDyn62 );
  layoutDyn6.addWidget( &formDyn62 );
  layoutMain.addLayout( &layoutDyn6 );

  // status
  QHBoxLayout layoutStatus;
  QLabel labelStatus("Status:");
  QRadioButton statusOn("On-line", &labelStatus);
  if( gen->status==true ) statusOn.setChecked(true);
  QRadioButton statusOff("Off-line", &labelStatus);
  if( gen->status==false ) statusOff.setChecked(true);
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
    gen->extId=formExtId.value();
    gen->status=statusOn.isChecked();
    gen->name=formName.text().toStdString();
    gen->busExtId=formBus.value();
    // dynamic data
    gen->model=formDyn01.value();
    gen->xl=formDyn11.value();
    gen->ra=formDyn12.value();
    gen->xd=formDyn21.value();
    gen->xd_1=formDyn22.value();
    gen->xd_2=formDyn23.value();
    gen->Td0_1=formDyn31.value();
    gen->Td0_2=formDyn32.value();
    gen->xq=formDyn41.value();
    gen->xq_1=formDyn42.value();
    gen->xq_2=formDyn43.value();
    gen->Tq0_1=formDyn51.value();
    gen->Tq0_2=formDyn52.value();
    if( formDyn61.value()==0 )
      gen->M=formDyn61.maximum();
    else
      gen->M=formDyn61.value();
    gen->D=formDyn62.value();

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
  formExtId.setValue( load->extId );
  layoutExtId.addWidget( &labelExtId );
  layoutExtId.addWidget( &formExtId );
  layoutMain.addLayout( &layoutExtId );
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
  formBus.setValue( load->busExtId );
  layoutBus.addWidget( &labelBus );
  layoutBus.addWidget( &formBus );
  layoutMain.addLayout( &layoutBus );

  // dynamic parameters: Vexpa, Vexpb, kpf, kqf
  // TODO

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
    load->extId = formExtId.value();
    load->busExtId = formBus.value();
    load->setType( type );
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
