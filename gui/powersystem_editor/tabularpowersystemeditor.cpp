
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

/*

// *****************************************************************************
// Implementation of the subclassing of QTableView to be used to preset network
// components

ComponentView::ComponentView(ComponentSet *set, QWidget *parent):QTableView(parent){
    // Adjust parameters to meet needs
    verticalHeader()->hide();
    verticalHeader()->setDefaultSectionSize(14);
    setShowGrid(false);
    this->setSelectionMode(QAbstractItemView::SingleSelection);
    this->setModel( set );
    this->resizeColumnsToContents();
    // Register in the static variables
    activeView = this;
    allViews.append(this);
}

// Modify mouse press handler to have only one item selected at every time
void ComponentView::mousePressEvent(QMouseEvent *event){
    // De-select everything in all views
    activeView = this;
    for(int i=0; i<allViews.size(); i++) allViews[i]->clearSelection();
    // Setup data that will registered in the drag
    for(int i=0; i<allViews.size(); i++)
        if( this == allViews.at(i) )
            dragType = i;
    // There are 2 views for each list. One extended and one minimal
    if( dragType>3 ) dragType -= 4;
    dragIdx = rowAt(event->pos().y());
    // Emit signal of new selection to provide hints
    emit componentSelected(dragType,dragIdx);
    // Record place of press in case of dragging
    if (event->button() == Qt::LeftButton)
        startDragPos = event->pos();
    QTableView::mousePressEvent(event);
}

// Modify mouse move handler to avoid dragging due to small movements
void ComponentView::mouseMoveEvent(QMouseEvent *event){
    if (event->buttons() & Qt::LeftButton) {
            int distance = (event->pos() - startDragPos).manhattanLength();
            if (distance >= 5) startDrag();
        }
    QTableView::mouseMoveEvent(event);
}

void ComponentView::startDrag(){
    QMimeData *mimeData = new QMimeData;
    mimeData->setText(QString("%0 %1").arg(dragType).arg(dragIdx));
    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->start();
    repaint();
}

// Initialization of static variables
ComponentView* ComponentView::activeView = 0;
QVector<ComponentView*> ComponentView::allViews = QVector<ComponentView*>();

*/
