/*!
\file gtdaviewer.h

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef GTDAVIEWER_H
#define GTDAVIEWER_H

#include "schematicviewer.h"

#include <map>

namespace elabtsaot{

enum GTDAViewerMode{
  GTDAVIEWER_MODE_IDLE,
  GTDAVIEWER_MODE_BRSTABCHECK,
  GTDAVIEWER_MODE_CCTCHECK
};

class GTDAViewer : public SchematicViewer {

 public:

  GTDAViewer( Powersystem const* pws, PwsSchematicModel const* smd,
              QWidget* parent = 0 );
  int init();

  virtual void paintEvent(QPaintEvent* event);

  void enterIdleMode();
  void enterBrStabCheckMode(
    std::map<unsigned int,std::map<unsigned int,bool> > brGenStab);
  void enterCCTCheckMode(
    std::map<unsigned int, std::pair<double,double> > cctMap );

 private:

  void _drawBrStabCheck( QPainter& painter );
  void _drawBrStabCheckSelectionMarkers( QPainter& painter );
  void _drawBrStabMarker( QPainter& painter,
                          PwsSchematicModelElement const& brCd,
                          bool isStable );
  void _drawGenStabMarker( QPainter& painter,
                           PwsSchematicModelElement const& genCd,
                           bool isStable );
  void _drawCCTCheck( QPainter& painter );
  void _drawCCTBrMarker( QPainter& painter,
                         PwsSchematicModelElement const& brCd,
                         double cctMin, double cctMax );

  Powersystem const* _pws;
  PwsSchematicModel const* _smd;
  int _mode;

  std::map<unsigned int,std::map<unsigned int,bool> > _brGenStab;
  std::map<unsigned int, std::pair<double,double> > _cctMap;

};

} // end of namespace elabtsaot

#endif // GTDAVIEWER_H
