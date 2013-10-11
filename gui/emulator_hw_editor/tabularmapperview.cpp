
#include "tabularmapperview.h"
using namespace elabtsaot;

#include "pwsmappermodelelement.h"
#include "tabularmapperinterface.h"

#include <QLabel>
#include <QTableView>
#include <QHeaderView>
#include <QMouseEvent>

TabularMapperView::TabularMapperView(int type,
                                     PwsMapperModel const* mmd,
                                     bool editable,
                                     QWidget* parent) :
    QSplitter( Qt::Vertical, parent ),
    _type(type),
    _view(new QTableView(parent)),
    _model(new TabularMapperInterface(type, mmd, this)) {

  QLabel* text = new QLabel( this );
  // Set parameters
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
  // _view and enable selection by rows
  if ( !editable ){
    _view->setEditTriggers( QAbstractItemView::NoEditTriggers );
    _view->setSelectionBehavior( QAbstractItemView::SelectRows ); //TODO test!
  }

  _view->show();
}

void TabularMapperView::updt(){
  _model->update();
}

void TabularMapperView::clearSelection(){
  _view->clearSelection();
}

unsigned int TabularMapperView::getComponentExtId(QModelIndex const& index){
  return static_cast<unsigned int>( _model->data( _model->index( index.row(),1),
                                                  Qt::DisplayRole ).toUInt() );
}

void TabularMapperView::viewClickedSlot(QModelIndex index){
  emit viewClicked(_type, index);
}
