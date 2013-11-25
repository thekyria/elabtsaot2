
#include "resultsvisualizationeditor.h"
using namespace elabtsaot;

#include "tdresults.h"

#include "guiauxiliary.h"
#include "addplotdialog.h"
#include "curvepropertiesdialog.h"
#include "deletecurvedialog.h"
#include "tdanalysiseditordialogs.h"
#include "resultsdbbrowser.h"
#include "plotpropertiesdialog.h"

#include <QToolBar>
#include <QAction>
#include <QIcon>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_series_data.h>
#include <qwt_legend.h>
#include <qwt_plot_renderer.h>
#include <qwt_plot_picker.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_zoomer.h>
#include <qwt_point_data.h>

//#include <vector>
using std::vector;
//#include <map>
using std::map;
#include <iostream>
using std::cout;
using std::endl;

ResultsVisualizationEditor::ResultsVisualizationEditor( TDResultsBank* trb,
                                                        QWidget* parent ) :
    QSplitter( Qt::Vertical, parent ), _trb(trb), _resultsDBBrowserShown(false){

  // ----- Raw results tab toolbar -----
  QToolBar* resultsVisualizationEditorToolbar = new QToolBar(this);
  this->addWidget( resultsVisualizationEditorToolbar );
  resultsVisualizationEditorToolbar->setFixedHeight(30);

  // Show resultsDB browser button
  showResultsDBBrowserAct = new QAction( QIcon(":/images/DBshow.png"),
                  "Show resultsDB browser", resultsVisualizationEditorToolbar );
  resultsVisualizationEditorToolbar->addAction( showResultsDBBrowserAct );
  showResultsDBBrowserAct->setCheckable( true );
  showResultsDBBrowserAct->setChecked( false );
  connect( showResultsDBBrowserAct, SIGNAL(triggered()),
           this, SLOT(showResultsDBBrowserSlot()) );

  // Update resultsDB browser button
  QAction* updateResultsDBBrowserAct = new QAction( QIcon(":/images/DBupdate.png"),
                  "Update resultsDB browser", resultsVisualizationEditorToolbar );
  resultsVisualizationEditorToolbar->addAction( updateResultsDBBrowserAct );
  connect( updateResultsDBBrowserAct, SIGNAL(triggered()),
           this, SLOT(updateResultsDBBrowserSlot()) );

  resultsVisualizationEditorToolbar->addSeparator(); // --- Separator ---

  // Add plot button
  QAction* addPlotAct = new QAction( QIcon(":/images/curve.png"),
                  "Add plot", resultsVisualizationEditorToolbar );
  resultsVisualizationEditorToolbar->addAction( addPlotAct );
  connect( addPlotAct, SIGNAL(triggered()),
           this, SLOT(addPlotSlot()) );

  // Plot properties button
  QAction* plotPropertiesAct = new QAction( QIcon(":/images/curveconf.png"),
                   "Plot properties", resultsVisualizationEditorToolbar);
  resultsVisualizationEditorToolbar->addAction( plotPropertiesAct );
  connect( plotPropertiesAct, SIGNAL(triggered()),
           this, SLOT(plotPropertiesSlot()) );

  // Delete plot button
  QAction* deletePlotAct = new QAction( QIcon(":/images/curvedel.png"),
                   "Delete plot", resultsVisualizationEditorToolbar );
  resultsVisualizationEditorToolbar->addAction( deletePlotAct );
  connect( deletePlotAct, SIGNAL(triggered()),
           this, SLOT(deletePlotSlot()) );

  resultsVisualizationEditorToolbar->addSeparator(); // --- Separator ---

  // Add selected results to new plot
  QAction* addCurveToNewPlotAct = new QAction( QIcon(":/images/resultsadd.png"),
            "Add results curve to new plot", resultsVisualizationEditorToolbar);
  resultsVisualizationEditorToolbar->addAction( addCurveToNewPlotAct );
  connect( addCurveToNewPlotAct, SIGNAL(triggered()),
           this, SLOT(addCurveToNewPlotSlot()) );

  // Add selected results to existing plot
  QAction* addCurveToExistingPlotAct = new QAction( QIcon(":/images/resultsmodify.png"),
       "Add results curve to existing plot", resultsVisualizationEditorToolbar);
  resultsVisualizationEditorToolbar->addAction( addCurveToExistingPlotAct );
  connect( addCurveToExistingPlotAct, SIGNAL(triggered()),
           this, SLOT(addCurveToExistingPlotSlot()) );

  // Results curve properties button
  QAction* curvePropertiesAct = new QAction( QIcon(":/images/resultsproperties.png"),
                    "Results curve Properties", resultsVisualizationEditorToolbar );
  resultsVisualizationEditorToolbar->addAction( curvePropertiesAct );
  connect( curvePropertiesAct, SIGNAL(triggered()),
           this, SLOT(curvePropertiesSlot()) );

  // Delete existing curve in a plot
  QAction* deleteCurveFromPlotAct = new QAction( QIcon(":/images/resultsdelete1.png"),
       "Delete curve from plot", resultsVisualizationEditorToolbar);
  resultsVisualizationEditorToolbar->addAction( deleteCurveFromPlotAct );
  connect( deleteCurveFromPlotAct, SIGNAL(triggered()),
           this, SLOT(deleteCurveFromPlotSlot()) );

  // Delete all existing curves in a plot
  QAction* deleteCurvesFromPlotAct = new QAction( QIcon(":/images/resultsdelete2.png"),
       "Delete curves from plot", resultsVisualizationEditorToolbar);
  resultsVisualizationEditorToolbar->addAction( deleteCurvesFromPlotAct );
  connect( deleteCurvesFromPlotAct, SIGNAL(triggered()),
           this, SLOT(deleteCurvesFromPlotSlot()) );

  resultsVisualizationEditorToolbar->addSeparator(); // --- Separator ---

  // Take snapshot button
  QAction* snapshotAct = new QAction( QIcon(":/images/screenshot.png"),
                     "Take snapshot", resultsVisualizationEditorToolbar );
  resultsVisualizationEditorToolbar->addAction( snapshotAct );
  connect( snapshotAct, SIGNAL(triggered()),
           this, SLOT(snapshotSlot()) );

  resultsVisualizationEditorToolbar->addSeparator(); // --- Separator ---

  // Main horizontal splitter widget (accomodates resultsDBBrowserBox&plotsBox)
  QSplitter* mainSplitter = new QSplitter( Qt::Horizontal );
  addWidget( mainSplitter );

  resultsDBBrowserBox = new QGroupBox( "ResultsDB browser", this );
  mainSplitter->addWidget( resultsDBBrowserBox );
  resultsDBBrowserLay = new QHBoxLayout();
  resultsDBBrowserBox->setLayout( resultsDBBrowserLay );
  resultsDBBrowserBox->setMaximumWidth( 500 );
  resultsDBBrowserBox->hide();

  resultsDBBrowser = new ResultsDBBrowser( _trb, this );
  resultsDBBrowserLay->addWidget( resultsDBBrowser );

  plotsBox = new QGroupBox( "Plotting area", this );
  mainSplitter->addWidget( plotsBox );
  plotsLay = new QVBoxLayout();
  plotsBox->setLayout( plotsLay );

}

void ResultsVisualizationEditor::showResultsDBBrowserSlot(){

  if ( _resultsDBBrowserShown ){
    // Results DB browser sidepane is shown; so, hide it and unpush the button
    showResultsDBBrowserAct->setChecked( false );
    resultsDBBrowserBox->hide();
    _resultsDBBrowserShown = false;
  } else {
    // Results DB browser sidepane is hidden; so, show it and push the button
    showResultsDBBrowserAct->setChecked( true );
    resultsDBBrowserBox->show();
    _resultsDBBrowserShown = true;
  }

  return;
}

void ResultsVisualizationEditor::updateResultsDBBrowserSlot(){
  resultsDBBrowser->updt();
  return;
}

void ResultsVisualizationEditor::addPlotSlot(){

  QString name;
  TDResultsPlotProperties tdrpp;
  AddPlotDialog dialog( name, tdrpp, true );
  int ans = dialog.exec();
  if ( !ans )
    // Dialog excecuted properly; so, add plot
    _addPlot ( name, tdrpp );

  return;
}

void ResultsVisualizationEditor::plotPropertiesSlot(){

  // Dialog to select plot for editing
  int plotId;
  int ans = TDAnalysisEditorDialogs::selectPlotDialog( _plots, &plotId );
  if ( ans ) return;

  // Dialog to actually edit plot
  QwtPlot* plot = _plots[plotId];
//  TDResultsPlotProperties& tdrpp( _plotPropertiesMap[plot] );
//  PlotPropertiesDialog dialog( *plot, tdrpp );
  PlotPropertiesDialog dialog( *plot );
  ans = dialog.exec();
  if ( !ans )
    plot->replot();

  return;
}

void ResultsVisualizationEditor::deletePlotSlot(){

  // Dialog to select plot for deletion
  int plotId;
  int ans = TDAnalysisEditorDialogs::selectPlotDialog( _plots, &plotId );
  if ( ans )
    return;

  // Remove plot entry from _plotPropertiesMap
  _plotPropertiesMap.erase( _plotPropertiesMap.find( _plots.at(plotId) ) );
  // Delete plot object proper
  delete _plots.at( plotId );
  // Remove from _plots vector
  _plots.erase( _plots.begin() + plotId );

  return;
}

void ResultsVisualizationEditor::addCurveToNewPlotSlot(){

  if ( !_resultsDBBrowserShown )
    return;

  // Retrieve selected element from the resultsDb browser
  TDResultsBankConstIterator it;
  int pos;
  int ans = resultsDBBrowser->getSelectedResults( &it, &pos );
//  cout << "resultsDBBrowser->getSelectedResults() returned " << ans << endl;
  if ( ans )
    return;

  TDResults res;
  ans = _trb->getResults( it , &res );
  if ( ans )
    // Could not retrieve the results
    return;

  // Find time series in the TDResults object
  int timePos = res.getTimePosition();
  if ( timePos < 0 )
    // There is no time series in the results!
    return;

  TDResultsPlotProperties tdrpp;
  tdrpp.variable = TDResultIdentifier::variableType( res.identifiers().at(pos).variable );
  tdrpp.units = res.units().at(pos);
  QString name = QString::fromStdString(res.names().at(pos));
  AddPlotDialog dialog( name, tdrpp, false );
  ans = dialog.exec();
  if ( ans )
    // Add plot dialog cancelled
    return;
  // Otherwise add plot
  QwtPlot* newPlot = _addPlot ( name, tdrpp );

  // Add curve proper
  QVector<double> xs = QVector<double>::fromStdVector( res.data().at(timePos) );
  QVector<double> ys = QVector<double>::fromStdVector( res.data().at(pos) );
  QwtPlotCurve* curve = new QwtPlotCurve();
  curve->setTitle( QString::fromStdString(res.names().at(pos)) );
  QPen curvePen;
  curvePen.setColor(Qt::black);
  curve->setPen(curvePen);
  QwtPointArrayData* curveData = new QwtPointArrayData( xs, ys );
  curve->setData(curveData);
  curve->attach( newPlot );
  newPlot->replot();

  return;
}

void ResultsVisualizationEditor::addCurveToExistingPlotSlot(){

  if ( !_resultsDBBrowserShown )
    return;

  // Retrieve selected element from the resultsDb browser
  TDResultsBankConstIterator it;
  int pos;
  int ans = resultsDBBrowser->getSelectedResults( &it, &pos );
//  cout << "resultsDBBrowser->getSelectedResults() returned " << ans << endl;
  if ( ans )
    return;

  TDResults res;
  ans = _trb->getResults( it , &res );
  if ( ans )
    // Could not retrieve the results
    return;

  // Find time series in the TDResults object
  int timePos = res.getTimePosition();
  if ( timePos < 0 )
    // There is no time series in the results!
    return;

  // Determing TDResultsPlotProperties required for the curve in question
  TDResultsPlotProperties tdrpp;
  tdrpp.variable = TDResultIdentifier::variableType( res.identifiers().at(pos).variable );
  tdrpp.units = res.units().at(pos);

  // Select plot according to curve TDResultsPlotProperties criteria
  int plotId;
  ans = TDAnalysisEditorDialogs::selectPlotDialog( _plots, _plotPropertiesMap,
                                                 tdrpp, &plotId );
  if ( ans )
    return;
  QwtPlot* plot = _plots.at(plotId);

  // Add curve proper
  QVector<double> xs = QVector<double>::fromStdVector( res.data().at(timePos) );
  QVector<double> ys = QVector<double>::fromStdVector( res.data().at(pos) );
  QwtPlotCurve* curve = new QwtPlotCurve();
  curve->setTitle( QString::fromStdString(res.names().at(pos)) );
  QPen curvePen;
  curvePen.setColor(QColor::fromHsv(qrand() % 256, 255, 190));
  curve->setPen(curvePen);
  QwtPointArrayData* curveData = new QwtPointArrayData( xs, ys );
  curve->setData(curveData);
  curve->attach( plot );
  plot->replot();

  return;
}

void ResultsVisualizationEditor::curvePropertiesSlot(){

  int plotId, curveId;
  QString curveName;
  QPen curvePen;
  CurvePropertiesDialog dialog( _plots, &plotId, &curveId, &curveName, &curvePen, this );
  int ans = dialog.exec();
  if ( ans )
    return;

  QwtPlot* plot = _plots.at( plotId );
  QwtPlotCurve* curve = static_cast<QwtPlotCurve*>(
        plot->itemList(QwtPlotItem::Rtti_PlotCurve).at( curveId ) );
  curve->setTitle( curveName );
  curve->setPen( curvePen );
  plot->replot();

  return;
}

void ResultsVisualizationEditor::deleteCurveFromPlotSlot(){

  int plotId, curveId;
  DeleteCurveDialog dialog( _plots, &plotId, &curveId, this );
  int ans = dialog.exec();
  if ( ans )
    return;

  QwtPlot* plot = _plots.at( plotId );
  plot->itemList(QwtPlotItem::Rtti_PlotCurve).at( curveId )->detach();
  plot->replot();

  return;
}

void ResultsVisualizationEditor::deleteCurvesFromPlotSlot(){

  // Dialog to select plot to delete all curves from
  int plotId;
  int ans = TDAnalysisEditorDialogs::selectPlotDialog( _plots, &plotId );
  if ( ans )
    return;

  _plots.at(plotId)->detachItems();
  _plots.at(plotId)->replot();

  return;
}

void ResultsVisualizationEditor::snapshotSlot(){

  // Dialog to select plot for snapshot
  int plotId;
  int ans = TDAnalysisEditorDialogs::selectPlotDialog( _plots, &plotId );
  if ( ans )
    return;

  // Dialog to select file to write the snapshot to
  QString filename = guiauxiliary::askFileName("png", false);
  if ( filename.isNull() )
    return;

  QSizeF size( 300 , 100 );
  int resolution = 300;
  QwtPlotRenderer screenshot;
  screenshot.renderDocument( _plots.at(plotId), filename, size, resolution );
  cout << "Plot saved in file: " << filename.toStdString() << endl;

  return;
}

QwtPlot* ResultsVisualizationEditor::_addPlot( QString const& name,
                                         TDResultsPlotProperties const& tdrpp ){

  // Create and store plot and its properties
  QwtPlot* newPlot = new QwtPlot();
  plotsLay->addWidget( newPlot );
  _plots.push_back( newPlot );
  _plotPropertiesMap[newPlot] = tdrpp;

  QFont axFont("Times", 10);
  QwtText xText("Time [sec]");
  xText.setFont(axFont);
  QwtText yText(
    QString::fromStdString(physics::variableToString(tdrpp.variable))
    + QString(" [") + QString::fromStdString( tdrpp.units ) + QString("]") );
  newPlot->setTitle( QwtText(name) );
  newPlot->setAxisTitle(QwtPlot::yLeft, yText);
  newPlot->setAxisTitle(QwtPlot::xBottom, xText);
  newPlot->insertLegend( new QwtLegend() );
  newPlot->setAxisScale(QwtPlot::xBottom, -1.0,4.0);
  newPlot->setAxisScale(QwtPlot::yLeft, -10.0,10.0);
  newPlot->setAxisAutoScale(QwtPlot::xBottom,true);
  newPlot->setAxisAutoScale(QwtPlot::yLeft,true);
  QwtPlotPicker* picker = new QwtPlotPicker(newPlot->canvas());
  picker->setTrackerMode(QwtPicker::AlwaysOn);
  QwtPlotMagnifier* magnifier = new  QwtPlotMagnifier(newPlot->canvas());
  magnifier->setMouseButton(Qt::RightButton,Qt::ControlModifier);
  magnifier->setMouseButton(Qt::RightButton,Qt::ControlModifier);
  QwtPlotPanner* panner = new QwtPlotPanner(newPlot->canvas());
  panner->setMouseButton(Qt::LeftButton,Qt::ControlModifier);
  panner->setCursor(Qt::ClosedHandCursor);
  QwtPlotZoomer* zoomer = new QwtPlotZoomer(newPlot->canvas(),true);
  zoomer->setEnabled(true);

  return newPlot;
}
