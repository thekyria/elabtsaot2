
#include "tabularmappereditor.h"
using namespace elabtsaot;

#include "tabularmapperview.h"
#include "pwsmappermodelelement.h"

#include <QMouseEvent>

TabularMapperEditor::TabularMapperEditor( PwsMapperModel const* mmd,
                                          bool editable,
                                          QWidget* parent) :
    QSplitter(Qt::Horizontal, parent), _mmd(mmd), _editable(editable),
    busView(new TabularMapperView(PWSMODELELEMENTTYPE_BUS, mmd,
                                  editable, this)),
    branchView(new TabularMapperView(PWSMODELELEMENTTYPE_BR, mmd,
                                     editable,  this)),
    genView(new TabularMapperView(PWSMODELELEMENTTYPE_GEN, mmd,
                                  editable, this)),
    loadView(new TabularMapperView(PWSMODELELEMENTTYPE_LOAD, mmd,
                                   editable,this)){

  // Tabular component views
  addWidget( busView );
  connect( busView, SIGNAL(viewClicked(int,QModelIndex)),
           this, SLOT(viewClicked(int,QModelIndex)) );
  addWidget( branchView );
  connect( branchView, SIGNAL(viewClicked(int,QModelIndex)),
           this, SLOT(viewClicked(int,QModelIndex)) );
  addWidget( genView );
  connect( genView, SIGNAL(viewClicked(int,QModelIndex)),
           this, SLOT(viewClicked(int,QModelIndex)) );
  addWidget( loadView );
  connect( loadView, SIGNAL(viewClicked(int,QModelIndex)),
           this, SLOT(viewClicked(int,QModelIndex)) );
}

void TabularMapperEditor::viewClicked(int type, QModelIndex index){

  unsigned int extId;

  switch (type){
  case PWSMODELELEMENTTYPE_BUS:
    // Clear selection in other views
    branchView->clearSelection();
    genView->clearSelection();
    loadView->clearSelection();

    extId = busView->getComponentExtId( index );
    break;

  case PWSMODELELEMENTTYPE_BR:
    // Clear selection in other views
    busView->clearSelection();
    genView->clearSelection();
    loadView->clearSelection();

    extId = branchView->getComponentExtId( index );
    break;

  case PWSMODELELEMENTTYPE_GEN:
    // Clear selection in other views
    busView->clearSelection();
    branchView->clearSelection();
    loadView->clearSelection();

    extId = genView->getComponentExtId( index );
    break;

  case PWSMODELELEMENTTYPE_LOAD:
    // Clear selection in other views
    busView->clearSelection();
    branchView->clearSelection();
    genView->clearSelection();

    extId = loadView->getComponentExtId( index );
    break;
  }

  emit componentSelected( type, extId );
}

void TabularMapperEditor::updt(){
  busView->updt();
  branchView->updt();
  genView->updt();
  loadView->updt();
}

void TabularMapperEditor::clearSelection(){
  busView->clearSelection();
  branchView->clearSelection();
  genView->clearSelection();
  loadView->clearSelection();
}
