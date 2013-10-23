
#include "powerfloweditor.h"
using namespace elabtsaot;

#include "powersystem.h"
#include "ssengine.h"
#include "tabularpowersystemeditor.h"
#include "guiauxiliary.h"

#include <QAction>
#include <QToolBar>

#include <iostream>
using std::cout;
using std::endl;

PowerFlowEditor::PowerFlowEditor( Powersystem*& pws,
                                  SSEngine* const& sse,
                                  QWidget* parent ) :
    QSplitter(Qt::Vertical,parent),
    _pws(pws),
    _sse(sse),
    _tbl(new TabularPowersystemEditor( _pws, true, this )) {

  addWidget(_tbl);

  // ----- Power flow editor toolbar -----
  QToolBar* pfToolbar = new QToolBar("Power flow toolbar", this);
  addWidget( pfToolbar );
  pfToolbar->setFixedHeight(30);

  // Update view act
  QAction* updtAct = new QAction( QIcon(":/images/update.png"), "Update view", pfToolbar );
  pfToolbar->addAction( updtAct );
  connect( updtAct, SIGNAL(triggered()), this, SLOT(updt()) );

  pfToolbar->addSeparator(); // -- Separator

  QAction* calculateYAct = new QAction( QIcon(), "Calculate Y", pfToolbar );
  pfToolbar->addAction( calculateYAct );
  connect( calculateYAct, SIGNAL(triggered()), this, SLOT(calculateYSlot()) );

  QAction* solvePowerFlowAct = new QAction( QIcon(), "Solve power flow", pfToolbar );
  pfToolbar->addAction( solvePowerFlowAct );
  connect( solvePowerFlowAct, SIGNAL(triggered()), this, SLOT(powerFlowPowersystemSlot()) );

  QAction* logPowerFlowAct = new QAction( QIcon(), "Log power flow results", pfToolbar );
  pfToolbar->addAction( logPowerFlowAct );
  connect( logPowerFlowAct, SIGNAL(triggered()), this, SLOT(logPowerFlowResultsSlot()) );

}

void PowerFlowEditor::updt(){ _tbl->updt(); }

#include <boost/numeric/ublas/io.hpp>

void PowerFlowEditor::calculateYSlot() const{
  ublas::matrix<complex> Y;
  ssengine::buildY(*_pws,Y);
  cout << "Y: " << Y << endl;
}

void PowerFlowEditor::powerFlowPowersystemSlot() const{
  int ans = _sse->solvePowerFlow( *_pws);
  if( !ans ){
    cout << "Power flow succedeed." << endl;
    // Update tabular editor to show results
    _tbl->updt();
  }else{
    cout << "Power flow failed with code " << ans << "." << endl;
  }
}

void PowerFlowEditor::logPowerFlowResultsSlot(){

  // Show log power flow results dialog
  bool toConsole = true;
  QString filename;
  int ans = logPowerFlowDialog(&toConsole, filename);

  if ( toConsole )
    ans = _pws->logPowerFlowResults(std::cout);
  else
    ans = _pws->logPowerFlowResults(filename.toStdString());

  if ( ans )
    cout << "Logging of power flow results failed! exit code " << ans << endl;

  return;
}

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

int PowerFlowEditor::logPowerFlowDialog(bool* toConsole, QString& filename){
  QDialog* dialog = new QDialog();
  dialog->setWindowTitle( "Log power flow results" );

  // Main vertical layout
  QVBoxLayout* layMain = new QVBoxLayout();
  dialog->setLayout( layMain );
  // Info label
  QLabel* infoLabel = new QLabel( "Log power flow results to: " );
  layMain->addWidget( infoLabel );
  // Horizontal button layout
  QHBoxLayout* layButtons = new QHBoxLayout();
  layMain->addLayout( layButtons );
  // Console button
  QPushButton* consoleButton = new QPushButton("Console");
  layButtons->addWidget( consoleButton );
  dialog->connect( consoleButton , SIGNAL(clicked()), dialog, SLOT(reject()) );
  // File button
  QPushButton* fileButton = new QPushButton("File");
  layButtons->addWidget( fileButton );
  dialog->connect( fileButton, SIGNAL(clicked()), dialog, SLOT(accept()) );

  // Execute dialog and check resulting argument
  if ( dialog->exec() ){
    // File button was pressed
    *toConsole = false;

    // Get a filename for writing
    QString tempFilename = guiauxiliary::askFileName("dat", false);
    if ( tempFilename.isNull() )
      return 1;

    filename = tempFilename;

  } else {
    // Console button was pressed
    *toConsole = true;
    filename.clear();
  }

  return 0;
}
