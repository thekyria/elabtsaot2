/*!
\file schematicviewer.h

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef SCHEMATICVIEWER_H
#define SCHEMATICVIEWER_H

#include <QFrame>

namespace elabtsaot{

class Powersystem;
class PwsSchematicModel;
class PwsSchematicModelElement;

enum SchematicViewerState{
  SCHVIEW_STATE_IDLE = 0,    //!< schematic is idle
  SCHVIEW_STATE_PREPAN = 1,  //!< schematic is about to pan
  SCHVIEW_STATE_PAN = 2,     //!< schematic is panning
  SCHVIEW_STATE_PREZOOM = 3, //!< schematic is about to zoom
  SCHVIEW_STATE_ZOOM = 4,    //!< schematic is zooming
  SCHVIEW_STATE_SELECT = 5  //!< the user is selecting elements in the schematic
};

class SchematicViewer : public QFrame {

  Q_OBJECT

 public:

  SchematicViewer( Powersystem const* pws, PwsSchematicModel const* smd,
                   QWidget* parent = 0 );

  virtual void paintEvent(QPaintEvent* event);
  virtual void mousePressEvent(QMouseEvent* event);
  virtual void mouseMoveEvent(QMouseEvent* event);
  virtual void mouseReleaseEvent(QMouseEvent* event);
  virtual void wheelEvent(QWheelEvent* event);
  virtual void keyPressEvent(QKeyEvent* event);
  virtual void keyReleaseEvent(QKeyEvent* event);

  void clearSelected();

  // Getters
  QVector< std::pair<int,int> > selected() const;
  bool isInvertible() const;
  QTransform inverseTransform() const;

 public slots:

  void componentSelectedSlot(int type, unsigned int extId);
  void zoomIn();
  //! Zooms out the viewer
  /*! \todo for large zoomout and alt+move (pan) goes crazy and crashes */
  void zoomOut();
  void zoomFit();
  void snapshot();

 protected:

  // Drawing functions
  void _drawGrid(QPainter& painter);
  //! \todo Perhaps a more sophisticated _drawComponents()? _drawBus(),
  //!       _drawBranch(), etc;
  void _drawComponents(QPainter& painter);
  void _drawBus( QPainter& painter, PwsSchematicModelElement const& busCd );
  void _drawBranch( QPainter& painter, PwsSchematicModelElement const& brCd );
  void _drawGenerator( QPainter& painter, PwsSchematicModelElement const& genCd );
  void _drawLoad( QPainter& painter, PwsSchematicModelElement const& loadCd );
  void _drawSelectionMarkers(QPainter& painter);
  std::pair<int,int> _getPointedComponent(float x_t, float y_t);
  std::pair<int,int> _getPointedComponent( QPointF const& p_t );
  QVector< std::pair<int,int> >
  _getPointedComponents( float x1_t, float y1_t,
                         float x2_t, float y2_t );
  QVector< std::pair<int,int> >
  _getPointedComponents( QPointF const& p1, QPointF const& p2 );

  // Variables that store editor's internal state
  int _state;                         // The internal state of the editor
  QVector< std::pair<int,int> > _selected; // Selected components
  float _scale;                       // Global view parameter: scale
  float _xref, _yref;                 // Global view parameter: reference point
  float _xclick, _yclick;             // Last point clicked (in schematic editor
                                      // absolute coordinates)
  float _xmouse, _ymouse;             // Track mouse position (in schematic
                                      // editor absolute coordinates)
  QTransform _translationTransform;
  QTransform _scalingTransform;
  QTransform _finalTransform;
  bool _isInvertible;
  QTransform _inverseTransform;

 private:

  Powersystem const* _pws;
  PwsSchematicModel const* _smd;

};

} // end of namespace elabtsaot

#endif // SCHEMATICVIEWER_H
