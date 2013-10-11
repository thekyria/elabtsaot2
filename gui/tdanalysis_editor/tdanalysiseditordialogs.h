/*!
\file tdanalysiseditordialogs.h

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef TDANALYSISEDITORDIALOGS_H
#define TDANALYSISEDITORDIALOGS_H

class QStringList;
class QString;
class QwtPlot;

#include <vector>
#include <stdint.h>
#include <map>

namespace elabtsaot{

class TDResults;
class TDResultsPlotProperties;

class TDAnalysisEditorDialogs {

 public:

  // ResultsRequestEditor dialogs
  static int setTDEngineTimeStepDialog( double* timeStep );

  static int selectPlotDialog( std::vector<QwtPlot*> const& plots,
                               int* selectedPlot );
  static int selectPlotDialog( std::vector<QwtPlot*> const& plots,
           std::map<QwtPlot*, TDResultsPlotProperties> const& plotPropertiesMap,
           TDResultsPlotProperties const& tdrpp,
           int* selectedPlot );

};

} // end of namespace elabtsaot

#endif // TDANALYSISEDITORDIALOGS_H
