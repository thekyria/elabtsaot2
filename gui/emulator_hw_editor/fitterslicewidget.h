/*!
\file fitterslicewidget.h
\brief Definition file for class FitterSliceWidget

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Fragkiskos Koufogiannis, fkoufis at gmail dot com
\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef FITTERSLICEWIDGET_H
#define FITTERSLICEWIDGET_H

#include "slicewidget.h"

namespace elabtsaot{

class EmulatorBranch;

class FitterSliceWidget : public SliceWidget {

  Q_OBJECT

 public:

  FitterSliceWidget( Slice* slc, QWidget* parent = 0 );
  void paintEvent(QPaintEvent *event);
  void mousePressEvent(QMouseEvent* event);
  void toggleReal();
  void toggleReal(bool showReal);

  bool isShowingReal() const;

 signals:

  void fittingPositionClicked( int emulator_tab,
                               int emulator_row,
                               int emulator_col,
                               int emulator_elm );

 private:

  void _drawRealMark(QPainter& painter, QPoint topleft);
  void _drawFitterBranch(QPainter& painter, QPointF p, QPointF q,
                         EmulatorBranch const& embr);

  static bool _isShowingReal;

};

} // end of namespace elabtsaot

#endif // FITTERSLICEWIDGET_H
