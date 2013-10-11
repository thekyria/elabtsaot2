
#include "slicewidget.h"
using namespace elabtsaot;

#include "slice.h"

#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>

#include <vector>
using std::vector;

#ifndef VERBOSE
#define VERBOSE 1
#endif // VERBOSE

#ifndef SCALEMIN
#define SCALEMIN 0.25
#endif // SCALEMIN

#ifndef SCALEMAX
#define SCALEMAX 4
#endif // SCALEMAX

#ifndef SCALESTEP
#define SCALESTEP 0.1
#endif // SCALESTEP

/* Implementation of graphic part of slice */
SliceWidget::SliceWidget( Slice const* slc, QWidget* parent) :
    QFrame(parent),
    _slc(slc),
    _sliceIndex(slc->sliceIndex),
    _state(SLWSTATE_IDLE),
    _isInvertible(true) {

  // Initialization of slice related internal variables
  _slc->ana.size(_rows,_cols);

  this->setFocusPolicy(Qt::ClickFocus);
  this->setMinimumHeight(200);
  this->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
  this->setLineWidth( 1 );
  this->setMidLineWidth( 2 );
  QPalette palette = this->palette();
  palette.setColor( backgroundRole(), QColor(255, 255, 255) );
  this->setPalette( palette );
  this->setAutoFillBackground( true );
}

// Static variable initialization
float SliceWidget::_step = 130;
float SliceWidget::_scale = 1;
float SliceWidget::_xref = 0;
float SliceWidget::_yref = 0;

void SliceWidget::paintEvent(QPaintEvent* event){
  return QFrame::paintEvent( event );
}

void SliceWidget::mousePressEvent(QMouseEvent *event){

  QPointF temp;
  if ( _isInvertible ){
    temp = _inverseTransform.map( event->localPos() );
    _xclick = temp.x();
    _yclick = temp.y();
    _xmouse = temp.x();
    _ymouse = temp.y();
  } else{
    // Should never happen!
    return;
  }

  switch( _state ){
  case SLWSTATE_PREPAN:
    _state = SLWSTATE_PAN;
    break;
  case SLWSTATE_PREZOOM:
    _state = SLWSTATE_ZOOM;
    break;
  }

  QFrame::mousePressEvent(event);
}

void SliceWidget::mouseMoveEvent(QMouseEvent *event){

  if ( _isInvertible ){
    QPointF temp = _inverseTransform.map( event->localPos() );
    _xmouse = temp.x();
    _ymouse = temp.y();
  } else{
    // Should never happen!
    return;
  }

  switch( _state ){

  case SLWSTATE_PAN:
    _xref += (_xmouse - _xclick);
    _yref += (_ymouse - _yclick);
    break;

  } // end of switch(case)

  repaint();

  return QFrame::mouseMoveEvent(event);
}

void SliceWidget::mouseReleaseEvent(QMouseEvent *event){

  if ( _isInvertible ){
    QPointF temp = _inverseTransform.map( event->localPos() );
    _xmouse = temp.x();
    _ymouse = temp.y();
  } else{
    // Should never happen!
    return;
  }

  switch( _state ){

  case SLWSTATE_PAN:
    _state = SLWSTATE_PREPAN;
    break;

  case SLWSTATE_ZOOM:
    _state = SLWSTATE_PREZOOM;
    break;

  } // end of switch( _state )

  QFrame::mouseReleaseEvent(event);
}

void SliceWidget::mouseDoubleClickEvent(QMouseEvent *event){
  // TODO!
}

void SliceWidget::wheelEvent(QWheelEvent* event){
  // event->delta() is in [eightths of a degree]
  int numDegrees = event->delta() / 8;
  // normal mice have wheel steps of [15 degrees]
  int numSteps = numDegrees / 15;

  switch( _state ){

  case SLWSTATE_IDLE:
  case SLWSTATE_PREPAN:
  case SLWSTATE_PAN:
    // Pan view
    if (event->orientation() == Qt::Horizontal){
      // Scroll horizontally
      _xref += numSteps * 5;
    } else{ // (event->orientation() == Qt::Vertical)
      // Scroll vertically
      _yref += numSteps * 5;
    }
    break;

  case SLWSTATE_PREZOOM:
  case SLWSTATE_ZOOM:
    // Zoom view
    _scale += numSteps * SCALESTEP;
    if ( _scale < SCALEMIN ){
      // Check zoom out limits
      _scale = SCALEMIN;
    } else if ( _scale > SCALEMAX ){
      // Zoom in
      _scale = SCALEMAX;
    }
    break;
  } // end of switch(_state)

  event->accept();
  repaint();

  return QFrame::wheelEvent(event);
}

void SliceWidget::keyPressEvent(QKeyEvent *event){

  switch( _state ){

  case SLWSTATE_IDLE:
    if( event->key()==Qt::Key_Alt )
      _state = SLWSTATE_PREPAN;
    else if( event->key()==Qt::Key_Control )
      _state = SLWSTATE_PREZOOM;
    break;

  } // end of switch( _state )

  return QFrame::keyPressEvent(event);
}

void SliceWidget::keyReleaseEvent(QKeyEvent *event){

  switch( _state ){

  case SLWSTATE_PREPAN:
  case SLWSTATE_PAN:
    if( event->key()==Qt::Key_Alt )
      _state = SLWSTATE_IDLE;
    break;

  case SLWSTATE_PREZOOM:
  case SLWSTATE_ZOOM:
    if( event->key()==Qt::Key_Control )
      _state = SLWSTATE_IDLE;
    break;
  } // end of switch( _state )

  return QFrame::keyReleaseEvent(event);
}

void SliceWidget::zoomIn(){
  // Maximum zoom in reached
  if( _scale >= SCALEMAX )
    return;

  // Zoom in while respecting the max zoom in limit
  if( _scale+SCALESTEP > SCALEMAX )
    _scale = SCALEMAX;
  else
    _scale += SCALESTEP;

  repaint();
}

void SliceWidget::zoomOut(){
  // Maximum zoom out reached
  if( _scale <= SCALEMIN )
    return;

  // Zoom out while respecting the max zoom out limit
  if( _scale-SCALESTEP < SCALEMIN )
    _scale = SCALEMIN;
  else
    _scale -= SCALESTEP;

  repaint();
}

void SliceWidget::zoomFit(){
  // TODO
}

void SliceWidget::_drawNode( QPainter& painter, QPointF p,
                             unsigned int row, unsigned int col ){

  QString nodeLabel;
  QPointF nodeLabelOffset = QPointF( 10, 15 );

  // Draw node proper
  painter.save();
  QPen nodePen = QPen();
  QBrush nodeBrush = QBrush();
  nodeBrush.setStyle(Qt::SolidPattern);
  painter.setPen(nodePen);
  nodeBrush.setColor(QColor(128,128,128));
  painter.setBrush(nodeBrush);
  painter.drawEllipse( p, 5,5 );
  painter.restore();

  // Draw node label
  painter.save();
  painter.setPen( Qt::black );
  nodeLabel = QString("%0(%1,%2)").arg( row*_cols + col + 1)
                                  .arg( row )
                                  .arg( col );
  painter.drawText( p + nodeLabelOffset, nodeLabel );
  painter.restore();

  return;
}
