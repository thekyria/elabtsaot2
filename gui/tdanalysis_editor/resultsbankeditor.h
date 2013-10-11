/*!
\file resultsbankeditor.h

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef RESULTSBANKEDITOR_H
#define RESULTSBANKEDITOR_H

#include <QSplitter>
class QTableWidget;

namespace elabtsaot{

class TDResultsBank;

class ResultsBankEditor : public QSplitter {

  Q_OBJECT

 public:

  ResultsBankEditor( TDResultsBank* trb, QWidget* parent = 0);

 public slots:

  void updt();
  void resetResultsDBSlot();
  void renameResultsSlot();
  void deleteSelectedResultsSlot();
  void showResultsDetailsSlot();
  // -----
  void importResultsSlot();
  void exportResultsSlot();

 private:

  TDResultsBank* _trb;

  QTableWidget* _resultsDBTable;

};

} // end of namespace elabtsaot

#endif // RESULTSBANKEDITOR_H
