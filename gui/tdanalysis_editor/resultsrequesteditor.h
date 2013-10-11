/*!
\file resultsrequestedior.h

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef RESULTSREQUESTEDITOR_H
#define RESULTSREQUESTEDITOR_H

#include <QSplitter>
class QTreeWidget;
class QTreeWidgetItem;

namespace elabtsaot{

class Scenario;
class ScenarioSet;
class TDEngine;
class TDResultsBank;

class ResultsRequestEditor : public QSplitter {

  Q_OBJECT

 public:

  ResultsRequestEditor( ScenarioSet const* scs,
                        TDEngine* const& tde,
                        TDResultsBank* trb,
                        QWidget* parent = 0 );

 public slots:

  void updateScenariosSlot();
  void TDRunSlot();
  void checkStabilitySlot();
  void CCTTestSlot();
  void TSATestSlot();

 signals:

  void resultsAdded();

 private:

  void _addScenarioItem( Scenario const& sce );
  int _cctDialog( double& precision, double& maxFaultTime );

  ScenarioSet const* _scs;
  TDEngine* const& _tde;
  TDResultsBank* _trb;

  QTreeWidget* scenariosTree;
  QTreeWidgetItem* _rootItem;

};

} // end of namespace elabtsaot

#endif // RESULTSREQUESTEDITOR_H
