
#include "resultsbankeditor.h"
using namespace elabtsaot;

#include "tdresultsbank.h"
#include "importerexporter.h"

#include "guiauxiliary.h"
#include "tdanalysiseditordialogs.h"
#include "resultsrequestdetailsdialog.h"
#include "resultsrenamedialog.h"

#include <QToolBar>
#include <QAction>
#include <QIcon>
#include <QString>
#include <QTableWidget>
#include <QLabel>
#include <QHeaderView>

#include <iostream>
using std::cout;
using std::endl;
//#include <string>
using std::string;

ResultsBankEditor::ResultsBankEditor( TDResultsBank* trb, QWidget* parent ) :
    QSplitter( Qt::Vertical, parent ), _trb(trb){

  // Label
  QLabel mainLabel("Results bank");
  addWidget( &mainLabel );

  // ------------ Results bank table ------------
  _resultsDBTable = new QTableWidget(this);
  addWidget( _resultsDBTable );
  _resultsDBTable->setColumnCount( 4 );
  _resultsDBTable->setColumnWidth(0,200);
  _resultsDBTable->setColumnWidth(1,350);
  _resultsDBTable->setColumnWidth(2,80);
  _resultsDBTable->setColumnWidth(3,350);
  QList<QString> labelList;
  labelList.push_back( QString("Title") );
  labelList.push_back( QString("Description") );
  labelList.push_back( QString("SHA1 digest") );
  labelList.push_back( QString("Footer") );
  QStringList* labels = new QStringList( labelList );
  _resultsDBTable->setHorizontalHeaderLabels( *labels );
  _resultsDBTable->verticalHeader()->hide();
  _resultsDBTable->setSelectionMode( QAbstractItemView::SingleSelection );
  _resultsDBTable->setEditTriggers( QAbstractItemView::NoEditTriggers );
  _resultsDBTable->setSelectionBehavior( QAbstractItemView::SelectRows );

  updt();

  // ------------ Toolbar ------------
  QToolBar* resultsBankEditorToolbar = new QToolBar( this );
  addWidget( resultsBankEditorToolbar );
  resultsBankEditorToolbar->setFixedHeight(30);

  QAction* resetResultsDBAct = new QAction( QIcon(":/images/DBreset.png"), "Reset resultsDB",
                                     resultsBankEditorToolbar );
  resultsBankEditorToolbar->addAction( resetResultsDBAct );
  connect( resetResultsDBAct, SIGNAL(triggered()),
           this, SLOT(resetResultsDBSlot()) );

  QAction* renameResultsAct = new QAction ( QIcon(), "Rename results",
                                            resultsBankEditorToolbar );
  resultsBankEditorToolbar->addAction( renameResultsAct );
  connect( renameResultsAct, SIGNAL(triggered()),
           this, SLOT(renameResultsSlot()) );

  QAction* deleteSelectedResultsAct = new QAction( QIcon(":/images/DBdelete.png"),"Delete selected results",
                                           resultsBankEditorToolbar );
  resultsBankEditorToolbar->addAction( deleteSelectedResultsAct );
  connect( deleteSelectedResultsAct, SIGNAL(triggered()),
           this, SLOT(deleteSelectedResultsSlot()) );

  QAction* showResultsDetailsAct = new QAction( QIcon(":/images/DBshow.png"), "Show TDResults details",
                                                resultsBankEditorToolbar );
  resultsBankEditorToolbar->addAction( showResultsDetailsAct );
  connect( showResultsDetailsAct, SIGNAL(triggered()),
           this, SLOT(showResultsDetailsSlot()) );

  resultsBankEditorToolbar->addSeparator(); // ----- seperator

  QAction* importResultsAct = new QAction( QIcon(":/images/DBimport.png"), "Import results",
                                           resultsBankEditorToolbar );
  resultsBankEditorToolbar->addAction( importResultsAct );
  connect( importResultsAct, SIGNAL(triggered()),
           this, SLOT(importResultsSlot()) );

  QAction* exportResultsAct = new QAction( QIcon(":/images/DBexport.png"), "Export results",
                                           resultsBankEditorToolbar );
  resultsBankEditorToolbar->addAction( exportResultsAct );
  connect( exportResultsAct, SIGNAL(triggered()),
           this, SLOT(exportResultsSlot()) );

}

void ResultsBankEditor::updt(){

  int k = 0;
  TDResultsBankConstIterator it;
  _resultsDBTable->setRowCount( _trb->getResultsDBCount() );
  for ( it = _trb->getResultsDBBegin() ; it != _trb->getResultsDBEnd() ; ++it ){

    // Retrieve results at current position of the iterator
    TDResults res = it->second;

    // Results title
    QTableWidgetItem* titleItem;
    titleItem = new QTableWidgetItem( QString::fromStdString(res.title()) );
//    titleItem->setFlags( Qt::ItemIsSelectable );
    _resultsDBTable->setItem( k, 0, titleItem );

    // Results description
    QTableWidgetItem* descItem;
    descItem = new QTableWidgetItem( QString::fromStdString(res.description()) );
//    descItem->setFlags( Qt::ItemIsSelectable );
    _resultsDBTable->setItem( k, 1, descItem );

    // Results SHA1 digest
    QTableWidgetItem* sha1Item;
    sha1Item = new QTableWidgetItem( QString::fromStdString(res.sha1Digest()) );
//    sha1Item->setFlags( Qt::ItemIsSelectable );
    sha1Item->setTextAlignment( Qt::AlignCenter );
    _resultsDBTable->setItem( k, 2, sha1Item );

    // Results footer
    QTableWidgetItem* footerItem;
    footerItem = new QTableWidgetItem( QString::fromStdString(res.footer()) );
    footerItem->setTextAlignment( Qt::AlignCenter );
    _resultsDBTable->setItem( k, 3, footerItem );

    // Increase row counter
    ++k;
  }
//  _resultsDBTable->resizeColumnsToContents();

  return;
}

void ResultsBankEditor::resetResultsDBSlot(){
  _trb->reset();
  updt();
  cout << "ResultsDB reset successfully! " << endl;
  return;
}

void ResultsBankEditor::renameResultsSlot(){
  int row = _resultsDBTable->currentRow();
  if ( row < 0 )
    return;

  QTableWidgetItem* titleItem = _resultsDBTable->item( _resultsDBTable->currentRow(), 0 );
  string oldTitle = titleItem->text().toStdString();
  string newTitle(oldTitle);

  ResultsRenameDialog dialog( newTitle, this );
  int ans = dialog.exec();
  if ( ans )
    return;

  ans = _trb->renameResults( oldTitle, newTitle );
  if ( ans )
    cout << "Error renaming results " << oldTitle << " to " << newTitle << endl;
  else
    cout << "Successfully renamed results " << oldTitle << " to " << newTitle << endl;

  updt();
  return;
}

void ResultsBankEditor::deleteSelectedResultsSlot(){

  int row = _resultsDBTable->currentRow();
  if ( row < 0 )
    return;

  QTableWidgetItem* titleItem = _resultsDBTable->item( _resultsDBTable->currentRow(), 0 );
  string title = titleItem->text().toStdString();
  int ans = _trb->deleteResults( title );
  if ( !ans )
    cout << "TDResults in resultsDB deleted succesfully!" << endl;
  else
    cout << "Deleting TDResults failed with code: " << ans << endl;

  updt();

  return;
}

void ResultsBankEditor::showResultsDetailsSlot(){

  int row = _resultsDBTable->currentRow();
  if ( row < 0 )
    return;

  QTableWidgetItem* titleItem = _resultsDBTable->item( _resultsDBTable->currentRow(), 0 );
  string title = titleItem->text().toStdString();
  TDResults res;
  int ans = _trb->getResults( title, &res );
  if ( ans ){
    cout << "Retrieving TDResults with title " << title;
    cout << " failed with code: " << ans << endl;
    cout << "Aborting ... " << endl;
    return;
  }

  ResultsRequestDetailsDialog dialog( &res, false );
  ans = dialog.exec();
  if ( !ans ){
    _trb->updateResults( title, res );
    updt();
  }

  return;
}

void ResultsBankEditor::importResultsSlot(){

  QString filename = guiauxiliary::askFileName("res", true);
  if( filename.isEmpty() )
    return;

  TDResults res;
  int ans = io::importTDResults( filename.toStdString(), &res );
  if ( ans ){
    cout << "Error importing results! Exit code: " << ans << endl;
    return;
  }
  ans = _trb->storeResults( res );
  if ( ans ){
    cout << "Error storing imported results in the resultsDB! Exit code: " << ans << endl;
    return;
  }

  cout << "Results successfully added to the results bank." << endl;
  updt();

  return;
}

void ResultsBankEditor::exportResultsSlot(){

  int row = _resultsDBTable->currentRow();
  if ( row < 0 )
    return;

  QString filename = guiauxiliary::askFileName("res", false);
  if ( filename.isNull() )
    return;

  QTableWidgetItem*
      titleItem = _resultsDBTable->item( _resultsDBTable->currentRow(), 0 );
  string title = titleItem->text().toStdString();
  TDResults res;
  int ans = _trb->getResults( title, &res );
  if ( ans ){
    cout << "Retrieving TDResults with title " << title;
    cout << " failed with code: " << ans << endl;
    cout << "Aborting ... " << endl;
    return;
  }

  ans = io::exportTDResults( filename.toStdString(), &res );
  if ( ans )
    cout << "Error exporting results! Exit code: " << ans << endl;
  else
    cout << "Results successfully exported to file "
         << filename.toStdString() << endl;

  return;
}
