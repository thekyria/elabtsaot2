
#include "tabularpowersystemeditor.h"
using namespace elabtsaot;

#include "tabularpowersystemview.h"
#include "pwsmappermodelelement.h"

TabularPowersystemEditor::TabularPowersystemEditor( Powersystem* pws,
                                                    bool editable,
                                                    QWidget* parent) :
    QSplitter( Qt::Vertical, parent),
    _busView(new TabularPowersystemView( PWSMODELELEMENTTYPE_BUS, pws,
                                         editable, this)),
    _branchView(new TabularPowersystemView( PWSMODELELEMENTTYPE_BR, pws,
                                            editable,  this)),
    _genView(new TabularPowersystemView( PWSMODELELEMENTTYPE_GEN, pws,
                                         editable, this)),
    _loadView(new TabularPowersystemView( PWSMODELELEMENTTYPE_LOAD, pws,
                                          editable, this)){

  addWidget( _busView );
  connect( _busView, SIGNAL(viewClicked(int,QModelIndex)),
           this, SLOT(viewClicked(int,QModelIndex)) );
  addWidget( _branchView );
  connect( _branchView, SIGNAL(viewClicked(int,QModelIndex)),
           this, SLOT(viewClicked(int,QModelIndex)) );
  addWidget( _genView );
  connect( _genView, SIGNAL(viewClicked(int,QModelIndex)),
           this, SLOT(viewClicked(int,QModelIndex)) );
  addWidget( _loadView );
  connect( _loadView, SIGNAL(viewClicked(int,QModelIndex)),
           this, SLOT(viewClicked(int,QModelIndex)) );
}

void TabularPowersystemEditor::viewClicked(int type, QModelIndex index){
  unsigned int extId;

  switch (type){
  case PWSMODELELEMENTTYPE_BUS:
    // Clear selection in other views
    _branchView->clearSelection();
    _genView->clearSelection();
    _loadView->clearSelection();

    extId = _busView->getComponentExtId( index );
    break;

  case PWSMODELELEMENTTYPE_BR:
    // Clear selection in other views
    _busView->clearSelection();
    _genView->clearSelection();
    _loadView->clearSelection();

    extId = _branchView->getComponentExtId( index );
    break;

  case PWSMODELELEMENTTYPE_GEN:
    // Clear selection in other views
    _busView->clearSelection();
    _branchView->clearSelection();
    _loadView->clearSelection();

    extId = _genView->getComponentExtId( index );
    break;

  case PWSMODELELEMENTTYPE_LOAD:
    // Clear selection in other views
    _busView->clearSelection();
    _branchView->clearSelection();
    _genView->clearSelection();

    extId = _loadView->getComponentExtId( index );
    break;

  default:
    // Unrecognized component type
    // Do nothing
    return;
    break;
  }

  emit componentSelected( type, extId );
}

void TabularPowersystemEditor::updt(){
  _busView->updt();
  _branchView->updt();
  _genView->updt();
  _loadView->updt();
}

void TabularPowersystemEditor::expandAll(){
  _busView->expandView();
  _branchView->expandView();
  _genView->expandView();
  _loadView->expandView();
}

void TabularPowersystemEditor::collapseAll(){
  _busView->collapseView();
  _branchView->collapseView();
  _genView->collapseView();
  _loadView->collapseView();
}
