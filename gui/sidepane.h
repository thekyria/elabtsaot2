/*!
\file sidepane.h
\brief Definition file for class SidePane

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Fragkiskos Koufogiannis, fkoufis at gmail dot com
\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef SIDEPANE_H
#define SIDEPANE_H

#include <QSplitter>
class QPushButton;

namespace elabtsaot{

class SidePane : public QSplitter{

  Q_OBJECT

 public:

  explicit SidePane(QWidget* parent = 0);

 public slots:

  void showHwSlot();

 signals:

  void buttonSplash_clicked();
  void buttonPowersystem_clicked();
  void buttonSSAnalysis_clicked();
  void buttonScenarios_clicked();
  void buttonTDAnalysis_clicked();

  void buttonCommunication_clicked();
  void buttonCalibration_clicked();
  void buttonMapper_clicked();
  void buttonFitter_clicked();
  void buttonAuxiliary_clicked();

 private:

  bool _isHwShown;

  QPushButton* buttonSplash;
  QPushButton* buttonPowersystem;
  QPushButton* buttonSSAnalysis;
  QPushButton* buttonScenarios;
  QPushButton* buttonTDAnalysis;

  QPushButton* buttonShowHw;
  QPushButton* buttonCommunication;
  QPushButton* buttonCalibration;
  QPushButton* buttonMapper;
  QPushButton* buttonFitter;
  QPushButton* buttonAuxiliary;

};

} // end of namespace elabtsaot

#endif // SIDEPANE_H
