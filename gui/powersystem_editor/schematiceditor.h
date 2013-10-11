/*!
\file schematiceditor.h
\brief Definition file for class SchematicEditor

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Fragkiskos Koufogiannis, fkoufis at gmail dot com
\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef SCHEMATICEDITOR_H
#define SCHEMATICEDITOR_H

#include "schematicviewer.h"

namespace elabtsaot{

enum SchematicEditorState{
  SCHEDIT_STATE_IDLE = SCHVIEW_STATE_IDLE,
  SCHEDIT_STATE_PREPAN = SCHVIEW_STATE_PREPAN,
  SCHEDIT_STATE_PAN = SCHVIEW_STATE_PAN,
  SCHEDIT_STATE_PREZOOM = SCHVIEW_STATE_PREZOOM,
  SCHEDIT_STATE_ZOOM = SCHVIEW_STATE_ZOOM,
  SCHEDIT_STATE_SELECT = SCHVIEW_STATE_SELECT,
  SCHEDIT_STATE_MOVE = 6,     //!< the user is moving elements in the schematic
  SCHEDIT_STATE_PREGEN = 7,   //!< the user is about to place a generator
  SCHEDIT_STATE_PRELOAD = 8,  //!< the user is about to place a load
  SCHEDIT_STATE_PREBUS = 9,   //!< the user is about to place a bus
  SCHEDIT_STATE_PREBRFR = 10, //!< the user is about to place the from end of a br.
  SCHEDIT_STATE_PREBRTO = 11  //!< the user is about to place the to end of a branch
};

class SchematicEditor : public SchematicViewer {

  Q_OBJECT

 public:

  SchematicEditor( Powersystem* pws, PwsSchematicModel* smd,
                   QWidget* parent = 0);

  virtual void mousePressEvent(QMouseEvent* event);
  virtual void mouseMoveEvent(QMouseEvent* event);
  virtual void mouseReleaseEvent(QMouseEvent* event);
  virtual void wheelEvent(QWheelEvent* event);

 private:

  Powersystem* _pws;
  PwsSchematicModel* _smd;

};

} // end of namespace elabtsaot

#endif // SCHEMATICEDITOR_H
