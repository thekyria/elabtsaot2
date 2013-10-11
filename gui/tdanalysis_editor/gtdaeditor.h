/*!
\file gtdaeditor.h

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef GTDAEDITOR_H
#define GTDAEDITOR_H

#include <QSplitter>

namespace elabtsaot{

class TDEngine;
class PwsSchematicModel;
class GTDAViewer;

class GTDAEditor : public QSplitter {

  Q_OBJECT

 public:

  GTDAEditor( PwsSchematicModel const* smd, TDEngine* const& tde,
              QWidget* parent = 0 );
  int init();

 public slots:

  void clearSlot();
  void branchStabCheckSlot();
  void cctCheckSlot();

 private:

  int _branchStabCheckDialog(double& faultTime, bool& trip);
  int _cctCheckDialog( double& precision,
                       bool& trip,
                       double& maxFaultTime,
                       double& sceStopTime );

  TDEngine* const& _tde;
  GTDAViewer* _gav;

};

} // end of namespace elabtsaot

#endif // GTDAEDITOR_H
