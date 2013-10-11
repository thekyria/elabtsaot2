/*!
\file resultsrequestdialog.h

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef RESULTSREQUESTDIALOG_H
#define RESULTSREQUESTDIALOG_H

#include <QObject>
class QLineEdit;

#include <vector>

namespace elabtsaot{

class TDResults;

class ResultsRequestDialog : QObject {

  Q_OBJECT

 public:

  ResultsRequestDialog( std::vector<TDResults*> res, QWidget* parent = 0 );
  int exec();

 public slots:

  void allResultsSlot(int);
  void allGenAnglesResultsSlot(int);
  void allGenVoltagesResultsSlot(int);
  void clearAllResultsSlot(int);
  void resultsDetailsSlot(int);

 private:

  std::vector<TDResults*> _res;

  std::vector<QLineEdit*> resultTitle;
  std::vector<QLineEdit*> resultDesc;

};

} // end of namespace elabtsaot

#endif // RESULTSREQUESTDIALOG_H
