
#include "powersystemeditor.h"
using namespace elabtsaot;

#include "powersystem.h"
#include "powersystemeditordialogs.h"
#include "tabularpowersystemeditor.h"
#include "schematiceditor.h"
#include "pwsschematicmodel.h"

#include <QToolBar>
#include <QAction>
#include <QComboBox>

#include <list>
using std::list;
//#include <vector>
using std::vector;
#include <iostream>
using std::cout;
using std::endl;

using std::pair;

PowersystemEditor::PowersystemEditor( Powersystem* pws,
                                      PwsModel* const pwsModel0,
                                      QWidget* parent ) :
    QSplitter(Qt::Vertical, parent),
    _pws(pws),
    _smd(new PwsSchematicModel(pws)),
    _pwsModel0(pwsModel0),
    _tbl(new TabularPowersystemEditor( _pws, true, this )),
    _sch(new SchematicEditor(_pws, _smd, this)){

  // ----- Tabular editor -----
  addWidget(_tbl);

  // ----- Network editor (tabular & schematic) toolbar -----
  QToolBar* ntwToolbar = new QToolBar("Network editor toolbar", this);
  addWidget( ntwToolbar );
  ntwToolbar->setFixedHeight(30);

  // -- Separator
  // Reset schematic act (button)
  QAction* resetSchematicAct = new QAction( QIcon(":/images/reset.png"),
                                            "Reset schematic", ntwToolbar );
  ntwToolbar->addAction( resetSchematicAct );
  connect( resetSchematicAct, SIGNAL( triggered() ),
           this, SLOT( resetSchematicSlot()) );

  // -- Separator
  ntwToolbar->addSeparator();

  // Clear power system act (button)
  QAction* clearPowersystemAct = new QAction( QIcon(":/images/clearall.png"),
                                              "Clear power system", ntwToolbar);
  ntwToolbar->addAction( clearPowersystemAct );
  connect( clearPowersystemAct, SIGNAL( triggered() ),
           this, SLOT( clearPowersystem() ) );

  // Remove selected elements act (button)
  QAction* removeElementAct = new QAction( QIcon(":/images/delete.png"),
                                        "Remove selected elements", ntwToolbar );
  ntwToolbar->addAction( removeElementAct );
  connect( removeElementAct, SIGNAL( triggered() ),
           this, SLOT( removeSelectedElements() ) );

  // Edit selected element act (button)
  QAction* editElementAct = new QAction( QIcon(":/images/edit.png"),
                                         "Edit element", ntwToolbar );
  ntwToolbar->addAction( editElementAct );
  connect( editElementAct, SIGNAL( triggered() ),
           this, SLOT( editElement() ) );

  // Add bus act (button)
  QAction* addBusAct = new QAction ( QIcon(":/images/bus.png"),
                                     "Add bus", ntwToolbar);
  ntwToolbar->addAction( addBusAct );
  connect( addBusAct, SIGNAL( triggered()),
           this, SLOT( addBusSlot() ) );

  // Add branch act (button)
  QAction* addBranchAct = new QAction ( QIcon(":/images/branch.png"),
                                        "Add branch", ntwToolbar);
  ntwToolbar->addAction( addBranchAct );
  connect( addBranchAct, SIGNAL( triggered()),
           this, SLOT( addBranchSlot() ) );

  // Add generator act (button)
  QAction* addGeneratorAct = new QAction ( QIcon(":/images/generator.png"),
                                           "Add generator", ntwToolbar);
  ntwToolbar->addAction( addGeneratorAct );
  connect( addGeneratorAct, SIGNAL( triggered()),
           this, SLOT( addGeneratorSlot() ) );

  // Add load act (button)
  QAction* addLoadAct = new QAction ( QIcon(":/images/load.png"),
                                      "Add load", ntwToolbar);
  ntwToolbar->addAction( addLoadAct );
  connect( addLoadAct, SIGNAL( triggered()),
           this, SLOT( addLoadSlot() ) );

  // Set slack generator button (action)
  QAction* setSlackGenAct = new QAction( QIcon(":/images/slack.png"),
                                         "Set slack generator", ntwToolbar);
  ntwToolbar->addAction( setSlackGenAct );
  connect( setSlackGenAct, SIGNAL( triggered() ),
           this, SLOT( setSlackGen() ) );

  // Validate powersystem button (action)
  QAction* validateAct = new QAction( QIcon(":/images/validate.png"),
                                      "Validate powersystem", ntwToolbar );
  ntwToolbar->addAction( validateAct );
  connect( validateAct, SIGNAL( triggered() ),
           this, SLOT(validatePowersystem()) );

  // -- Separator
  ntwToolbar->addSeparator();

  // Zoom in button (action)
  QAction* zoomInAct = new QAction( QIcon(":/images/zoom-in.png"),
                                    "Zoom in", ntwToolbar );
  ntwToolbar->addAction( zoomInAct );
  connect( zoomInAct, SIGNAL( triggered() ),
           _sch, SLOT(zoomIn()) );

  // Zoom out button (action)
  QAction* zoomOutAct = new QAction( QIcon(":/images/zoom-out.png"),
                                     "Zoom out", ntwToolbar );
  ntwToolbar->addAction( zoomOutAct );
  connect( zoomOutAct, SIGNAL( triggered() ),
           _sch, SLOT(zoomOut()) );

  // Zoom fit button (action)
  QAction* zoomFitAct = new QAction( QIcon(":/images/zoom-fit.png"),
                                     "Zoom fit", ntwToolbar );
  ntwToolbar->addAction( zoomFitAct );
  connect( zoomFitAct, SIGNAL( triggered() ),
           _sch, SLOT(zoomFit()) );

  // Take snapshot view (action)
  QAction* takeSnapshotAct = new QAction( QIcon(":/images/snapshot.png"),
                                          "Take snapshot", ntwToolbar );
  ntwToolbar->addAction( takeSnapshotAct );
  connect( takeSnapshotAct, SIGNAL( triggered() ),
           _sch, SLOT(snapshot()) );

  // ----- Schematic editor -----
  addWidget(_sch);

  connect( _tbl, SIGNAL(componentSelected(int,uint)),
           _sch, SLOT(componentSelectedSlot(int,uint)) );
}

PowersystemEditor::~PowersystemEditor(){ delete _smd; }

int PowersystemEditor::init(){

  // Update the powersystem schematic model
  int ans = _smd->init();
  if ( ans ) return 1;
  ans = _smd->planarizeSchematic(_pws);
  if ( ans ) return 2;

  // Update other models associated with the powersyste
  if (_pwsModel0)
    ans = _pwsModel0->init();

  // Update the GUI
  updt();

  return 0;
}

int PowersystemEditor::init( PwsSchematicModel* smd ){

  // Validate the new schematic model against the currnet powersystem
  int ans = smd->validate();
  if ( ans ) return 1;

  // If validation was successful then copy the provided schematic model
  ans = _smd->copy( *smd );
  if ( ans ) return 2;

  // Update the GUI
  updt();

  return 0;
}

void PowersystemEditor::updt(){
  _tbl->updt();
  _sch->repaint();
}

PwsSchematicModel* PowersystemEditor::smd(){ return _smd; }
PwsSchematicModel const* PowersystemEditor::smd() const{ return _smd; }

void PowersystemEditor::resetSchematicSlot(){

  int ans = _smd->planarizeSchematic(_pws);
  if ( ans )
    cout << "Reset schematic failed with code " << ans << endl;
  else
    cout << "Reset schematic suceeded!" << endl;

  // Update the GUI
  updt();

  return;
}

void PowersystemEditor::clearPowersystem(){

  _pws->clear();
  _smd->clear();
  if (_pwsModel0)
    _pwsModel0->clear();

  _tbl->updt();
  _sch->clearSelected();
  _sch->repaint();

  return;
}

void PowersystemEditor::removeSelectedElements(){

  // PERHAPS: if ( _tbl->hasFocus() ){ ... } else if ( _sch->hasFocus() ){ ... }

  // Schematic editor has focus so delete elements selected in the schematic
  // view
  // --- Determine selected elements ---
  QVector< pair<int,int> > selectedElements = _sch->selected();

  // --- Remove selected elements from the powersystem and the component model ---
  int type;
  unsigned int extId;
  for(unsigned int k = 0 ; static_cast<int>(k)!=selectedElements.size() ; ++k){
    type = selectedElements.at(k).first;
    extId = selectedElements.at(k).second;
    switch ( type ){
    case PWSMODELELEMENTTYPE_BUS:
      _pws->deleteBus( extId , true );
      break;
    case PWSMODELELEMENTTYPE_BR:
      _pws->deleteBranch( extId );
      break;
    case PWSMODELELEMENTTYPE_GEN:
      _pws->deleteGen( extId );
      break;
    case PWSMODELELEMENTTYPE_LOAD:
      _pws->deleteLoad( extId );
      break;
    }

    // Remove element from the powersystem schematic model
    _smd->remElement( type, extId );
    // Remove element from other models associated with the powersystem
    if (_pwsModel0)
      _pwsModel0->remElement( type, extId );
  }

  // --- Update tabular and schematic views ---
  _tbl->updt();
  _sch->clearSelected();
  _sch->repaint();

  return;
}

void PowersystemEditor::editElement(){
  if ( _tbl->hasFocus() ){
    // Tabular editor has focus so edit elements selected in the tabular view
    // TODO: delete selected elements in tabular view

  } else if ( _sch->hasFocus() ){
    // Schematic editor has focus so edit elements selected in the schematic
    // view

    // --- Determine selected elements ---
    QVector< pair<int,int> > selectedElements = _sch->selected();
    if ( selectedElements.size() > 1 )
      // Cannot edit element if more than one selected!
      return;

    int type = selectedElements.at(0).first;
    unsigned int extId = selectedElements.at(0).second;
    int ans;
    switch ( type ){
    case PWSMODELELEMENTTYPE_BUS:{
      Bus* pBus = 0;
      ans = _pws->getBus( extId , pBus );
      if ( ans ){
        cout << "Powersystem::getBus() failed" << endl;
        return;
      }
      ans = pwsEditorDialogs::busDialog( pBus );
      // If dialog failed then exit without doing nothing
      if ( ans )
        return;
      break; }

    case PWSMODELELEMENTTYPE_BR:{
      Branch* pBr= 0;
      ans = _pws->getBranch( extId , pBr );
      if ( ans ){
        cout << "Powersystem::getBranch() failed" << endl;
        return;
      }
      ans = pwsEditorDialogs::branchDialog( pBr );
      // If dialog failed then exit without doing nothing
      if ( ans )
        return;
      break; }

    case PWSMODELELEMENTTYPE_GEN:{
      Generator* pGen = 0;
      ans = _pws->getGenerator( extId , pGen );
      if ( ans ){
        cout << "Powersystem::getGenerator() failed" << endl;
        return;
      }
      ans = pwsEditorDialogs::genDialog( pGen );
      // If dialog failed then exit without doing nothing
      if ( ans )
        return;
      break; }

    case PWSMODELELEMENTTYPE_LOAD:{
      Load* pLoad = 0;
      ans = _pws->getLoad( extId , pLoad );
      if ( ans ){
        cout << "Powersystem::getLoad() failed" << endl;
        return;
      }
      ans = pwsEditorDialogs::loadDialog( pLoad );
      // If dialog failed then exit without doing nothing
      if ( ans )
        return;
      break; }

    }
  }

  // --- Update the tabular and the schematic views ---
  _tbl->updt();

  return;
}

void PowersystemEditor::addBusSlot(){

  // --- Pop up bus dialog ---
  Bus bus;
  int ans = pwsEditorDialogs::busDialog( &bus );
  // If dialog failed then exit without doing nothing
  if ( ans )
    return;
  // otherwise ...

  // --- Add bus to the powersystem ---
  ans = _pws->addBus( bus );
  if ( ans ){
    cout << "Adding bus to the powersystem failed with code " << ans << endl;
    return;
  }

  // --- Add the bus element (in the middle of the '_sch' widget) ---
  QPoint temp = QPoint( _sch->width()/2 , _sch->height()/2 );
  if (_sch->isInvertible() ){
    temp = _sch->inverseTransform().map( temp );
  } else{
    // Should never happen!
    return;
  }
  ans = _smd->addBusElement( bus, temp.x(), temp.y() );
  if ( ans ){
    cout << "Adding bus element to the powersystem schematic model "
         << "failed with code " << ans << endl;
    return;
  }

  // --- Add the element to other models associated with the powersystem ---
  if (_pwsModel0)
    _pwsModel0->addBusElement( bus, true );

  // --- Update the tabular and the schematic views ---
  _tbl->updt();
  _sch->repaint();

  return;
}

void PowersystemEditor::addBranchSlot(){

  // --- Pop up branch dialog ---
  Branch branch;
  int ans = pwsEditorDialogs::branchDialog( &branch );
  // If dialog failed then exit without doing nothing
  if ( ans )
    return;
  // otherwise ...

  // --- Add branch to the powersystem ---
  ans = _pws->addBranch( branch );
  if ( ans ){
    cout << "Adding branch to the powersystem failed with code " << ans << endl;
    return;
  }

  // --- Add the branch element to the powersystem schematic model ---
  ans = _smd->addBranchElement( branch );
  if ( ans ){
    cout << "Adding branch element to the powersystem schematic model "
         << "failed with code " << ans << endl;
    return;
  }

  // --- Add the element to other models associated with the powersystem ---
  if (_pwsModel0)
    _pwsModel0->addBranchElement( branch, true );

  // --- Update the tabular and the schematic views ---
  _tbl->updt();
  _sch->repaint();

  return;
}

void PowersystemEditor::addGeneratorSlot(){

  // --- Pop up generator dialog ---
  Generator gen;
  int ans = pwsEditorDialogs::genDialog( &gen );
  // If dialog failed then exit without doing nothing
  if ( ans )
    return;
  // otherwise ...

  // --- Add generator to the powersystem ---
  ans = _pws->addGen( gen );
  if ( ans ){
    cout << "Adding generator to the powersystem failed with code " << ans << endl;
    return;
  }

  // --- Add the generator component to the powersystem schematic model ---
  ans = _smd->addGenElement( gen );
  if ( ans ){
    cout << "Adding generator element to the powersystem schematic model "
         << "failed with code " << ans << endl;
    return;
  }

  // --- Add the element to other models associated with the powersystem ---
  if (_pwsModel0)
    _pwsModel0->addGenElement( gen, true );

  // --- Update the tabular and the schematic views ---
  _tbl->updt();
  _sch->repaint();

  return;
}

void PowersystemEditor::addLoadSlot(){

  // --- Pop up load dialog ---
  Load load;
  int ans = pwsEditorDialogs::loadDialog( &load );
  // If dialog failed then exit without doing nothing
  if ( ans )
    return;
  // otherwise ...

  // --- Add the load to the powersystem ---
  ans = _pws->addLoad( load );
  if ( ans ){
    cout << "Adding load to the powersystem failed with code " << ans << endl;
    return;
  }

  // --- Add the load component to the powersystem schematic model ---
  ans = _smd->addLoadElement( load );
  if ( ans ){
    cout << "Adding load element to the powersystem schematic model "
         << "failed with code " << ans << endl;
    return;
  }

  // --- Add the element to other models associated with the powersystem ---
  if (_pwsModel0)
    _pwsModel0->addLoadElement( load, true );

  // --- Update tabular and the schematic views ---
  _tbl->updt();
  _sch->repaint();

  return;
}

void PowersystemEditor::setSlackGen(){

  int newSlackGenExtId;
  int ans = pwsEditorDialogs::slackDialog( _pws->slackBusExtId(),
                                                   _pws->slackGenExtId(),
                                                   &newSlackGenExtId );

  // If dialog failed the exit without doing anything
  if ( ans )
    return;
  // otherwise ...

  // Check the value input by the user; if it is a valid generator then set it
  // as slack and its respective bus as slack bus
  int newSlackGenIntId = _pws->getGen_intId( newSlackGenExtId );
  if ( newSlackGenIntId >= 0 ){
    _pws->set_slackGenExtId( newSlackGenExtId );
    int newSlackBusExtId = _pws->getGenerator(newSlackGenIntId)->busExtId();
    _pws->set_slackBusExtId( newSlackBusExtId );
  } else{
    cout << "Invalid generator input" << endl;
  }

  return;
}

void PowersystemEditor::validatePowersystem(){
  int ans = _pws->validate();
  if( ans )
    cout << "Network validation failed with code " << ans << "." << endl;
  else
    cout << "Network validation succedeed." << endl;

  return;
}
