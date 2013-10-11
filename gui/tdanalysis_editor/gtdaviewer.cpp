
#include "gtdaviewer.h"
using namespace elabtsaot;

#include "branch.h"
#include "powersystem.h"
#include "pwsschematicmodel.h"

#include <QPainter>
#include <QPaintEvent>

//#include <map>
using std::map;
using std::pair;

GTDAViewer::GTDAViewer( Powersystem const* pws, PwsSchematicModel const* smd,
                        QWidget* parent ) :
    SchematicViewer(pws, smd, parent),
    _pws(pws), _smd(smd), _mode(GTDAVIEWER_MODE_IDLE){}

int GTDAViewer::init(){
  // Clear internal graphical
  _brGenStab.clear();
  _cctMap.clear();
  enterIdleMode();
  return 0;
}

void GTDAViewer::paintEvent(QPaintEvent* event){

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

  switch ( _mode ){
  case GTDAVIEWER_MODE_IDLE:
    break;
  case GTDAVIEWER_MODE_BRSTABCHECK:
    _drawBrStabCheck( painter );
    _drawBrStabCheckSelectionMarkers( painter );
    break;
  case GTDAVIEWER_MODE_CCTCHECK:
    _drawCCTCheck( painter );
    break;
  default:
    // Do nothing
    break;
  }

  event->accept();
}

void GTDAViewer::enterIdleMode() {
  _mode = GTDAVIEWER_MODE_IDLE;
  repaint(); // update widget
  return;
}

void GTDAViewer::enterBrStabCheckMode(
  map<unsigned int,map<unsigned int,bool> > brGenStab){

  _mode = GTDAVIEWER_MODE_BRSTABCHECK;
  _brGenStab = brGenStab;

//  // DEBUG ------------
//  cout << "GTDAViewer::enterBrStabCheckMode()" << endl;
//  for ( map<unsigned int,map<unsigned int,bool> >::const_iterator itBr = _brGenStab.begin() ;
//        itBr != _brGenStab.end() ; ++itBr ){
//    cout << "Generator stability for branch " << itBr->first << ": " << endl;
//    for ( map<unsigned int,bool>::const_iterator itGen = (itBr->second).begin() ;
//          itGen != (itBr->second).end() ; ++itGen )
//      cout << "gen" << itGen->first << ": " << itGen->second << "; ";
//    cout << endl;
//  }
//  // END OF DEBUG -----

  repaint(); // update widget
  return;
}

void GTDAViewer::enterCCTCheckMode(
  std::map<unsigned int, std::pair<double,double> > cctMap ){
  _mode = GTDAVIEWER_MODE_CCTCHECK;
  _cctMap = cctMap;

  repaint(); // update widget
  return;
}

void GTDAViewer::_drawBrStabCheck( QPainter& painter ){

  for ( map<unsigned int,map<unsigned int,bool> >::const_iterator itBr = _brGenStab.begin() ;
        itBr != _brGenStab.end() ; ++itBr ){

    // Determine overall pws stability for fault on branch extId #(itBr->first)
    bool pwsStable = true;
    for ( map<unsigned int,bool>::const_iterator itGen = (itBr->second).begin() ;
          itGen != (itBr->second).end() ; ++itGen ){
      if (!itGen->second){
        pwsStable = false;
        break;
      }
    }

    // Draw branch stability marker
    PwsSchematicModelElement const*
        cd = _smd->element(PWSMODELELEMENTTYPE_BR, itBr->first);
    _drawBrStabMarker(painter, *cd, pwsStable );
  }

  return;
}

void GTDAViewer::_drawBrStabCheckSelectionMarkers( QPainter& painter ){


  // brStabCheck Selection markers are drawn only if:
  // - exactly one component is selected
  // - the selected component is a branch
  if ( _selected.size() != 1)
    return;
  if ( _selected[0].first != PWSMODELELEMENTTYPE_BR )
    return;

  painter.save(); // save painter options

  map<unsigned int, bool> genStable = _brGenStab[_selected[0].second];
  for ( map<unsigned int,bool>::const_iterator itGen = genStable.begin() ;
        itGen != genStable.end() ; ++itGen ){
    PwsSchematicModelElement const*
        cd = _smd->element( PWSMODELELEMENTTYPE_GEN, itGen->first );
    _drawGenStabMarker(painter, *cd, itGen->second);
  }

  painter.restore(); // restore old painter options

  return;
}

void GTDAViewer::_drawBrStabMarker( QPainter& painter,
                                    PwsSchematicModelElement const& brCd,
                                    bool isStable ){

  // Assert that argument passed is indeed a branch
  if (brCd.type != PWSMODELELEMENTTYPE_BR ) return;

  // Change painter options according to isStable flag
  painter.save();
  QPen* pen = new QPen( isStable ? QColor(0,255,0, 128) : QColor(255,0,0, 128) );
  pen->setWidth( 5 );
//  pen->setStyle( Qt::DashLine );
  painter.setPen( *pen );

  // Draw component
  QPoint P0( brCd.x1, brCd.y1 );
  QPoint P1( brCd.x2, brCd.y2 );
  painter.drawLine( P0, P1 );

  // Restore painter options
  painter.restore();

  return;
}

void GTDAViewer::_drawGenStabMarker( QPainter& painter,
                                     PwsSchematicModelElement const& genCd,
                                     bool isStable ){
  // Assert that argument passed is indeed a branch
  if ( genCd.type != PWSMODELELEMENTTYPE_GEN ) return;

  // Find bounding box and augment by 2
  float top = qMin( genCd.y1, genCd.y2 );
  float bot = qMax( genCd.y1, genCd.y2 );
  float lef = qMin( genCd.x1, genCd.x2 );
  float rig = qMax( genCd.x1, genCd.x2 );
  QRect rect = QRect( QPoint(lef-2,top-2), QPoint(rig+2,bot+2) );

  // Draw selection rectangle
  painter.save();
  QBrush brush( isStable ? QColor(0,255,0, 128) :  QColor(255,0,0, 128) );
//  painter.setBrush( brush );
  painter.fillRect( rect, brush );
  painter.restore();

  return;
}

void GTDAViewer::_drawCCTCheck( QPainter& painter ){

  for ( map<unsigned int, pair<double,double> >::const_iterator itBr = _cctMap.begin() ;
        itBr != _cctMap.end() ; ++itBr ){

    // Retrieve CCT min & max for each branch
    double cctMin = itBr->second.first;
    double cctMax = itBr->second.second;

    // Draw CCT branch marker
    PwsSchematicModelElement const*
        cd = _smd->element(PWSMODELELEMENTTYPE_BR, itBr->first);
    _drawCCTBrMarker( painter, *cd, cctMin, cctMax);
  }

  return;
}

void GTDAViewer::_drawCCTBrMarker( QPainter& painter,
                                   PwsSchematicModelElement const& brCd,
                                   double cctMin, double cctMax ){

  // Assert that argument passed is indeed a branch
  if (brCd.type != PWSMODELELEMENTTYPE_BR ) return;

  // Save painter options
  painter.save();

  // Draw CCT min marker
  QPoint P0( brCd.x1, brCd.y1 );
  QPoint P1( brCd.x2, brCd.y2 );
  QPointF Pmin( (P0+P1)/2 + QPointF(15,0) );
  QString CCTMinLabel;
  CCTMinLabel.sprintf("%3.0f ms",cctMin*1000.0);
//  QString CCTMinLabel = QString("%0 ms").arg(cctMin*1000.0);
  QPen* penMin = new QPen( QColor(0,255,0, 128) );
  painter.setPen( *penMin );
  painter.drawText( Pmin, CCTMinLabel);

  // Draw CCT max marker
  QPointF Pmax( Pmin + QPointF(0,15) );
  QString CCTMaxLabel;
  CCTMaxLabel.sprintf("%3.0f ms",cctMax*1000.0);
//  QString CCTMaxLabel = QString("%0 ms").arg(cctMax*1000.0);
  QPen* penMax = new QPen( QColor(255,0,0, 128) );
  painter.setPen( *penMax );
  painter.drawText( Pmax, CCTMaxLabel );

  // Restore painter options
  painter.restore();

  return;
}
