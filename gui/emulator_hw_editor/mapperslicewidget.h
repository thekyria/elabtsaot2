/*!
\file mapperslicewidget.h
\brief Definition file for class MapperSliceWidget

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Fragkiskos Koufogiannis, fkoufis at gmail dot com
\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef MAPPERSLICEWIDGET_H
#define MAPPERSLICEWIDGET_H

#include "slicewidget.h"

namespace elabtsaot{

class Powersystem;
class PwsMapperModel;
struct Slice;

class MapperSliceWidget : public SliceWidget {

  Q_OBJECT

 public:

  MapperSliceWidget( Slice const* slc,
                     Powersystem const* pws,
                     PwsMapperModel const* mmd,
                     int emulatorSize,
                     QWidget* parent = 0 );
  void paintEvent(QPaintEvent *event);
  void mousePressEvent(QMouseEvent* event);

 signals:

  void mappingPositionClicked( int mapper_tab, int mapper_row, int mapper_col );

 private:

  void _drawNode( QPainter& painter, QPointF p,
                  unsigned int row, unsigned int col,
                  int hint,
                  QString const& label = "[]" );
  void _drawMapperBranch( QPainter& painter, QPointF p, QPointF q,
                          int hint,
                          QString const& label = "[]" );

  Powersystem const* _pws;
  PwsMapperModel const* _mmd;
  // Slice* SliceWidget::_slc
  int _emulatorSize;

};

} // end of namespace elabtsaot

#endif // MAPPERSLICEWIDGET_H
