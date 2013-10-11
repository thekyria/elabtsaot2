
#include "plotpropertiesdialog.h"
using namespace elabtsaot;

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QGroupBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QPushButton>

#include <qwt_plot.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_curve.h>
#include <qwt_series_data.h>
#include <qwt_legend.h>

#include <limits>
using std::numeric_limits;

PlotPropertiesDialog::PlotPropertiesDialog( QwtPlot& plot ) : _plot(plot) {}
//PlotPropertiesDialog::PlotPropertiesDialog( QwtPlot& plot,
//                                            TDResultsPlotProperties& tdrpp ) :
//  _plot(plot), _tdrpp(tdrpp){}

int PlotPropertiesDialog::exec(){

  QDialog* dialog = new QDialog();
  dialog->setWindowTitle("Plot properties");
  QVBoxLayout* mainLay = new QVBoxLayout();
  dialog->setLayout( mainLay );

  QHBoxLayout* nameLay = new QHBoxLayout();
  mainLay->addLayout( nameLay );
  QLabel* nameLabel = new QLabel("Name");
  nameLay->addWidget( nameLabel );
  QLineEdit* nameForm = new QLineEdit( _plot.title().text() );
  nameLay->addWidget( nameForm );

  QHBoxLayout* legendLay = new QHBoxLayout();
  mainLay->addLayout( legendLay );
  QLabel* legendLabel = new QLabel("Legend");
  legendLay->addWidget( legendLabel );
  legendCheck = new QCheckBox("On");
  legendLay->addWidget( legendCheck );
  legendCheck->setChecked( !_plot.legend()->isHidden() );
  legendPosCombo = new QComboBox();
  legendLay->addWidget( legendPosCombo );
  legendPosCombo->addItem( "Left", QwtPlot::LeftLegend );
  legendPosCombo->addItem( "Right", QwtPlot::RightLegend );
  legendPosCombo->addItem( "Bottom", QwtPlot::BottomLegend );
  legendPosCombo->addItem( "Top", QwtPlot::TopLegend );
  connect( legendCheck, SIGNAL(toggled(bool)),
           this, SLOT(legendCheckChanged(bool)) );

  // Horizontal (x-) axis options
  QGroupBox* xAxisBox = new QGroupBox();
  mainLay->addWidget( xAxisBox );
  QVBoxLayout* xAxisLay = new QVBoxLayout();
  xAxisBox->setLayout( xAxisLay );

  QHBoxLayout* xNameLay = new QHBoxLayout();
  xAxisLay->addLayout( xNameLay );
  QLabel* xNameLabel = new QLabel("x-axis name");
  xNameLay->addWidget(xNameLabel);
  QLineEdit* xNameForm=new QLineEdit(_plot.axisTitle(QwtPlot::xBottom).text());
  xNameLay->addWidget( xNameForm );

  QHBoxLayout* xMinLay = new QHBoxLayout();
  xAxisLay->addLayout( xMinLay );
  QLabel* xMinLabel = new QLabel("x-axis min");
  xMinLay->addWidget(xMinLabel);
  xMinForm = new QDoubleSpinBox();
  xMinLay->addWidget( xMinForm );
//  xMinForm->setRange( -numeric_limits<double>::max(),
//                      numeric_limits<double>::max() );
  xMinForm->setDecimals( 3 );
  xMinForm->setValue( _plot.axisScaleDiv(QwtPlot::xBottom).lowerBound() );
  connect( xMinForm, SIGNAL(valueChanged(double)),
           this, SLOT( xMinValueChanged(double)) );

  QHBoxLayout* xMaxLay = new QHBoxLayout();
  xAxisLay->addLayout( xMaxLay );
  QLabel* xMaxLabel = new QLabel("x-axis max");
  xMaxLay->addWidget(xMaxLabel);
  xMaxForm = new QDoubleSpinBox();
  xMaxLay->addWidget( xMaxForm );
//  xMaxForm->setRange( -numeric_limits<double>::max(),
//                      numeric_limits<double>::max() );
  xMaxForm->setDecimals( 3 );
  xMaxForm->setValue( _plot.axisScaleDiv(QwtPlot::xBottom).upperBound() );
  connect( xMaxForm, SIGNAL(valueChanged(double)),
           this, SLOT( xMaxValueChanged(double)) );

  // Vertical (y-) axis options
  QGroupBox* yAxisBox = new QGroupBox();
  mainLay->addWidget( yAxisBox );
  QVBoxLayout* yAxisLay = new QVBoxLayout();
  yAxisBox->setLayout( yAxisLay );

  QHBoxLayout* yNameLay = new QHBoxLayout();
  yAxisLay->addLayout( yNameLay );
  QLabel* yNameLabel = new QLabel("y-axis name");
  yNameLay->addWidget(yNameLabel);
  QLineEdit* yNameForm=new QLineEdit(_plot.axisTitle(QwtPlot::yLeft).text());
  yNameLay->addWidget( yNameForm );

  QHBoxLayout* yMinLay = new QHBoxLayout();
  yAxisLay->addLayout( yMinLay );
  QLabel* yMinLabel = new QLabel("y-axis min");
  yMinLay->addWidget(yMinLabel);
  yMinForm = new QDoubleSpinBox();
  yMinLay->addWidget( yMinForm );
  yMinForm->setRange( -numeric_limits<double>::max(),
                      numeric_limits<double>::max() );
  yMinForm->setDecimals( 3 );
  yMinForm->setValue( _plot.axisScaleDiv(QwtPlot::yLeft).lowerBound() );
  connect( yMinForm, SIGNAL(valueChanged(double)),
           this, SLOT( yMinValueChanged(double)) );

  QHBoxLayout* yMaxLay = new QHBoxLayout();
  yAxisLay->addLayout( yMaxLay );
  QLabel* yMaxLabel = new QLabel("y-axis max");
  yMaxLay->addWidget(yMaxLabel);
  yMaxForm = new QDoubleSpinBox();
  yMaxLay->addWidget( yMaxForm );
  yMaxForm->setRange( -numeric_limits<double>::max(),
                      numeric_limits<double>::max() );
  yMaxForm->setDecimals( 3 );
  yMaxForm->setValue( _plot.axisScaleDiv(QwtPlot::yLeft).upperBound() );
  connect( yMaxForm, SIGNAL(valueChanged(double)),
           this, SLOT( yMaxValueChanged(double)) );

  // Ok & Cancel buttons
  QHBoxLayout* buttonLay = new QHBoxLayout();
  mainLay->addLayout( buttonLay );
  QPushButton* okb = new QPushButton("OK");
  buttonLay->addWidget( okb );
  dialog->connect( okb, SIGNAL(clicked()), dialog, SLOT(accept()) );
  QPushButton* cancelb = new QPushButton("Cancel");
  buttonLay->addWidget( cancelb );
  dialog->connect( cancelb, SIGNAL(clicked()), dialog, SLOT(reject()) );

  if ( dialog->exec() ){
    // Dialog executed properly
    _plot.setTitle( nameForm->text() );
    _plot.legend()->setHidden( !legendCheck->isChecked() );
    _plot.plotLayout()->setLegendPosition( static_cast<QwtPlot::LegendPosition>(
            legendPosCombo->itemData(legendPosCombo->currentIndex()).toInt()) );
    _plot.setAxisTitle( QwtPlot::xBottom, xNameForm->text() );
    _plot.setAxisScale( QwtPlot::xBottom, xMinForm->value(), xMaxForm->value());
    _plot.setAxisTitle( QwtPlot::yLeft, yNameForm->text() );
    _plot.setAxisScale( QwtPlot::yLeft, yMinForm->value(), yMaxForm->value() );

    return 0;

  } else {
    // Dialog failed
    return 1;
  }
  return 0;
}

void PlotPropertiesDialog::legendCheckChanged( bool val ){
  legendPosCombo->setEnabled( val );
  return;
}

void PlotPropertiesDialog::xMinValueChanged( double val ){
  if ( val > xMaxForm->value() )
    xMaxForm->setValue( val );
  return;
}

void PlotPropertiesDialog::xMaxValueChanged( double val ){
  if ( val < xMinForm->value() )
    xMinForm->setValue( val );
  return;
}

void PlotPropertiesDialog::yMinValueChanged( double val ){
  if ( val > yMaxForm->value() )
    yMaxForm->setValue( val );
  return;
}

void PlotPropertiesDialog::yMaxValueChanged( double val ){
  if ( val < yMinForm->value() )
    yMinForm->setValue( val );
  return;
}
