
#include "mappereditor.h"
using namespace elabtsaot;

#include "emulator.h"

#include "tabularmappereditor.h"
#include "mapperemulatoreditor.h"

#include <QToolBar>
#include <QAction>
#include <QIcon>

//#include <vector>
using std::vector;
using std::pair;
#include <iostream>
using std::cout;
using std::endl;
#include <utility>
using std::make_pair;

MapperEditor::MapperEditor( Emulator* emu, QWidget* parent ) :
    QSplitter(Qt::Vertical, parent ), _emu(emu){

  // --- Tabular component editor ---
  tbl = new TabularMapperEditor( emu->mmd(), false, this );
  addWidget( tbl );
  connect( tbl, SIGNAL(componentSelected(int,uint)),
           this, SLOT(componentSelectedSlot(int,uint)) );

  // --- Emulator editor (mapper edition) ---
  mee = new MapperEmulatorEditor( emu->pws(), emu->mmd(), emu->emuhw(), this );
  addWidget( mee );
  connect( mee , SIGNAL(mappingPositionClicked(int,int,int)),
           this, SLOT(mappingPositionClickedSlot(int,int,int)) );

  // --- Mapper editor toolbar ---
  QToolBar* mapToolbar = new QToolBar("Mapper editor toolbar", this);
  this->addWidget( mapToolbar );
  mapToolbar->setFixedHeight(30);

  // Update view act
  QAction* updtAct = new QAction( QIcon(":/images/update.png"),
                                  "Update mapper editor view", mapToolbar );
  mapToolbar->addAction( updtAct );
  connect( updtAct, SIGNAL(triggered()),
           this, SLOT(updt()) );

  mapToolbar->addSeparator(); // -- Separator

  // Reset mapping act (button)
  QAction* resetMappingAct = new QAction( QIcon(":/images/reset.png"),
                                          "Reset mapping", mapToolbar );
  mapToolbar->addAction(resetMappingAct);
  connect( resetMappingAct, SIGNAL(triggered()),
           this, SLOT(resetMappingSlot()) );

  // Automap act (button)
  QAction* autoMappingAct = new QAction( QIcon(":/images/execute.png"),
                                         "Automap", mapToolbar );
  mapToolbar->addAction(autoMappingAct);
  connect( autoMappingAct, SIGNAL(triggered()),
           this, SLOT(autoMappingSlot()) );

  // Validate mapping act (button)
  QAction* validateMappingAct = new QAction( QIcon(":/images/validate.png"),
                                             "Validate mapping", mapToolbar );
  mapToolbar->addAction(validateMappingAct);
  connect( validateMappingAct, SIGNAL(triggered()),
           this, SLOT(validateMappingSlot()) );

  mapToolbar->addSeparator(); // -- Separator

  // Unmap act (button)
  QAction* unmapComponentAct = new QAction( QIcon(":/images/unmap.png"),
                                            "Unmap component", mapToolbar);
  mapToolbar->addAction(unmapComponentAct);
  connect( unmapComponentAct, SIGNAL(triggered()),
           this, SLOT(unmapComponentSlot()) );

  mapToolbar->addSeparator(); // -- Separator

  // Zoom in act (button)
  QAction* zoomInAct = new QAction( QIcon(":/images/zoom-in.png"),
                                    "Zoom in", mapToolbar );
  mapToolbar->addAction(zoomInAct);
  connect( zoomInAct, SIGNAL(triggered()), mee, SLOT(zoomIn()) );

  // Zoom out act (button)
  QAction* zoomOutAct = new QAction( QIcon(":/images/zoom-out.png"),
                                     "Zoom out", mapToolbar );
  mapToolbar->addAction(zoomOutAct);
  connect( zoomOutAct, SIGNAL(triggered()), mee, SLOT(zoomOut()) );

  // Zoom fit act (button)
  QAction* zoomFitAct = new QAction( QIcon(":/images/zoom-fit.png"),
                                     "Zoom fit", mapToolbar );
  mapToolbar->addAction(zoomFitAct);
  connect( zoomFitAct, SIGNAL(triggered()), mee, SLOT(zoomFit()) );

  mapToolbar->addSeparator(); // -- Separator
}

// init() initializes the editors gui (rebuilds it) according to the feeder
// backend structures
int MapperEditor::init(){

  if ( mee->init() )
    return 3;

  updt();

  return 0;
}

// updt() updats the editors gui (only repaints it) according to the feeder
// backend structures
void MapperEditor::updt(){

  _emu->cleaMappingHints();

  _selected.clear();
  tbl->clearSelection();
  tbl->updt();
  mee->repaintTabs();

  return;
}

void MapperEditor::resetMappingSlot(){

  int ans = _emu->resetMapping();
  if ( ans )
    cout << "Reset mapping failed with exit code " << ans << endl;
  else
    cout << "Reset mapping succeeded!" << endl;

  updt();

  return;
}

void MapperEditor::autoMappingSlot(){

  int ans = _emu->autoMapping();
  if ( ans )
    cout << "Automatic mapping failed with exit code " << ans << endl;
  else
    cout << "Automatic mapping succeeded!" << endl;

  updt();

  return;
}

void MapperEditor::validateMappingSlot(){

  int ans = _emu->validateMapping();
  if ( ans )
    cout << "Validation of current mapping failed with exit code "<<ans << endl;
  else
    cout << "Validation of current mapping succeeded!" << endl;

  updt();

  return;
}

void MapperEditor::unmapComponentSlot(){

  for ( size_t k = 0 ; k != _selected.size() ; ++k )
    _emu->unmapComponent( _selected.at(k).first, _selected.at(k).second );

  updt();

  return;
}

void MapperEditor::componentSelectedSlot(int type, unsigned int extId){

  _selected.clear();
  _selected.push_back( make_pair(type,extId) );

  _emu->hintComponent( type, extId );

  mee->repaintTabs();

  return;
}

void MapperEditor::mappingPositionClickedSlot( int mapper_tab,
                                               int mapper_row,
                                               int mapper_col ){

  if ( _selected.size() == 1 ){
    int type = _selected.at(0).first;
    unsigned int extId = _selected.at(0).second;
    int ans = _emu->mapComponent( type, extId,
                                  mapper_tab, mapper_row, mapper_col );
    if ( ans ){
      cout << "Mapping of component: " << _selected.at(0).first << ","
                                       << _selected.at(0).second << " ";
      cout << "at position " << mapper_tab << ","
                             << mapper_row << ","
                             << mapper_col << " ";
      cout << "failed with code " << ans << endl;

    } else{
      cout << "Mapping of component: " << _selected.at(0).first << ","
                                       << _selected.at(0).second << " ";
      cout << "at position " << mapper_tab << ","
                             << mapper_row << ","
                             << mapper_col << " ";
      cout << "succeded!" << endl;

      updt();
    }
  }

  return;
}
