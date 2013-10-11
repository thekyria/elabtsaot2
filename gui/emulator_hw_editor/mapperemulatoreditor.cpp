
#include "mapperemulatoreditor.h"
using namespace elabtsaot;

#include "emulatorhw.h"
#include "powersystem.h"
#include "pwsmappermodel.h"

#include "mapperslicewidget.h"

#include <QSplitter>

//#include <vector>
using std::vector;

MapperEmulatorEditor::MapperEmulatorEditor( Powersystem const* pws,
                                            PwsMapperModel const* mmd,
                                            EmulatorHw const* emuhw,
                                            QWidget* parent) :
    QTabWidget( parent ), _pws(pws), _mmd(mmd), _emuhw(emuhw){

  setMinimumHeight(200);
  init();
}

int MapperEmulatorEditor::init(){

  this->clear();
  _slices.clear();

  // Create tabs
  for ( size_t k = 0 ; k != _emuhw->sliceSet.size() ; ++k ){

    QSplitter* tab = new QSplitter(Qt::Horizontal);
    addTab( tab, QString("Slice %0").arg(k) );

    // In the tab add the slice widget for emulator slice k
    MapperSliceWidget* sliceCore;
    sliceCore = new MapperSliceWidget( &_emuhw->sliceSet[k], _pws, _mmd,
                                       _emuhw->sliceSet.size(), this );
    connect( sliceCore, SIGNAL(mappingPositionClicked(int,int,int)),
             this, SIGNAL(mappingPositionClicked(int,int,int)) );
    _slices.push_back( sliceCore );
    tab->addWidget( sliceCore );
  }

  return 0;
}

void MapperEmulatorEditor::repaintTabs(){
  for ( size_t k = 0; k != _slices.size() ; ++k )
    _slices.at(k)->repaint();
}

void MapperEmulatorEditor::zoomIn(){
  if ( _slices.size() > 0 )
    _slices.at(0)->zoomIn();
}

void MapperEmulatorEditor::zoomOut(){
  if ( _slices.size() > 0 )
    _slices.at(0)->zoomOut();
}

void MapperEmulatorEditor::zoomFit(){
  if ( _slices.size() > 0 )
    _slices.at(0)->zoomFit();
}
