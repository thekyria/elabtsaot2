/*!
\file fitteremutab.h
\brief Definition file for class FitterEmuTab

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef FITTEREMUTAB_H
#define FITTEREMUTAB_H

#include <QSplitter>
class QGroupBox;
class QDoubleSpinBox;

namespace elabtsaot{

class Emulator;

class FitterEmuTab : public QSplitter {

  Q_OBJECT

 public:

  FitterEmuTab( Emulator* emu, QWidget* parent = 0 );
  void updt();

 public slots:

  void ratioZSlot( double val );
  void ratioVSlot( double val );

 private:

  void _initGlobalParamsBox();

  Emulator* _emu;

  // Global emulator parameters pane
  QGroupBox* globalParamsBox;

  QDoubleSpinBox* ratioZForm;
  QDoubleSpinBox* ratioVForm;
  QDoubleSpinBox* ratioIForm;

};

} // end of namespace elabtsaot

#endif // FITTEREMUTAB_H
