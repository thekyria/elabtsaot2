
#include "fitteremulatoreditor.h"
using namespace elabtsaot;

#include "emulatorhw.h"

#include "fitterslicetab.h"
#include "fitteremutab.h"

#include <QSplitter>

//#include <vector>
using std::vector;

FitterEmulatorEditor::FitterEmulatorEditor( EmulatorHw* emuhw, QWidget *parent) :
    QTabWidget( parent ), _emuhw(emuhw){
  setMinimumHeight(200);
  init(); // Initialize slice tabs
}

int FitterEmulatorEditor::init(){

  // Remove all tabs
  this->clear();

  // Clear slice tabs from the store
  _sliceTabs.clear(); // drop slice tabs from container
  // Create slice tabs back
  for ( size_t k = 0 ; k != _emuhw->sliceSet.size() ; ++k ){
    FitterSliceTab* tab = new FitterSliceTab( &_emuhw->sliceSet[k], this );
    connect( tab, SIGNAL(fittingPositionClicked(int,int,int,int)),
             this, SIGNAL(fittingPositionClicked(int,int,int,int)) );
    _sliceTabs.push_back( tab );
    // and add them to the QTabWidget (this)
    this->addTab( tab, QString("Slice %0").arg(k) );
  }

  return 0;
}

void FitterEmulatorEditor::updt(){

  // Slice tab
  for ( size_t k = 0; k != _sliceTabs.size() ; ++k ){
    // Update the non-FitterSliceWidget part of the FitterSliceTab
    _sliceTabs.at(k)->updt();
    // Repaint the FitterSliceWidget part of the FitterSliceTab
    _sliceTabs.at(k)->repaint();
  }
}

bool FitterEmulatorEditor::isShowingReal() const{
  if ( _sliceTabs.size() > 0 )
    return _sliceTabs.at(0)->isShowingReal();
  else
    // Normally should never be reached!
    return true;
}

void FitterEmulatorEditor::toggleReal(){
  if ( _sliceTabs.size() > 0 )
    _sliceTabs.at(0)->toggleReal();
}

void FitterEmulatorEditor::zoomIn(){
  if ( _sliceTabs.size() > 0 )
    _sliceTabs.at(0)->zoomIn();
}

void FitterEmulatorEditor::zoomOut(){
  if ( _sliceTabs.size() > 0 )
    _sliceTabs.at(0)->zoomOut();
}

void FitterEmulatorEditor::zoomFit(){
  if ( _sliceTabs.size() > 0 )
    _sliceTabs.at(0)->zoomFit();
}
