
#include "resultsdbbrowser.h"
using namespace elabtsaot;

//#include <string>
using std::string;

ResultsDBBrowser::ResultsDBBrowser( TDResultsBank* trb, QWidget* parent ) :
    QTreeWidget(parent), _trb(trb){

  setColumnCount( 5 );
  QStringList headerlist;
  headerlist << "DB element" << "SHA1 digest" << "Units" << "Ext Id" << "Data count";
  setHeaderLabels( headerlist );

  QStringList rootlist;
  rootlist << "Results DB";
  _rootItem = new QTreeWidgetItem(rootlist);
  _rootItem->setIcon( 0, QIcon(":/images/lightning.png") );
  addTopLevelItem(_rootItem);
  expandAll();
}

#define TDRESULTS_ROLE 0
#define TDRESULTSDATA_ROLE 0

void ResultsDBBrowser::updt(){

  _rootItem->takeChildren();
  blockSignals(true);

  TDResultsBankConstIterator it;
  for ( it = _trb->getResultsDBBegin() ; it != _trb->getResultsDBEnd() ; ++it ){

    // Retrieve results at current position of the iterator
    TDResults res = it->second;

    QTreeWidgetItem* resItem = new QTreeWidgetItem(_rootItem);
    _rootItem->addChild(resItem);

    resItem->setData(0,TDRESULTS_ROLE, QVariant( QString::fromStdString(res.title()) ));
    resItem->setData(1,TDRESULTS_ROLE, QVariant( QString::fromStdString(res.sha1Digest()) ));

    for ( size_t m = 0 ; m != res.dataExists().size() ; ++m ){
      if ( !res.dataExists().at(m) )
        continue;
      if ( res.identifiers().at(m).elementType == TDRESULTS_OTHER
           && res.identifiers().at(m).variable == TDRESULTS_TIME )
        continue;
      QTreeWidgetItem* resDataItem = new QTreeWidgetItem( resItem );
      resItem->addChild( resDataItem );

      resDataItem->setData(0,TDRESULTSDATA_ROLE, QVariant( QString::fromStdString(res.names().at(m)) ));
      resDataItem->setData(2,TDRESULTSDATA_ROLE, QVariant( QString::fromStdString(res.units().at(m)) ));
      resDataItem->setData(3,TDRESULTSDATA_ROLE, QVariant::fromValue( res.identifiers().at(m).extId  ));
      resDataItem->setData(4,TDRESULTSDATA_ROLE, QVariant::fromValue( res.data().at(m).size() ));
    }
  }

  blockSignals(false);
  return;
}

int ResultsDBBrowser::getSelectedResults( TDResultsBankConstIterator* it,
                                          int* pos ) const{

  QList<QTreeWidgetItem*> selection = selectedItems ();
  if ( selection.count() != 1 )
    return 1;

  QTreeWidgetItem* item = selection.first();
  if ( item == _rootItem )
    // Item is the tree root
    return 2;
  if ( item->parent() == _rootItem )
    // Selected item is a TDResults object
    return 3;

  string resTitle = item->parent()->data(0,TDRESULTS_ROLE).toString().toStdString();
  TDResultsBankConstIterator tempIt = _trb->findInResultsDB( resTitle );
  if ( tempIt == _trb->getResultsDBEnd() )
    // Cannot find TDResults in the TDResultsDB
    return 4;

  // TODO perhaps counting too much on the name in the following!
  string resDataName = item->data(0, TDRESULTSDATA_ROLE).toString().toStdString();
  unsigned int resDataExtId = item->data(3, TDRESULTSDATA_ROLE).toUInt();
  TDResults res = tempIt->second;
  size_t k;
  bool found = false; // TODO redundant
  for ( k = 0 ; k != res.dataExists().size() ; ++k ){
    if ( !resDataName.compare(res.names().at(k)) ){
      // names match
      if ( resDataExtId == res.identifiers().at(k).extId ){
        // extIds match
        if ( res.dataExists().at(k) )
          found = true;
        break;
      }
    }
  }
  if ( !found )
    // Result data was not found in TDResults
    return 5;

  *pos = k;
  *it = tempIt;

  return 0;
}
