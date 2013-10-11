
#include "gtdaeditor.h"
using namespace elabtsaot;

#include "tdengine.h"
#include "powersystem.h"
#include "scenario.h"
#include "auxiliary.h"
#include "precisiontimer.h"

#include "gtdaviewer.h"

#include <QToolBar>
#include <QAction>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QPushButton>

#include <vector>
using std::vector;
#include <map>
using std::map;
using std::pair;
using std::make_pair;
#include <iostream>
using std::cout;
using std::endl;

GTDAEditor::GTDAEditor( PwsSchematicModel const* smd, TDEngine* const& tde,
                        QWidget *parent) :
    QSplitter(Qt::Vertical, parent),
    _tde(tde),
    _gav(new GTDAViewer(tde->getPws(), smd, this)){

  // ----- Toolbar -----
  QToolBar* gtdaEditorToolbar = new QToolBar( this );
  addWidget( gtdaEditorToolbar );
  gtdaEditorToolbar->setFixedHeight(30);

  gtdaEditorToolbar->addSeparator(); // -----

  QAction* clearGTDAEditorAct( new QAction( QIcon(),
                                          "Clear editor", gtdaEditorToolbar ) );
  gtdaEditorToolbar->addAction( clearGTDAEditorAct );
  connect( clearGTDAEditorAct, SIGNAL(triggered()),
           this, SLOT(clearSlot()) );

  QAction* branchStabCheckAct = new QAction( QIcon(),
                                   "Branch stability check", gtdaEditorToolbar);
  gtdaEditorToolbar->addAction( branchStabCheckAct );
  connect( branchStabCheckAct, SIGNAL(triggered()),
           this, SLOT(branchStabCheckSlot()) );

  QAction* cctCheckAct = new QAction( QIcon(),
                                      "CCT check", gtdaEditorToolbar );
  gtdaEditorToolbar->addAction( cctCheckAct );
  connect( cctCheckAct, SIGNAL(triggered()),
           this, SLOT(cctCheckSlot()) );

  gtdaEditorToolbar->addSeparator(); // -----

  QAction* zoomInAct = new QAction( QIcon(":/images/zoom-in.png"),
                                    "Zoom in", gtdaEditorToolbar );
  gtdaEditorToolbar->addAction( zoomInAct );
  connect(zoomInAct, SIGNAL(triggered()), _gav, SLOT(zoomIn()));

  QAction* zoomOutAct = new QAction( QIcon(":/images/zoom-out.png"),
                                     "Zoom out", gtdaEditorToolbar );
  gtdaEditorToolbar->addAction( zoomOutAct );
  connect(zoomOutAct, SIGNAL(triggered()), _gav, SLOT(zoomOut()));

  QAction* zoomFitAct = new QAction( QIcon(":/images/zoom-fit.png"),
                                     "Zoom fit", gtdaEditorToolbar );
  gtdaEditorToolbar->addAction( zoomFitAct );
  connect(zoomFitAct, SIGNAL(triggered()), _gav, SLOT(zoomFit()));

  gtdaEditorToolbar->addSeparator(); // -----

  // ----- GTDAViewer -----
  addWidget( _gav );

}

int GTDAEditor::init(){
  int ans = _gav->init();
  if (ans) return 1;
  return 0;
}

void GTDAEditor::clearSlot(){
  _gav->enterIdleMode();
  return;
}

#define FAULTSTART 0.0
#define FAULTLOCATION 0.5
#define MAXFAULTTIME 1.5
#define SCESTOPTIME FAULTSTART+MAXFAULTTIME+0.5

void GTDAEditor::branchStabCheckSlot(){

  // Profiling timer
  PrecisionTimer timer;

  Powersystem const* pws(_tde->getPws());
  // Show dialog for the user to select parameters of the check
  double faultTime; // in sec
  bool trip(false);
  int ans = _branchStabCheckDialog(faultTime, trip);
  if ( ans ) return;

  // ----- Create scenarios corresponding to 3ph faults in the middle of -----
  // ----- the branches -----
  vector<Scenario> scenarios;
  for ( size_t k = 0 ; k != pws->getBrSet_size() ; ++k ){
    unsigned int brExtId = pws->getBranch(k)->extId();
    Scenario tempSce( k,
                      "Scenario for branch " + auxiliary::to_string(brExtId),
                      auxiliary::to_string(faultTime)
                        + " 3ph fault in the middle of branch "
                        + auxiliary::to_string(brExtId)
                        + (trip ? " with " : " without ") + "branch trip" );
    tempSce.set_stopTime(SCESTOPTIME);
    Event evOn( "fault on", FAULTSTART, EVENT_ELEMENT_TYPE_BRANCH, brExtId,
                EVENT_EVENTTYPE_BRFAULT, true, FAULTLOCATION);
    Event evOff( "fault off", FAULTSTART+faultTime, EVENT_ELEMENT_TYPE_BRANCH, brExtId,
                 EVENT_EVENTTYPE_BRFAULT, false, FAULTLOCATION );
    tempSce.insertEvent( evOn );
    tempSce.insertEvent( evOff );
    if ( trip ){
      Event evTrip( "trip", FAULTSTART+faultTime, EVENT_ELEMENT_TYPE_BRANCH, brExtId,
                    EVENT_EVENTTYPE_BRTRIP, true );
      tempSce.insertEvent( evTrip );
    }
    scenarios.push_back(tempSce);
  }
  // DEBUG ------------
//  for ( size_t k = 0 ; k != scenarios.size() ; ++k ){
//    cout << "Scenario " << k << ": " << endl;
//    scenarios[k].display();
//  }
//  cout << endl;
  // END OF DEBUG -----

  // ----- Run stability check for the aforecreated scenarios -----
  timer.Start();
  vector<map<unsigned int,bool> > genStable;
  ans = _tde->checkStability(scenarios, genStable );
  double elapsedTime = timer.Stop();
  // Check exit code
  if ( ans ){
    cout << "TD engine check stability failed with code " << ans << endl;
    return;
  }
  // Parse checkStability results
  map<unsigned int,map<unsigned int,bool> > brGenStab;
  for ( unsigned int k = 0 ; k != pws->getBrSet_size() ; ++k ){
    unsigned int brExtId = pws->getBranch(k)->extId();
    brGenStab[brExtId] = genStable[k];
  }

  // ----- Output results to console -----
  // Detailed (per gen) results
  // In the same time create brStab per-branch stability map
  map<unsigned int,bool> brStab;
  for ( map<unsigned int,map<unsigned int,bool> >::const_iterator itBr = brGenStab.begin() ;
        itBr != brGenStab.end() ; ++itBr ){
    bool brIsStable = true;
    cout << "Generator stability for branch " << itBr->first << ": " << endl;
    for ( map<unsigned int,bool>::const_iterator itGen = (itBr->second).begin() ;
          itGen != (itBr->second).end() ; ++itGen ){
      cout << "gen" << itGen->first << ": " << itGen->second << "; ";
      if ( !itGen->second )
        brIsStable = false;
    }
    cout << endl;
    // Update overally branch stability flag
    brStab[itBr->first] = brIsStable;
  }
  // Comprehensive (per branch) results
  for ( map<unsigned int,bool>::const_iterator itBr = brStab.begin() ;
        itBr != brStab.end() ; ++itBr ){
    cout << "Power system overall stability for branch " << itBr->first << ": ";
    cout << itBr->second << endl;
  }

  // ----- Update GTDAViewer GUI -----
  _gav->enterBrStabCheckMode( brGenStab );


  // Log elapsed time for the cctCheck
  cout << "Time elapsed for branch stability check: " << elapsedTime << " sec" << endl;

  return;
}

void GTDAEditor::cctCheckSlot(){

  // Profiling timer
  PrecisionTimer timer;

  // Spawn dialog to set CCT check precision
  double precision = 0.010; // in sec
  bool trip(false);
  double maxFaultTime(MAXFAULTTIME); // maximum fault time for the CCT bisection
                                // method (according to Lanz) set as the 3 phase
                                // fault OFF event of the scenario
  double sceStopTime(SCESTOPTIME);
  int ans = _cctCheckDialog( precision, trip, maxFaultTime, sceStopTime );
  if ( ans ) return;

  // ----- Create scenarios corresponding to 3ph faults in the middle of -----
  // ----- the branches -----
  Powersystem const* pws(_tde->getPws());
  vector<Scenario> scenarios;
  for ( size_t k = 0 ; k != pws->getBrSet_size() ; ++k ){
    unsigned int brExtId = pws->getBranch(k)->extId();
    Scenario tempSce( k,
                      "Scenario for branch " + auxiliary::to_string(brExtId),
                      "CCT for 3ph fault in the middle of branch "
                        + auxiliary::to_string(brExtId)
                        + (trip ? " with " : " without ") + "branch trip" );
    tempSce.set_stopTime(sceStopTime);
    Event evOn( "fault on", FAULTSTART, EVENT_ELEMENT_TYPE_BRANCH, brExtId,
                EVENT_EVENTTYPE_BRFAULT, true, FAULTLOCATION );
    Event evOff( "fault off", FAULTSTART+maxFaultTime, EVENT_ELEMENT_TYPE_BRANCH, brExtId,
                 EVENT_EVENTTYPE_BRFAULT, false, FAULTLOCATION );
    tempSce.insertEvent( evOn );
    tempSce.insertEvent( evOff );
    if ( trip ){
      Event evTrip( "trip", FAULTSTART+maxFaultTime, EVENT_ELEMENT_TYPE_BRANCH, brExtId,
                    EVENT_EVENTTYPE_BRTRIP, true );
      tempSce.insertEvent( evTrip );
    }
    scenarios.push_back(tempSce);
  }
  // DEBUG ------------
//  for ( size_t k = 0 ; k != scenarios.size() ; ++k ){
//    cout << "Scenario " << k << ": " << endl;
//    scenarios[k].display();
//  }
//  cout << endl;
  // END OF DEBUG -----

  // Run checkCCT algorith proper
  vector<double> cctMin, cctMax;
  timer.Start();
  ans = _tde->checkCCT(scenarios, precision, cctMin, cctMax);
  double elapsedTime = timer.Stop();
  if ( ans ){
    cout << "CheckCCT failed with code " << ans << endl;
    return;
  }

  // ----- Parse results and output detailedly to console -----
  map<unsigned int, pair<double,double> > cctMap;
  for ( size_t k = 0 ; k != pws->getBrSet_size() ; ++k ){
    unsigned int brExtId = pws->getBranch(k)->extId();
    cctMap[brExtId] = make_pair(cctMin[k], cctMax[k]);
  }
  for ( map<unsigned int, pair<double,double> >::const_iterator itBr = cctMap.begin() ;
        itBr != cctMap.end() ; ++itBr ){
    cout << "CCT for branch " << itBr->first << ": ";
    cout << itBr->second.first << " - " << itBr->second.second << endl;
  }

  // ----- Update GTDAViewer GUI -----
  _gav->enterCCTCheckMode( cctMap );

  // Log elapsed time for the cctCheck
  cout << "Time elapsed for CCT check: " << elapsedTime << " sec" << endl;

  return;
}

int GTDAEditor::_branchStabCheckDialog(double& faultTime, bool& trip){

  // Create dialog
  QDialog* dialog = new QDialog();
  dialog->setWindowTitle("Branch stability check dialog");
  QVBoxLayout* mainLay = new QVBoxLayout();
  dialog->setLayout(mainLay);

  QHBoxLayout* faultTimeLay(new QHBoxLayout());
  mainLay->addLayout(faultTimeLay);
  faultTimeLay->addWidget( new QLabel("3ph fault on time [ms]:") );
  QDoubleSpinBox* faultTimeForm(new QDoubleSpinBox(dialog));
  faultTimeLay->addWidget(faultTimeForm);
  faultTimeForm->setRange(0.0, 1500.0);
  faultTimeForm->setDecimals(0);
  faultTimeForm->setSingleStep( 10.0 );
  faultTimeForm->setValue( 200.0 );

  QHBoxLayout* tripLay(new QHBoxLayout());
  mainLay->addLayout(tripLay);
  tripLay->addWidget( new QLabel("Post-fault trip branch:") );
  QCheckBox* tripForm(new QCheckBox(dialog) );
  tripLay->addWidget(tripForm);
  tripForm->setChecked(true);

  // Buttons
  QHBoxLayout *layoutButtons = new QHBoxLayout();
  mainLay->addLayout( layoutButtons );
  QPushButton *ok =  new QPushButton("Ok");
  layoutButtons->addWidget( ok );
  connect( ok, SIGNAL(clicked()), dialog, SLOT(accept()));
  QPushButton *cancel = new QPushButton("Cancel");
  layoutButtons->addWidget( cancel );
  connect( cancel, SIGNAL(clicked()), dialog, SLOT(reject()));

  // Execute dialog
  if ( dialog->exec() ){
    // Dialog executed properly
    faultTime = faultTimeForm->value()/1000.0;
    trip = tripForm->isChecked();
    return 0;

  } else {
    // Dialog failed
    return 1;
  }
  return 0;
}

int GTDAEditor::_cctCheckDialog( double& precision,
                                 bool& trip,
                                 double& maxFaultTime,
                                 double& sceStopTime ){

  // Create dialog
  QDialog* dialog = new QDialog();
  dialog->setWindowTitle("CCT check dialog");
  QVBoxLayout* mainLay = new QVBoxLayout();
  dialog->setLayout(mainLay);

  QHBoxLayout* precisionLay( new QHBoxLayout() );
  mainLay->addLayout( precisionLay );
  QLabel* precisionLabel( new QLabel("Precision (cctMax - cctMin) [msec]"));
  precisionLay->addWidget( precisionLabel );
  QDoubleSpinBox* precisionForm( new QDoubleSpinBox(dialog) );
  precisionLay->addWidget( precisionForm );
  precisionForm->setRange( 1.0, 999.0 );
  precisionForm->setDecimals( 0 );
  precisionForm->setValue( 1000*precision );

  QHBoxLayout* tripLay(new QHBoxLayout());
  mainLay->addLayout(tripLay);
  tripLay->addWidget( new QLabel("Post-fault trip branch:"));
  QCheckBox* tripForm(new QCheckBox(dialog) );
  tripLay->addWidget(tripForm);
  tripForm->setChecked(true);

  QHBoxLayout* maxFaultTimeLay(new QHBoxLayout());
  mainLay->addLayout(maxFaultTimeLay);
  maxFaultTimeLay->addWidget(new QLabel("Max fault time [sec]:"));
  QDoubleSpinBox* maxFaultTimeForm(new QDoubleSpinBox(dialog));
  maxFaultTimeLay->addWidget(maxFaultTimeForm);
  maxFaultTimeForm->setDecimals(3);
  maxFaultTimeForm->setRange(0.001,5.000);
  maxFaultTimeForm->setValue( maxFaultTime );

  QHBoxLayout* sceStopTimeLay(new QHBoxLayout());
  mainLay->addLayout(sceStopTimeLay);
  sceStopTimeLay->addWidget(new QLabel("Simulation time [sec]:"));
  QDoubleSpinBox* sceStopTimeForm(new QDoubleSpinBox(dialog));
  sceStopTimeLay->addWidget(sceStopTimeForm);
  sceStopTimeForm->setDecimals(3);
  sceStopTimeForm->setRange(1.500,10.000);
  sceStopTimeForm->setValue( sceStopTime );

  // Buttons
  QHBoxLayout *layoutButtons = new QHBoxLayout();
  mainLay->addLayout( layoutButtons );
  QPushButton *ok =  new QPushButton("Ok");
  layoutButtons->addWidget( ok );
  connect( ok, SIGNAL(clicked()), dialog, SLOT(accept()));
  QPushButton *cancel = new QPushButton("Cancel");
  layoutButtons->addWidget( cancel );
  connect( cancel, SIGNAL(clicked()), dialog, SLOT(reject()));

  // Execute dialog
  if ( dialog->exec() ){
    // Dialog executed properly
    precision = precisionForm->value()/1000;
    trip = tripForm->isChecked();
    maxFaultTime = maxFaultTimeForm->value();
    sceStopTime = sceStopTimeForm->value();
    return 0;

  } else {
    // Dialog failed
    return 1;
  }
  return 0;
}
