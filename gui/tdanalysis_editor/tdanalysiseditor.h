/*!
\file tdanalysiseditor.h

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef TDANALYSISEDITOR_H
#define TDANALYSISEDITOR_H

#include <QSplitter>
class QToolBar;
class QComboBox;

namespace elabtsaot{

class PwsSchematicModel;
class ScenarioSet;
class TDEngine;
class TDEmulator;
class Simulator_sw;
class TDResultsBank;
class GTDAEditor;

class TDAnalysisEditor : public QSplitter{

  Q_OBJECT

 public:

  TDAnalysisEditor( PwsSchematicModel const* smd,
                    ScenarioSet const* scs,
                    TDEngine*& tde,
                    TDEmulator* tde_hwe,
                    Simulator_sw* tde_swe,
                    TDResultsBank* trb,
                    QWidget* parent = 0 );
  int init();

 public slots:

  void TDEngineSelectionSlot(int);
  void setTDEngineTimeStepSlot();
  void setOptionsSlot();

 private:

  TDEngine*& _tde;
  TDEmulator* _tde_hwe;
  Simulator_sw* _tde_swe;

  GTDAEditor* gtdaEditor;
  QComboBox* tdEngineSelectBox;

};

} // end of namespace elabtsaot

#endif // TDANALYSISEDITOR_H
