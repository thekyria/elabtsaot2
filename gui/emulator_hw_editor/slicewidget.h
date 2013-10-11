/*!
\file slicewidget.h
\brief Definition file for class SliceWidget

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Fragkiskos Koufogiannis, fkoufis at gmail dot com
\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef SLICEWIDGET_H
#define SLICEWIDGET_H

#include <QFrame>

namespace elabtsaot{

struct Slice;

enum SliceWidgetState{
  SLWSTATE_IDLE    = 0, //!< slice widget is idle
  SLWSTATE_PREPAN  = 1, //!< slice widget is about to pan
  SLWSTATE_PAN     = 2, //!< slice widget is panning
  SLWSTATE_PREZOOM = 3, //!< slice widget is about to zoom
  SLWSTATE_ZOOM    = 4  //!< slice widget is zooming
};

class SliceWidget : public QFrame{

  Q_OBJECT

 public:

  SliceWidget( Slice const* slc, QWidget* parent = 0);
  virtual ~SliceWidget(){}

  virtual void paintEvent(QPaintEvent* event);
  virtual void mousePressEvent(QMouseEvent* event);
  void mouseMoveEvent(QMouseEvent* event);
  void mouseReleaseEvent(QMouseEvent* event);
  void mouseDoubleClickEvent(QMouseEvent* event);
  void wheelEvent(QWheelEvent* event);
  void keyPressEvent(QKeyEvent* event);
  void keyReleaseEvent(QKeyEvent* event);

  int sliceIndex(); // getter for _sliceIndex

 public slots:

  // Zooming functionality
  void zoomIn();
  void zoomOut();
  void zoomFit();

 protected:

  virtual void _drawNode( QPainter& painter, QPointF p,
                          unsigned int row, unsigned int col );

  // slice related members
  Slice const* _slc;
  unsigned int _rows, _cols;
  int _sliceIndex;

  // rendering related members
  int _state;
  static float _step;               // distance between 2 consequtive nodes [px]
  static float _scale;
  static float _xref, _yref;        // Global view parameter: reference point
  float _xclick, _yclick;           // Last point clicked (in emulator editor
                                    // absolute coordinates)
  float _xmouse, _ymouse;           // Track mouse position (in emulator
                                    // editor absolute coordinates)

  QTransform _translationTransform;
  QTransform _scalingTransform;
  QTransform _finalTransform;
  bool _isInvertible;
  QTransform _inverseTransform;

};

} // end of namespace elabtsaot

#endif // SLICEWIDGET_H
