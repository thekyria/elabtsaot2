/*!
\file fitterslicetab.h
\brief Definition file for class FitterSliceTab

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Fragkiskos Koufogiannis, fkoufis at gmail dot com
\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef FITTERSLICETAB_H
#define FITTERSLICETAB_H

#include <QSplitter>
#include <QVector>
class QGroupBox;
class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QDoubleSpinBox;
class QSpinBox;
class QComboBox;
class QTabWidget;
class QFrame;
class QFormLayout;
class QTableWidget;

namespace elabtsaot{

class FitterSliceWidget;
struct Slice;

class FitterSliceTab : public QSplitter {

  Q_OBJECT

 public:

  FitterSliceTab( Slice* slc, QWidget* parent = 0 );
  void updt();
  bool isShowingReal() const;

 public slots:

  // Global slice parameters slots
  void gotGainSlot(double val);
  void gotOffsetSlot(double val);
  void realVrefValSlot(double val);
  void realVrefTapSlot(int tap);
  void imagVrefValSlot(double val);
  void imagVrefTapSlot(int tap);

  // Pipelines slots
  void genPipeSlot(int row, int col);
  void zloadPipeSlot(int row, int col);
  void iloadPipeSlot(int row, int col);
  void ploadPipeSlot(int row, int col);

  // Core fitter slice widget slots
  void toggleReal();
  void toggleReal(bool isShowingReal);
  void zoomIn();
  void zoomOut();
  void zoomFit();

 signals:

  void fittingPositionClicked( int emulator_tab,
                               int emulator_row,
                               int emulator_col,
                               int emulator_elm );

 private:

  void _init_globalParamsBox();
  void _init_pipeBox();

  Slice* _slc;
  FitterSliceWidget* _sliceCore;

  // Global slice parameters pane
  QGroupBox* globalParamsBox;
  QDoubleSpinBox* gotGainForm;              // GOT gain
  QDoubleSpinBox* gotOffsetForm;            // GOT offset
  QDoubleSpinBox* realVrefValForm;          // real V ref val
  QSpinBox* realVrefTapForm;                // real V ref tap
  QDoubleSpinBox* imagVrefValForm;          // imag V ref
  QSpinBox* imagVrefTapForm;                // imag V ref tap

  // Pipelines pane
  QGroupBox* pipeBox;
  QTableWidget* genPipeTable;
  QTableWidget* zloadPipeTable;
  QTableWidget* iloadPipeTable;
  QTableWidget* ploadPipeTable;

};

} // end of namespace elabtsaot

#endif // FITTERSLICETAB_H
