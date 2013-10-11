/*!
\file resultsvisualizationeditor.h

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef RESULTSVISUALIZATIONEDITOR_H
#define RESULTSVISUALIZATIONEDITOR_H

#include <QSplitter>
class QGroupBox;
class QHBoxLayout;
class QVBoxLayout;
class QwtPlot;

#include <vector>
#include <map>

#include "tdresultsplotproperties.h"

namespace elabtsaot{

class TDResultsBank;
class ResultsDBBrowser;

class ResultsVisualizationEditor : public QSplitter{

  Q_OBJECT

 public:

  ResultsVisualizationEditor( TDResultsBank* trb,
                              QWidget* parent = 0 );

 public slots:

  void showResultsDBBrowserSlot();
  void updateResultsDBBrowserSlot();
  // -----
  void addPlotSlot();
  void plotPropertiesSlot();
  void deletePlotSlot();
  // -----
  void addCurveToNewPlotSlot();
  void addCurveToExistingPlotSlot();
  void curvePropertiesSlot();
  void deleteCurveFromPlotSlot();
  void deleteCurvesFromPlotSlot();
  // -----
  void snapshotSlot();

 private:

  QwtPlot* _addPlot( QString const& name, TDResultsPlotProperties const& tdrpp);

  TDResultsBank* _trb;
  std::vector<QwtPlot*> _plots;
  std::map<QwtPlot*, TDResultsPlotProperties> _plotPropertiesMap;

  bool _resultsDBBrowserShown;
  ResultsDBBrowser* resultsDBBrowser;
  QAction* showResultsDBBrowserAct;
  QGroupBox* resultsDBBrowserBox;
  QHBoxLayout* resultsDBBrowserLay;
  QGroupBox* plotsBox;
  QVBoxLayout* plotsLay;

};

} // end of namespace elabtsaot

#endif // RESULTSVISUALIZATIONEDITOR_H
