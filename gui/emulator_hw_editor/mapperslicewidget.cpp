
#include "mapperslicewidget.h"
using namespace elabtsaot;

#include "slice.h"
#include "powersystem.h"
#include "pwsmappermodel.h"
#include "auxiliary.h"

#include <QPainter>
#include <QPaintEvent>

#include <vector>
using std::vector;
#include <iostream>
using std::cout;
using std::endl;
#include <cmath> // for fmod(double,double)

using std::pair;

MapperSliceWidget::MapperSliceWidget( Slice const* slc,
                                      Powersystem const* pws,
                                      PwsMapperModel const* mmd,
                                      int emulatorSize,
                                      QWidget* parent ) :
    SliceWidget( slc, parent ),
    _pws(pws), _mmd(mmd), _emulatorSize(emulatorSize) {}

void MapperSliceWidget::paintEvent(QPaintEvent *event){

  // --- Get mapper info
  vector<vector<vector<int> > > hints = _mmd->hints();

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
  vector<vector<QPointF> >
  nodeOrigin = vector<vector<QPointF> >( _rows,vector<QPointF>( _cols,QPointF()));
  vector<bool> embr_exist;
  QPointF embrP; // starting drawing point of the emulator branch
  QPointF embrQ; // ending drawing point of the emulator branch
  QPointF embrOffsetX = QPointF( 5, 0 );
  QPointF embrOffsetY = QPointF( 0, -5 );
  int ndm_tab, ndm_row, ndm_col; // NoDe Mapping tab/row/column index
  int brm_tab, brm_row, brm_col; // BRanch Mapping tab/row/column index
  vector< pair<int,unsigned int> > elementsMapped;
  QString label;
  // ----
  ndm_tab = 2*_sliceIndex;
  for(size_t k = 0; k != _rows; ++k){
    ndm_row = 2*k;
    for(size_t m = 0; m != _cols; ++m){
      ndm_col = 2*m;

      // ----- Draw node -----
      nodeOrigin[k][m] = QPointF( (m+1)*_step, (_rows - k)*_step );

      label.clear();
      elementsMapped = _mmd->elementsMapped( ndm_tab, ndm_row, ndm_col);
      for ( size_t n = 0 ; n != elementsMapped.size() ; ++n ){
        switch ( elementsMapped.at(n).first ){

        case PWSMODELELEMENTTYPE_BUS:{
          Bus const* pBus;
          if (!_pws->getBus(elementsMapped.at(n).second, pBus)){
            label.append( "BUS " );
            label.append( QString::number( pBus->extId ) );
            label.append( " " );
          }
          break; }

          // Redundant! Should never be the case! Branches cannot be mapped in
          // [2*x][2*y][2*z] positions!
//        case PWSMODELELEMENTTYPE_BR:
//          Branch br;
//          if (!_pws->getBranch( elementsMapped.at(n).second, &br)){
//            label.append( "BR " );
//            label.append( QString::number( br.extId() ) );
//            label.append( " " );
//          }
//          break;

        case PWSMODELELEMENTTYPE_GEN:{
          Generator const* pGen;
          if (!_pws->getGenerator( elementsMapped.at(n).second, pGen)){
            label.append( "GEN " );
            label.append( QString::number( pGen->extId() ) );
            label.append( " " );
          }
          break; }

        case PWSMODELELEMENTTYPE_LOAD:{
          Load const* pLoad;
          if (!_pws->getLoad( elementsMapped.at(n).second, pLoad)){
            label.append( "LOAD " );
            label.append( QString::number( pLoad->extId() ) );
            label.append( " " );
          }
          break; }
        }
      }
      _drawNode( painter, nodeOrigin[k][m], k, m,
                 hints[ndm_tab][ndm_row][ndm_col], label);

      // ----- Draw atom emulator branches -----
      Atom const* am = _slc->ana.getAtom(k,m);
      embr_exist = am->embr_exist();

      if ( embr_exist[EMBRPOS_U] ){
        embrP = nodeOrigin[k][m] + embrOffsetY;
        embrQ = nodeOrigin[k][m] + QPointF(0, -125);

        if ( k == _rows-1 ){
          // Top-row atom. So EMBRPOS_U branches are interslice ones
          brm_tab = ndm_tab + 1;
          brm_row = ndm_row;
          brm_col = ndm_col;
          if ( brm_tab < static_cast<int>(hints.size()) ){
            // We are not at the top tab, so interslice connections are possible

            // Extract label of the mapped branch
            elementsMapped = _mmd->elementsMapped( brm_tab, brm_row, brm_col);
            if ( elementsMapped.size() > 1){
              cout << "More than one physical branches mapped at map position ";
              cout << "["<<brm_tab<<"]["<<brm_row<<"]["<<brm_col<<"]" << endl;
              return;
            }
            label.clear();
            for ( size_t k = 0 ; k != elementsMapped.size() ; ++k ){
              Branch const* pBr;
              if (!_pws->getBranch( elementsMapped.at(k).second, pBr)){
                label.append( "BR " );
                label.append( QString::number( pBr->extId ) );
                label.append( " " );
              }
            }

            _drawMapperBranch( painter, embrP, embrQ,
                               hints[brm_tab][brm_row][brm_col], label );

          } else{
            // We are at the top slice (tab). So interslice connections are
            // floating and are therefore drawn as inactive
            _drawMapperBranch( painter, embrP, embrQ, MAPHINT_INACTIVE );
          }

        } else{
          // Normal EMBRPOS_U branch
          brm_tab = ndm_tab;
          brm_row = ndm_row + 1;
          brm_col = ndm_col;

          // Extract label of the mapped branch
          elementsMapped = _mmd->elementsMapped( brm_tab, brm_row, brm_col);
          if ( elementsMapped.size() > 1){
            cout << "More than one physical branches mapped at map position ";
            cout << "["<<brm_tab<<"]["<<brm_row<<"]["<<brm_col<<"]" << endl;
            return;
          }
          label.clear();
          for ( size_t k = 0 ; k != elementsMapped.size() ; ++k ){
            Branch const* pBr;
            if (!_pws->getBranch( elementsMapped.at(k).second, pBr)){
              label.append( "BR " );
              label.append( QString::number(pBr->extId) );
              label.append( " " );
            }
          }

          _drawMapperBranch( painter, embrP, embrQ,
                             hints[brm_tab][brm_row][brm_col], label );
        }
      }

      if ( embr_exist[EMBRPOS_UR] ){
        embrP = nodeOrigin[k][m] + embrOffsetX + embrOffsetY;
        embrQ = nodeOrigin[k][m] + QPointF(125, -125);

        if ( k == _rows-1 && m == _cols-1 ){
          // This branch is floating! Draw it as inactive
          _drawMapperBranch( painter, embrP, embrQ, MAPHINT_INACTIVE);

        } else if ( k == _rows-1 ){
          // Top-row atom. So EMBRPOS_UR branches are interslice ones
          brm_tab = ndm_tab + 1;
          brm_row = ndm_row;
          brm_col = ndm_col + 1;
          if ( brm_tab < static_cast<int>(hints.size()) ){
            // We are not at the top tab, so interslice connections are possible

            // Extract label of the mapped branch
            elementsMapped = _mmd->elementsMapped( brm_tab, brm_row, brm_col );
            if ( elementsMapped.size() > 1){
              cout << "More than one physical branches mapped at map position ";
              cout << "["<<brm_tab<<"]["<<brm_row<<"]["<<brm_col<<"]" << endl;
              return;
            }
            label.clear();
            for ( size_t k = 0 ; k != elementsMapped.size() ; ++k ){
              Branch const* pBr;
              if (!_pws->getBranch( elementsMapped.at(k).second, pBr)){
                label.append( "BR " );
                label.append( QString::number(pBr->extId) );
                label.append( " " );
              }
            }

            _drawMapperBranch( painter, embrP, embrQ,
                               hints[brm_tab][brm_row][brm_col], label );
          } else{
            // We are at the top slice (tab). So interslice connections are
            // floating and are therefore drawn as inactive
            _drawMapperBranch( painter, embrP, embrQ, MAPHINT_INACTIVE );
          }

        } else if ( m == _cols-1 ){
          // Last-column atom. So EMBRPOS_UR branches are interslice ones
          brm_tab = ndm_tab + 1;
          brm_row = ndm_row + 1;
          brm_col = ndm_col;
          if ( brm_tab < static_cast<int>(hints.size()) ){
            // We are not at the top tab, so interslice connections are possible

            // Extract label of the mapped branch
            elementsMapped = _mmd->elementsMapped( brm_tab, brm_row, brm_col);
            if ( elementsMapped.size() > 1){
              cout << "More than one physical branches mapped at map position ";
              cout << "["<<brm_tab<<"]["<<brm_row<<"]["<<brm_col<<"]" << endl;
              return;
            }
            label.clear();
            for ( size_t k = 0 ; k != elementsMapped.size() ; ++k ){
              Branch const* pBr;
              if (!_pws->getBranch(elementsMapped.at(k).second, pBr)){
                label.append( "BR " );
                label.append( QString::number(pBr->extId) );
                label.append( " " );
              }
            }

            _drawMapperBranch( painter, embrP, embrQ,
                               hints[brm_tab][brm_row][brm_col], label );
          } else{
            // We are at the top slice (tab). So interslice connections are
            // floating and are therefore drawn as inactive
            _drawMapperBranch( painter, embrP, embrQ, MAPHINT_INACTIVE );
          }

        } else{
          // Normal EMBRPOS_UR branch
          brm_tab = ndm_tab;
          brm_row = ndm_row + 1;
          brm_col = ndm_col + 1;

          // Extract label of the mapped branch
          elementsMapped = _mmd->elementsMapped( brm_tab, brm_row, brm_col);
          if ( elementsMapped.size() > 1){
            cout << "More than one physical branches mapped at map position ";
            cout << "["<<brm_tab<<"]["<<brm_row<<"]["<<brm_col<<"]" << endl;
            return;
          }
          label.clear();
          for ( size_t k = 0 ; k != elementsMapped.size() ; ++k ){
            Branch const* pBr;
            if (!_pws->getBranch( elementsMapped.at(k).second, pBr)){
              label.append( "BR " );
              label.append( QString::number(pBr->extId) );
              label.append( " " );
            }
          }

          _drawMapperBranch( painter, embrP, embrQ,
                             hints[brm_tab][brm_row][brm_col], label );
        }
      }

      if ( embr_exist[EMBRPOS_R] ){
        embrP = nodeOrigin[k][m] + embrOffsetX;
        embrQ = nodeOrigin[k][m] + QPointF(125, 0);

        if ( k == _rows-1 && m == _cols-1 ){
          // This branch is floating! Draw it as inactive
          _drawMapperBranch( painter, embrP, embrQ,
                             MAPHINT_INACTIVE );

        } else if ( m == _cols-1 ){
          // Last-column atom. So EMBRPOS_R branches are interslice ones
          brm_tab = ndm_tab + 1;
          brm_row = ndm_row;
          brm_col = ndm_col;
          if ( brm_tab < static_cast<int>(hints.size()) ){
            // We are not at the top tab, so interslice connections are possible

            // Extract label of the mapped branch
            elementsMapped = _mmd->elementsMapped( brm_tab, brm_row, brm_col);
            if ( elementsMapped.size() > 1){
              cout << "More than one physical branches mapped at map position ";
              cout << "["<<brm_tab<<"]["<<brm_row<<"]["<<brm_col<<"]" << endl;
              return;
            }
            label.clear();
            for ( size_t k = 0 ; k != elementsMapped.size() ; ++k ){
              Branch const *pBr;
              if (!_pws->getBranch( elementsMapped.at(k).second, pBr)){
                label.append( "BR " );
                label.append( QString::number(pBr->extId) );
                label.append( " " );
              }
            }

            _drawMapperBranch( painter, embrP, embrQ,
                               hints[brm_tab][brm_row][brm_col], label );
          } else{
            // We are at the top slice (tab). So interslice connections are
            // floating and are therefore drawn as inactive
            _drawMapperBranch( painter, embrP, embrQ, MAPHINT_INACTIVE );
          }

        } else {
          // Normal EMBRPOS_R branch
          brm_tab = ndm_tab;
          brm_row = ndm_row;
          brm_col = ndm_col + 1;

          // Extract label of the mapped branch
          elementsMapped = _mmd->elementsMapped( brm_tab, brm_row, brm_col);
          if ( elementsMapped.size() > 1){
            cout << "More than one physical branches mapped at map position ";
            cout << "["<<brm_tab<<"]["<<brm_row<<"]["<<brm_col<<"]" << endl;
            return;
          }
          label.clear();
          for ( size_t k = 0 ; k != elementsMapped.size() ; ++k ){
            Branch const* pBr;
            if (!_pws->getBranch( elementsMapped.at(k).second, pBr)){
              label.append( "BR " );
              label.append( QString::number(pBr->extId) );
              label.append( " " );
            }
          }

          _drawMapperBranch( painter, embrP, embrQ,
                             hints[brm_tab][brm_row][brm_col], label );
        }
      }

      // Redundant! There are no EMBRPOS_DR branches in the current system!
//      if ( embr_exist[EMBRPOS_DR] ){
//        embrP = nodeOrigin[k][m] + embrOffsetX - embrOffsetY;
//        embrQ = nodeOrigin[k][m] + QPointF(125, 125);

//        brm_tab = ndm_tab;
//        brm_row = ndm_row - 1;
//        brm_col = ndm_col + 1;

//        _drawMapperBranch( painter, embrP, embrQ,
//                           hints[brm_tab][brm_row][brm_col] );
//      }

      if ( embr_exist[EMBRPOS_D] ){
        embrP = nodeOrigin[k][m] - embrOffsetY;
        embrQ = nodeOrigin[k][m] + QPointF(0, 125);

        if ( k == 0){
          // Bottom-row atom. So EMBRPOS_D branches are interslice ones.
          brm_tab = ndm_tab + 1;
          brm_row = ndm_row;
          brm_col = ndm_col;
          if ( brm_tab < static_cast<int>(hints.size()) ){
            // We are not at the top tab, so interslice connections are possible

            // Extract label of the mapped branch
            elementsMapped = _mmd->elementsMapped( brm_tab, brm_row, brm_col);
            if ( elementsMapped.size() > 1){
              cout << "More than one physical branches mapped at map position ";
              cout << "["<<brm_tab<<"]["<<brm_row<<"]["<<brm_col<<"]" << endl;
              return;
            }
            label.clear();
            for ( size_t k = 0 ; k != elementsMapped.size() ; ++k ){
              Branch const* pBr;
              if (!_pws->getBranch( elementsMapped.at(k).second, pBr)){
                label.append( "BR " );
                label.append( QString::number(pBr->extId) );
                label.append( " " );
              }
            }

            _drawMapperBranch( painter, embrP, embrQ,
                               hints[brm_tab][brm_row][brm_col], label );
          } else{
            // We are at the top slice (tab). So interslice connections are
            // floating and are therefore drawn as inactive
            _drawMapperBranch( painter, embrP, embrQ, MAPHINT_INACTIVE );
          }

        } else {
          // Normal EMBRPOS_D branch
          brm_tab = ndm_tab;
          brm_row = ndm_row - 1;
          brm_col = ndm_col;

          // Extract label of the mapped branch
          elementsMapped = _mmd->elementsMapped( brm_tab, brm_row, brm_col);
          if ( elementsMapped.size() > 1){
            cout << "More than one physical branches mapped at map position ";
            cout << "["<<brm_tab<<"]["<<brm_row<<"]["<<brm_col<<"]" << endl;
            return;
          }
          label.clear();
          for ( size_t k = 0 ; k != elementsMapped.size() ; ++k ){
            Branch const* pBr;
            if (!_pws->getBranch( elementsMapped.at(k).second, pBr)){
              label.append( "BR " );
              label.append( QString::number(pBr->extId) );
              label.append( " " );
            }
          }

          _drawMapperBranch( painter, embrP, embrQ,
                             hints[brm_tab][brm_row][brm_col], label );
        }
      }

      // Redundant! There are no EMBRPOS_DL branches in the current system!
//      if ( embr_exist[EMBRPOS_DL] ){
//        embrP = nodeOrigin[k][m] - embrOffsetX - embrOffsetX;
//        embrQ = nodeOrigin[k][m] + QPointF(-125, 125);

//        brm_tab = ndm_tab;
//        brm_row = ndm_row - 1;
//        brm_col = ndm_col - 1;

//        _drawMapperBranch( painter, embrP, embrQ,
//                           hints[brm_tab][brm_row][brm_col] );
//      }

      if ( embr_exist[EMBRPOS_L] ){
        embrP = nodeOrigin[k][m] - embrOffsetX;
        embrQ = nodeOrigin[k][m] + QPointF(-125, 0);

        if ( m == 0 ){
          // First-column atom. So EMBRPOS_L branches are interslice ones.
          brm_tab = ndm_tab + 1;
          brm_row = ndm_row;
          brm_col = ndm_col;
          if ( brm_tab < static_cast<int>(hints.size()) ){
            // We are not at the top tab, so interslice connections are possible

            // Extract label of the mapped branch
            elementsMapped = _mmd->elementsMapped( brm_tab, brm_row, brm_col);
            if ( elementsMapped.size() > 1){
              cout << "More than one physical branches mapped at map position ";
              cout << "["<<brm_tab<<"]["<<brm_row<<"]["<<brm_col<<"]" << endl;
              return;
            }
            label.clear();
            for ( size_t k = 0 ; k != elementsMapped.size() ; ++k ){
              Branch const* pBr;
              if (!_pws->getBranch( elementsMapped.at(k).second, pBr)){
                label.append( "BR " );
                label.append( QString::number(pBr->extId) );
                label.append( " " );
              }
            }

            _drawMapperBranch( painter, embrP, embrQ,
                               hints[brm_tab][brm_row][brm_col], label );
          } else{
            // We are at the top slice (tab). So interslice connections are
            // floating and are therefore drawn as inactive
            _drawMapperBranch( painter, embrP, embrQ, MAPHINT_INACTIVE );
          }

        } else {
          // Normal EMBRPOS_L branch
          brm_tab = ndm_tab;
          brm_row = ndm_row;
          brm_col = ndm_col - 1;

          // Extract label of the mapped branch
          elementsMapped = _mmd->elementsMapped( brm_tab, brm_row, brm_col);
          if ( elementsMapped.size() > 1){
            cout << "More than one physical branches mapped at map position ";
            cout << "["<<brm_tab<<"]["<<brm_row<<"]["<<brm_col<<"]" << endl;
            return;
          }
          label.clear();
          for ( size_t k = 0 ; k != elementsMapped.size() ; ++k ){
            Branch const* pBr;
            if (!_pws->getBranch( elementsMapped.at(k).second, pBr)){
              label.append( "BR " );
              label.append( QString::number(pBr->extId) );
              label.append( " " );
            }
          }

          _drawMapperBranch( painter, embrP, embrQ, hints[brm_tab][brm_row][brm_col], label );
        }
      }

      // Redundant! There are no EMBRPOS_UL branches in the current system!
//      if ( embr_exist[EMBRPOS_UL] ){
//        embrP = nodeOrigin[k][m] - embrOffsetX + embrOffsetX;
//        embrQ = nodeOrigin[k][m] + QPointF(-125, -125);

//        brm_tab = ndm_tab;
//        brm_row = ndm_row + 1;
//        brm_col = ndm_col - 1;

//        _drawMapperBranch( painter, embrP, embrQ,
//                           hints[brm_tab][brm_row][brm_col] );
//      }
    }
  }

  event->accept();
}

void MapperSliceWidget::mousePressEvent(QMouseEvent *event){

  SliceWidget::mousePressEvent(event);

  // parent protected methods _xclick & _yclick contain the coordinates of the
  // click event

  /*
    the mapper slice widget is meshed in [x x y] = [step x step = [120 x 120]
    blocks that represent the atoms of the emulator

    e.g. for 4x6 grid

          -5  125  235  355  475  595  715

    -125   |----|----|----|----|----|----|
           | 19 | 20 | 21 | 22 | 23 | 24 |
      -5   |----|----|----|----|----|----|
           | 13 | 14 | 15 | 16 | 17 | 18 |
     125   |----|----|----|----|----|----|
           | 07 | 08 | 09 | 10 | 11 | 12 |
     235   |----|----|----|----|----|----|
           | 01 | 02 | 03 | 04 | 05 | 06 |
     355   |----|----|----|----|----|----|
  */

  // Get which block was clicked
  int xblock = floor((_xclick + 5)/_step) - 1;
  int yblock = (_rows - 1) - floor((_yclick - 5) / _step);
  if ( yblock >= -1 && yblock <= static_cast<int>(_rows)-1 )
    if ( xblock == -1)
      xblock = 0;
  if ( xblock >= 0 && xblock <= static_cast<int>(_cols)-1 )
    if ( yblock == -1 )
      yblock = 0;

  // If block clicked is outside slice _atomSet size then return without doing
  // anything
  if (    xblock < 0 || xblock >= static_cast<int>(_cols)
       || yblock < 0 || yblock >= static_cast<int>(_rows) ){
    return;
  }

  // Get block relative coordinates; origin at node center, x increasing to
  // EMBRPOS_R and y increasing to EMBRPOS_U
  float xrel = _xclick - (xblock+1)*_step;
  float yrel = - (_yclick - (_rows-yblock)*_step);

  // Find out what was clicked (if anything), in mapper coordinates (like hints)
  int mapper_tab = -1;
  int mapper_row = -1;
  int mapper_col = -1;

  QPointF point_rel = QPointF(xrel, yrel);
  QPointF point_node = QPointF(0,0);
  QPointF point_embr_UR = QPointF(_step/2,_step/2);

  if ( QLineF(point_rel, point_node).length() <= 5 ){
    mapper_tab = 2*_sliceIndex;
    mapper_row = 2*yblock;
    mapper_col = 2*xblock;

  } else if (    point_rel.y() >= -5 && point_rel.y() <= 5
              && point_rel.x() >= 5 ){
    if (    yblock == static_cast<int>(_rows)-1
         && xblock == static_cast<int>(_cols)-1 ){
      // Top-right-most atom embr EMBRPOS_R
      // This embr is floating! Do nothing

    } else if ( xblock == static_cast<int>(_cols)-1 ){
      // Last-column atom embr EMBRPOS_R
      // This embr is an interslice one; so check whether we are not at the top
      // slice
      if (_sliceIndex != _emulatorSize-1){
        mapper_tab = 2*_sliceIndex + 1;
        mapper_row = 2*yblock;
        mapper_col = 2*xblock;
      }
    } else {
      // Normal embr EMBRPOS_R
      mapper_tab = 2*_sliceIndex;
      mapper_row = 2*yblock;
      mapper_col = 2*xblock + 1;
    }

  } else if ( QLineF(point_rel,point_embr_UR).length() <= 10 ){
    if (    yblock == static_cast<int>(_rows)-1
         && xblock == static_cast<int>(_cols)-1 ){
      // Top-right-most atom embr EMBRPOS_UR
      // This embr is floating! Do nothing
    } else if (    yblock == static_cast<int>(_rows)-2
                && xblock == static_cast<int>(_cols)-1 ){
      // TODO check what is going on here!!
    } else if ( xblock == static_cast<int>(_cols)-1 ){
      // Last-column atom embr EMBRPOS_UR
      // This embr is an interslice one; so check whether we are not at the top
      // slice
      if (_sliceIndex != _emulatorSize-1){
        mapper_tab = 2*_sliceIndex + 1;
        mapper_row = 2*yblock + 1;
        mapper_col = 2*xblock;
      }
    } else if ( yblock == static_cast<int>(_rows)-1 ) {
      // Top-row atom embr EMBRPOS_UR
      // This embr is an interslice one; so check whether we are not at the top
      // slice
      if (_sliceIndex != _emulatorSize-1){
        mapper_tab = 2*_sliceIndex + 1;
        mapper_row = 2*yblock;
        mapper_col = 2*xblock + 1;
      }
    } else {
      // Normal embr EMBRPOS_UR
      mapper_tab = 2*_sliceIndex;
      mapper_row = 2*yblock + 1;
      mapper_col = 2*xblock + 1;
    }

  } else if (    point_rel.x() >= -5 && point_rel.x() <= 5
            && point_rel.y() >= 5 ) {
    if ( yblock == static_cast<int>(_rows)-1 ){
      // Top-row atom embr EMBRPOS_U
      // This embr is an interslice one; so check whether we are not at the top
      // slice
      if (_sliceIndex != _emulatorSize-1){
        mapper_tab = 2*_sliceIndex + 1;
        mapper_row = 2*yblock;
        mapper_col = 2*xblock;
      }
    } else {
      // Normal embr EMBRPOS_U
      mapper_tab = 2*_sliceIndex;
      mapper_row = 2*yblock + 1;
      mapper_col = 2*xblock;
    }

  } else if (    point_rel.y() >= -5 && point_rel.y() <= 5
            && point_rel.x() <= -5 ) {
    if ( yblock == static_cast<int>(_rows)-1 ){
      // Top-row atom embr EMBRPOS_L (actually EMBRPOS_L embrs can exist only on
      // first-column atoms and are interslice)
      // This embr dows not physically exist!
    } else {
      // Normal embr EMBRPOS_L (actually EMBRPOS_L embrs can exist only on
      // first-column atoms and are interslice)
      if (_sliceIndex != _emulatorSize-1){
        mapper_tab = 2*_sliceIndex + 1;
        mapper_row = 2*yblock;
        mapper_col = 2*xblock;
      }
    }

  } else if (    point_rel.x() >= -5 && point_rel.x() <= 5
            && point_rel.y() <= -5 ) {
    if ( xblock == 0 ){
      // First-column atom embr EMBRPOS_D (actually EMBRPOS_D embrs can exist
      // only on bottom-row atoms and are interslice)
      // This embr does not physically exist!
    }else if ( xblock == static_cast<int>(_cols)-1 ){
      // Last-column atom embr EMBRPOS_D (actually EMBRPOS_D embrs can exist
      // only on bottom-row atoms and are interslice)
      // This embr does not physically exist!
    } else {
      // Normal embr EMBRPOS_D (actually EMBRPOS_D embrs can exist only on
      // bottom-row atoms and are interslice)
      if (_sliceIndex != _emulatorSize-1){
        mapper_tab = 2*_sliceIndex + 1;
        mapper_row = 2*yblock;
        mapper_col = 2*xblock;
      }
    }
  }
  if ( mapper_tab >= 0 && mapper_row >= 0 && mapper_col >= 0 ){
    emit mappingPositionClicked(mapper_tab, mapper_row, mapper_col);
  }

  event->accept();

  return;
}

void MapperSliceWidget::_drawNode(QPainter& painter, QPointF p,
                                  unsigned int row, unsigned int col, int hint,
                                  QString const& label){

  QString nodeLabel;
  QPointF nodeLabelOffset = QPointF( 10, 15 );

  // Draw node proper
  painter.save();
  QPen nodePen = QPen();
  QBrush nodeBrush = QBrush();
  nodeBrush.setStyle(Qt::SolidPattern);
  painter.setPen(nodePen);
  // Color node according to the hint
  switch ( hint ){
  case MAPHINT_INACTIVE:
    // Inactive (idle) nodes are grey
    nodeBrush.setColor(QColor(128,128,128));
    break;

  case MAPHINT_ACTIVEVALID:
    // Valid active nodes are green
    nodeBrush.setColor(QColor(0,255,0));
    break;

  case MAPHINT_MAPPED:
    // Already mapped nodes are blue
    nodeBrush.setColor(QColor(0,0,255));
    break;

  default:
    // Default color: grey
    nodeBrush.setColor(QColor(128,128,128));
    break;
  }
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
  painter.drawText( p - nodeLabelOffset, label );
  painter.restore();

  return;
}

void MapperSliceWidget::_drawMapperBranch(QPainter& painter,
                                          QPointF p, QPointF q,
                                          int hint,
                                          QString const& label ){

  QPointF labelOffset = QPointF( 10, 15 );

  // Save painter to stack
  painter.save();
  // Modify painter to draw emulator branches for the mapper
  QPen linePen = QPen();
  QBrush lineBrush = QBrush();
  linePen.setWidth(2);
  linePen.setCapStyle(Qt::RoundCap);
  lineBrush.setStyle(Qt::SolidPattern);
  // Color branch according to the hint
  switch ( hint ){
  case MAPHINT_INACTIVE:
    // Inactive (idle) nodes are grey
    lineBrush.setColor(QColor(128,128,128));
    linePen.setColor(QColor(128,128,128));
    break;

  case MAPHINT_ACTIVEVALID:
    // Valid active nodes are green
    lineBrush.setColor(QColor(0,255,0));
    linePen.setColor(QColor(0,255,0));
    break;

  case MAPHINT_MAPPED:
    // Already mapped nodes are blue
    lineBrush.setColor(QColor(0,0,255));
    linePen.setColor(QColor(0,0,255));
    break;

  default:
    // Default color: grey
    lineBrush.setColor(QColor(128,128,128));
    linePen.setColor(QColor(128,128,128));
    break;
  }
  painter.setBrush(lineBrush);
  painter.setPen(linePen);
  // Draw emulator branches proper
  painter.drawLine( p, q );
  // Restore painter to its previous state
  painter.restore();

  // Draw branch label
  painter.save();
  painter.setPen( Qt::black );
  painter.drawText( (p+q)/2 + labelOffset, label );
  painter.restore();

  return;
}
