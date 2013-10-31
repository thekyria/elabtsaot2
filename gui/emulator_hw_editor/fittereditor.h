/*!
\file fittereditor.h
\brief Definition file for class FitterEditor

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Fragkiskos Koufogiannis, fkoufis at gmail dot com
\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef FITTEREDITOR_H
#define FITTEREDITOR_H

#include <QSplitter>
class QToolBar;
class QAction;

namespace elabtsaot{

class FitterEmulatorEditor;

class Emulator;
class Powersystem;
class EmulatorHw;
class PwsMapperModel;

class FitterEditor : public QSplitter{

  Q_OBJECT

 public:

  FitterEditor( Emulator* emu, QWidget* parent);
  int init();
  static int askDouble( double* v, QString const& text );

 public slots:

  void updt();
  void resetFittingSlot();
  void autoFittingPFSlot();
  void autoFittingTDSlot();
  void validateFittingPFSlot();
  void validateFittingTDSlot();
  void fittingPositionClickedSlot(int emu_tab, int emu_row, int emu_col, int emu_elm );

 private:

  Emulator* _emu;
  EmulatorHw* _emuhw;

  FitterEmulatorEditor* fee;

};

} // end of namespace elabtsaot

#endif // FITTEREDITOR_H
