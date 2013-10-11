
#include "resultsrequestdialog.h"
using namespace elabtsaot;

#include "tdresults.h"

#include "tdanalysiseditordialogs.h"
#include "resultsrequestdetailsdialog.h"

#include <QLabel>
#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QSignalMapper>
#include <QTableWidget>
#include <QHeaderView>

//#include <vector>
using std::vector;

ResultsRequestDialog::ResultsRequestDialog( vector<TDResults*> res,
                                            QWidget* parent ) :
    QObject(parent), _res(res){}

int ResultsRequestDialog::exec(){

  QDialog dialog;
  dialog.setWindowTitle("Results request");
  QVBoxLayout layoutMain;
  dialog.setLayout(&layoutMain);

  // Main layout widgets
  QHBoxLayout* headerLay = new QHBoxLayout();
  layoutMain.addLayout( headerLay );
  QLabel* titleLabel = new QLabel("Title");
  headerLay->addWidget( titleLabel );
  titleLabel->setFixedWidth( 150 );
  QLabel* descLabel = new QLabel("Description");
  headerLay->addWidget( descLabel );
  descLabel->setFixedWidth( 150 );
  QLabel* place1Label = new QLabel(" ");
  headerLay->addWidget( place1Label );
  place1Label->setMinimumWidth( 150 );

  // Signal mapper for 'all results' operation
  QSignalMapper* signalMapperAll = new QSignalMapper( &dialog );
  connect( signalMapperAll, SIGNAL(mapped(int)),
           this, SLOT(allResultsSlot(int)) );
  // Signal mapper for 'all gen angles' operation
  QSignalMapper* signalMapperAllGenAngles = new QSignalMapper( &dialog );
  connect( signalMapperAllGenAngles, SIGNAL(mapped(int)),
           this, SLOT(allGenAnglesResultsSlot(int)) );
  // Signal mapper for 'all gen voltages' operation
  QSignalMapper* signalMappperAllGenVoltages = new QSignalMapper( &dialog );
  connect( signalMappperAllGenVoltages, SIGNAL(mapped(int)),
           this, SLOT(allGenVoltagesResultsSlot(int)) );
  // Signal mapper for 'clear all results' operation
  QSignalMapper* signalMapperClearAll = new QSignalMapper( &dialog );
  connect( signalMapperClearAll, SIGNAL(mapped(int)),
           this, SLOT(clearAllResultsSlot(int)) );
  // Signal mapper for 'results details' operation
  QSignalMapper* signalMapperDetails = new QSignalMapper( &dialog );
  connect( signalMapperDetails, SIGNAL(mapped(int)),
           this, SLOT(resultsDetailsSlot(int)) );
  vector<QPushButton*> allResultsBut;
  vector<QPushButton*> allGenAnglesResultsBut;
  vector<QPushButton*> allGenVoltagesResultsBut;
  vector<QPushButton*> clearAllResultsBut;
  vector<QPushButton*> resultDetails;
  for ( size_t k = 0 ; k != _res.size() ; ++k ){

    QHBoxLayout* resultLay = new QHBoxLayout();
    layoutMain.addLayout( resultLay );

    resultTitle.push_back( new QLineEdit(QString::fromStdString(_res[k]->title())) );
    resultLay->addWidget( resultTitle[k] );
    resultTitle[k]->setFixedWidth( 200 );

    resultDesc.push_back( new QLineEdit(QString::fromStdString( _res[k]->description())) );
    resultLay->addWidget( resultDesc[k] );
    resultDesc[k]->setFixedWidth( 200 );

    // All results button
    allResultsBut.push_back( new QPushButton("All") );
    resultLay->addWidget( allResultsBut[k] );
    allResultsBut[k]->setFixedWidth( 80 );
    signalMapperAll->setMapping( allResultsBut[k], k );
    connect( allResultsBut[k], SIGNAL(clicked()),
             signalMapperAll, SLOT(map()) );

    // All generator angles button
    allGenAnglesResultsBut.push_back( new QPushButton("All gen angles") );
    resultLay->addWidget( allGenAnglesResultsBut[k] );
    allGenAnglesResultsBut[k]->setFixedWidth( 80 );
    signalMapperAllGenAngles->setMapping( allGenAnglesResultsBut[k], k );
    connect( allGenAnglesResultsBut[k], SIGNAL(clicked()),
             signalMapperAllGenAngles, SLOT(map()) );

    // All generator voltages button
    allGenVoltagesResultsBut.push_back( new QPushButton("All gen voltages") );
    resultLay->addWidget( allGenVoltagesResultsBut[k] );
    allGenVoltagesResultsBut[k]->setFixedWidth( 80 );
    signalMappperAllGenVoltages->setMapping( allGenVoltagesResultsBut[k], k );
    connect( allGenVoltagesResultsBut[k], SIGNAL(clicked()),
             signalMappperAllGenVoltages, SLOT(map()) );

    // Clear all results button
    clearAllResultsBut.push_back( new QPushButton("Clear all") );
    resultLay->addWidget( clearAllResultsBut[k] );
    clearAllResultsBut[k]->setFixedWidth( 80 );
    signalMapperClearAll->setMapping( clearAllResultsBut[k], k );
    connect( clearAllResultsBut[k], SIGNAL(clicked()),
             signalMapperClearAll, SLOT(map()) );

    // Details button
    resultDetails.push_back( new QPushButton("Details ...") );
    resultLay->addWidget( resultDetails[k] );
    resultDetails[k]->setFixedWidth( 80 );
    signalMapperDetails->setMapping( resultDetails[k], k );
    connect( resultDetails[k], SIGNAL(clicked()),
             signalMapperDetails, SLOT(map()) );
  }

  // Buttons
  QHBoxLayout layoutButtons;
  layoutMain.addLayout( &layoutButtons );
  QPushButton ok("Ok");
  layoutButtons.addWidget( &ok );
  dialog.connect(&ok, SIGNAL(clicked()), &dialog, SLOT(accept()));
  QPushButton cancel("Cancel");
  layoutButtons.addWidget( &cancel );
  dialog.connect(&cancel, SIGNAL(clicked()), &dialog, SLOT(reject()));

  if( dialog.exec() ){
    // Dialog executed properly
    for ( size_t k = 0 ; k != _res.size() ; ++k ){
      _res[k]->set_title( resultTitle[k]->text().toStdString() );
      _res[k]->set_description( resultDesc[k]->text().toStdString() );
    }

    return 0;

  } else {
    // Dialog cancelled or not executed correctly
    return 1;
  }

  return 0;
}

void ResultsRequestDialog::allResultsSlot(int resultsIndex){

  TDResults* res = _res[resultsIndex];
  for ( size_t k = 0 ; k != res->dataExists().size() ; ++k )
    res->set_dataExists(k, true);

  return;
}

void ResultsRequestDialog::allGenAnglesResultsSlot(int resultsIndex){

  TDResults* res = _res[resultsIndex];
  for ( size_t k = 1 ; k != res->dataExists().size() ; ++k ){
    if (    res->identifiers().at(k).elementType == TDRESULTS_GENERATOR
         && res->identifiers().at(k).variable == TDRESULTS_ANGLE )
      res->set_dataExists(k, true);
    else
      res->set_dataExists(k, false);
  }

  return;
}

void ResultsRequestDialog::allGenVoltagesResultsSlot(int resultsIndex){

  TDResults* res = _res[resultsIndex];
  for ( size_t k = 1 ; k != res->dataExists().size() ; ++k ){
    if (    res->identifiers().at(k).elementType == TDRESULTS_GENERATOR
        && (    res->identifiers().at(k).variable == TDRESULTS_VOLTAGE_REAL
             || res->identifiers().at(k).variable == TDRESULTS_VOLTAGE_REAL ) )
      res->set_dataExists(k, true);
    else
      res->set_dataExists(k, false);
  }
  return;
}

void ResultsRequestDialog::clearAllResultsSlot(int resultsIndex){

  TDResults* res = _res[resultsIndex];
  for ( size_t k = 1 ; k < res->dataExists().size() ; ++k )
    res->set_dataExists(k, false);

  return;
}

void ResultsRequestDialog::resultsDetailsSlot(int resultsIndex){

  TDResults* res = _res[resultsIndex];
  ResultsRequestDetailsDialog dialog(res, true);
  dialog.exec();
  resultDesc[resultsIndex]->setText( QString::fromStdString(res->description()) );

  return;
}
