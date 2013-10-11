/*!
\file resultsrenamedialog.h

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef RESULTSRENAMEDIALOG_H
#define RESULTSRENAMEDIALOG_H

#include <QObject>

#include <string>

namespace elabtsaot{

class ResultsRenameDialog : QObject {

  Q_OBJECT

 public:

  ResultsRenameDialog( std::string& newTitle, QWidget* parent = 0 );
  int exec();

 private:

  std::string& _newTitle;

};

} // end of namespace elabtsaot

#endif // RESULTSRENAMEDIALOG_H
