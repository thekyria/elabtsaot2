
#include "resultsrequesteditor.h"
using namespace elabtsaot;

#include "scenarioset.h"
#include "tdengine.h"
#include "tdresultsbank.h"
#include "resultsrequestdialog.h"
#include "auxiliary.h"
#include "precisiontimer.h"

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QToolBar>
#include <QAction>
#include <QIcon>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QLabel>

#include <string>
using std::string;
#include <iostream>
using std::cout;
using std::endl;
//#include <vector>
using std::vector;
//#include <map>
using std::map;
#include <cmath>

ResultsRequestEditor::ResultsRequestEditor( ScenarioSet const* scs,
                                            TDEngine* const& tde,
                                            TDResultsBank* trb,
                                            QWidget* parent ) :
    QSplitter(Qt::Vertical, parent),
    _scs(scs), _tde(tde), _trb(trb){

  // --- Scenario treeview ---
  scenariosTree = new QTreeWidget(this);
  this->addWidget(scenariosTree);
  scenariosTree->setColumnCount(8);
  QStringList headerlist;
  headerlist << "Name";                     // header for column 0
  headerlist << "Status";                   // header for column 1
  headerlist << "Start time";               // header for column 2
  headerlist << "Stop time";                // header for column 3
  headerlist << "Engine compatible";        // header for column 4
  headerlist << "CCT comp";                 // header for column 5
  headerlist << "TSA comp";                 // header for column 6
  headerlist << "Stable";                   // header for column 7
  headerlist << "CCT min [s]";              // header for column 8
  headerlist << "CCT max [s]";              // header for column 9
  headerlist << "TSI";                      // header for column 10
  scenariosTree->setHeaderLabels(headerlist);

  scenariosTree->setColumnWidth(0,230);
  scenariosTree->setColumnWidth(1,70);
  scenariosTree->setColumnWidth(2,70);
  scenariosTree->setColumnWidth(3,70);
  scenariosTree->setColumnWidth(4,120);
  scenariosTree->setColumnWidth(5,120);
  scenariosTree->setColumnWidth(6,120);
  scenariosTree->setColumnWidth(7,80);
  scenariosTree->setColumnWidth(8,120);
  scenariosTree->setColumnWidth(9,120);
  scenariosTree->setColumnWidth(10,120);

  // Alternatively:
//  for (size_t k = 0 ; k != scenariosTree->columnCount() ; ++k )
//    scenariosTree->resizeColumnToContents(k);

  QStringList rootlist;
  rootlist << "Scenario Set";
  _rootItem = new QTreeWidgetItem( rootlist );
  _rootItem->setIcon( 0, QIcon(":/images/lightning.png") );
  scenariosTree->addTopLevelItem(_rootItem);

  // --- Scenario results tab toolbar ---
  QToolBar* resultsRequestEditorToolbar = new QToolBar(this);
  this->addWidget( resultsRequestEditorToolbar );
  resultsRequestEditorToolbar->setFixedHeight(30);

  QAction* updatescenariosAct = new QAction( QIcon(":/images/update.png"),
                                             "Load Scenarios to be run",this);
  resultsRequestEditorToolbar->addAction(updatescenariosAct);
  connect( updatescenariosAct, SIGNAL(triggered()),
           this, SLOT(updateScenariosSlot()) );

  resultsRequestEditorToolbar->addSeparator(); // -----

  QAction* TDRunAct = new QAction( QIcon(":/images/fulltd.png"),
                                   "Full TD run", this);
  resultsRequestEditorToolbar->addAction( TDRunAct );
  connect( TDRunAct, SIGNAL(triggered()),
           this, SLOT(TDRunSlot()) );

  resultsRequestEditorToolbar->addSeparator(); // -----

  QAction* checkStabilityAct = new QAction( QIcon(":/images/stability.png"),
                                           "Stability test",this);
  resultsRequestEditorToolbar->addAction(checkStabilityAct);
  connect( checkStabilityAct, SIGNAL(triggered()),
           this, SLOT(checkStabilitySlot()) );

  QAction* ccttestAct = new QAction( QIcon(":/images/ccttest.png"),
                                     "CCT test",this);
  resultsRequestEditorToolbar->addAction(ccttestAct);
  connect( ccttestAct, SIGNAL(triggered()),
           this, SLOT(CCTTestSlot()) );

  QAction* tsatestAct = new QAction( QIcon(":/images/tsatest.png"),
                                     "TSA test",this);
  resultsRequestEditorToolbar->addAction(tsatestAct);
  connect( tsatestAct, SIGNAL(triggered()),
           this, SLOT(TSATestSlot()) );
}

void ResultsRequestEditor::updateScenariosSlot(){

  // Remove old entries
  _rootItem->takeChildren();

  // For each one of the scenarios in the scenario set add an entry to the tree
  // view
  for ( size_t k = 0 ; k != _scs->scenarios.size() ; ++k )
    _addScenarioItem( _scs->scenarios[k] );
  scenariosTree->expandAll();

  return;
}

void ResultsRequestEditor::TDRunSlot(){

  vector<TDResults*> res;
  size_t k; // counter

  // ----- Ask the user to detailedly request results -----
//  // In the following if sceResIdMap is finally utilized, perhaps think of
//  // boost::bimap
//  map<size_t,size_t> sceResIdMap; // scenarioId -> resultsId
  map<size_t,size_t> resSceIdMap; // resultsId -> scenarioId
  for ( k = 0 ; k != _scs->scenarios.size() ; ++k ){
    if ( !_scs->scenarios[k].status() )
      continue;
    if ( !_tde->isEngineCompatible( _scs->scenarios[k] ) )
      continue;
    string title( "TD results for " + _scs->scenarios[k].name() );
    TDResults* tempRes = new TDResults( _tde->getPws(), &_scs->scenarios[k] );
    tempRes->set_title( title );
    res.push_back( tempRes );
    resSceIdMap[res.size()-1] = k;
//    sceResIdMap[k] = res.size()-1;
  }
  ResultsRequestDialog dialog( res );
  int ans = dialog.exec();
  if ( ans ){
    // The user has cancelled the results retrieval
    // Free allocated memory for TDResults and return
    for ( k = 0 ; k != res.size() ; ++k )
      delete res.at(k);
    return;
  }

  // ----- Excecute the simulations to get the results -----
  vector<bool> tdRunSucceded( res.size() );
  for ( k = 0 ; k != res.size() ; ++k ){
    ans = _tde->simulate( _scs->scenarios[resSceIdMap.at(k)], *res[k] );
    if ( ans ){
      cout << "Starting simulation: " << res[k]->title() << " ... ";
      cout << " failed(" << ans << ")" << endl;
      tdRunSucceded[k] = false;
    } else {
      cout << "Starting simulation: " << res[k]->title() << " ... ";
      cout << " ok!" << endl;
      tdRunSucceded[k] = true;
    }
  }

  // ----- Store the results in the global TDResults bank -----
  for ( k = 0 ; k != res.size() ; ++k ){
    if ( tdRunSucceded[k] ){
      ans = _trb->storeResults( *res[k] );
      if ( ans )
        cout << "Storing results: " << res[k]->title()
             << " failed with code " << ans << endl;
      else
        cout << "Storing results: " << res[k]->title()
             << " succeded!" << endl;
    }
  }

  // ----- Free allocated memory for TDResults and return -----
  for ( k = 0 ; k != res.size() ; ++k )
    delete res.at(k);

  emit resultsAdded();

  return;
}

void ResultsRequestEditor::checkStabilitySlot(){

  // ----- Determine  stability (detailed = per gen) by using the TDEngine -----
  vector<map<unsigned int,bool> > genStable( _scs->scenarios.size() );
  vector<map<unsigned int,double> > genInstabilityTime( _scs->scenarios.size() );
  PrecisionTimer timer; timer.Start();
  int ans = _tde->checkStability( _scs->scenarios, genStable, &genInstabilityTime );
  double elapsedTime = timer.Stop();
  if ( ans ){
    cout << "Stability check failed with code " << ans << endl;
    return;
  }

  // ----- Output detailed stability results to console -----
  for ( size_t k = 0 ; k != _scs->scenarios.size() ; ++k ){
    if ( !_scs->scenarios[k].status() ) continue;

    cout << "Scenario: " << _scs->scenarios[k].name() << endl;
//    cout << "Detailed generator angle stability results" << endl;
    for ( map<unsigned int,bool>::const_iterator itF = genStable[k].begin();
          itF != genStable[k].end() ; ++itF ){
        cout << "Generator extId" << itF->first << ": ";
        cout << "is stable " << itF->second << " ";
//        cout << "instab time " << genInstabilityTime[k][itF->first];
        cout << endl;
    }
//    cout << endl;
  }
  cout << "Stability check on scenario set completed in "
       << elapsedTime << " sec" << endl;
  // TODO come up with a more elegant way of presenting the results above

  // ----- Update GUI -----
  for ( size_t k = 0 ; k != _scs->scenarios.size() ; ++k ){
    bool tempPwsStable = true;
    for ( map<unsigned int,bool>::const_iterator itF = genStable[k].begin();
          itF != genStable[k].end() ; ++itF ){
      if ( !itF->second ){
        tempPwsStable = false;
        break;
      }
    }

    QTreeWidgetItem* treeItem = _rootItem->child(k);
    treeItem->setTextAlignment(7, Qt::AlignCenter);
    if ( tempPwsStable ){
      treeItem->setBackgroundColor(7,Qt::green);
      treeItem->setData(7,0,QVariant("STABLE"));
    } else {
      treeItem->setBackgroundColor(7,Qt::red);
      treeItem->setData(7,0,QVariant("UNSTABLE"));
    }
  }

  return;
}

void ResultsRequestEditor::CCTTestSlot(){

  if ( _scs->scenarios.size() == 0 ){
    cout << "Load a scenario set first" << endl;
    return;
  }

  double precision, maxFaultTime;
  int ans = _cctDialog( precision, maxFaultTime );
  if ( ans ) return;

  // Prepare input scenarios according to maxFault time
  vector<Scenario> scenarios(_scs->scenarios);
  for ( size_t k = 0 ; k != scenarios.size() ; ++k ){

    // Skip for inactive scenarios
    Scenario* tempSce = &scenarios[k];
    if(!tempSce->status()) continue;

    // All events after the fault on (assumed to be first), ie at time off
    // are modified according to maxFaultTime
    for ( size_t m = 1 ; m != tempSce->getEventSetSize() ; ++m ){
      Event atOff = tempSce->getEvent(m);
      atOff.set_time( maxFaultTime );
      tempSce->editEvent( m, atOff );
    }
  }

  // Run checkCCT algorith proper
  vector<double> cctMin, cctMax;
  PrecisionTimer timer; timer.Start();
  ans = _tde->checkCCT(scenarios, precision, cctMin, cctMax);
  double elapsedTime = timer.Stop();
  if ( ans ){
    cout << "CCT check failed with code " << ans << endl;
    return;
  }

  // ----- Output detailed stability results to console -----
  for ( size_t k = 0 ; k != scenarios.size() ; ++k ){
    Scenario* tempSce = &scenarios[k];
    if ( !tempSce->status() ) continue; // Skip for inactive scenarios
    // Show CCT results
    cout << "Scenario: " << tempSce->name() << endl;
    cout << "CCT = " << cctMin[k] << " - " << cctMax[k] << endl;
//    cout << endl;
  }
  cout << "CCT check on scenario set completed in "
       << elapsedTime << " sec" << endl;

  // Update GUI
  for ( size_t k = 0 ; k != scenarios.size() ; ++k ){
    QTreeWidgetItem* treeItem = _rootItem->child(k);
    // Lower CCT  boundary found
    treeItem->setTextAlignment(8,Qt::AlignCenter);
//    treeItem->setBackgroundColor(8,Qt::lightGray);
    treeItem->setData(8,0,QVariant(cctMin[k]));
    // Upper CCT boundary found
    treeItem->setTextAlignment(9,Qt::AlignCenter);
//    treeItem->setBackgroundColor(9,Qt::lightGray);
    treeItem->setData(9,0,QVariant(cctMax[k]));
  }
}

void ResultsRequestEditor::TSATestSlot(){

  for ( int k = 0 ; k != _rootItem->childCount() ; ++k ){
    QTreeWidgetItem* treeItem = _rootItem->child(k);
    treeItem->setTextAlignment(10,Qt::AlignCenter);
//    treeItem->setBackgroundColor(10,Qt::lightGray);
    treeItem->setData(10,0,QVariant("##TSI##"));
  }

  return;
}

void ResultsRequestEditor::_addScenarioItem( Scenario const& sce ){

  // Block signals from scenariosTree to prevent slots from being auto-executed
  scenariosTree->blockSignals(true);

  QTreeWidgetItem* sceItem = new QTreeWidgetItem(_rootItem);
  _rootItem->addChild(sceItem);

//  headerlist << "Stable";                   // header for column 7
//  headerlist << "CCT min num";              // header for column 8
//  headerlist << "CCT max num";              // header for column 9
//  headerlist << "TSA number";               // header for column 10

  // ----- Name -----
  sceItem->setData( 0, Qt::DisplayRole, QString::fromStdString( sce.name() ) );
  sceItem->setIcon( 0, QIcon(":/images/scenario.png") );

  // ----- Status -----
  sceItem->setBackgroundColor(1, sce.status() ? Qt::green : Qt::red );
  sceItem->setData(1,0,QVariant(sce.status() ? "ACTIVE" : "DISABLED") );
  // Alternatively
//  QCheckBox* chk= new QCheckBox();
//  chk->setChecked( sce.status() );
//  scenariosTree->setItemWidget( sceItem, 1, chk);

  // ----- Start time -----
  sceItem->setData( 2, Qt::DisplayRole, QVariant( sce.startTime() ) );
  sceItem->setBackgroundColor(2, QColor(0xFFFFCC));

  // ----- Stop time -----
  sceItem->setData( 3, Qt::DisplayRole, QVariant( sce.stopTime() ) );
  sceItem->setBackgroundColor(3, QColor(0xFFFFCC) );

  // ----- Engine compatible -----
  if ( _tde->isEngineCompatible( sce ) ){
    sceItem->setBackgroundColor( 4, Qt::green );
    sceItem->setData( 4, Qt::DisplayRole, QVariant("Yes") );
  } else {
    sceItem->setBackgroundColor( 4, Qt::red );
    sceItem->setData( 4, Qt::DisplayRole, QVariant("No") );
  }

  // ----- CCT compatible -----
  if ( sce.checkCCTCompatibility() ){
    sceItem->setData( 5, Qt::DisplayRole, QVariant("True") );
    sceItem->setBackgroundColor( 5, Qt::green );
  } else {
    sceItem->setData( 5, Qt::DisplayRole, QVariant("False") );
    sceItem->setBackgroundColor( 5, Qt::red );
  }

  // ----- TSA compatible -----
  if ( sce.checkTSACompatibility() ){
    sceItem->setData( 6, Qt::DisplayRole, QVariant("True") );
    sceItem->setBackgroundColor( 6, Qt::green );
  } else {
    sceItem->setData( 6, Qt::DisplayRole, QVariant("False") );
    sceItem->setBackgroundColor( 6, Qt::red );
  }

  scenariosTree->blockSignals(false); // Un-block signals from scenariosTree

  return;
}

int ResultsRequestEditor::_cctDialog( double& precision, double& maxFaultTime ){

  // Create dialog
  QDialog* dialog = new QDialog();
  dialog->setWindowTitle("Critical clearence precision");
  QVBoxLayout* layoutMain = new QVBoxLayout();
  dialog->setLayout(layoutMain);

  // Precision
  QLabel precisionLabel("CCT precision [msec]");
  layoutMain->addWidget(&precisionLabel);
  QDoubleSpinBox precisionForm;
  layoutMain->addWidget(&precisionForm);
  precisionForm.setDecimals(0);
  precisionForm.setRange(0.0, 3000.0);
  precisionForm.setValue(10.0);

  // maxFaultTime
  QLabel maxFaultTimeLabel("Max fault time [sec]");
  layoutMain->addWidget(&maxFaultTimeLabel);
  QDoubleSpinBox maxFaultTimeForm;
  layoutMain->addWidget(&maxFaultTimeForm);
  maxFaultTimeForm.setDecimals(3);
  maxFaultTimeForm.setRange(0.001, 5.000);
  maxFaultTimeForm.setValue(1.500);

  // Buttons
  QHBoxLayout *layoutButtons = new QHBoxLayout();
  layoutMain->addLayout( layoutButtons );
  QPushButton *ok =  new QPushButton("Ok");
  layoutButtons->addWidget( ok );
  connect( ok, SIGNAL(clicked()), dialog, SLOT(accept()));
  QPushButton *cancel = new QPushButton("Cancel");
  layoutButtons->addWidget( cancel );
  connect( cancel, SIGNAL(clicked()), dialog, SLOT(reject()));

  // Execute dialog
  if ( dialog->exec() ){
    // Dialog executed properly
    precision = precisionForm.value() / 1000.0;
    maxFaultTime = maxFaultTimeForm.value();
    return 0;

  } else {
    // Dialog failed
    return 1;
  }
  return 0;
}
