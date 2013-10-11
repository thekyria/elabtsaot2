
#include "loadfloweditor.h"
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

LoadflowEditor::LoadflowEditor( Powersystem*& pws,
                                SSEngine* const& sse,
                                QWidget* parent ) :
    QSplitter(Qt::Vertical,parent),
    _pws(pws),
    _sse(sse),
    _tbl(new TabularPowersystemEditor( _pws, true, this )) {

  addWidget(_tbl);

  // ----- Loadflow editor toolbar -----
  QToolBar* lfToolbar = new QToolBar("Loadflow toolbar", this);
  addWidget( lfToolbar );
  lfToolbar->setFixedHeight(30);

  QAction* solveLoadflowAct = new QAction( QIcon(),
                                           "Solve loadflow", lfToolbar );
  lfToolbar->addAction( solveLoadflowAct );
  connect( solveLoadflowAct, SIGNAL(triggered()),
           this, SLOT(loadflowPowersystem()) );

  QAction* logLoadflowAct = new QAction( QIcon(),
                                         "Log loadflow results", lfToolbar );
  lfToolbar->addAction( logLoadflowAct );
  connect( logLoadflowAct, SIGNAL(triggered()),
           this, SLOT(logLoadflowResults()) );

}

void LoadflowEditor::updt(){ _tbl->updt(); }

void LoadflowEditor::loadflowPowersystem() const{

  int ans = _sse->solveLoadflow( *_pws);
  if( !ans ){
    cout << "Loadflow succedeed." << endl;
    // Update tabular editor to show results
    _tbl->updt();
  }else{
    cout << "Loadflow failed with code " << ans << "." << endl;
  }

  return;
}

void LoadflowEditor::logLoadflowResults(){

  // Show log loadflow results dialog
  bool toConsole = true;
  QString filename;
  int ans = logLoadflowDialog(&toConsole, filename);

  if ( toConsole )
    ans = _pws->log_loadflow_results();
  else
    ans = _pws->log_loadflow_results(filename.toStdString());

  if ( ans )
    cout << "Logging of loadflow results failed! exit code " << ans << endl;

  return;
}

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

int LoadflowEditor::logLoadflowDialog(bool* toConsole, QString& filename){
  QDialog* dialog = new QDialog();
  dialog->setWindowTitle( "Log loadflow results" );

  // Main vertical layout
  QVBoxLayout* layMain = new QVBoxLayout();
  dialog->setLayout( layMain );
  // Info label
  QLabel* infoLabel = new QLabel( "Log loadflow results to: " );
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
