
#include "fitterslicewidget.h"
using namespace elabtsaot;

#include "slice.h"
#include "emulatorbranch.h"

#include <QPainter>
#include <QPaintEvent>

using std::vector;

FitterSliceWidget::FitterSliceWidget( Slice* slc,
                                      QWidget* parent ):
    SliceWidget( slc, parent ){}

bool FitterSliceWidget::_isShowingReal = true;

void FitterSliceWidget::paintEvent(QPaintEvent *event){
  // --- Set up painter ---
  QPainter painter(this);

  // Draw real mark on top left of the widget - pseudostatic
  _drawRealMark( painter, QPoint(2,2) );

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
  vector<vector<QPointF> >
  nodeOrigin = vector<vector<QPointF> >( _rows,vector<QPointF>( _cols,QPointF()));
  vector<bool> embr_exist;
  EmulatorBranch embr;
  QPointF embrP; // starting drawing point of the emulator branch
  QPointF embrQ; // ending drawing point of the emulator branch
  QPointF embrOffsetX = QPointF( 5, 0 );
  QPointF embrOffsetY = QPointF( 0, -5 );

  for(size_t k = 0; k != _rows; ++k){
    for(size_t m = 0; m != _cols; ++m){

      // Draw node
      nodeOrigin[k][m] = QPointF( (m+1)*_step, (_rows - k)*_step );
      _drawNode( painter, nodeOrigin[k][m], k, m);

      // Draw atom emulator branches
      Atom const* am = _slc->ana.getAtom(k,m);
      embr_exist = am->embr_exist();

      if ( embr_exist[EMBRPOS_U] ){
        embr = am->embr(EMBRPOS_U, _isShowingReal);
        embrP = nodeOrigin[k][m] + embrOffsetY;
        embrQ = nodeOrigin[k][m] + QPointF(0, -125);

        _drawFitterBranch( painter, embrP, embrQ, embr );
      }
      if ( embr_exist[EMBRPOS_UR] ){
        embr = am->embr(EMBRPOS_UR, _isShowingReal);
        embrP = nodeOrigin[k][m] + embrOffsetX + embrOffsetY;
        embrQ = nodeOrigin[k][m] + QPointF(125, -125);

        _drawFitterBranch( painter, embrP, embrQ, embr );
      }
      if ( embr_exist[EMBRPOS_R] ){
        embr = am->embr(EMBRPOS_R, _isShowingReal);
        embrP = nodeOrigin[k][m] + embrOffsetX;
        embrQ = nodeOrigin[k][m] + QPointF(125, 0);

        _drawFitterBranch( painter, embrP, embrQ, embr );
      }
      if ( embr_exist[EMBRPOS_DR] ){
        embr = am->embr(EMBRPOS_DR, _isShowingReal);
        embrP = nodeOrigin[k][m] + embrOffsetX - embrOffsetY;
        embrQ = nodeOrigin[k][m] + QPointF(125, 125);

        _drawFitterBranch( painter, embrP, embrQ, embr );
      }
      if ( embr_exist[EMBRPOS_D] ){
        embr = am->embr(EMBRPOS_D, _isShowingReal);
        embrP = nodeOrigin[k][m] - embrOffsetY;
        embrQ = nodeOrigin[k][m] + QPointF(0, 125);

        _drawFitterBranch( painter, embrP, embrQ, embr );
      }
      if ( embr_exist[EMBRPOS_DL] ){
        embr = am->embr(EMBRPOS_DL, _isShowingReal);
        embrP = nodeOrigin[k][m] - embrOffsetX - embrOffsetX;
        embrQ = nodeOrigin[k][m] + QPointF(-125, 125);

        _drawFitterBranch( painter, embrP, embrQ, embr );
      }
      if ( embr_exist[EMBRPOS_L] ){
        embr = am->embr(EMBRPOS_L, _isShowingReal);
        embrP = nodeOrigin[k][m] - embrOffsetX;
        embrQ = nodeOrigin[k][m] + QPointF(-125, 0);

        _drawFitterBranch( painter, embrP, embrQ, embr );
      }
      if ( embr_exist[EMBRPOS_UL] ){
        embr = am->embr(EMBRPOS_UL, _isShowingReal);
        embrP = nodeOrigin[k][m] - embrOffsetX + embrOffsetX;
        embrQ = nodeOrigin[k][m] + QPointF(-125, -125);

        _drawFitterBranch( painter, embrP, embrQ, embr );
      }
    }
  }

  event->accept();
}

// TODO: change into something less hardcoded!
void FitterSliceWidget::mousePressEvent(QMouseEvent* event){

  SliceWidget::mousePressEvent(event);
  // parent protected methods _xclick & _yclick contain the coordinates of the
  // click event

  /*
    the fitter slice widget is meshed in [x x y] = [step x step] = [120 x 120]
    blocks that represent the atoms of the emulator

    e.g. for 4x6 grid

           -5  115  235  355  475  595  715

            |    |    |    |    |    |    |
    -125   -|----|----|----|----|----|----|-
            | 19 | 20 | 21 | 22 | 23 | 24 |
      -5   -|----|----|----|----|----|----|-
            | 13 | 14 | 15 | 16 | 17 | 18 |
     115   -|----|----|----|----|----|----|-
            | 07 | 08 | 09 | 10 | 11 | 12 |
     235   -|----|----|----|----|----|----|-
            | 01 | 02 | 03 | 04 | 05 | 06 |
     355   -|----|----|----|----|----|----|-
            |    |    |    |    |    |    |
  */

  // Get which block was clicked
  int xblock = floor((_xclick + 10)/_step) - 1;
  int yblock = (_rows - 1) - floor((_yclick - 10) / _step);
  if ( yblock >= -1 && yblock <= static_cast<int>(_rows)-1 )
    if ( xblock == -1)
      xblock = 0;
  if ( xblock >= 0 && xblock <= static_cast<int>(_cols)-1 )
    if ( yblock == -1 )
      yblock = 0;

  // If block clicked is outside slice ana._atomSet size then return without doing
  // anything
  if (    xblock < 0 || xblock >= static_cast<int>(_cols)
       || yblock < 0 || yblock >= static_cast<int>(_rows) ){
    return;
  }

  // Get block relative coordinates; origin at node center, x increasing to
  // embr_R and y increasing to embr_U
  float xrel = _xclick - (xblock+1)*_step;
  float yrel = - (_yclick - (_rows-yblock)*_step);

  // Find out what was clicked (if anything)
  int emulator_tab = _sliceIndex;
  int emulator_row = yblock;
  int emulator_col = xblock;

  /*
    emulator_elm follows the following convention for the element clicked:
    0b0..0zyyyxxx

    z   - 0: non-node; 1: node
    yyy - 0: near pot sw; 1: near pot res; 2: far pot sw; 3: far pot res;
          4: short circt sw; 5: mid gnd sw
    xxx - embr pos according to EmbrPosition 0: EMBRPOS_ U; 1: UR ...; 7: UL
  */
  int emulator_elm = -1;

  QPointF point_rel = QPointF(xrel, yrel);
  QPointF point_node = QPointF(0,0);

  // ----- NODE -----
  if ( QLineF(point_rel, point_node).length() <= 5 ){
    // Node clicked
    emulator_elm = 0;
    emulator_elm |= (1 << 6);
  }

  // ----- EMBRPOS_U -----
  else if ( xrel >= -2.5 && xrel <= 2.5 && yrel >= 5+5 && yrel <= 10+5 ) {
    // Embr EMBRPOS_U near pot sw clicked
    emulator_elm = 0;
    emulator_elm |= EMBRPOS_U;
    emulator_elm |= (0 << 3);

  } else if ( xrel >= -5 && xrel <= 5 && yrel >= 15+5 && yrel <= 55+5 ) {
    // Embr EMBRPOS_U near pot res clicked
    emulator_elm = 0;
    emulator_elm |= EMBRPOS_U;
    emulator_elm |= (1 << 3);

  } else if ( xrel >= -2.5 && xrel <= 2.5 && yrel >= 110+5 && yrel <= 115+5 ) {
    // Embr EMBRPOS_U far pot sw clicked
    emulator_elm = 0;
    emulator_elm |= EMBRPOS_U;
    emulator_elm |= (2 << 3);

  } else if ( xrel >= -5 && xrel <= 5 && yrel >= 65+5 && yrel <= 105+5 ) {
    // Embr EMBRPOS_U far pot res clicked
    emulator_elm = 0;
    emulator_elm |= EMBRPOS_U;
    emulator_elm |= (3 << 3);

  } else if ( xrel >= -10 && xrel <= -5 && yrel >= 57.5+5 && yrel <= 62.5+5 ) {
    // Embr EMBRPOS_U short circuiting sw clicked
    emulator_elm = 0;
    emulator_elm |= EMBRPOS_U;
    emulator_elm |= (4 << 3);

  } else if ( xrel >= 5 && xrel <= 10 && yrel >= 57.5+5 && yrel <= 62.5+5 ) {
    // Embr EMBRPOS_U mid grounding sw clicked
    emulator_elm = 0;
    emulator_elm |= EMBRPOS_U;
    emulator_elm |= (5 << 3);
  }

  // ----- EMBRPOS_UR -----
  else if ( QLineF( point_rel,QPointF(12.5,12.5) ).length() <= 2.5 ) {
    // Embr EMBRPOS_UR near pot sw clicked
    emulator_elm = 0;
    emulator_elm |= EMBRPOS_UR;
    emulator_elm |= (0 << 3);

  } else if ( QLineF( point_rel,QPointF(40.0,40.0) ).length() <= 5 ) {
    // Embr EMBRPOS_UR near pot res clicked
    emulator_elm = 0;
    emulator_elm |= EMBRPOS_UR;
    emulator_elm |= (1 << 3);

  } else if ( QLineF( point_rel,QPointF(117.5,117.5) ).length() <= 2.5 ) {
    // Embr EMBRPOS_UR far pot sw clicked
    emulator_elm = 0;
    emulator_elm |= EMBRPOS_UR;
    emulator_elm |= (2 << 3);

  } else if ( QLineF( point_rel,QPointF(90.0,90.0) ).length() <= 5 ) {
    // Embr EMBRPOS_UR far pot res clicked
    emulator_elm = 0;
    emulator_elm |= EMBRPOS_UR;
    emulator_elm |= (3 << 3);

  } else if ( QLineF( point_rel,QPointF(57.0,72.0) ).length() <= 2.5 ) {
    // Embr EMBRPOS_UR short circuiting sw clicked
    emulator_elm = 0;
    emulator_elm |= EMBRPOS_UR;
    emulator_elm |= (4 << 3);

  } else if ( QLineF( point_rel,QPointF(72.0,57.0) ).length() <= 2.5 ) {
    // Embr EMBRPOS_UR mid grounding sw cliked
    emulator_elm = 0;
    emulator_elm |= EMBRPOS_UR;
    emulator_elm |= (5 << 3);
  }

  // ----- EMBRPOS_R -----
  else if ( xrel >= 5+5 && xrel <= 10+5 && yrel >= -2.5 && yrel <= 2.5 ) {
    // Embr EMBRPOS_R near pot sw clicked
    emulator_elm = 0;
    emulator_elm |= EMBRPOS_R;
    emulator_elm |= (0 << 3);

  } else if ( xrel >= 15+5 && xrel <= 55+5 && yrel >= -5 && yrel <= 5 ) {
    // Embr EMBRPOS_R near pot res clicked
    emulator_elm = 0;
    emulator_elm |= EMBRPOS_R;
    emulator_elm |= (1 << 3);

  } else if ( xrel >= 110+5 && xrel <= 115+5 && yrel >= -2.5 && yrel <= 2.5 ) {
    // Embr EMBRPOS_R far pot sw clicked
    emulator_elm = 0;
    emulator_elm |= EMBRPOS_R;
    emulator_elm |= (2 << 3);

  } else if ( xrel >= 65+5 && xrel <= 105+5 && yrel >= -5 && yrel <= 5 ) {
    // Embr EMBRPOS_R far pot res clicked
    emulator_elm = 0;
    emulator_elm |= EMBRPOS_R;
    emulator_elm |= (3 << 3);

  } else if ( xrel >= 57.5+5 && xrel <= 62.5+5 && yrel >= 5 && yrel <= 10 ) {
    // Embr EMBRPOS_R short circuiting sw clicked
    emulator_elm = 0;
    emulator_elm |= EMBRPOS_R;
    emulator_elm |= (4 << 3);

  } else if ( xrel >= 57.5+5 && xrel <= 62.5+5 && yrel >= -10 && yrel <= -5 ) {
    // Embr EMBRPOS_R mid grounding sw cliked
    emulator_elm = 0;
    emulator_elm |= EMBRPOS_R;
    emulator_elm |= (5 << 3);
  }

  // ----- EMBRPOS_D -----
  else if ( xrel >= -2.5 && xrel <= 2.5 && yrel >= -10-5 && yrel <= -5-5 ) {
    // Embr EMBRPOS_D near pot sw clicked
    emulator_elm = 0;
    emulator_elm |= EMBRPOS_D;
    emulator_elm |= (0 << 3);

  } else if ( xrel >= -5 && xrel <= 5 && yrel >= -55-5 && yrel <= -15-5 ) {
    // Embr EMBRPOS_D near pot res clicked
    emulator_elm = 0;
    emulator_elm |= EMBRPOS_D;
    emulator_elm |= (1 << 3);

  } else if ( xrel >= -2.5 && xrel <= 2.5 && yrel >= -115-5 && yrel <= -110-5 ) {
    // Embr EMBRPOS_D far pot sw clicked
    emulator_elm = 0;
    emulator_elm |= EMBRPOS_D;
    emulator_elm |= (2 << 3);

  } else if ( xrel >= -5 && xrel <= 5 && yrel >= -105-5 && yrel <= -65-5 ) {
    // Embr EMBRPOS_D far pot res clicked
    emulator_elm = 0;
    emulator_elm |= EMBRPOS_D;
    emulator_elm |= (3 << 3);

  } else if ( xrel>=5.0 && xrel<=10.0 && yrel>=-62.5-5 && yrel<=-57.5-5 ){
    // Embr EMBRPOS_D short circuiting sw clicked
    emulator_elm = 0;
    emulator_elm |= EMBRPOS_D;
    emulator_elm |= (4 << 3);

  } else if ( xrel>=-10.0 && xrel<=-5.0 && yrel>=-62.5-5 && yrel<=-57.5-5 ) {
    // Embr EMBRPOS_D mid grounding sw cliked
    emulator_elm = 0;
    emulator_elm |= EMBRPOS_D;
    emulator_elm |= (5 << 3);
  }

  // ----- EMBRPOS_L -----
  else if ( xrel >= -10-5 && xrel <= -5-5 && yrel >= -2.5 && yrel <= 2.5 ) {
    // Embr EMBRPOS_L near pot sw clicked
    emulator_elm = 0;
    emulator_elm |= EMBRPOS_L;
    emulator_elm |= (0 << 3);

  } else if ( xrel >= -55-5 && xrel <= -15-5 && yrel >= -5 && yrel <= 5 ) {
    // Embr EMBRPOS_L near pot res clicked
    emulator_elm = 0;
    emulator_elm |= EMBRPOS_L;
    emulator_elm |= (1 << 3);

  } else if ( xrel >= -115-5 && xrel <= -110-5 && yrel >= -2.5 && yrel <= 2.5 ) {
    // Embr EMBRPOS_L far pot sw clicked
    emulator_elm = 0;
    emulator_elm |= EMBRPOS_L;
    emulator_elm |= (2 << 3);

  } else if ( xrel >= -105-5 && xrel <= -65-5 && yrel >= -5 && yrel <= 5 ) {
    // Embr EMBRPOS_L far pot res clicked
    emulator_elm = 0;
    emulator_elm |= EMBRPOS_L;
    emulator_elm |= (3 << 3);

  } else if ( xrel >= -62.5-5 && xrel <= -57.5-5 && yrel>=-10.0 && yrel<=-5.0 ){
    // Embr EMBRPOS_L short circuiting sw clicked
    emulator_elm = 0;
    emulator_elm |= EMBRPOS_L;
    emulator_elm |= (4 << 3);

  } else if ( xrel >= -62.5-5 && xrel <= -57.5-5 && yrel >= 5 && yrel <= 10 ) {
    // Embr EMBRPOS_L mid grounding sw cliked
    emulator_elm = 0;
    emulator_elm |= EMBRPOS_L;
    emulator_elm |= (5 << 3);
  }

  // Sanitize emulator_elm based on atom position
  if ( emulator_elm >= 0 ){
    int isNode = ((1<<6) & emulator_elm);           // extracting z
    if ( !isNode ){
      int embrPos = (7 & emulator_elm);               // extracting xxx
//      int embrElm = (((7<<3) & emulator_elm) >> 3);   // extracting yyy
      switch ( embrPos ){
      case EMBRPOS_U:
        // All atoms have embr at pos EMBRPOS_U
        break;
      case EMBRPOS_UR:
        // All atoms have embr at pos EMBRPOS_UR
        break;
      case EMBRPOS_R:
        // All atoms have embr at pos EMBRPOS_R
        break;
      case EMBRPOS_D:
        // Only atoms in the bottom row and not in the first or last column have
        // embr at pos EMBRPOS_D
        if (    emulator_row != 0
             || emulator_col == 0
             || emulator_col == static_cast<int>(_cols)-1 )
          emulator_elm = -1;
        break;
      case EMBRPOS_L:
        // Only atoms in the first column and not in the last row have embr at
        // pos EMBRPOS_L
        if (    emulator_col != 0
             || emulator_row == static_cast<int>(_rows)-1 )
          emulator_elm = -1;
        break;
      default:
        // Should never be reached!
        emulator_elm = -1;
        break;
      }
    }
  }

  // Emit fittingPositionClicked signal
  if (    emulator_tab >= 0
       && emulator_row >= 0
       && emulator_col >= 0
       && emulator_elm >= 0 ){
    emit fittingPositionClicked( emulator_tab,
                                 emulator_row,
                                 emulator_col,
                                 emulator_elm );
  }

  event->accept();

  return;
}

void FitterSliceWidget::toggleReal(){
  _isShowingReal = !_isShowingReal;
  update();
}

void FitterSliceWidget::toggleReal(bool showReal){
  _isShowingReal = showReal;
  update();
}

bool FitterSliceWidget::isShowingReal() const{ return _isShowingReal; }

void FitterSliceWidget::_drawRealMark(QPainter& painter, QPoint topleft){

  painter.save();
  painter.fillRect( QRect( topleft, QSize(55,20) ), Qt::cyan );
  painter.setPen(QPen(Qt::darkCyan, 2));
  painter.setFont(QFont("Times", 10, -1, true));
  painter.drawText( QRect(topleft+QPoint(5,5),QSize(45,10)),
                    QString(_isShowingReal?"Real":"Imag"),
                    QTextOption(Qt::AlignCenter) );
  painter.restore();

  return;
}

// Draw emulator branch between points P and Q as shown below
/*
    -------------------------/SW/------------------------
  /                                                       \
 /                                                         \
o-----/SW/-|NearResistor|-----o------|FarResistor|-/SW/-----o
p                             |                             q
                             /SW/
                             _|_
                              _
*/
void FitterSliceWidget::_drawFitterBranch(QPainter& painter,
                                          QPointF p, QPointF q,
                                          EmulatorBranch const& embr){
  // Push current painter on stack
  painter.save();

  // Modify painter to draw emulator branches for the fitter
  painter.setPen( QPen() );
  QBrush on = QBrush();
  on.setStyle(Qt::SolidPattern);
  on.setColor(QColor(0,255,0));
  QBrush off = QBrush();
  off.setStyle(Qt::SolidPattern);
  off.setColor(QColor(255,0,0));
  painter.setBrush( QBrush() );
  painter.setFont( QFont("Times",10) );

  // Set transformation of coordinate system
  float angle;
  if ( q.x() != p.x() ){
    // Emulator branch is not vertical
    if ( q.y() == p.y() ){
      // Horizontal emulator branch
      if ( q.x() >= p.x() ){
        // Emulator branch pointing to the right (x+)
        angle = 0;
      } else { // q.x() < p.x()
        // Emulator branch pointing to the left (x-)
        angle = 180;
      }
    } else {
      // Non-horizontal emulator branch
      angle = atan( (q-p).y()/(q-p).x()  )*180./M_PI;
    }
  } else if ( q.y() < p.y() ){ // && q.x() == p.x()
    // Emulator branch is vertical - pointing up
    angle = -90; // counter-clockwise rotation
  } else if ( q.y() > p.y() ){ // && q.x() == p.x()
    // Emulator branch is vertical - pointing down
    angle = 90; // clockwise rotation
  } else { // q.x() == p.x() && q.y() == p.x()
    // Error! Starting and ending point coincide
    return;
  }
  float scale = sqrt( (q-p).x()*(q-p).x() + (q-p).y()*(q-p).y() )/120.;
  QTransform transform = painter.transform();
  transform.translate(p.x(), p.y());
  transform.scale( scale, scale );
  transform.rotate( angle );
  painter.setTransform( transform );

  // Main part of emulator branch
  int pot_near_r = static_cast<int>( embr.pot_near_r() );
  int pot_far_r = static_cast<int>( embr.pot_far_r() );

  painter.drawLine( QPointF(0,0), QPointF(5,0) );

  painter.save();
  painter.setBrush( embr.pot_near_sw() ? on : off );
  painter.drawRect( QRect(5,-2.5,5,5 ) );
  painter.restore();

  painter.drawLine( QPointF(10,0), QPointF(15,0));
  painter.drawRect( QRect(15,-5,40,10) );
  painter.drawText( QRect(15,-5,40,10), Qt::AlignCenter,
                    QString("%0").arg(pot_near_r) );

  painter.drawLine( QPointF(55,0), QPointF(65,0) );

  painter.drawRect( QRect(65,-5,40,10) );
  painter.drawText( QRect(65,-5,40,10), Qt::AlignCenter,
                    QString("%0").arg(pot_far_r) );
  painter.drawLine( QPointF(105,0), QPointF(110,0) );

  painter.save();
  painter.setBrush( embr.pot_far_sw() ? on : off );
  painter.drawRect( QRect(110,-2.5, 5,5) );
  painter.restore();

  painter.drawLine( QPointF(115,0), QPointF(120,0) );

  // Grounding switch
  painter.drawLine( QPointF(60,0), QPointF(60,5) );
  painter.save();
  painter.setBrush( embr.sw_mid()? on : off );
  painter.drawRect( QRect(57.5,5,5,5) );
  painter.restore();
  painter.drawLine( QPointF(60,10), QPointF(60,15) );
  painter.drawLine( QPointF(55,15), QPointF(65,15) );
  painter.drawLine( QPointF(58,22), QPointF(62,22) );

  // Short-circuiting branch
  painter.drawLine( QPointF(0,0), QPointF(7.5,-7.5) );
  painter.drawLine( QPointF(7.5,-7.5), QPointF(57.5,-7.5));
  painter.save();
  painter.setBrush( embr.sw_sc() ? on : off );
  painter.drawRect( QRect(57.5,-10.0,5,5) );
  painter.restore();
  painter.drawLine( QPointF(62.5,-7.5), QPointF(112.5,-7.5) );
  painter.drawLine( QPointF(112.5,-7.5), QPointF(120,0) );

  // Restore painter to its previous state
  painter.restore();

  return;
}
