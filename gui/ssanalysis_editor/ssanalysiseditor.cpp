
#include "ssanalysiseditor.h"
using namespace elabtsaot;

#include "propertydialog.h"
#include "powerfloweditor.h"
//#include "ssengine.h"
#include "moteurrenard.h"
#include "moteurfengtian.h"

#include <QToolBar>
#include <QAction>

//#include <vector>
using std::vector;
//#include <list>
using std::list;
#include <iostream>
using std::cout;
using std::endl;

enum AnalysisEditorTDESelection{
  SSANL_SSE_MRN,
  SSANL_SSE_MAT,
  SSANL_SSE_FEN
};

SSAnalysisEditor::SSAnalysisEditor( Powersystem*& pws,
                                    SSEngine*& sse,
                                    MoteurRenard* sse_mrn,
                                    MoteurFengtian* sse_fen,
                                    QWidget* parent ) :
    QSplitter(Qt::Vertical, parent),
    _sse(sse),
    _sse_mrn(sse_mrn),
    _sse_fen(sse_fen) {

  setMinimumHeight(300);

  // ----- Network editor engine toolbar -----
  QToolBar* sseToolbar = new QToolBar("Steady state engine toolbar", this);
  addWidget( sseToolbar );
  sseToolbar->setFixedHeight(30);

  ssEngineSelectBox = new QComboBox(this);
  sseToolbar->addWidget(ssEngineSelectBox);
  ssEngineSelectBox->addItem( QIcon(),
                            QString::fromStdString(sse_mrn->getDescription()),
                            QVariant(SSANL_SSE_MRN) );
  ssEngineSelectBox->addItem( QIcon(),
                              QString::fromStdString(sse_fen->getDescription()),
                              QVariant(SSANL_SSE_FEN) );
  connect( ssEngineSelectBox, SIGNAL(currentIndexChanged(int)),
           this, SLOT(SSEngineSelectionSlot(int)) );

  // Set options
  QAction* setOptionsAct = new QAction( QIcon(), "Set options", sseToolbar );
  sseToolbar->addAction( setOptionsAct );
  connect( setOptionsAct, SIGNAL( triggered() ),
           this, SLOT( SSsetOptionsSlot() ) );

  // ----- Container tab widget -----
  QTabWidget* TDAnalysisTabWidget(new QTabWidget(this));
  this->addWidget( TDAnalysisTabWidget );

  // Power flow editor tab
  powerFlowEditor = new PowerFlowEditor( pws, sse, this);
  TDAnalysisTabWidget->addTab( powerFlowEditor, "Power Flow" );
}

void SSAnalysisEditor::updt(){ powerFlowEditor->updt(); }

void SSAnalysisEditor::SSEngineSelectionSlot(int index){
  switch ( ssEngineSelectBox->itemData(index).toInt() ){

  case SSANL_SSE_FEN:
    _sse = _sse_fen;
    break;

  case SSANL_SSE_MRN:
  default:
    _sse = _sse_mrn;
    break;
  }
  cout << "Steady state engine set to: " << _sse->getDescription() << endl;
}

void SSAnalysisEditor::SSsetOptionsSlot(){
  list<elabtsaot::property> optionValues;
  vector<property_type> availableOptions = _sse->getPropertyTypes();
  PropertyDialog dialog( availableOptions, optionValues );
  int ans = dialog.exec();
  if ( ans ) return;
  _sse->updateProperties( optionValues );
}
