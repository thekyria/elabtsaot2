
#include "schematicviewer.h"
using namespace elabtsaot;

#include "powersystem.h"
#include "pwsschematicmodel.h"
#include "guiauxiliary.h"

#include <QColor>
#include <QBrush>
#include <QPen>
#include <QPixmap>
#include <QPainter>
#include <QToolBar>
#include <QFileDialog>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPalette>

#include <vector>
using std::vector;
//#include <utility>
using std::pair;
#include <math.h>
#include <algorithm>
using std::min;
using std::max;
#include <iostream>
using std::cout;
using std::endl;
//#include <set>
using std::set;

#ifndef VERBOSE
#define VERBOSE 1
#endif // VERBOSE

#ifndef GRID
#define GRID 10
#endif // GRID

#ifndef SCALEMIN
#define SCALEMIN 0.25
#endif // SCALEMIN

#ifndef SCALEMAX
#define SCALEMAX 4
#endif // SCALEMAX

#ifndef SCALESTEP
#define SCALESTEP 0.1
#endif // SCALESTEP

SchematicViewer::SchematicViewer( Powersystem const* pws,
                                  PwsSchematicModel const* smd,
                                  QWidget* parent ) :
    QFrame(parent),
    _state(SCHVIEW_STATE_IDLE),
    _scale(1), _xref(0), _yref(0), _isInvertible(true),
    _pws(pws), _smd(smd){
  setFocusPolicy(Qt::ClickFocus);
  setMinimumHeight(200);
  setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
  setLineWidth( 1 );
  setMidLineWidth( 2 );
  QPalette palette = this->palette();
  palette.setColor( backgroundRole(), QColor(255, 255, 255) );
  setPalette( palette );
  setAutoFillBackground( true );
}

void SchematicViewer::paintEvent(QPaintEvent* event){

  // --- Set up painter ---
  QPainter painter(this);
  // Setup painter transforms
  _translationTransform.reset();
  _translationTransform.translate( _xref , _yref);
  _scalingTransform.reset();
  _scalingTransform.scale( _scale , _scale );
  // Final tranformation: [translation] followed by a [scaling]
  _finalTransform = _scalingTransform * _translationTransform;
  painter.setTransform(_finalTransform);
  // Calculate inverse transform for mouse tracking reasons
  _inverseTransform = _finalTransform.inverted( &_isInvertible );

  // --- Draw widget ---
  _drawGrid( painter );
  _drawComponents( painter );
  _drawSelectionMarkers( painter );

  event->accept();
}

void SchematicViewer::mousePressEvent(QMouseEvent* event){

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

  case SCHVIEW_STATE_IDLE:{
    pair<int,int> comp = _getPointedComponent( QPointF(_xclick, _yclick) );

//    if( _selected.indexOf(comp)>=0 ){ // Point is a selected component
//      _state = SCHVIEW_STATE_MOVE;
//    } else
    if( comp.first>=0 ){ // Point is an unselected component
      _selected.clear();
      if( comp.first>=0 )
        _selected.push_back( comp );
    }else{ // Point is in free-space
      _selected.clear();
      _state = SCHVIEW_STATE_SELECT;
    } }
    break;

  case SCHVIEW_STATE_PREPAN:
    _state = SCHVIEW_STATE_PAN;
    break;

  case SCHVIEW_STATE_PREZOOM:
    _state = SCHVIEW_STATE_ZOOM;
    break;

  } // end of switch(_state)

  repaint();

  return QFrame::mousePressEvent(event);
}

void SchematicViewer::mouseMoveEvent(QMouseEvent* event){

  if ( _isInvertible ){
    QPointF temp = _inverseTransform.map( event->localPos() );
    _xmouse = temp.x();
    _ymouse = temp.y();
  } else{
    // Should never happen!
    return;
  }

  switch( _state ){

  case SCHVIEW_STATE_PAN:
    _xref += (_xmouse - _xclick);
    _yref += (_ymouse - _yclick);
    break;

  } // end of switch(case)

  repaint();

  return QFrame::mouseMoveEvent(event);
}

void SchematicViewer::mouseReleaseEvent(QMouseEvent* event){

  if ( _isInvertible ){
    QPointF temp = _inverseTransform.map( event->localPos() );
    _xmouse = temp.x();
    _ymouse = temp.y();
  } else{
    // Should never happen!
    return;
  }

  switch( _state ){

  case SCHVIEW_STATE_PAN:
    _state = SCHVIEW_STATE_PREPAN;
    break;

  case SCHVIEW_STATE_ZOOM:
    _state = SCHVIEW_STATE_PREZOOM;
    break;

  case SCHVIEW_STATE_SELECT:
    _selected = _getPointedComponents(QPointF(_xclick,_yclick),
                                     QPointF(_xmouse,_ymouse));
    _state = SCHVIEW_STATE_IDLE;
    break;

  } // end of switch( _state )

  repaint();

  return QFrame::mouseReleaseEvent(event);
}

void SchematicViewer::wheelEvent(QWheelEvent* event){

  // event->delta() is in [eightths of a degree]
  int numDegrees = event->delta() / 8;
  // normal mice have wheel steps of [15 degrees]
  int numSteps = numDegrees / 15;

  switch( _state ){

  case SCHVIEW_STATE_PREPAN:
    // do nothing!
    break;

  case SCHVIEW_STATE_IDLE:
  case SCHVIEW_STATE_SELECT:
    // Pan view
    if (event->orientation() == Qt::Horizontal){
      // Scroll horizontally
      _xref += numSteps * 5;
    } else{ // (event->orientation() == Qt::Vertical)
      // Scroll vertically
      _yref += numSteps * 5;
    }
    break;

  case SCHVIEW_STATE_PREZOOM:
  case SCHVIEW_STATE_ZOOM:
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

void SchematicViewer::keyPressEvent(QKeyEvent* event){

  switch( _state ){

  case SCHVIEW_STATE_IDLE:
    if( event->key()==Qt::Key_Alt )
      _state = SCHVIEW_STATE_PREPAN;
    else if( event->key()==Qt::Key_Control )
      _state = SCHVIEW_STATE_PREZOOM;
    break;

  } // end of switch( _state )

  return QFrame::keyPressEvent(event);
}
void SchematicViewer::keyReleaseEvent(QKeyEvent* event){

  switch( _state ){

  case SCHVIEW_STATE_PREPAN:
  case SCHVIEW_STATE_PAN:
    if( event->key()==Qt::Key_Alt )
      _state = SCHVIEW_STATE_IDLE;
    break;

  case SCHVIEW_STATE_PREZOOM:
  case SCHVIEW_STATE_ZOOM:
    if( event->key()==Qt::Key_Control )
      _state = SCHVIEW_STATE_IDLE;
    break;
  } // end of switch( _state )

  return QFrame::keyReleaseEvent(event);
}

// Getters
void SchematicViewer::clearSelected(){ _selected.clear(); }
QVector< pair<int,int> > SchematicViewer::selected() const{ return _selected; }
bool SchematicViewer::isInvertible() const{ return _isInvertible; }
QTransform SchematicViewer::inverseTransform() const{ return _inverseTransform;}

void SchematicViewer::componentSelectedSlot(int type, unsigned int extId){
  _selected.clear();
  _selected.push_back( pair<int,int>( type , static_cast<int>(extId) ) );

  repaint();

  return;
}

void SchematicViewer::zoomIn(){
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

void SchematicViewer::zoomOut(){
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

void SchematicViewer::zoomFit(){
  // Zoom fit by computing bounding rectangle
  float minx =  10000;
  float maxx = -10000;
  float miny =  10000;
  float maxy = -10000;

  // --- Find bounding corners of viewing canvas ---
  PwsSchematicModelElement const* cd;
  size_t mdlId;

  // Cycle through all Buses
  for( mdlId = 0; mdlId!=_smd->busElements_size(); ++mdlId ){

    // Retrieve component element
    cd = _smd->elementByIndex(PWSMODELELEMENTTYPE_BUS, mdlId);

    // Check for new possible boundary
    if( cd->x1 < minx )
      minx = cd->x1;
    if( cd->x2 < minx )
      minx = cd->x2;
    if( cd->x1 > maxx )
      maxx = cd->x1;
    if( cd->x2 > maxx )
      maxx = cd->x2;
    if( cd->y1 < miny )
      miny = cd->y1;
    if( cd->y2 < miny )
      miny = cd->y2;
    if( cd->y1 > maxy )
      maxy = cd->y1;
    if( cd->y2 > maxy )
      maxy = cd->y2;
  }

  // Cycle through all Branches
  for( mdlId = 0; mdlId!=_smd->branchElements_size(); ++mdlId ){

    // Retrieve component element
    cd = _smd->elementByIndex(PWSMODELELEMENTTYPE_BR, mdlId);

    // Check for new possible boundary
    if( cd->x1 < minx )
      minx = cd->x1;
    if( cd->x2 < minx )
      minx = cd->x2;
    if( cd->x1 > maxx )
      maxx = cd->x1;
    if( cd->x2 > maxx )
      maxx = cd->x2;
    if( cd->y1 < miny )
      miny = cd->y1;
    if( cd->y2 < miny )
      miny = cd->y2;
    if( cd->y1 > maxy )
      maxy = cd->y1;
    if( cd->y2 > maxy )
      maxy = cd->y2;
  }

  // Cycle through all Generators
  for( mdlId = 0; mdlId!=_smd->genElements_size(); ++mdlId ){

    // Retrieve component element
    cd = _smd->elementByIndex(PWSMODELELEMENTTYPE_GEN, mdlId);

    // Check for new possible boundary
    if( cd->x1 < minx )
      minx = cd->x1;
    if( cd->x2 < minx )
      minx = cd->x2;
    if( cd->x1 > maxx )
      maxx = cd->x1;
    if( cd->x2 > maxx )
      maxx = cd->x2;
    if( cd->y1 < miny )
      miny = cd->y1;
    if( cd->y2 < miny )
      miny = cd->y2;
    if( cd->y1 > maxy )
      maxy = cd->y1;
    if( cd->y2 > maxy )
      maxy = cd->y2;
  }

  // Cycle through all Loads
  for( mdlId = 0; mdlId!=_smd->loadElements_size(); ++mdlId ){

    // Retrieve component element
    cd = _smd->elementByIndex(PWSMODELELEMENTTYPE_LOAD, mdlId);

    // Check for new possible boundary
    if( cd->x1 < minx )
      minx = cd->x1;
    if( cd->x2 < minx )
      minx = cd->x2;
    if( cd->x1 > maxx )
      maxx = cd->x1;
    if( cd->x2 > maxx )
      maxx = cd->x2;
    if( cd->y1 < miny )
      miny = cd->y1;
    if( cd->y2 < miny )
      miny = cd->y2;
    if( cd->y1 > maxy )
      maxy = cd->y1;
    if( cd->y2 > maxy )
      maxy = cd->y2;
  }

  // Add some padding around bounding rectangle
  minx -= 30; maxx += 30;
  miny -= 30; maxy += 30;

  // Set _xref, _yref and _scale
  _xref = -minx;
  _yref = -miny;
  _scale = min( width()/(maxx-minx) , height()/(maxy-miny) );
  // Saturate _scale to its bounds
  if( _scale > SCALEMAX )
    _scale = SCALEMAX;
  if( _scale < SCALEMIN )
    _scale = SCALEMAX;

  // Update view
  repaint();
}

void SchematicViewer::snapshot(){

  // --- Find bounding corners of viewing canvas ---
  zoomFit();

  // --- File to save result ---
  QString filename = guiauxiliary::askFileName("png", false);
  if ( filename.isNull() )
    return;

  // --- Write snapshot to file ---
  // Set up  painter
  QImage img(width(), height(), QImage::Format_ARGB32);
  img.fill( Qt::transparent );
  QPainter painter(&img);

  _drawGrid( painter );
  _drawComponents( painter );

  // Save file
  img.save(filename);
}

void SchematicViewer::_drawGrid(QPainter& painter){

  // TODO: purge from '_scale' and '_xref' ...

  QTransform oldTransform = painter.transform();
  QTransform newTransform;
  newTransform.reset();
  painter.setTransform( newTransform );

  float xstart = fmod(_xref,GRID)*_scale;
  if( xstart<0 )
    xstart += GRID*_scale;
  float ystart = fmod(_yref,GRID)*_scale;
  if( ystart<0 )
    ystart += GRID*_scale;
  for( float x = xstart; x < width(); x += GRID*_scale )
    for( float y = ystart; y < height(); y += GRID*_scale )
      painter.drawPoint(x,y);

  painter.setTransform( oldTransform );
}

void SchematicViewer::_drawComponents(QPainter& painter){

  // Setup painter
  painter.setPen( QPen() );
  painter.setBrush( QBrush() );
  painter.setFont(QFont("Helvetica", 12));

  size_t mdlId;
  PwsSchematicModelElement const* cd;
  // --- Buses ---
  for( mdlId = 0; mdlId!=_smd->busElements_size(); ++mdlId ){
    cd = _smd->elementByIndex(PWSMODELELEMENTTYPE_BUS, mdlId);
    _drawBus(painter, *cd);
  }

  // --- Branches ---
  for( mdlId = 0; mdlId!=_smd->branchElements_size(); ++mdlId ){
    cd = _smd->elementByIndex(PWSMODELELEMENTTYPE_BR, mdlId);
    _drawBranch(painter, *cd);
  }

  // --- Generators ---
  for( mdlId = 0; mdlId!=_smd->genElements_size(); ++mdlId ){
    cd = _smd->elementByIndex(PWSMODELELEMENTTYPE_GEN, mdlId);
    _drawGenerator(painter, *cd);
  }

  // --- Loads ---
  for( mdlId = 0; mdlId!=_smd->loadElements_size(); ++mdlId ){
    cd = _smd->elementByIndex(PWSMODELELEMENTTYPE_LOAD, mdlId);
    _drawLoad(painter, *cd);
  }

  return;
}

void SchematicViewer::_drawBus( QPainter& painter,
                                PwsSchematicModelElement const& busCd ){

  // Assert that argument passed is indeed a bus
  if (busCd.type != PWSMODELELEMENTTYPE_BUS )
    // Otherwise do nothing
    return;

  // Retrieve component itself
  Bus const* pBus;
  if( _pws->getBus( busCd.extId, pBus ) )
    // If retrieval fails do nothing
    return;

  // Store old brush of the painter for restoring later
  QBrush oldBrush = painter.brush();

  // Draw component
  QBrush busBrush = QBrush();
  busBrush.setStyle(Qt::SolidPattern);
  busBrush.setColor(QColor(0,0,0));
  painter.setBrush(busBrush);
  painter.drawRect( busCd.x1, busCd.y1, busCd.x2-busCd.x1, busCd.y2-busCd.y1 );
  float xmax = max(busCd.x1, busCd.x2);
  float ymid = (busCd.y1 + busCd.y2)/2;
  painter.drawText( QPoint( xmax+15, ymid ), pBus->name.c_str() );

  // Restore painter's old brush
  painter.setBrush( oldBrush );

  return;
}

void SchematicViewer::_drawBranch( QPainter& painter,
                                   PwsSchematicModelElement const& brCd ){

  // Assert that argument passed is indeed a branch
  if (brCd.type != PWSMODELELEMENTTYPE_BR )
    // Otherwise do nothing
    return;

  // Retrieve component itself
  Branch const* pBr;
  if( _pws->getBranch( brCd.extId, pBr ) )
    // If retrieval fails do nothing
    return;

  // Draw component
  QPoint P0( brCd.x1, brCd.y1 );
  QPoint P1( brCd.x2, brCd.y2 );
  painter.drawLine( P0, P1 );
  painter.drawText( (P0+P1)/2 , QString(pBr->extId) );

  return;
}

void SchematicViewer::_drawGenerator( QPainter& painter,
                                      PwsSchematicModelElement const& genCd ){

  // Assert that argument passed is indeed a generator
  if (genCd.type != PWSMODELELEMENTTYPE_GEN )
    // Otherwise do nothing
    return;

  // Retrieve component itself
  Generator const* pGen;
  if( _pws->getGenerator( genCd.extId, pGen ) )
    // If retrieval fails do nothing
    return;

  // Draw component
  painter.setBrush( QBrush() );
  // Compute points of interest - according to generator drawing orientation
  QPoint P0, P1, P2;

//  if ( (genCd.x2 - genCd.x1 == 20) && (genCd.y2 - genCd.y1 == -25) ){
  if ( (genCd.x2 >= genCd.x1) && (genCd.y2 < genCd.y1) ){
    // Up orientation
    P0 = QPoint( genCd.x1 + 10, genCd.y1 );
    P1 = P0 + QPoint(   0 , -10);
    P2 = P0 + QPoint(   0 , -17);
//  } else if ( (genCd.x2 - genCd.x1 == 25) && (genCd.y2 - genCd.y1 == 20) ){
  } else if ( (genCd.x2 >= genCd.x1) && (genCd.y2 >= genCd.y1) ){
    // Right orientation
    P0 = QPoint( genCd.x1, genCd.y1 + 10 );
    P1 = P0 + QPoint(  10 ,   0);
    P2 = P0 + QPoint(  17 ,   0);
//  } else if ( (genCd.x2 - genCd.x1 == -20) && (genCd.y2 - genCd.y1 == 25) ){
  } else if ( (genCd.x2 < genCd.x1) && (genCd.y2 >= genCd.y1) ){
    // Down orientation
    P0 = QPoint( genCd.x1 - 10, genCd.y1 );
    P1 = P0 + QPoint(   0 ,  10);
    P2 = P0 + QPoint(   0 ,  17);
//  } else if ( (genCd.x2 - genCd.x1 == -25) && (genCd.y2 - genCd.y1 == -20) ){
  } else if ( (genCd.x2 < genCd.x1) && (genCd.y2 < genCd.y1) ){
    // Left orientation
    P0 = QPoint( genCd.x1, genCd.y1 - 10 );
    P1 = P0 + QPoint( -10 ,   0);
    P2 = P0 + QPoint( -17 ,   0);
  } else {
    // Unrecognized orientation! Normally, should never happen
    cout << "Unrecognized generator orientation!" << endl;
  }
  painter.drawLine(P0,P1);
  painter.drawEllipse( QPointF(P2) , 7.0, 7.0 );
  float xmax = max(genCd.x1, genCd.x2);
  float ymid = (genCd.y1 + genCd.y2) / 2;
  painter.drawText( QPoint(xmax + 15, ymid ), pGen->name().c_str() );

  return;
}

void SchematicViewer::_drawLoad( QPainter& painter,
                                 PwsSchematicModelElement const& loadCd ){

  // Assert that argument passed is indeed a load
  if (loadCd.type != PWSMODELELEMENTTYPE_LOAD )
    // Otherwise do nothing
    return;

  // Retrieve component itself
  Load const* pLoad;
  if( _pws->getLoad( loadCd.extId, pLoad ) )
    // If retrieval fails do nothing
    return;

  // Draw component
  painter.setBrush( QBrush() );
  // Compute points of interest - according to load drawing orientation
  QPoint P0, P1, P2, P3;

//  if ( (genCd.x2 - genCd.x1 == 20) && (genCd.y2 - genCd.y1 == -25) ){
  if ( (loadCd.x2 >= loadCd.x1) && (loadCd.y2 < loadCd.y1) ){
    // Up orientation
    P0 = QPoint( loadCd.x1 + 10, loadCd.y1 );
    P1 = P0 + QPoint(  -8 , -10 );
    P2 = P0 + QPoint(   8 , -10 );
    P3 = P0 + QPoint(   0 , -24 );
//  } else if ( (genCd.x2 - genCd.x1 == 25) && (genCd.y2 - genCd.y1 == 20) ){
  } else if ( (loadCd.x2 >= loadCd.x1) && (loadCd.y2 >= loadCd.y1) ){
    // Right orientation
    P0 = QPoint( loadCd.x1 , loadCd.y1 - 10 );
    P1 = P0 + QPoint(  10 ,   8 );
    P2 = P0 + QPoint(  10 ,  -8 );
    P3 = P0 + QPoint(  24 ,   0 );
//  } else if ( (genCd.x2 - genCd.x1 == -20) && (genCd.y2 - genCd.y1 == 25) ){
  } else if ( (loadCd.x2 < loadCd.x1) && (loadCd.y2 >= loadCd.y1) ){
    // Down orientation
    P0 = QPoint( loadCd.x1 - 10 , loadCd.y1 );
    P1 = P0 + QPoint(  -8 ,  10 );
    P2 = P0 + QPoint(   8 ,  10 );
    P3 = P0 + QPoint(   0 ,  24 );
//  } else if ( (genCd.x2 - genCd.x1 == -25) && (genCd.y2 - genCd.y1 == -20) ){
  } else if ( (loadCd.x2 < loadCd.x1) && (loadCd.y2 < loadCd.y1) ){
    // Left orientation
    P0 = QPoint( loadCd.x1 , loadCd.y1 + 10 );
    P1 = P0 + QPoint( -10 ,   8 );
    P2 = P0 + QPoint( -10 ,  -8 );
    P3 = P0 + QPoint( -24 ,   0 );
  } else {
    // Unrecognized orientation! Normally, should never happen
    cout << "Unrecognized generator orientation!" << endl;
  }

  painter.drawLine(P0, (P1+P2)/2);
  painter.drawLine(P1,P2);
  painter.drawLine(P1,P3);
  painter.drawLine(P2,P3);
  float xmax = max(loadCd.x1, loadCd.x2);
  float ymid = (loadCd.y1 + loadCd.y2)/2;
  painter.drawText( QPoint(xmax + 15, ymid), QString("LD%1").arg(pLoad->extId) );

  return;
}

void SchematicViewer::_drawSelectionMarkers(QPainter& painter){

  // Store old pen - so as to restore later
  QPen oldPen = painter.pen();

  // Create new pen for the selection markers
  QPen* pen = new QPen( QColor( 0, 0, 255, 128) );
  pen->setStyle( Qt::DashLine );
  painter.setPen( *pen );

  if( _state == SCHVIEW_STATE_SELECT ){
    // Draw selection rectangle, if applicable
    painter.drawRect( _xclick , _yclick , _xmouse-_xclick , _ymouse-_yclick );

  } else{
    PwsSchematicModelElement const* cd;
    for( int k = 0; k != _selected.size(); ++k ){

      // Draw selection around a generator/load/bus
      if( _selected.at(k).first    == PWSMODELELEMENTTYPE_BUS
          || _selected.at(k).first == PWSMODELELEMENTTYPE_GEN
          || _selected.at(k).first == PWSMODELELEMENTTYPE_LOAD ){
        // Retrieve component element
        cd = _smd->element( _selected.at(k).first,
                                       _selected.at(k).second );
        if( cd->type < 0 )
          continue;

        // Find bounding box and augment by 2
        float top = qMin( cd->y1, cd->y2 );
        float bot = qMax( cd->y1, cd->y2 );
        float lef = qMin( cd->x1, cd->x2 );
        float rig = qMax( cd->x1, cd->x2 );
        QRect rect = QRect( QPoint(lef-2,top-2), QPoint(rig+2,bot+2) );

        // Draw selection rectangle
        painter.drawRect( rect );
      }

      // Draw selection around a branch
      if( _selected.at(k).first == PWSMODELELEMENTTYPE_BR ){
        cd = _smd->element( PWSMODELELEMENTTYPE_BR,
                                       _selected.at(k).second );
        if( cd->type != PWSMODELELEMENTTYPE_BR )
          continue;

        QLine* line = new QLine(cd->x1, cd->y1, cd->x2, cd->y2);
        int oldWidth = pen->width();
        pen->setWidth( 5 );
        painter.setPen( *pen );
        painter.drawLine( *line );
        delete line;
        // restore old pen width
        pen->setWidth( oldWidth );
        painter.setPen( *pen );
      }
    }
  }

  // Restore old pen
  delete pen;
  painter.setPen( oldPen );

  return;
}

pair<int,int> SchematicViewer::_getPointedComponent(float x_t,
                                                    float y_t){
  return _getPointedComponent(QPointF(x_t, y_t));
}

pair<int,int> SchematicViewer::_getPointedComponent(QPointF const& p_t){
  pair<int,int> ans = pair<int,int>(-1,0);

  PwsSchematicModelElement const* cd;
  size_t mdlId;
  // Iterate through all buses' elements
  for( mdlId = 0; mdlId!=_smd->busElements_size(); ++mdlId ){
    cd = _smd->elementByIndex(PWSMODELELEMENTTYPE_BUS, mdlId);

    if (  p_t.x() >= cd->x1 && p_t.x() <= cd->x2    // x inside limits of bus
       && p_t.y() >= cd->y1 && p_t.y() <= cd->y2 ){ // y inside limits of bus
      return ans = pair<int,int>(PWSMODELELEMENTTYPE_BUS, cd->extId);
    }
  }

  // Iterate through all branches's elements
  for( mdlId = 0; mdlId!=_smd->branchElements_size(); ++mdlId ){
    cd = _smd->elementByIndex(PWSMODELELEMENTTYPE_BR, mdlId);

    // Distance of a point (x,y) from the line passing through {(x1,y1),(y1,y2)}
    // d = abs( (x2-x1)(y1-y)-(x1-x)(y2-y1) ) / sqrt( (x2-x1)^2 + (y2-y1)^2 )
    float d = abs( (cd->x2-cd->x1)*(cd->y1-p_t.y()) - (cd->x1-p_t.x())*(cd->y2-cd->y1) )
              / sqrt( pow(cd->x2-cd->x1,2) + pow(cd->y2-cd->y1,2) );
    // Find pmid1 - the 25% point of the line (from p1 to p2)
    float xmid1 = 0.75*cd->x1 + 0.25*cd->x2;
    float ymid1 = 0.75*cd->y1 + 0.25*cd->y2;
    // Find pmid2 - the 75% point of the line (from p1 to p2)
    float xmid2 = 0.25*cd->x1 + 0.75*cd->x2;
    float ymid2 = 0.25*cd->y1 + 0.75*cd->y2;
    // Find bounding box of pmid1 and pmid2 and augment it by 5 to each direction
    float xmin = min(xmid1, xmid2); xmin -= 5.0;
    float ymin = min(ymid1, ymid2); ymin -= 5.0;
    float xmax = max(xmid1, xmid2); xmax += 5.0;
    float ymax = max(ymid1, ymid2); ymax += 5.0;
    // Check distance from line and that pointed point :) is within line limits
    if( d <= 5                                          // distance within 'limits'
       && xmin <= p_t.x() && p_t.x() <= xmax    // x inside specified x limits
       && ymin <= p_t.y() && p_t.y() <= ymax ){ // y inside specified y limits
      return ans = pair<int,int>(PWSMODELELEMENTTYPE_BR, cd->extId);
    }
  }

  // Iterate through all generators' elements
  for( mdlId = 0; mdlId!=_smd->genElements_size(); ++mdlId ){
    cd = _smd->elementByIndex(PWSMODELELEMENTTYPE_GEN, mdlId);

    // Calculate distance from generator's center
    float d = sqrt( pow(p_t.x()-(cd->x1+cd->x2)/2 ,2)
                    + pow(p_t.y()-(cd->y1+cd->y2)/2,2) );
    if( d < 6 )
      return ans =  pair<int,int>(PWSMODELELEMENTTYPE_GEN, cd->extId);
  }

  // Iterate through all loads' elements
  for( mdlId = 0; mdlId!=_smd->loadElements_size(); ++mdlId ){
    cd = _smd->elementByIndex(PWSMODELELEMENTTYPE_LOAD, mdlId);

    // Calculate distance from loads's center
    float d = sqrt( pow(p_t.x()-(cd->x1+cd->x2)/2 ,2)
                    + pow(p_t.y()-(cd->y1+cd->y2)/2,2) );
    if( d < 6 )
      return ans =  pair<int,int>(PWSMODELELEMENTTYPE_LOAD, cd->extId);
  }

  return ans;
}

QVector< pair<int,int> >
SchematicViewer::_getPointedComponents( float x1_t, float y1_t,
                                        float x2_t, float y2_t ){
  return _getPointedComponents( QPointF(x1_t, y1_t), QPointF(x2_t, y2_t) );
}

QVector<pair<int,int> >
SchematicViewer::_getPointedComponents(QPointF const& p1_t,
                                       QPointF const& p2_t){
  QVector< pair<int,int> > ans;

  // Find bounding box of the selection
  float sel_xmin, sel_ymin, sel_xmax, sel_ymax;
  sel_xmin = min(p1_t.x(), p2_t.x());
  sel_ymin = min(p1_t.y(), p2_t.y());
  sel_xmax = max(p1_t.x(), p2_t.x());
  sel_ymax = max(p1_t.y(), p2_t.y());

  PwsSchematicModelElement const* cd;
  size_t mdlId;
  float xmin, xmax, ymin, ymax;
  // Iterate through all buses' elements
  for( mdlId = 0; mdlId!=_smd->busElements_size(); ++mdlId ){
    cd = _smd->elementByIndex(PWSMODELELEMENTTYPE_BUS, mdlId);
    // Find bounding box of the component
    xmin = min(cd->x1, cd->x2);
    ymin = min(cd->y1, cd->y2);
    xmax = max(cd->x1, cd->x2);
    ymax = max(cd->y1, cd->y2);
    // Check if selection box completely surrounds bounding box of the component
    if (    sel_xmin <= xmin && sel_ymin <= ymin
         && xmax <= sel_xmax && ymax <= sel_ymax )
      ans.push_back( pair<int,int>(cd->type, cd->extId) );
  }

  // Iterate through all branches's elements
  for( mdlId = 0; mdlId!=_smd->branchElements_size(); ++mdlId ){
    cd = _smd->elementByIndex(PWSMODELELEMENTTYPE_BR, mdlId);
    // Find bounding box of the component
    xmin = min(cd->x1, cd->x2);
    ymin = min(cd->y1, cd->y2);
    xmax = max(cd->x1, cd->x2);
    ymax = max(cd->y1, cd->y2);
    // Check if selection box completely surrounds bounding box of the component
    if (    sel_xmin <= xmin && sel_ymin <= ymin
         && xmax <= sel_xmax && ymax <= sel_ymax )
      ans.push_back( pair<int,int>(cd->type, cd->extId) );
  }

  // Iterate through all generators' elements
  for( mdlId = 0; mdlId!=_smd->genElements_size(); ++mdlId ){
    cd = _smd->elementByIndex(PWSMODELELEMENTTYPE_GEN, mdlId);
    if( cd->type<0 )
      break;
    // Find bounding box of the component
    xmin = min(cd->x1, cd->x2);
    ymin = min(cd->y1, cd->y2);
    xmax = max(cd->x1, cd->x2);
    ymax = max(cd->y1, cd->y2);
    // Check if selection box completely surrounds bounding box of the component
    if (    sel_xmin <= xmin && sel_ymin <= ymin
         && xmax <= sel_xmax && ymax <= sel_ymax )
      ans.push_back( pair<int,int>(cd->type, cd->extId) );
  }

  // Iterate through all loads' elements
  for( mdlId = 0; mdlId!=_smd->loadElements_size(); ++mdlId ){
    cd = _smd->elementByIndex(PWSMODELELEMENTTYPE_LOAD, mdlId);
    if( cd->type<0 )
      break;
    // Find bounding box of the component
    xmin = min(cd->x1, cd->x2);
    ymin = min(cd->y1, cd->y2);
    xmax = max(cd->x1, cd->x2);
    ymax = max(cd->y1, cd->y2);
    // Check if selection box completely surrounds bounding box of the component
    if (    sel_xmin <= xmin && sel_ymin <= ymin
         && xmax <= sel_xmax && ymax <= sel_ymax )
      ans.push_back( pair<int,int>(cd->type, cd->extId) );
  }

  return ans;
}

