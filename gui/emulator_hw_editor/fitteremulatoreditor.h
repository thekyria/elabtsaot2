/*!
\file fitteremulatoreditor.h
\brief Definition file for class FitterEmulatorEditor

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
\author Fragkiskos Koufogiannis, fkoufis at gmail dot com
*/

#ifndef FITTEREMULATOREDITOR_H
#define FITTEREMULATOREDITOR_H

#include <QTabWidget>

#include <vector>

namespace elabtsaot{

class EmulatorHw;

class FitterSliceTab;

class FitterEmulatorEditor : public QTabWidget{

  Q_OBJECT

 public:

  FitterEmulatorEditor( EmulatorHw* emuhw, QWidget *parent = 0 );
  int init();

  void updt();
  bool isShowingReal() const;

 public slots:

  void toggleReal();
  void zoomIn();
  void zoomOut();
  void zoomFit();

 signals:

  void fittingPositionClicked( int emulator_tab,
                               int emulator_row,
                               int emulator_col,
                               int emulator_elm );

 private:

  EmulatorHw* _emuhw;

  std::vector<FitterSliceTab*> _sliceTabs;

};

} // end of namespace elabtsaot

#endif // FITTEREMULATOREDITOR_H
