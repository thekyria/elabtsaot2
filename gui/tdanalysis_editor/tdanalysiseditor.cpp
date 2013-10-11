
#include "tdanalysiseditor.h"
using namespace elabtsaot;

#include "resultsrequesteditor.h"
#include "resultsbankeditor.h"
#include "resultsvisualizationeditor.h"
#include "gtdaeditor.h"
//#include "tdengine.h"
  #include "tdemulator.h"
  #include "simulator_sw.h"
#include "tdanalysiseditordialogs.h"
#include "auxiliary.h"
#include "propertydialog.h"

#include <QToolBar>
#include <QAction>
#include <QIcon>
#include <QComboBox>

#include <string>
using std::string;
#include <iostream>
using std::cout;
using std::endl;
//#include <vector>
using std::vector;
//#include <list>
using std::list;

enum TDAnalysisEditorTDESelection{
  TDANL_TDE_HWE,
  TDANL_TDE_SWE,
  TDANL_TDE_MAT
};

// TODO: remove tde_hwe, tde_swe; create the notion of an 'engine bank'
TDAnalysisEditor::TDAnalysisEditor( PwsSchematicModel const* smd,
                                    ScenarioSet const* scs,
                                    TDEngine*& tde,
                                    TDEmulator* tde_hwe,
                                    Simulator_sw* tde_swe,
                                    TDResultsBank* trb,
                                    QWidget* parent ) :
    QSplitter(Qt::Vertical, parent),
    _tde(tde),
    _tde_hwe(tde_hwe),
    _tde_swe(tde_swe) {
  setMinimumHeight(300);

  // ----- Analysis editor toolbar -----
  QToolBar* analysisEditorToolbar(new QToolBar("TD Engine", this));
  this->addWidget( analysisEditorToolbar );
  analysisEditorToolbar->setFixedHeight(30);

  // TDEngine select box
  tdEngineSelectBox = new QComboBox(this);
  analysisEditorToolbar->addWidget(tdEngineSelectBox);
  tdEngineSelectBox->addItem( QIcon(":/images/hardware.png"),
                            QString::fromStdString(_tde_hwe->getDescription()),
                            QVariant(TDANL_TDE_HWE) );
  tdEngineSelectBox->addItem( QIcon(":/images/software.png"),
                            QString::fromStdString(_tde_swe->getDescription()),
                            QVariant(TDANL_TDE_SWE) );
  connect( tdEngineSelectBox, SIGNAL(currentIndexChanged(int)),
           this, SLOT(TDEngineSelectionSlot(int)) );

  // Set TDEngine time step
  QAction* setTDEngineTimeStepAct = new QAction( QIcon(":/images/timestep.png"),
                                       "Set time step for the TD engine", this);
  analysisEditorToolbar->addAction( setTDEngineTimeStepAct );
  connect( setTDEngineTimeStepAct, SIGNAL(triggered()),
           this, SLOT(setTDEngineTimeStepSlot()) );

  // Set options
  QAction* setOptionsAct = new QAction( QIcon(),
                                        "Set options", this);
  analysisEditorToolbar->addAction( setOptionsAct );
  connect( setOptionsAct, SIGNAL(triggered()),
           this, SLOT(setOptionsSlot()) );


  // ----- Analysis editor tab widget -----
  QTabWidget* TDAnalysisTabWidget(new QTabWidget(this));
  this->addWidget( TDAnalysisTabWidget );

  // Setup results request and review tab
  QSplitter* resultsRequestAndOverviewTab = new QSplitter( Qt::Vertical, this );
  TDAnalysisTabWidget->addTab( resultsRequestAndOverviewTab,
                             "Results request and overview");
  ResultsRequestEditor*
  resultsRequestEditor = new ResultsRequestEditor( scs, tde, trb,
                                                 resultsRequestAndOverviewTab );
  resultsRequestAndOverviewTab->addWidget( resultsRequestEditor );
  ResultsBankEditor*
  resultsBankEditor = new ResultsBankEditor( trb,resultsRequestAndOverviewTab );
  resultsRequestAndOverviewTab->addWidget( resultsBankEditor );
  connect( resultsRequestEditor, SIGNAL(resultsAdded()),
           resultsBankEditor, SLOT(updt()) );

  // Results visualization tab
  ResultsVisualizationEditor*
  resultsVisualizationEditor = new ResultsVisualizationEditor( trb, this );
  TDAnalysisTabWidget->addTab( resultsVisualizationEditor,
                             "Results visualization" );

  // Graphical Time-Domain Analysis (GTDA) editor tab
  gtdaEditor = new GTDAEditor( smd, tde, this);
  TDAnalysisTabWidget->addTab( gtdaEditor, "Graphical TD Analysis" );

  // ----- To update the GUI -----
  TDEngineSelectionSlot( tdEngineSelectBox->currentIndex() );
}

int TDAnalysisEditor::init(){
  int ans = gtdaEditor->init();
  if ( ans ) return 1;
  return 0;
}

void TDAnalysisEditor::TDEngineSelectionSlot(int index){
    switch ( tdEngineSelectBox->itemData(index).toInt() ){
    case TDANL_TDE_HWE:
      _tde = _tde_hwe;
      break;

    case TDANL_TDE_SWE:
    default:
      _tde = _tde_swe;
      break;
    }

    cout << "Time domain engine set to: " << _tde->getDescription() << endl;

    return;
}

void TDAnalysisEditor::setTDEngineTimeStepSlot(){
  double timeStep = _tde->getTimeStep();
  int ans = TDAnalysisEditorDialogs::setTDEngineTimeStepDialog( &timeStep );
  if ( ans )
    return;

  double mismatch = 0;
  ans = _tde->setTimeStep( timeStep, &mismatch );
  if ( ans ){
    cout << "Setting the time step option for the engine failed with code ";
    cout << ans << endl;
    return;
  } else {
    double trueTimeStep = _tde->getTimeStep();
    cout << "TDEngine time step set to: " << trueTimeStep << " seconds ";
    if ( mismatch != 0 )
      cout << "(mismatch of " << mismatch << " from " << timeStep << ")";
    cout << endl;
  }

  return;
}

void TDAnalysisEditor::setOptionsSlot(){
  list<elabtsaot::property> propertyList = _tde->getPropertyList();
  vector<property_type> propertyTypes = _tde->getPropertyTypes();
  PropertyDialog dialog( propertyTypes, propertyList );
  int ans = dialog.exec();
  if ( ans ) return;
  _tde->updateProperties( propertyList );
}
