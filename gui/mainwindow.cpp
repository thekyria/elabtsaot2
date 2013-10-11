
#include "mainwindow.h"
using namespace elabtsaot;

#include "importerexporter.h"
#include "powersystem.h"
#include "emulator.h"

#include "scenarioset.h"
#include "tdresultsbank.h"
#include "tdemulator.h"
#include "simulator_sw.h"

#include "guiauxiliary.h"
#include "sidepane.h"
#include "console.h"
#include "powersystemeditor.h"
  #include "pwsschematicmodel.h"
#include "ssanalysiseditor.h"
#include "scenarioeditor.h"
#include "tdanalysiseditor.h"
#include "communicationeditor.h"
#include "calibrationeditor.h"
#include "mappereditor.h"
#include "fittereditor.h"
#include "auxiliaryeditor.h"

#include <QLayout>
#include <QTextEdit>
#include <QMenuBar>
#include <QMenu>
#include <QFileDialog>
#include <QLabel>
#include <QDebug>
#include <QSplitter>
#include <QStackedWidget>

//#include <vector>
using std::vector;
#include <iostream>
using std::cout;
using std::endl;
//#include <string>
using std::string;

enum MWShowEditor{
  MWSHOW_SPLASH = 0,
  MWSHOW_NET = 1,
  MWSHOW_SSANL = 2,
  MWSHOW_SCE = 3,
  MWSHOW_TDANL = 4,
  MWSHOW_COM = 5,
  MWSHOW_CAL = 6,
  MWSHOW_MAP = 7,
  MWSHOW_FIT = 8,
  MWSHOW_AUX = 9
};

MainWindow::MainWindow( Powersystem*& pws,
                        Emulator*& emu,
                        SSEngine*& sse,
                        MoteurRenard*& sse_mrn,
                        MoteurFengtian*& sse_fen,
                        TDEngine*& tde,
                        TDEmulator*& tde_hwe,
                        Simulator_sw*& tde_swe,
                        ScenarioSet*& scs,
                        TDResultsBank*& trb ) :
    _pws(pws),
    _emu(emu),
    _tde_hwe(tde_hwe),
    _scs(scs),
    _trb(trb){

  // ----- Initialize the components -----
  _con = new Console(this);    // required for calls to 'cout<<'
  _tde_hwe->setLogger( _con ); // redirect Logger of _tde_hwe to _con
  tde_swe->setLogger( _con );  // redirect Logger of _tde_swe to _con


  _sip = new SidePane(this);
  connect( _sip, SIGNAL(buttonSplash_clicked()),
           this, SLOT(showSplash()) );
  connect( _sip, SIGNAL(buttonPowersystem_clicked()),
           this, SLOT(showPowersystem()) );
  connect( _sip, SIGNAL(buttonSSAnalysis_clicked()),
           this, SLOT(showSSAnalysis()) );
  connect( _sip, SIGNAL(buttonScenarios_clicked()),
           this, SLOT(showScenarios()) );
  connect( _sip, SIGNAL(buttonTDAnalysis_clicked()),
           this, SLOT(showTDAnalysis()) );
  connect( _sip, SIGNAL(buttonCommunication_clicked()),
           this, SLOT(showCommunication()) );
  connect( _sip, SIGNAL(buttonCalibration_clicked()),
           this, SLOT(showCalibration()) );
  connect( _sip, SIGNAL(buttonMapper_clicked()),
           this, SLOT(showMapper()) );
  connect( _sip, SIGNAL(buttonFitter_clicked()),
           this, SLOT(showFitter()) );
  connect( _sip, SIGNAL(buttonAuxiliary_clicked()),
           this, SLOT(showAuxiliary()) );
  _net = new PowersystemEditor( _pws, _emu->mmd(), this );
  _net->init();
  _SSanl = new SSAnalysisEditor( _pws,
                                 sse,
                                 sse_mrn,
                                 sse_fen,
                                 this );
  _sce = new ScenarioEditor( _scs, _pws, this );
  _TDanl = new TDAnalysisEditor( _net->smd(),
                                 _scs,
                                 tde,
                                 _tde_hwe,
                                 tde_swe,
                                 _trb,
                                 this );
//  connect( _anl, SIGNAL(mmdChanged()),
//           this, SLOT(mmdChangedSlot()) );
//  connect( _anl, SIGNAL(emuChanged(bool)),
//           this, SLOT(emuChangedSlot(bool)) );
  // Hardware related editors
  _com = new CommunicationEditor( _emu, this );
  connect( _com, SIGNAL(usbChanged()),
           this, SLOT(usbChangedSlot()) );
  connect( _com, SIGNAL(emuChanged(bool)),
           this, SLOT(emuChangedSlot(bool)) );
  _cal = new CalibrationEditor( _emu, _con );
  _map = new MapperEditor( _emu, this );
  _fit = new FitterEditor( _emu, this );
  _aux = new AuxiliaryEditor( _emu, _tde_hwe, this );
  _com->autoAssignSlicesToDevicesSlot();

  // ----- Initialize the menus  -----
  // File menu
  QMenu* fileMenu = menuBar()->addMenu("File");
  QAction* loadAct = new QAction( "Load", this);
  connect( loadAct, SIGNAL(triggered()), this, SLOT(loadSlot()) );
  fileMenu->addAction( loadAct );

  QAction* exitAct = new QAction( "Exit", this);
  connect( exitAct, SIGNAL(triggered()), this, SLOT(close()));
  fileMenu->addAction( exitAct );

  // Project menu
  QMenu* projectMenu = menuBar()->addMenu("Project");
  QAction* importPowersystemAct  = new QAction( "Import Powersystem", this );
  projectMenu->addAction( importPowersystemAct );
  connect( importPowersystemAct, SIGNAL(triggered()),
           this, SLOT(importPowersystemSlot()) );

  QAction* importSchematicAct = new QAction( "Import Schematic", this );
  projectMenu->addAction( importSchematicAct );
  connect( importSchematicAct, SIGNAL(triggered()),
           this, SLOT(importSchematicSlot()) );

  QAction* importMappingAct = new QAction( "Import Mapping", this );
  projectMenu->addAction( importMappingAct );
  connect( importMappingAct, SIGNAL(triggered()),
           this, SLOT(importMappingSlot()) );

  QAction* importScenarioSetAct = new QAction( "Import Scenario Set", this );
  projectMenu->addAction( importScenarioSetAct );
  connect( importScenarioSetAct, SIGNAL(triggered()),
           this, SLOT(importScenarioSetSlot()) );

  projectMenu->addSeparator();

  QAction* exportPowersystemAct  = new QAction( "Export Powersystem", this );
  projectMenu->addAction( exportPowersystemAct );
  connect( exportPowersystemAct, SIGNAL(triggered()),
           this, SLOT(exportPowersystemSlot()) );

  QAction* exportSchematicAct = new QAction( "Export Schematic", this );
  projectMenu->addAction( exportSchematicAct );
  connect( exportSchematicAct, SIGNAL(triggered()),
           this, SLOT(exportSchematicSlot()) );

  QAction* exportMappingAct = new QAction( "Export Mapping", this );
  projectMenu->addAction( exportMappingAct );
  connect( exportMappingAct, SIGNAL(triggered()),
           this, SLOT(exportMappingSlot()) );

  QAction* exportScenarioSetAct = new QAction( "Export Scenario Set", this );
  projectMenu->addAction( exportScenarioSetAct );
  connect( exportScenarioSetAct, SIGNAL(triggered()),
           this, SLOT(exportScenarioSetSlot()) );

  // Help menu
  QMenu* helpMenu = menuBar()->addMenu("Help");
  QAction* aboutAct = new QAction( "About", this );
  helpMenu->addAction( aboutAct );


  // ----- Initialize graphical layout -----
  QSplitter* mainVerticalSplitter = new QSplitter( Qt::Vertical, this );
  QSplitter* topHorizontalSplitter = new QSplitter( Qt::Horizontal, this );
  mainVerticalSplitter->setChildrenCollapsible(false);

  setCentralWidget( mainVerticalSplitter );
  mainVerticalSplitter->addWidget( topHorizontalSplitter );
  mainVerticalSplitter->addWidget( _con );

  topHorizontalSplitter->addWidget( _sip );

  // StackedWidget that shows the currently visible windows
  topStack = new QStackedWidget( this );
  topHorizontalSplitter->addWidget( topStack );

  /* Indexes of topStack internal list widgets
    0 | splash | splash screen
    1 | _net   | power system editor
    2 | _SSanl | Steady State analysis editor
    3 | _sce   | scenario editor
    4 | _TDanl | Time Domain analysis editor
    5 | _com   | communication editor
    6 | _cal   | calibration editor
    7 | _map   | mapper editor
    8 | _fit   | fitter editor
    9 | _aux   | auxiliary editor             */
  QFrame* splash = new QFrame( this ); // Splash screen -> 0
  topStack->addWidget( splash );
  splash->setFrameStyle( QFrame::StyledPanel );
  splash->setStyleSheet( "background-image: url(:/images/splash.png);"
                         "background-position: center center;"
                         "background-repeat: no-repeat;" );
  topStack->addWidget( _net );   // powersystem editor -> 1
  topStack->addWidget( _SSanl ); // steady state analysis editor -> 2
  topStack->addWidget( _sce );   // scenario editor -> 3
  topStack->addWidget( _TDanl ); // analysis editor -> 4
  // Hardware specific editors
  topStack->addWidget( _com );   // communication editor -> 5
  topStack->addWidget( _cal );   // calibration editor -> 6
  topStack->addWidget( _map );   // mapper editor -> 7
  topStack->addWidget( _fit );   // fitter editor -> 8
  topStack->addWidget( _aux );   // auxiliary editor -> 9

  // ---------------------------------------------------------------------------
  // ----- Finalize initialization -----
  showSplash();
  // Set some parameters of the window
  setWindowTitle("Power Network Emulator");
  showMaximized();
  setMinimumSize(800,600);
  cout << "Welcome to power network emulator." << endl;
}

MainWindow::~MainWindow(){}

Console* MainWindow::con(){ return _con; }
ScenarioEditor* MainWindow::sce(){ return _sce; }

void MainWindow::showSplash(){ topStack->setCurrentIndex(MWSHOW_SPLASH); }
void MainWindow::showPowersystem(){ topStack->setCurrentIndex(MWSHOW_NET); }
void MainWindow::showSSAnalysis(){ topStack->setCurrentIndex(MWSHOW_SSANL); }
void MainWindow::showScenarios(){ topStack->setCurrentIndex(MWSHOW_SCE); }
void MainWindow::showTDAnalysis(){ topStack->setCurrentIndex(MWSHOW_TDANL); }

void MainWindow::showCommunication(){ topStack->setCurrentIndex(MWSHOW_COM); }
void MainWindow::showCalibration(){ topStack->setCurrentIndex(MWSHOW_CAL); }
void MainWindow::showMapper(){ topStack->setCurrentIndex(MWSHOW_MAP); }
void MainWindow::showFitter(){ topStack->setCurrentIndex(MWSHOW_FIT); }
void MainWindow::showAuxiliary(){ topStack->setCurrentIndex(MWSHOW_AUX); }

void MainWindow::loadSlot(){

  // Ask for project filename
  QString filename = guiauxiliary::askFileName("etp", true);
  if( filename.isEmpty() )
    return;

  // Parse project file and determine pws, map and scs filenames
  string pwsfilename, schfilename, mapfilename, scsfilename;
  int ans = io::importProject( filename.toStdString(),
                               pwsfilename,
                               schfilename,
                               mapfilename,
                               scsfilename );
  if ( ans ){
    cout << "Project files determined:" << endl;
    cout << " .. pws = " << pwsfilename << endl;
    cout << " .. sch = " << schfilename << endl;
    cout << " .. map = " << mapfilename << endl;
    cout << " .. scs = " << scsfilename << endl;
  } else {
    cout << "Invalid project file!" << endl;
    return;
  }

  // Check whether the files really exist
  QFileInfo pwsinfo(QString::fromStdString(pwsfilename));
  if ( pwsinfo.exists() ){
    importPowersystem( QString::fromStdString(pwsfilename) );
  }
  QFileInfo schinfo(QString::fromStdString(schfilename));
  if ( schinfo.exists() ){
    importSchematic( QString::fromStdString(schfilename) );
  }
  QFileInfo mapinfo(QString::fromStdString(mapfilename));
  if ( mapinfo.exists() ){
    importMapping( QString::fromStdString(mapfilename) );
  }
  QFileInfo scsinfo(QString::fromStdString(scsfilename));
  if ( scsinfo.exists() ){
    importScenarioSet( QString::fromStdString(scsfilename) );
  }

  return;
}

void MainWindow::importPowersystemSlot(){

  QString filename = guiauxiliary::askFileName("xml", true);
  if( filename.isEmpty() )
    return;

  return importPowersystem(filename);
}

void MainWindow::importSchematicSlot(){

  QString filename = guiauxiliary::askFileName("xml", true);
  if( filename.isEmpty() )
    return;

  return importSchematic(filename);
}

void MainWindow::importMappingSlot(){

  QString filename = guiauxiliary::askFileName("xml", true);
  if( filename.isEmpty() )
    return;

  return importMapping(filename);
}

void MainWindow::importScenarioSetSlot(){

  QString filename = guiauxiliary::askFileName("xml", true);
  if (filename.isEmpty() )
    return;

  return importScenarioSet(filename);
}

void MainWindow::exportPowersystemSlot(){

  QString filename = guiauxiliary::askFileName("xml", false);
  if( filename.isEmpty() )
    return;

  int ans = io::exportPowersystem( filename.toStdString(), _pws );
  if ( ans )
    cout << "Error exporting topology." << endl;
  else
    cout << "Topology exported successfully." << endl;

  return;
}

void MainWindow::exportSchematicSlot(){

  QString filename = guiauxiliary::askFileName("xml", false);
  if( filename.isEmpty() )
    return;

  int ans = io::exportSchematic( filename.toStdString(), _net->smd() );
  if ( ans )
    cout << "Error exporting schematic." << endl;
  else
    cout << "Schematic exported successfully." << endl;

  return;
}

void MainWindow::exportMappingSlot(){

  QString filename = guiauxiliary::askFileName("xml", false);
  if( filename.isEmpty() )
    return;

  int ans = io::exportMapping( filename.toStdString(), _emu->mmd() );
  if ( ans )
    cout << "Error exporting mapping." << endl;
  else
    cout << "Mapping exported successfully." << endl;

  return;
}

void MainWindow::exportScenarioSetSlot(){

  QString filename = guiauxiliary::askFileName("xml", false);
  if( filename.isEmpty() )
    return;

  int ans = io::exportScenarioSet( filename.toStdString(), _scs );
  if ( ans )
    cout << "Error exporting scenario set." << endl;
  else
    cout << "Scenario set exported successfully." << endl;

  return;
}

void MainWindow::usbChangedSlot(){

  // Update calibration editor (directly bound to the emulator)
  int ans = _cal->init();
  if ( ans ){
    cout << "Updating calibration editor failed with code " << ans << endl;
  } else {
    cout << "Calibration editor successfully updated" << endl;
  }

  return;
}

void MainWindow::emuChangedSlot( bool complete ){

  // If change in the emulator is complete reinitialize mapper and fitter
  // editors (rebuild the respective GUI)
  if ( complete ){
    // Update calibration editor (directly bound to the emulator)
    int ans = _cal->init(); // TODO check when to use _cal->init()/updt()
    _cal->updt();           // TODO check when to use _cal->init()/updt()

    // Reinitialize mapper editor (directly bound to emulator)
    ans = _map->init();
    if ( ans ){
      cout << "Updating mapper editor failed with code " << ans << endl;
    } else {
      cout << "Mapper editor successfully updated" << endl;
    }

    // Reinitialize fitter editor (directly bound to emulator)
    ans = _fit->init();
    if ( ans ){
      cout << "Updating fitter editor failed with code " << ans << endl;
    } else {
      cout << "Fitter editor successfully updated" << endl;
    }
  }

  // Else, just update the fitter editor to incorporate any changes
  else {
    _cal->updt(); // TODO check when to use _cal->init() & _cal->updt()
//    _cal->init();
    _fit->updt();
  }

  return;
}

void MainWindow::mmdChangedSlot(){
  _map->updt();
  return;
}

void MainWindow::importPowersystem( QString const& filename ){
  int ans;
  // ----- Update backend components -----
  // Import Powersystem proper - update Powersystem _pws
  ans = io::importPowersystem( filename.toStdString(), _pws );
  if( ans ){
    cout << "Error importing powersystem " << ans << endl;
    return;
  }
  // Initialize _emu
  ans = _emu->set_pws( _pws );
  if ( ans ){
    cout << "Hardware emulator representation powersystem change error " << ans << endl;
    return;
  }

  // ----- Update frontend (GUI) components -----
  // Re-initialize _net - also initializes _net.smd
  ans = _net->init();
  if ( ans ){
    cout << "Powersystem editor initialization error " << ans << endl;
    return;
  }
  // Update SSAnalysisEditor - according to _pws changes
  _SSanl->updt();
  // Update MapperEditor _map - according to _emu.mmd changes
  _map->updt();
  // Initialize TDAnalysisEditor - for updating the GTDEditor
  ans = _TDanl->init();
  if ( ans ){
    cout << "Analysis editor initialization error " << ans << endl;
    return;
  }

  cout << "Powersystem imported successfully." << endl;
  return;
}

void MainWindow::importSchematic( QString const& filename ){
  // Import schematic model proper to a temporary object
  PwsSchematicModel* tempSmd = new PwsSchematicModel( _pws );
  int ans = io::importSchematic( filename.toStdString(), tempSmd );
  if ( ans ){
    cout << "Error importing schematic " << ans << endl;
    return;
  }
  // Reinitialize the powersystem editor with the new schematic model object
  ans = _net->init( tempSmd );
  if ( ans ){
    cout << "Error incorporating schematic model changes "
         << "into the powersystem editor " << ans << endl;
    return;
  }

  cout << "Schematic imported successfully." << endl;
  return;
}

void MainWindow::importMapping(const QString& filename){
  int ans;
  // ----- Update backend components -----
  // Import mmd proper
//  mmd->clear();
  ans = io::importMapping( filename.toStdString(),
                           _emu->mmd(), _emu->emuhw(), _pws );
  if ( ans ){
    cout << "Error importing mapping " << ans << endl;
    return;
  }
  ans = _emu->validateMapping();

  // ----- Update frontend (GUI) components -----
  _map->updt(); // Update MapperEditor _map - according to mmd changes

  cout << "Mapping imported successfully." << endl;
  return;
}

void MainWindow::importScenarioSet( QString const& filename ){
  int ans;
  // ----- Update backend components -----
  ans = io::importScenarioSet( filename.toStdString(), _scs );
  if ( ans ){
    cout << "Importing scenario set failed with exit code " << ans << endl;
    return;
  }

  // ----- Update frontend (GUI) components -----
  _sce->updt(); // Update ScenarioEditor _map - according to _scs changes

  cout << "Scenario set imported successfully." << endl;
  return;
}
