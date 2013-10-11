/*!
\file schematicmover.h

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef SCHEMATICMOVER_H
#define SCHEMATICMOVER_H

#include "schematicviewer.h"

namespace elabtsaot{

enum SchematicMoverState{
  SCHMOVE_STATE_IDLE = SCHVIEW_STATE_IDLE,
  SCHMOVE_STATE_PREPAN = SCHVIEW_STATE_PREPAN,
  SCHMOVE_STATE_PAN = SCHVIEW_STATE_PAN,
  SCHMOVE_STATE_PREZOOM = SCHVIEW_STATE_PREZOOM,
  SCHMOVE_STATE_ZOOM = SCHVIEW_STATE_ZOOM,
  SCHMOVE_STATE_SELECT = SCHVIEW_STATE_SELECT,
  SCHMOVE_STATE_MOVE = 6      //!< the user is moving elements in the schematic
};

class SchematicMover : public SchematicViewer {

  Q_OBJECT

 public:

  SchematicMover( Powersystem const* pws, PwsSchematicModel* smd,
                  QWidget* parent = 0 );

  virtual void mousePressEvent(QMouseEvent* event);
  virtual void mouseMoveEvent(QMouseEvent* event);
  virtual void mouseReleaseEvent(QMouseEvent* event);
  virtual void wheelEvent(QWheelEvent* event);

private:

 Powersystem const* _pws;
 PwsSchematicModel* _smd;

};

} // end of namespace elabtsaot

#endif // SCHEMATICMOVER_H
