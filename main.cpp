
#include "importerexporter.h"

#include "powersystem.h"
#include "emulator.h"

#include "ssengine.h"
#include "moteurrenard.h"
#include "moteurfengtian.h"

#include "tdengine.h"
#include "scenarioset.h"
#include "tdresultsbank.h"
#include "tdemulator.h"
#include "simulator_sw.h"

#include "mainwindow.h"
using namespace elabtsaot;

#include <QDebug>
#include <QApplication>
#include <QIcon>
#include <QTime>

/*! This is the main function. It spawns an object of MainWindow and then
executes it returning where the callback loop is escaped. */
int main(int argc, char *argv[]){
  int ans;
  // ----- Initialize backend components -----
  qDebug() << "Creating powersystem ... ";
  Powersystem* pws( new Powersystem("unnamed power system", 100, 50) );

  qDebug() << "Creating hardware platform representation ... ";
  Emulator* emu( new Emulator(pws) );
  qDebug() << "Initializing hardware platform representation ... ";
  ans = emu->init();
  if ( ans )
    qDebug() << "Hardware platform representation initialization failed with code " << ans;

  // Steady state engines
  qDebug() << "Creating SSEngine 'MoteurRenard' ... ";
  MoteurRenard* sse_mrn( new MoteurRenard() );

  qDebug() << "Creating SSEngine 'MoteurFengtian' ... ";
  MoteurFengtian* sse_fen( new MoteurFengtian(emu, NULL) );

  qDebug() << "Setting SSEngine ptr ... ";
  SSEngine* sse( sse_mrn );

  // Time domain engines
  qDebug() << "Creating TDEngine 'TDEmulator' ... ";
  TDEmulator* tde_hwe( new TDEmulator(pws, emu, NULL) );
  qDebug() << "Initializing TDEngine 'TDEmulator' ... ";

  qDebug() << "Creating TDEngine 'Simulator_hw' ... ";
  Simulator_sw* tde_swe( new Simulator_sw(pws, sse, NULL) );
  qDebug() << "Initializing TDEngine 'Simulator_hw' ... ";
  ans = tde_swe->init();
  if ( ans )
    qDebug() << "Software TDengine initialization failed with code " << ans;

  qDebug() << "Setting TDEngine ptr ... ";
  TDEngine* tde( tde_hwe );

  qDebug() << "Creating scenario set ... ";
  ScenarioSet* scs( new ScenarioSet() );

  qDebug() << "Creating TD results bank ... ";
  TDResultsBank* trb( new TDResultsBank() );

  //! ----- Fire up a Qt application -----
  // Initialize resources
  qsrand(QTime(0,0,0).secsTo(QTime::currentTime())); // RNG seed
  Q_INIT_RESOURCE(elab_tsaot2);
  // Set up app parameters
  QApplication app(argc, argv);
  app.setWindowIcon( QIcon(":/images/elab-tsaot_logo.png") );
  app.setOrganizationName("ELAB EPFL");
  app.setApplicationName("elab-tsaot");
  // Create an application object and show it
  MainWindow* mainWindow( new MainWindow( pws,
                                          emu,
                                          sse,
                                          sse_mrn,
                                          sse_fen,
                                          tde,
                                          tde_hwe,
                                          tde_swe,
                                          scs,
                                          trb) );
  mainWindow->show();

  // Application is event-driven
  ans = app.exec();

  // Garbage collection
  delete pws;
  delete emu;
  delete sse_mrn;
  delete sse_fen;
  delete tde_hwe;
  delete tde_swe;
  delete scs;
  delete trb;

  return ans;
}
