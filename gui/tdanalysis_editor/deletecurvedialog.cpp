
#include "deletecurvedialog.h"
using namespace elabtsaot;

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_series_data.h>
#include <qwt_legend.h>

#include <QDialog>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

//#include <vector>
using std::vector;

DeleteCurveDialog::DeleteCurveDialog( vector<QwtPlot*> const& plots,
                                      int* plotId, int* curveId,
                                      QWidget* parent ) :
    QObject(parent), _plots(plots), _plotId(plotId), _curveId(curveId){}

int DeleteCurveDialog::exec(){

  QDialog dialog;
  dialog.setWindowTitle("Delete curve from plot");
  QVBoxLayout* layoutMain = new QVBoxLayout();
  dialog.setLayout(layoutMain);

  // Plot box
  QHBoxLayout *layoutPlot = new QHBoxLayout();
  layoutMain->addLayout(layoutPlot);
  QLabel *labelPlot = new QLabel("Plot:", &dialog);
  layoutPlot->addWidget(labelPlot);
  _formPlot = new QComboBox(&dialog);
  layoutPlot->addWidget(_formPlot);
  for ( size_t k = 0 ; k != _plots.size() ; ++k )
    _formPlot->addItem( _plots[k]->title().text() );
  connect( _formPlot, SIGNAL(currentIndexChanged(int)),
           this, SLOT(plotSelectedSlot(int)) );

  // Curves box
  QHBoxLayout* layoutCurve = new QHBoxLayout();
  layoutMain->addLayout(layoutCurve);
  QLabel *labelCurve = new QLabel("Curve:", &dialog);
  layoutCurve->addWidget(labelCurve);
  _formCurve = new QComboBox(&dialog);
  layoutCurve->addWidget(_formCurve);

  // Buttons
  QHBoxLayout *layoutButtons = new QHBoxLayout();
  layoutMain->addLayout( layoutButtons );
  QPushButton *ok =  new QPushButton("Ok", &dialog);
  layoutButtons->addWidget( ok );
  connect( ok, SIGNAL(clicked()), &dialog, SLOT(accept()) );
  QPushButton *cancel = new QPushButton("Cancel", &dialog);
  layoutButtons->addWidget( cancel );
  connect( cancel, SIGNAL(clicked()), &dialog, SLOT(reject()) );

  // Initialization
  plotSelectedSlot( _formPlot->currentIndex() );

  if ( dialog.exec() ){
    // Dialog executed properly
    if ( _formPlot->currentIndex() >= 0 && _formCurve->currentIndex() >= 0 ){
      *_plotId = _formPlot->currentIndex();
      *_curveId = _formCurve->currentIndex();
      return 0;
    } else {
      return 2;
    }

  } else {
    // Dialog cancelled or not executed correctly
    return 1;
  }

  return 0;
}

void DeleteCurveDialog::plotSelectedSlot(int plotId){

  if ( plotId < 0 )
    return;

  // Remove all previous items from the list
  _formCurve->clear();

  // Retrieve currently selected plot
  QwtPlot* plot = _plots[plotId];

  // Add items according to currently selected plot
  for( int k = 0 ; k != plot->itemList(QwtPlotItem::Rtti_PlotCurve).size() ; ++k )
    _formCurve->addItem(
          plot->itemList(QwtPlotItem::Rtti_PlotCurve).at(k)->title().text() );

  return;
}

