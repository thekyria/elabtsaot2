
#include "tabularpowersystemview.h"
using namespace elabtsaot;

#include "tabularpowersysteminterface.h"
#include "pwsmappermodelelement.h"

#include <QLabel>
#include <QTableView>
#include <QHeaderView>

TabularPowersystemView::TabularPowersystemView(int type,
                                               Powersystem* pws,
                                               bool editable,
                                               QWidget* parent) :
    QSplitter( Qt::Vertical, parent ),
    _type(type),
    _view(new QTableView(parent)),
    _model(new TabularPowersystemInterface( type, pws, this )){

  QLabel* text;
  switch( type ){

  case PWSMODELELEMENTTYPE_BUS:
    text = new QLabel( "Buses" );
    break;

  case PWSMODELELEMENTTYPE_BR:
    text = new QLabel( "Branches" );
    break;

  case PWSMODELELEMENTTYPE_GEN:
    text = new QLabel( "Generators" );
    break;

  case PWSMODELELEMENTTYPE_LOAD:
    text = new QLabel( "Loads" );
    break;

  default:
    text = new QLabel( "Error!" );
    break;
  }
  text->setFixedHeight(10);

  // Create main _view
  _view->setModel( _model );
  _view->verticalHeader()->hide();
  _view->verticalHeader()->setDefaultSectionSize(14);
  _view->setSelectionMode(QAbstractItemView::SingleSelection);
  _view->setShowGrid(false);
  _view->resizeColumnsToContents();
  connect( _view, SIGNAL(pressed(QModelIndex)),
           this, SLOT(viewClickedSlot(QModelIndex)) );

  // Tidy up widgets
  addWidget( text );
  addWidget( _view );
  setCollapsible(0,false);
  setCollapsible(1,false);

  // If the 'editable' flag is set to false then disable editting for the table
  // view and enable selection by rows
  if ( !editable ){
    _view->setEditTriggers( QAbstractItemView::NoEditTriggers );
    _view->setSelectionBehavior( QAbstractItemView::SelectRows );
  }

  _view->show();
}

void TabularPowersystemView::mouseDoubleClickEvent(QMouseEvent *){
  if( _view->isHidden() ){
    _view->show();
  }else{
    _view->hide();
  }
}

void TabularPowersystemView::updt(){
  _model->update();
}

void TabularPowersystemView::clearSelection(){
  _view->clearSelection();
}

unsigned int
TabularPowersystemView::getComponentExtId(QModelIndex const& index){
  return static_cast<unsigned int>( _model->data( _model->index( index.row(),0),
                                                  Qt::DisplayRole ).toUInt() );
}

void TabularPowersystemView::collapseView(){
  _view->hide();
}

void TabularPowersystemView::expandView(){
  _view->show();
}

void TabularPowersystemView::viewClickedSlot(QModelIndex index){
  emit viewClicked(_type, index);
}
