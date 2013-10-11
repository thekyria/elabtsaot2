/*!
\file mapperemulatoreditor.h
\brief Definition file for class MapperEmulatorEditor

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Fragkiskos Koufogiannis, fkoufis at gmail dot com
\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef MAPPEREMULATOREDITOR_H
#define MAPPEREMULATOREDITOR_H

#include <QTabWidget>

#include <vector>

namespace elabtsaot{

class EmulatorHw;
class Powersystem;
class PwsMapperModel;
class MapperSliceWidget;

class MapperEmulatorEditor : public QTabWidget{

  Q_OBJECT

 public:

  // ------------ functions ------------
  MapperEmulatorEditor( Powersystem const* pws,
                        PwsMapperModel const* mmd,
                        EmulatorHw const* emuhw,
                        QWidget* parent = 0);
  int init();
  void repaintTabs();

 public slots:

  void zoomIn();
  void zoomOut();
  void zoomFit();

 signals:

  void mappingPositionClicked( int mapper_tab, int mapper_row, int mapper_col );

 private:

  Powersystem const* _pws;
  PwsMapperModel const* _mmd;
  EmulatorHw const* _emuhw;
  std::vector<MapperSliceWidget*> _slices;

};

} // end of namespace elabtsaot

#endif // MAPPEREMULATOREDITOR_H
