/*!
\file addplotdialog.h

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef ADDPLOTDIALOG_H
#define ADDPLOTDIALOG_H

#include <QObject>
class QComboBox;
class QLineEdit;

namespace elabtsaot{

class TDResultsPlotProperties;

class AddPlotDialog : public QObject {

  Q_OBJECT

 public:

  AddPlotDialog( QString& name, TDResultsPlotProperties& plotProperties,
                 bool editable, QWidget* parent = 0 );
  int exec();

 public slots:

  void variableChangedSlot(int);

 private:

  QString& _name;
  TDResultsPlotProperties& _plotProperties;
  bool _editable;

  QComboBox* elTypeCombo;
  QComboBox* variableCombo;
  QLineEdit* descVal;
  QLineEdit* unitsVal;

};

} // end of namespace elabtsaot

#endif // ADDPLOTDIALOG_H
