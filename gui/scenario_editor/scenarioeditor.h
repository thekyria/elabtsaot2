/*!
\file scenarioeditor.h

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef SCENARIOEDITOR_H
#define SCENARIOEDITOR_H

#include <QSplitter>
class QTreeWidgetItem;
class QTreeWidget;
class QComboBox;

namespace elabtsaot{

class ScenarioSet;
class Scenario;
class Event;
class Powersystem;

class ScenarioEditor : public QSplitter{

  Q_OBJECT

 public:

  ScenarioEditor( ScenarioSet* scs, Powersystem const* pws,
                  QWidget *parent = 0);
  void updt();

 public slots:

  void addScenarioSlot();
  void addEventSlot();
  void editSelectedSlot();
  void deleteSelectedSlot();
  void deleteAllSlot();

  void autoGenerateScenariosSlot();

  void checkPwsCompatibilitySlot();
  void checkCCTCompatibilitySlot();
  void checkTSACompatibilitySlot();
  void sortScenarioEventsSlot();

  void itemChanged(QTreeWidgetItem* item, int column);

 private:

  int _scenarioDialog( Scenario* sce );
  int _eventDialog( Event* evn, int* sceId = NULL );

  void _addScenarioItem( Scenario const& sce );
  void _addEventItem( Event const& evn, int sceId );

  ScenarioSet* _scs;
  Powersystem const* _pws;

  QTreeWidget* scenariosTree;
  QTreeWidgetItem* _rootItem;

};

} // end of namespace elabtsaot

#endif
