/*!
\file resultsdbbrowser.h

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef RESULTSDBBROWSER_H
#define RESULTSDBBROWSER_H

#include <QTreeWidget>

#include "tdresultsbank.h"

namespace elabtsaot{

class ResultsDBBrowser : public QTreeWidget {

  Q_OBJECT

 public:

  ResultsDBBrowser( TDResultsBank* trb, QWidget* parent = 0 );
  void updt();
  int getSelectedResults( TDResultsBankConstIterator* it, int* pos ) const;

 private:

  TDResultsBank* _trb;

  QTreeWidgetItem* _rootItem;

};

} // end of namespace elabtsaot

#endif // RESULTSDBBROWSER_H
