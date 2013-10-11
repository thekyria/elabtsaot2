/*!
\file resultsrequestdetailsdialog.h

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef RESULTSREQUESTDETAILSDIALOG_H
#define RESULTSREQUESTDETAILSDIALOG_H

#include <QObject>
class QTableWidget;

namespace elabtsaot{

class TDResults;

class ResultsRequestDetailsDialog : public QObject {

  Q_OBJECT

 public:

  ResultsRequestDetailsDialog( TDResults* res,
                               bool request = false,
                               QWidget* parent = 0 );
  int exec();

 private slots:

  void selectAll();
  void deselectAll();

 private:

  TDResults* _res;
  bool _request;

  QTableWidget* dataTable;

};

} // end of namespace elabtsaot

#endif // RESULTSREQUESTDETAILSDIALOG_H
