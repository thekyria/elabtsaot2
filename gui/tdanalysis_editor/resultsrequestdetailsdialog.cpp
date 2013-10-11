
#include "resultsrequestdetailsdialog.h"
using namespace elabtsaot;

#include "tdresults.h"

#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QGroupBox>
#include <QTextEdit>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>

ResultsRequestDetailsDialog::ResultsRequestDetailsDialog( TDResults* res,
                                                          bool request,
                                                          QWidget* parent ) :
    QObject( parent ), _res(res), _request(request){}

int ResultsRequestDetailsDialog::exec(){

  QDialog* dialog = new QDialog();
  dialog->setWindowTitle( "TD results details" );
  QVBoxLayout* layoutMain = new QVBoxLayout();
  dialog->setLayout( layoutMain );

  // sha1Digest
  QHBoxLayout* sha1Lay = new QHBoxLayout();
  layoutMain->addLayout( sha1Lay );
  QLabel* sha1Label = new QLabel( "SHA1 Digest", dialog );
  sha1Lay->addWidget( sha1Label );
  QLineEdit* sha1Form = new QLineEdit(QString::fromStdString(_res->sha1Digest()),
                                      dialog);
  sha1Lay->addWidget( sha1Form );
  sha1Form->setEnabled( false );

  // Title
  QHBoxLayout* titleLay = new QHBoxLayout();
  layoutMain->addLayout( titleLay );
  QLabel* titleLabel = new QLabel( "Title", dialog );
  titleLay->addWidget( titleLabel );
  QLineEdit* titleForm = new QLineEdit( QString::fromStdString(_res->title()),
                                        dialog);
  titleLay->addWidget( titleForm );
  titleForm->setEnabled( false );

  // Description
  QGroupBox* descBox = new QGroupBox( "Description", dialog );
  layoutMain->addWidget( descBox );
  QVBoxLayout* descLay = new QVBoxLayout();
  descBox->setLayout( descLay );
  QTextEdit* descForm = new QTextEdit( QString::fromStdString(_res->description()),
                                       dialog );
  descLay->addWidget( descForm );
  descForm->setFixedHeight(80);
//  descForm->setMaximumSize(200, 150);

  // Stored data table
  dataTable = new QTableWidget();
  layoutMain->addWidget( dataTable );
  dataTable->setColumnCount( _request ? 4 : 5 );
  dataTable->setRowCount( _res->dataExists().size() );
  QList<QString> labelList;
  // Column 0
  if ( _request )
    labelList.push_back( QString("Request") );
  else
    labelList.push_back( QString("Exists") );
  // Column 1
  labelList.push_back( QString("Variable name") );
  // Column 2
  labelList.push_back( QString("Units") );
  // Column 3
  labelList.push_back( QString("El. extId") );
  if ( !_request )
    // Column 4
    labelList.push_back( QString("Data count") );
  QStringList* labels = new QStringList( labelList );
  dataTable->setHorizontalHeaderLabels( *labels );
  dataTable->verticalHeader()->hide();
  dataTable->setSelectionMode( QAbstractItemView::SingleSelection );
  dataTable->setEditTriggers( QAbstractItemView::DoubleClicked );
  dataTable->setSelectionBehavior( QAbstractItemView::SelectItems );

  for ( size_t k = 0 ; k != _res->dataExists().size(); ++k ){
    QTableWidgetItem* existsItem;
    existsItem = new QTableWidgetItem();
    existsItem->setCheckState( _res->dataExists().at(k) ? Qt::Checked : Qt::Unchecked);
    if ( !_request )
      existsItem->setFlags( Qt::ItemIsSelectable );
//    else // _request
//    existsItem->setFlags( Qt::ItemIsEditable );
    dataTable->setItem( k, 0, existsItem );

    QTableWidgetItem* nameItem;
    nameItem = new QTableWidgetItem( QString::fromStdString( _res->names().at(k)) );
    nameItem->setFlags( Qt::ItemIsSelectable );
    dataTable->setItem( k, 1, nameItem );

    QTableWidgetItem* unitItem;
    unitItem = new QTableWidgetItem( QString::fromStdString( _res->units().at(k)) );
    unitItem->setFlags( Qt::ItemIsSelectable );
    dataTable->setItem( k, 2, unitItem );

    QTableWidgetItem* extIdItem;
    extIdItem = new QTableWidgetItem( QString::number(_res->identifiers().at(k).extId) );
    extIdItem->setFlags( Qt::ItemIsSelectable );
    dataTable->setItem( k, 3, extIdItem );

    if ( !_request ){
      QTableWidgetItem* dataCountItem;
      dataCountItem = new QTableWidgetItem( QString::number( _res->data().at(k).size()) );
      dataCountItem->setFlags( Qt::ItemIsSelectable );
      dataTable->setItem( k, 4, dataCountItem );
    }
  }
  dataTable->resizeColumnsToContents();

  // (De-)select all
  QHBoxLayout* selectLay = new QHBoxLayout();
  // De-select button layout showing only when in _request mode
  if ( _request )
    layoutMain->addLayout( selectLay );
  QPushButton* selectBut = new QPushButton("Select all");
  selectLay->addWidget( selectBut );
  connect( selectBut, SIGNAL(clicked()), this, SLOT(selectAll()) );
  QPushButton* deselectBut = new QPushButton("Deselect all");
  selectLay->addWidget( deselectBut );
  connect( deselectBut, SIGNAL(clicked()), this, SLOT(deselectAll()) );

  // Description
  QGroupBox* footerBox = new QGroupBox( "Footer", dialog );
  layoutMain->addWidget( footerBox );
  QVBoxLayout* footerLay = new QVBoxLayout();
  footerBox->setLayout( footerLay );
  QTextEdit* footerForm = new QTextEdit( QString::fromStdString(_res->footer()),
                                       dialog );
  footerLay->addWidget( footerForm );
  footerForm->setFixedHeight(80);

  // Buttons
  QHBoxLayout* layoutButtons = new QHBoxLayout();
  layoutMain->addLayout( layoutButtons );
  QPushButton* ok = new QPushButton("Ok");
  layoutButtons->addWidget( ok );
  dialog->connect( ok, SIGNAL(clicked()), dialog, SLOT(accept()));
  QPushButton* cancel = new QPushButton("Cancel");
  layoutButtons->addWidget( cancel );
  dialog->connect( cancel, SIGNAL(clicked()), dialog, SLOT(reject()));

  if( dialog->exec() ){
    // Dialog executed properly
//    _res->set_title();
    _res->set_description( descForm->toPlainText().toStdString() );
    _res->set_footer( footerForm->toPlainText().toStdString() );
    if ( _request )
      for ( size_t k = 0 ; k != _res->dataExists().size() ; ++k )
        _res->set_dataExists(k, (dataTable->item(k,0)->checkState()==Qt::Checked));
    return 0;
  } else {
    // Dialog cancelled or not executed correctly
    return 1;
  }

  return 0;
}

void ResultsRequestDetailsDialog::selectAll(){
  for ( int k = 0 ; k != dataTable->rowCount() ; ++k )
    dataTable->item( k,0 )->setCheckState( Qt::Checked );
  return;
}

void ResultsRequestDetailsDialog::deselectAll(){
  // Note: skips first row; usually time
  for ( int k = 1 ; k != dataTable->rowCount() ; ++k )
    dataTable->item( k,0 )->setCheckState( Qt::Unchecked );
  return;
}
