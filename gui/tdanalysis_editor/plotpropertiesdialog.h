/*!
\file plotpropertiesdialog.h

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef PLOTPROPERTIESDIALOG_H
#define PLOTPROPERTIESDIALOG_H

class QwtPlot;

#include <QObject>
class QCheckBox;
class QComboBox;
class QDoubleSpinBox;

namespace elabtsaot{

class TDResultsPlotProperties;

class PlotPropertiesDialog : public QObject {

  Q_OBJECT

 public:

//  PlotPropertiesDialog( QwtPlot& plot, TDResultsPlotProperties& tdrpp );
  PlotPropertiesDialog( QwtPlot& plot );
  int exec();

 public slots:

  void legendCheckChanged(bool);
  void xMinValueChanged(double);
  void xMaxValueChanged(double);
  void yMinValueChanged(double);
  void yMaxValueChanged(double);

 private:

  QwtPlot& _plot;
//  TDResultsPlotProperties& _tdrpp;

  QCheckBox* legendCheck;
  QComboBox* legendPosCombo;
  QDoubleSpinBox* xMinForm;
  QDoubleSpinBox* xMaxForm;
  QDoubleSpinBox* yMinForm;
  QDoubleSpinBox* yMaxForm;

};

} // end of namespace elabtsaot

#endif // PLOTPROPERTIESDIALOG_H
