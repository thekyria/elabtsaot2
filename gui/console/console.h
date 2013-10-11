/*!
\file console.h
\brief Definition file for class Console

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
\author Fragkiskos Koufogiannis, fkoufis at gmail dot com
*/

#ifndef CONSOLE_H
#define CONSOLE_H

#include "logger.h"

#include <QSplitter>
#include <QLineEdit>

class QTextEdit;
class QProgressBar;
class QLineEdit;

namespace elabtsaot{

class QDebugStream;

//! Class that implements the lower screen console of the application
class Console : public QSplitter, public Logger {

  Q_OBJECT

 public:

  Console(QWidget *parent = 0); //! Constructor
  ~Console();                   //! Destructor
  void notifyProgress(double val);

 private slots:

  void scrollDown();            //! Autoscrolling of the outBox
  void interpretCommand();

 private:

  QProgressBar* progressBar;
  QTextEdit* outBox;            //! Output widget
  QLineEdit* inBox;             //! Input widget
  QDebugStream* _redirector;    //! Redirection class

};

} // end of namespace elabtsaot

#endif // CONSOLE_H
