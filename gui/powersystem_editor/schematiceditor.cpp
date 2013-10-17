
#include "schematiceditor.h"
using namespace elabtsaot;

#include "powersystem.h"
#include "pwsschematicmodel.h"

#include <QMouseEvent>

//#include <vector>
using std::vector;
//#include <set>
using std::set;
//#include <utility>
using std::pair;
#include <iostream>
using std::cout;
using std::endl;

#ifndef SCALEMIN
#define SCALEMIN 0.25
#endif // SCALEMIN

#ifndef SCALEMAX
#define SCALEMAX 4
#endif // SCALEMAX

#ifndef SCALESTEP
#define SCALESTEP 0.1
#endif // SCALESTEP

SchematicEditor::SchematicEditor( Powersystem* pws, PwsSchematicModel* smd,
                                  QWidget* parent) :
    SchematicViewer(pws, smd, parent),
    _pws(pws), _smd(smd) {}

void SchematicEditor::mousePressEvent(QMouseEvent* event){

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

  case SCHEDIT_STATE_IDLE:{
    pair<int,int> comp = _getPointedComponent( QPointF(_xclick, _yclick) );

    if( _selected.indexOf(comp)>=0 ){ // Point is a selected component
      _state = SCHEDIT_STATE_MOVE;
    }else if( comp.first>=0 ){ // Point is an unselected component
      _selected.clear();
      if( comp.first>=0 )
        _selected.push_back( comp );
    }else{ // Point is in free-space
      _selected.clear();
      _state = SCHEDIT_STATE_SELECT;
    } }
    break;

  case SCHEDIT_STATE_PREPAN:
    _state = SCHEDIT_STATE_PAN;
    break;

  case SCHEDIT_STATE_PREZOOM:
    _state = SCHEDIT_STATE_ZOOM;
    break;

  case SCHEDIT_STATE_PREGEN:{
    pair<int,int> comp = _getPointedComponent( QPointF(_xclick, _yclick) );
    if( comp.first == PWSMODELELEMENTTYPE_BUS ){
      // Retrieve bus object
      Bus const* pBus;
      _pws->getBus(comp.second, pBus);

      // Create generator object
      Generator gen;
      gen.extId = rand()%1000;
      gen.busExtId = pBus->extId;
      _pws->addGen(gen);
      cout << "Generator created." << endl;
      _smd->addGenElement( gen );
    }
    _state = SCHEDIT_STATE_IDLE; }
    break;

  case SCHEDIT_STATE_PRELOAD:{
    pair<int,int> comp = _getPointedComponent( QPointF(_xclick, _yclick) );
    if( comp.first == PWSMODELELEMENTTYPE_BUS ){
      // Retrieve bus object
      Bus const* pBus;
      _pws->getBus(comp.second, pBus);

      // Create load object
      Load load;
      load.extId = rand()%1000;
      load.busExtId = pBus->extId;
      _pws->addLoad(load);
      cout << "Load created." << endl;
      _smd->addLoadElement( load );
    }
    _state = SCHEDIT_STATE_IDLE; }
    break;

  } // end of switch(_state)

  repaint();

  return QFrame::mousePressEvent(event);
}

void SchematicEditor::mouseMoveEvent(QMouseEvent* event){

  if ( _isInvertible ){
    QPointF temp = _inverseTransform.map( event->localPos() );
    _xmouse = temp.x();
    _ymouse = temp.y();
  } else{
    // Should never happen!
    return;
  }

  switch( _state ){

  case SCHEDIT_STATE_PAN:
    _xref += (_xmouse - _xclick);
    _yref += (_ymouse - _yclick);
    break;

  case SCHEDIT_STATE_MOVE:
    int k;
    PwsSchematicModelElement* cdBus;
    PwsSchematicModelElement* cdEl;
//    int ans;
    int busIntId, elExtId;
    set<size_t> busElMap;
    set<size_t>::iterator i;

    float dx = _xmouse - _xclick;
    float dy = _ymouse - _yclick;
    // Necessary! (in fragkis' notation this would be x/y-anchor)
    _xclick = _xmouse;
    _yclick = _ymouse;

    // Cycle through all selected components
    for( k = 0 ; k != _selected.size(); ++k ){
      // Only buses are directly moveable - if other type of comp, continue
      if ( _selected.at(k).first != PWSMODELELEMENTTYPE_BUS )
        continue;

      // Retrieve component element
      cdBus = _smd->element( PWSMODELELEMENTTYPE_BUS, _selected.at(k).second );
      if ( cdBus == NULL )
        cout << "Bus update failed! extId = " << _selected.at(k).second << endl;
      // Displace bus
      cdBus->x1 += dx; cdBus->y1 += dy;
      cdBus->x2 += dx; cdBus->y2 += dy;

      // Find internal index of bus in the powersystem
      busIntId = _pws->getBus_intId( _selected.at(k).second );

      // --- Move all branches attached to the bus ---
      busElMap = _pws->getBusBrMap(busIntId);
      for ( i = busElMap.begin() ; i != busElMap.end() ; ++i ){
        Branch const* pBr;
        elExtId = _pws->getBr_extId( *i );
        if ( _pws->getBranch( elExtId, pBr ) )
          cout << "Powersystem::getBranch() failed" << endl;

        cdEl = _smd->element( PWSMODELELEMENTTYPE_BR, elExtId );
        if ( cdEl == NULL )
          cout << "Branch update failed! extId=" << elExtId << endl;
        // Displace branch
        if ( pBr->fromBusExtId == static_cast<int>(cdBus->extId) ){
          // Bus moved is at the from end of the branch
          cdEl->x1 += dx;
          cdEl->y1 += dy;
        }
        if ( pBr->toBusExtId == static_cast<int>(cdBus->extId) ){
          // Bus moved is at the to end of the branch
          cdEl->x2 += dx;
          cdEl->y2 += dy;
        }
      }

      // --- Move all generators attached to the bus ---
      busElMap = _pws->getBusGenMap(busIntId);
      for ( i = busElMap.begin() ; i != busElMap.end() ; ++i ){
        elExtId = _pws->getGen_extId( *i );
        cdEl = _smd->element( PWSMODELELEMENTTYPE_GEN, elExtId );
        if ( cdEl == NULL )
          cout << "Gen update failed! extId="<< elExtId << endl;
        // Displace generator
        cdEl->x1 += dx;
        cdEl->y1 += dy;
        cdEl->x2 += dx;
        cdEl->y2 += dy;
      }

      // --- Move all loads attached to the bus ---
      busElMap = _pws->getBusLoadMap(busIntId);
      for ( i = busElMap.begin() ; i != busElMap.end() ; ++i ){
        elExtId = _pws->getLoad_extId( *i );
        cdEl = _smd->element( PWSMODELELEMENTTYPE_LOAD, elExtId );
        if ( cdEl == NULL )
          cout << "Load update failed! extId="<< elExtId << endl;
        // Displace Load
        cdEl->x1 += dx;
        cdEl->y1 += dy;
        cdEl->x2 += dx;
        cdEl->y2 += dy;
      }
    }
    break;

  } // end of switch(case)

  repaint();

  return QFrame::mouseMoveEvent(event);
}

void SchematicEditor::mouseReleaseEvent(QMouseEvent* event){

  if ( _isInvertible ){
    QPointF temp = _inverseTransform.map( event->localPos() );
    _xmouse = temp.x();
    _ymouse = temp.y();
  } else{
    // Should never happen!
    return;
  }

  switch( _state ){

  case SCHEDIT_STATE_PAN:
    _state = SCHEDIT_STATE_PREPAN;
    break;

  case SCHEDIT_STATE_ZOOM:
    _state = SCHEDIT_STATE_PREZOOM;
    break;

  case SCHEDIT_STATE_SELECT:
    _selected = _getPointedComponents(QPointF(_xclick,_yclick),
                                     QPointF(_xmouse,_ymouse));
    _state = SCHEDIT_STATE_IDLE;
    break;

  case SCHEDIT_STATE_MOVE:
    _state = SCHEDIT_STATE_IDLE;
    break;

  } // end of switch( _state )

  repaint();

  return QFrame::mouseReleaseEvent(event);
}

void SchematicEditor::wheelEvent(QWheelEvent* event){

  // event->delta() is in [eightths of a degree]
  int numDegrees = event->delta() / 8;
  // normal mice have wheel steps of [15 degrees]
  int numSteps = numDegrees / 15;

  switch( _state ){

  case SCHEDIT_STATE_PREPAN:
    // do nothing!
    break;

  case SCHEDIT_STATE_IDLE:
  case SCHEDIT_STATE_SELECT:
  case SCHEDIT_STATE_MOVE:
  case SCHEDIT_STATE_PREGEN:
  case SCHEDIT_STATE_PRELOAD:
  case SCHEDIT_STATE_PREBUS:
  case SCHEDIT_STATE_PREBRFR:
  case SCHEDIT_STATE_PREBRTO:
    // Pan view
    if (event->orientation() == Qt::Horizontal){
      // Scroll horizontally
      _xref += numSteps * 5;
    } else{ // (event->orientation() == Qt::Vertical)
      // Scroll vertically
      _yref += numSteps * 5;
    }
    break;

  case SCHEDIT_STATE_PREZOOM:
  case SCHEDIT_STATE_ZOOM:
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
