
#include "scenarioeditor.h"
using namespace elabtsaot;

#include "event.h"
#include "scenarioset.h"
#include "autogeneratescenariosdialog.h"

#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QToolBar>
#include <QAction>
#include <QIcon>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QList>
#include <QStringList>
#include <QCheckBox>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QFormLayout>

#include <iostream>
using std::cout;
using std::endl;

ScenarioEditor::ScenarioEditor( ScenarioSet* scs, Powersystem const* pws,
                                QWidget *parent ) :
    QSplitter( Qt::Vertical, parent), _scs(scs), _pws(pws) {

  // ----- Main tree widget -----
  scenariosTree = new QTreeWidget(this);
  addWidget(scenariosTree);

  scenariosTree->setColumnCount(16);
  QStringList headerlist;
  headerlist << "Name"          // header for column 0
             << "Status"        // header for column 1
             << "Start [s]"     // header for column 2
             << "Stop [s]"      // header for column 3
             << "Pws comp"      // header for column 4
             << "CCT comp"      // header for column 5
             << "TSA comp"      // header for column 6
             << "Element type"  // header for column 7
             << "Element extId" // header for column 8
             << "Event type"    // header for column 9
             << "Time"          // header for column 10
             << "Bool"          // header for column 11
             << "Double"        // header for column 12
             << "Double1"       // header for column 13
             << "Double2"       // header for column 14
             << "Desc";         // header for column 15
  scenariosTree->setHeaderLabels(headerlist);
  for ( int k = 0 ; k != scenariosTree->columnCount() ; ++k )
    scenariosTree->resizeColumnToContents(k);

  scenariosTree->setColumnWidth(0,200); // Name
  //  scenariosTree->setColumnWidth(1,50); // Status
  //  scenariosTree->setColumnWidth(2,70); // Start time
  //  scenariosTree->setColumnWidth(3,70); // Stop time
  //  scenariosTree->setColumnWidth(4,150); // Powersystem comp
  //  scenariosTree->setColumnWidth(5,150); // CCT comp
  //  scenariosTree->setColumnWidth(6,150); // TSA comp
  //  scenariosTree->setColumnWidth(7,100); // Element type
  //  scenariosTree->setColumnWidth(9,120); // Element extId
  //  scenariosTree->setColumnWidth(10,100); // Event type
  //  scenariosTree->setColumnWidth(11,80); // Time
  //  scenariosTree->setColumnWidth(12,50); // bool arg
  //  scenariosTree->setColumnWidth(13,50); // arg0
  //  scenariosTree->setColumnWidth(14,50); // arg1
  //  scenariosTree->setColumnWidth(15,200); // arg2

  QStringList rootlist;
  rootlist << "Scenario Set";
  _rootItem = new QTreeWidgetItem(rootlist);
  scenariosTree->addTopLevelItem(_rootItem);
  _rootItem->setIcon(0,QIcon(":/images/lightning.png"));
  scenariosTree->expandAll();

  connect( scenariosTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
           this, SLOT(itemChanged(QTreeWidgetItem*,int)) );

  // ----- Scenario Editor Toolbar -----
  QToolBar* sceToolbar = new QToolBar("Scenario editor toolbar",this);
  addWidget(sceToolbar);
  sceToolbar->setFixedHeight(30);
  sceToolbar->addSeparator();

  // Add scenario
  QAction* addScenarioAct = new QAction( QIcon(":/images/scenario.png"),
                                         "Add Scenario", this );
  sceToolbar->addAction(addScenarioAct);
  connect( addScenarioAct, SIGNAL(triggered()),
           this, SLOT(addScenarioSlot()) );

  // Add event
  QAction* addEventAct = new QAction( QIcon(":/images/event.png"),
                                      "Add Event", this );
  sceToolbar->addAction(addEventAct);
  connect( addEventAct, SIGNAL(triggered()),
           this, SLOT(addEventSlot()) );

  // Edit selected
  QAction* editSelectedAct = new QAction( QIcon(":/images/event_edit.png"),
                                          "Edit selected", this);
  sceToolbar->addAction(editSelectedAct);
  connect( editSelectedAct, SIGNAL(triggered()),
           this, SLOT(editSelectedSlot()) );

  // Delete selected
  QAction* deleteSelectedAct = new QAction( QIcon(":/images/delete.png"),
                                            "Delete selected", this );
  sceToolbar->addAction(deleteSelectedAct);
  connect( deleteSelectedAct, SIGNAL(triggered()),
           this, SLOT(deleteSelectedSlot()) );

  // Delete all
  QAction* deleteAllAct = new QAction( QIcon(":/images/delete.png"),
                                       "Delete all", this );
  sceToolbar->addAction(deleteAllAct);
  connect( deleteAllAct, SIGNAL(triggered()),
           this, SLOT(deleteAllSlot()) );

  sceToolbar->addSeparator(); // ----------

  // Auto generate scenarios
  QAction* autoGenerateScenariosAct = new QAction( QIcon(":/images/.png"),
                                              "Auto generate scenarios", this );
  sceToolbar->addAction( autoGenerateScenariosAct );
  connect( autoGenerateScenariosAct, SIGNAL(triggered()),
           this, SLOT(autoGenerateScenariosSlot()) );

  sceToolbar->addSeparator(); // ----------

  // Check powersystem compatibility
  QAction* checkPwsCompatibilitySlotAct = new QAction ( QIcon(":/images/pwscomp.png"),
                                                    "Check power system compatibility",this);
  sceToolbar->addAction(checkPwsCompatibilitySlotAct);
  connect( checkPwsCompatibilitySlotAct, SIGNAL(triggered()),
           this, SLOT(checkPwsCompatibilitySlot()) );

  // Check CCT compatibility
  QAction* checkCCTCompatibilitySlotAct = new QAction ( QIcon(":/images/cct_checks.png"),
                                                    "Check CCT compatibility",this);
  sceToolbar->addAction(checkCCTCompatibilitySlotAct);
  connect( checkCCTCompatibilitySlotAct, SIGNAL(triggered()),
           this, SLOT(checkCCTCompatibilitySlot()) );

  // Check TSA compatibility
  QAction* checkTSACompatibilitySlotAct = new QAction ( QIcon(":/images/tsa_checks.png"),
                                                    "Check TSA compatibility",this);
  sceToolbar->addAction(checkTSACompatibilitySlotAct);
  connect( checkTSACompatibilitySlotAct, SIGNAL(triggered()),
           this, SLOT(checkTSACompatibilitySlot()) );

  // Sort events of scenarios
  QAction* sortScenarioEventsSlotAct = new QAction ( QIcon(":/images/sort.png"),
                                                 "Sort Events of scenarios",this);
  sceToolbar->addAction(sortScenarioEventsSlotAct);
  connect( sortScenarioEventsSlotAct, SIGNAL(triggered()),
           this, SLOT(sortScenarioEventsSlot()) );
}

void ScenarioEditor::updt(){

  _rootItem->takeChildren();
  scenariosTree->blockSignals(true);

  for(size_t i = 0 ; i < _scs->scenarios.size() ; ++i) {
    _addScenarioItem( _scs->scenarios[i] );
    for(size_t k = 0 ; k != _scs->scenarios[i].getEventSetSize() ; ++k){
      _addEventItem( _scs->scenarios[i].getEvent(k), i );
    }
  }

  // Update compatibility flags
  checkPwsCompatibilitySlot();
  checkCCTCompatibilitySlot();
  checkTSACompatibilitySlot();

  scenariosTree->blockSignals(false);
}

void ScenarioEditor::addScenarioSlot(){

  Scenario* tempScenario = new Scenario( rand()%1000 );
  int ans = _scenarioDialog( tempScenario );
  if ( ans )
    return;

  // Add new scenario to the scenario set
  _scs->scenarios.push_back(*tempScenario);

  // Add new scenario item to the tree widget
  _addScenarioItem( *tempScenario );

  delete tempScenario;
  return;
}

void ScenarioEditor::addEventSlot(){

  // Check whether there is a scenario to add the event to
  if ( _scs->scenarios.size() <= 0 )
    return;

  int sceId = -1;
  Event* tempEvent = new Event();
  int ans = _eventDialog( tempEvent, &sceId );
  if ( ans )
    return;

  // Add new event to the scenario set
  _scs->scenarios[sceId].insertEvent(*tempEvent);
  // Add new event item to the tree widget
  _addEventItem( *tempEvent, sceId );

  delete tempEvent;
  return;
}

void ScenarioEditor::editSelectedSlot(){

  QList<QTreeWidgetItem*> selection = scenariosTree->selectedItems();
  if ( selection.count() != 1 )
    return;

  QTreeWidgetItem* item = selection.first();
  if ( item == _rootItem )
    // Item is the tree root
    return;

  int id = item->parent()->indexOfChild( item );
  if ( item->parent() == _rootItem ){
    // Selected item is a scenario
    _scenarioDialog( &_scs->scenarios[id] );
  } else {
    // Selected item is an event
    QTreeWidgetItem* sceItem = item->parent();
    int sceId = sceItem->parent()->indexOfChild( sceItem );
    _eventDialog( _scs->scenarios[sceId].getRawEvent(id), &sceId );
  }
  updt();

  return;
}

void ScenarioEditor::deleteSelectedSlot(){

  QList<QTreeWidgetItem*> selection = scenariosTree->selectedItems();
  if ( selection.count() != 1 )
    return;

  QTreeWidgetItem* item = selection.first();
  if ( item == _rootItem )
    // Item is the tree root
    return;

  int ans;
  int id = item->parent()->indexOfChild( item );
  if ( item->parent() == _rootItem ){
    // Selected item is a scenario
    ans = _scs->deleteScenario( id );
  } else {
    // Selected item is an event
    QTreeWidgetItem* sceItem = item->parent();
    int sceId = sceItem->parent()->indexOfChild( sceItem );
    ans = _scs->scenarios[sceId].removeEvent(id);
  }
  if ( ans )
    cout << "Deleting of element failed with code " << ans << endl;
  else
    cout << "Deleting of element succeded!" << endl;
  updt();

  return;
}

void ScenarioEditor::deleteAllSlot(){
  _scs->scenarios.clear();
  updt();
  return;
}

void ScenarioEditor::autoGenerateScenariosSlot(){

  // Spawn auto generate scenarios dialog
  bool trip, ignoreLowZBranches;
  double scenarioStopTime, faultLocation, faultStart, faultStop;
  AutoGenerateScenariosDialog* dialog( new AutoGenerateScenariosDialog(
    scenarioStopTime, trip, ignoreLowZBranches,
    faultLocation, faultStart, faultStop, this) );
  int ans = dialog->exec();
  if ( ans ) return;

  // Create scenarios according to user inputs
  _scs->createScenariosPerBranch( _pws, scenarioStopTime, trip,
                     ignoreLowZBranches, faultLocation, faultStart, faultStop );

  // Update GUI
  updt();

  return;
}

void ScenarioEditor::checkPwsCompatibilitySlot(){

  scenariosTree->blockSignals(true);
  for (int k=0;k<_rootItem->childCount();++k){
    if  (_scs->scenarios[k].checkPwsCompatibility(*_pws)){
      _rootItem->child(k)->setData( 4, Qt::DisplayRole, QVariant("Yes"));
      _rootItem->child(k)->setBackgroundColor( 4, Qt::green);
    } else{
      _rootItem->child(k)->setData( 4, Qt::DisplayRole, QVariant("No"));
      _rootItem->child(k)->setBackgroundColor( 4, Qt::red);
    }
  }
  scenariosTree->blockSignals(false);

  return;
}

void ScenarioEditor::checkCCTCompatibilitySlot(){

  scenariosTree->blockSignals( true );
  for ( int k = 0 ; k != _rootItem->childCount() ; ++k ){
    if ( _scs->scenarios[k].checkCCTCompatibility() ){
      _rootItem->child(k)->setData( 5, Qt::DisplayRole, QVariant("True") );
      _rootItem->child(k)->setBackgroundColor( 5, Qt::green );
    } else {
      _rootItem->child(k)->setData( 5, Qt::DisplayRole, QVariant("False") );
      _rootItem->child(k)->setBackgroundColor( 5, Qt::red );
    }
  }
  scenariosTree->blockSignals( false );

  return;
}

void ScenarioEditor::checkTSACompatibilitySlot(){

  scenariosTree->blockSignals( true );
  for ( int k = 0 ; k != _rootItem->childCount() ; ++k ){
    if ( _scs->scenarios[k].checkTSACompatibility() ){
      _rootItem->child(k)->setData( 6, Qt::DisplayRole, QVariant("True") );
      _rootItem->child(k)->setBackgroundColor( 6, Qt::green );
    } else {
      _rootItem->child(k)->setData( 6, Qt::DisplayRole, QVariant("False") );
      _rootItem->child(k)->setBackgroundColor( 6, Qt::red );
    }
  }
  scenariosTree->blockSignals( false );

  return;
}

void ScenarioEditor::sortScenarioEventsSlot(){
  for (size_t k=0;k<_scs->scenarios.size();++k)
    _scs->scenarios[k].sort_t();
  for (size_t k=0;k<_scs->scenarios.size();++k){
    scenariosTree->blockSignals(true);
    _rootItem->child(k)->sortChildren(10,Qt::AscendingOrder);
    scenariosTree->blockSignals(false);
  }
}

void ScenarioEditor::itemChanged( QTreeWidgetItem* item, int column ){

  scenariosTree->blockSignals(true);

  // Retrieve changed event
  int indexofevent = item->parent()->indexOfChild(item);
  int indexofscenario = item->parent()->parent()->indexOfChild(item->parent());
  Event* evn( _scs->scenarios[indexofscenario].getRawEvent(indexofevent) );

  bool ok;
  unsigned int tempUInt;

  switch ( column ){
  case 0:
    evn->set_name(item->data(0,Qt::DisplayRole).toString().toStdString());
    break;

  case 7: // Element type
    tempUInt = item->data(7,Qt::DisplayRole).toInt(&ok);
    evn->set_element_type( tempUInt );
    if ( item->data(7,Qt::DisplayRole).toInt()
           == EVENT_ELEMENT_TYPE_BUS ) {
      item->setIcon(7,QIcon(":/images/bus.png"));
      item->setData(7,Qt::DisplayRole,QVariant("0: Bus"));
    } else if ( item->data(7,Qt::DisplayRole).toInt()
                  == EVENT_ELEMENT_TYPE_BRANCH ) {
      item->setIcon(7,QIcon(":/images/branch.png"));
      item->setData(7,Qt::DisplayRole,QVariant("1: Branch"));
    } else if ( item->data(7,Qt::DisplayRole).toInt()
                  == EVENT_ELEMENT_TYPE_GEN ) {
      item->setIcon(7,QIcon(":/images/generator.png"));
      item->setData(7,Qt::DisplayRole,QVariant("2: Generator"));
    } else if ( item->data(7,Qt::DisplayRole).toInt()
                  == EVENT_ELEMENT_TYPE_LOAD ) {
      item->setIcon(7,QIcon(":/images/load.png"));
      item->setData(7,Qt::DisplayRole,QVariant("3: Load"));
    } else { // Should never be reached
      item->setIcon(7,QIcon());
      item->setData(7,Qt::DisplayRole,QVariant("Unknown element"));
    }
    break;

  case 8: // Element extId
    if (item->data(8,Qt::DisplayRole).toInt()<0){
      cout<<"Please enter a positive number for extid"<<endl;
      scenariosTree->blockSignals(false);
      return;
    }
    evn->set_element_extId(item->data(8,Qt::DisplayRole).toInt());
    break;

  case 9:{ // Event type
    // Store user input
    evn->set_event_type(item->data(9,Qt::DisplayRole).toInt());
    // Decorate GUI
    QString eventTypeLabel;
    switch ( evn->element_type() ){
    case EVENT_ELEMENT_TYPE_BUS:
      switch ( item->data(9,Qt::DisplayRole).toInt() ){
      case EVENT_EVENTTYPE_BUSFAULT:
        eventTypeLabel = "0: 3ph fault";
        break;
      default:
        eventTypeLabel = "Unknown event type";
        break;
      }
      break;

    case EVENT_ELEMENT_TYPE_BRANCH:
      switch ( item->data(9,Qt::DisplayRole).toInt() ){
      case EVENT_EVENTTYPE_BRFAULT:
        eventTypeLabel = "0: 3ph fault";
        break;
      case EVENT_EVENTTYPE_BRTRIP:
        eventTypeLabel = "1: trip";
        break;
      case EVENT_EVENTTYPE_BRSHORT:
        eventTypeLabel = "2: short";
        break;
      default:
        eventTypeLabel = "Unknown event type";
        break;
      }
      break;

    case EVENT_ELEMENT_TYPE_GEN:
      switch ( item->data(9,Qt::DisplayRole).toInt() ){
      case EVENT_EVENTTYPE_GENTRIP:
        eventTypeLabel = "0: trip";
        break;
      case EVENT_EVENTTYPE_GENPCHANGE:
        eventTypeLabel = "1: P change";
        break;
      case EVENT_EVENTTYPE_GENQCHANGE:
        eventTypeLabel = "2: Q change";
        break;
      case EVENT_EVENTTYPE_GENVCHANGE:
        eventTypeLabel = "3: Vset change";
        break;
      default:
        eventTypeLabel = "Unknown event type";
        break;
      }
      break;

    case EVENT_ELEMENT_TYPE_LOAD:
      switch ( item->data(9,Qt::DisplayRole).toInt() ){
      case EVENT_EVENTTYPE_LOADTRIP:
        eventTypeLabel = "0: trip";
        break;
      case EVENT_EVENTTYPE_LOADPCHANGE:
        eventTypeLabel = "1: P change";
        break;
      case EVENT_EVENTTYPE_LOADQCHANGE:
        eventTypeLabel = "2: Q change";
        break;
      default:
        eventTypeLabel = "Unknown event type";
        break;
      }
      break;

    default:
      eventTypeLabel = "Unknown event type";
      break;
    }
    item->setData( 9, Qt::DisplayRole, QVariant(eventTypeLabel) );
    break;}

  case 10: // Time
    if (item->data(10,Qt::DisplayRole).toDouble()<0){
      cout<<"Please enter a positive number for time"<<endl;
      scenariosTree->blockSignals(false);
      return;
    }
    evn->set_time(item->data(10,Qt::DisplayRole).toDouble());
    break;

  case 11: // Bool arg
    evn->set_bool_arg(item->data(11,Qt::DisplayRole).toBool());
    if (item->data(11,Qt::DisplayRole).toBool()){
      item->setIcon(11,QIcon(":/images/up.png"));
    }
    else{
      item->setIcon(11,QIcon(":/images/down.png"));
    }
    break;

  case 12: // Double arg
    if (item->data(12,Qt::DisplayRole).toDouble()<0){
      cout<<"Please enter a positive number for double_arg_0"<<endl;
      scenariosTree->blockSignals(false);
      return;
    }
    evn->set_double_arg(item->data(12,Qt::DisplayRole).toDouble());
    break;

  case 13: // Double arg 1
    if (item->data(13,Qt::DisplayRole).toDouble()<0){
      cout<<"Please enter a positive number for double_arg_1"<<endl;
      scenariosTree->blockSignals(false);
      return;
    }
    evn->set_double_arg_1(item->data(13,Qt::DisplayRole).toDouble());
    break;

  case 14: // Double arg 2
    if (item->data(14,Qt::DisplayRole).toDouble()<0){
      cout<<"Please enter a positive number for double_arg_2"<<endl;
      scenariosTree->blockSignals(false);
      return;
    }
    evn->set_double_arg_2(item->data(14,Qt::DisplayRole).toDouble());
    break;

  case 15: // Description
    evn->set_description(item->data(15,Qt::DisplayRole).toString().toStdString());
    break;

  }
  scenariosTree->blockSignals(false);

  return;
}

int ScenarioEditor::_scenarioDialog( Scenario* sce ){

  QDialog* dialog = new QDialog();
  dialog->setWindowTitle("Add scenario");
  QVBoxLayout* mainLay = new QVBoxLayout();
  dialog->setLayout( mainLay );

  QSpinBox* extIdForm = new QSpinBox();
  extIdForm->setRange(0, 9999);
  extIdForm->setValue( sce->extId() );
  QLineEdit* nameForm = new QLineEdit( QString::fromStdString(sce->name()) );
  QLineEdit* descForm = new QLineEdit( QString::fromStdString(sce->description()) );
  QDoubleSpinBox* startForm = new QDoubleSpinBox();
  startForm->setRange( 0.0, 100 );
  startForm->setDecimals( 3 );
  startForm->setValue( sce->startTime() );
  QDoubleSpinBox* stopForm = new QDoubleSpinBox();
  stopForm->setRange( 0.0, 1000 );
  stopForm->setDecimals( 3 );
  stopForm->setValue( sce->stopTime() );
  QCheckBox* statusForm = new QCheckBox();
  statusForm->setChecked( sce->status() );

  QFormLayout* formLay = new QFormLayout();
  mainLay->addLayout( formLay );
  formLay->addRow( "Name:", nameForm );
  formLay->addRow( "Description:", descForm );
  formLay->addRow( "Start time [s]:", startForm );
  formLay->addRow( "Stop time [s]:", stopForm );
  formLay->addRow( "Status:", statusForm );

  QHBoxLayout* buttonLay = new QHBoxLayout();
  mainLay->addLayout( buttonLay );
  QPushButton ok("Ok");
  buttonLay->addWidget(&ok);
  dialog->connect(&ok, SIGNAL(clicked()), dialog, SLOT(accept()));
  QPushButton cancel("Cancel");
  buttonLay->addWidget(&cancel);
  dialog->connect(&cancel, SIGNAL(clicked()), dialog, SLOT(reject()));

  if( dialog->exec() ){
    // Dialog executed properly
    // Update scenario variables
    sce->set_extId( static_cast<unsigned int>(extIdForm->value()) );
    sce->set_name( nameForm->text().toStdString() );
    sce->set_description( descForm->text().toStdString() );
    sce->set_startTime( startForm->value() );
    sce->set_stopTime( stopForm->value() );
    sce->set_status( statusForm->isChecked() );

    return 0;

  } else {
    // Dialog cancelled or not executed correctly
    return 1;
  }

  return 0;
}

int ScenarioEditor::_eventDialog( Event* evn, int* sceId ){

  QDialog* dialog = new QDialog();
  dialog->setWindowTitle("Add event");
  QVBoxLayout* mainLay = new QVBoxLayout();
  dialog->setLayout( mainLay );

  // Scenario form - available only when a sceId output pointer arg has been
  // provided
  QComboBox* sceForm;
  if ( sceId ){
    sceForm = new QComboBox();
    for ( unsigned int k = 0 ; k != _scs->scenarios.size() ; ++k )
      sceForm->addItem( QString::fromStdString(_scs->scenarios[k].name()) );
    if ( *sceId >= 0 && *sceId < sceForm->count() )
      sceForm->setCurrentIndex( *sceId );
  }

  // Name
  QLineEdit* nameForm = new QLineEdit( QString::fromStdString(evn->name()) );

  // Description
  QLineEdit* descForm = new QLineEdit( QString::fromStdString(evn->description()) );

  // Time
  QDoubleSpinBox* timeForm = new QDoubleSpinBox();
  timeForm->setRange( 0.0, 100.0 );
  timeForm->setDecimals( 3 );
  timeForm->setValue( evn->time() );

  // Element type
  QSpinBox* elTypeForm = new QSpinBox();
//  elTypeForm->setMinimum( 0 );
  elTypeForm->setValue( evn->element_type() );

  // Element ext Id
  QSpinBox* elExtIdForm = new QSpinBox();
  elExtIdForm->setMinimum( 0 );
  elExtIdForm->setValue( evn->element_extId() );

  QSpinBox* evnTypeForm = new QSpinBox();
//  evnTypeForm->setMinimum( 0 );
  evnTypeForm->setValue( evn->event_type() );

  // Bool arg
  QCheckBox* boolForm = new QCheckBox();
  boolForm->setChecked( evn->bool_arg() );

  // Double arg
  QDoubleSpinBox* doubleForm = new QDoubleSpinBox();
  doubleForm->setDecimals( 5 );
  doubleForm->setSingleStep( 0.00001 );
  doubleForm->setValue( evn->double_arg() );

  // Double arg 1
  QDoubleSpinBox* double1Form = new QDoubleSpinBox();
  double1Form->setDecimals( 5 );
  double1Form->setSingleStep( 0.00001 );
  double1Form->setValue( evn->double_arg_1() );

  // Double arg 2
  QDoubleSpinBox* double2Form = new QDoubleSpinBox();
  double2Form->setDecimals( 5 );
  double2Form->setSingleStep( 0.00001 );
  double2Form->setValue( evn->double_arg_2() );

  // Uint arg
  QSpinBox* uintForm = new QSpinBox();
  uintForm->setValue( evn->uint_arg() );
  QCheckBox* statusForm = new QCheckBox();
  statusForm->setChecked( evn->status() );

  QFormLayout* formLay = new QFormLayout();
  mainLay->addLayout( formLay );
  if ( sceId )
    formLay->addRow( "Scenario:", sceForm );
  formLay->addRow( "Name:", nameForm );
  formLay->addRow( "Description:", descForm );
  formLay->addRow( "Time [s]:", timeForm );
  formLay->addRow( "Element type:", elTypeForm );
  formLay->addRow( "Element extId:", elExtIdForm );
  formLay->addRow( "Event type:", evnTypeForm );
  formLay->addRow( "Bool arg:", boolForm );
  formLay->addRow( "Double arg:", doubleForm );
  formLay->addRow( "Double arg1:", double1Form );
  formLay->addRow( "Double arg2:", double2Form );
  formLay->addRow( "Uint arg:", uintForm );
  formLay->addRow( "Status:", statusForm );

  QHBoxLayout* buttonLay = new QHBoxLayout();
  mainLay->addLayout( buttonLay );
  QPushButton ok("Ok");
  buttonLay->addWidget(&ok);
  dialog->connect(&ok, SIGNAL(clicked()), dialog, SLOT(accept()));
  QPushButton cancel("Cancel");
  buttonLay->addWidget(&cancel);
  dialog->connect(&cancel, SIGNAL(clicked()), dialog, SLOT(reject()));

  if( dialog->exec() ){
    // Dialog executed properly
    // Update event variables
    evn->set_name( nameForm->text().toStdString() );
    evn->set_description( descForm->text().toStdString() );
    evn->set_time( timeForm->value() );
    evn->set_element_type( elTypeForm->value() );
    evn->set_element_extId( elExtIdForm->value() );
    evn->set_event_type( evnTypeForm->value() );
    evn->set_bool_arg( boolForm->isChecked() );
    evn->set_double_arg( doubleForm->value() );
    evn->set_double_arg_1( double1Form->value() );
    evn->set_double_arg_2( double2Form->value() );
    evn->set_uint_arg( uintForm->value() );
    evn->set_status( statusForm->isChecked() );

    // Update scenario selection
    if ( sceId )
      *sceId = sceForm->currentIndex();

    return 0;

  } else {
    // Dialog cancelled or not executed correctly
    return 1;
  }

  return 0;
}

void ScenarioEditor::_addScenarioItem( Scenario const& sce ){

  scenariosTree->blockSignals(true);

  QTreeWidgetItem* sceItem = new QTreeWidgetItem(_rootItem);
  _rootItem->addChild(sceItem);

  // Name
  sceItem->setData( 0, Qt::DisplayRole, QString::fromStdString( sce.name() ) );
  sceItem->setIcon( 0, QIcon(":/images/scenario.png") );

  // Status
  QCheckBox* chk= new QCheckBox();
  chk->setChecked( sce.status() );
  scenariosTree->setItemWidget( sceItem, 1, chk);

  // Start time
  sceItem->setData( 2, Qt::DisplayRole, QVariant( sce.startTime() ) );

  // Stop time
  sceItem->setData( 3, Qt::DisplayRole, QVariant( sce.stopTime() ) );

  // Pws compatible
  if  ( sce.checkPwsCompatibility(*_pws)){
    sceItem->setData( 4, Qt::DisplayRole, QVariant("Yes"));
    sceItem->setBackgroundColor( 4, Qt::green);
  } else{
    sceItem->setData( 4, Qt::DisplayRole, QVariant("No"));
    sceItem->setBackgroundColor( 4, Qt::red);
  }

  // CCT compatible
  if ( sce.checkCCTCompatibility() ){
    sceItem->setData( 5, Qt::DisplayRole, QVariant("True") );
    sceItem->setBackgroundColor( 5, Qt::green );
  } else {
    sceItem->setData( 5, Qt::DisplayRole, QVariant("False") );
    sceItem->setBackgroundColor( 5, Qt::red );
  }

  // TSA compatible
  if ( sce.checkTSACompatibility() ){
    sceItem->setData( 6, Qt::DisplayRole, QVariant("True") );
    sceItem->setBackgroundColor( 6, Qt::green );
  } else {
    sceItem->setData( 6, Qt::DisplayRole, QVariant("False") );
    sceItem->setBackgroundColor( 6, Qt::red );
  }

  // Description
  sceItem->setData( 15, Qt::DisplayRole, QString::fromStdString(sce.description()) );

  scenariosTree->blockSignals(false);
  return;
}

void ScenarioEditor::_addEventItem( Event const& evn, int sceId ){

  scenariosTree->blockSignals(true);

  QTreeWidgetItem* evnItem = new QTreeWidgetItem;
  _rootItem->child( sceId )->addChild(evnItem); // TODO check whether sceId behaves well
  _rootItem->child( sceId )->setExpanded(true);
  evnItem->setFlags( evnItem->flags()|Qt::ItemIsEditable );

  // Name
  evnItem->setData( 0, Qt::DisplayRole, QString::fromStdString(evn.name()) );
  evnItem->setIcon( 0, QIcon(":/images/event.png"));

  // Status
  QCheckBox* chk= new QCheckBox();
  chk->setChecked( evn.status() );
  scenariosTree->setItemWidget( evnItem, 1, chk);

  // Element type
  evnItem->setData( 7, Qt::DisplayRole, evn.element_type() );
  switch ( evn.element_type() ){
  case EVENT_ELEMENT_TYPE_BUS:
    evnItem->setIcon( 7, QIcon(":/images/bus.png"));
    evnItem->setData( 7, Qt::DisplayRole, QVariant("0: Bus"));
    break;
  case EVENT_ELEMENT_TYPE_BRANCH:
    evnItem->setIcon( 7, QIcon(":/images/branch.png"));
    evnItem->setData( 7, Qt::DisplayRole, QVariant("1: Branch"));
    break;
  case EVENT_ELEMENT_TYPE_GEN:
    evnItem->setIcon( 7, QIcon(":/images/generator.png"));
    evnItem->setData( 7, Qt::DisplayRole, QVariant("2: Generator"));
    break;
  case EVENT_ELEMENT_TYPE_LOAD:
    evnItem->setIcon( 7, QIcon(":/images/load.png"));
    evnItem->setData( 7, Qt::DisplayRole, QVariant("3: Load"));
    break;
  default:
    evnItem->setIcon( 7, QIcon());
    evnItem->setData( 7, Qt::DisplayRole, QVariant("Other"));
  }

  // Element extId
  evnItem->setData( 8, Qt::DisplayRole, evn.element_extId() );
  // Event type
  evnItem->setData( 9, Qt::DisplayRole, evn.event_type() );
  // Time
  evnItem->setData( 10, Qt::DisplayRole, evn.time() );
  // Bool arg
  if ( evn.bool_arg() ){
    evnItem->setIcon( 11, QIcon(":/images/up.png") );
    evnItem->setData( 11, Qt::DisplayRole, true );
  } else {
    evnItem->setIcon( 11, QIcon(":/images/down.png") );
    evnItem->setData( 11, Qt::DisplayRole, false );
  }
  // Double
  evnItem->setData( 12, Qt::DisplayRole, evn.double_arg() );
  // Double1
  evnItem->setData( 13, Qt::DisplayRole, evn.double_arg_1() );
  // Double2
  evnItem->setData( 14, Qt::DisplayRole, evn.double_arg_2() );
  // Description
  evnItem->setData( 15, Qt::DisplayRole, QString::fromStdString(evn.description()) );

  scenariosTree->blockSignals(false);
  return;
}
