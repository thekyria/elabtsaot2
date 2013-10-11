
#include "curvepropertiesdialog.h"
using namespace elabtsaot;

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_series_data.h>
#include <qwt_legend.h>

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QSplitter>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QColorDialog>

//#include <vector>
using std::vector;

CurvePropertiesDialog::CurvePropertiesDialog( vector<QwtPlot*> const& plots,
                                              int* plotId, int* curveId,
                                              QString* curveName,
                                              QPen* curvePen,
                                              QWidget* parent ) :
  QObject( parent ),
  _plots(plots),
  _plotId(plotId),
  _curveId(curveId),
  _curveName(curveName),
  _curvePen(curvePen),
  _color(curvePen->color()){}

int CurvePropertiesDialog::exec(){

  QDialog* dialog = new QDialog();
  dialog->setWindowTitle("Curve properties");
  QVBoxLayout* layoutMain = new QVBoxLayout();
  dialog->setLayout(layoutMain);

  // Plot box
  QHBoxLayout *layoutPlot = new QHBoxLayout();
  layoutMain->addLayout(layoutPlot);
  QLabel *labelPlot = new QLabel("Plot:");
  layoutPlot->addWidget(labelPlot);
  _formPlot = new QComboBox();
  layoutPlot->addWidget(_formPlot);
  for ( size_t k = 0 ; k != _plots.size() ; ++k )
    _formPlot->addItem( _plots[k]->title().text() );
  connect( _formPlot, SIGNAL(currentIndexChanged(int)),
           this, SLOT(plotSelectedSlot(int)) );

  // Curves box
  QHBoxLayout* layoutCurve = new QHBoxLayout();
  layoutMain->addLayout(layoutCurve);
  QLabel *labelCurve = new QLabel("Curve:");
  layoutCurve->addWidget(labelCurve);
  _formCurve = new QComboBox();
  layoutCurve->addWidget(_formCurve);
  connect( _formCurve, SIGNAL(currentIndexChanged(int)),
           this, SLOT(curveSelectedSlot(int)) );

  // Curve name box
  QHBoxLayout *layoutName = new QHBoxLayout();
  layoutMain->addLayout(layoutName);
  QLabel *labelName = new QLabel("Title:");
  layoutName->addWidget(labelName);
  _formName = new QLineEdit();
  layoutName->addWidget(_formName);

  // color
  QHBoxLayout *layoutColor = new QHBoxLayout();
  layoutMain->addLayout(layoutColor);
  QLabel *labelColor = new QLabel("Color:");
  layoutColor->addWidget(labelColor);
  _colorButton = new QPushButton(QIcon(), "Choose color");
  layoutColor->addWidget( _colorButton );
  _setColor( _color );
  connect( _colorButton, SIGNAL(clicked()),
           this, SLOT(chooseColorSlot()) );
//  _formColor = new QComboBox();
//  layoutColor->addWidget(_formColor);
//  _formColor->addItem("Black");
//  _formColor->addItem("Dark Cyan");
//  _formColor->addItem("Dark Magenta");
//  _formColor->addItem("Dark Yellow");
//  _formColor->addItem("Dark Gray");
//  _formColor->addItem("Dark Blue");
//  _formColor->addItem("Dark Green");
//  _formColor->addItem("Dark Red");


  //Style
  QHBoxLayout *layoutStyle = new QHBoxLayout();
  layoutMain->addLayout(layoutStyle);
  QLabel *labelStyle = new QLabel("Style:");
  layoutStyle->addWidget(labelStyle);
  _formStyle = new QComboBox();
  layoutStyle->addWidget(_formStyle);
  _formStyle->addItem("SolidLine");
  _formStyle->addItem("DashLine");
  _formStyle->addItem("DashDotDotLine");
  _formStyle->addItem("DotLine");

  //Width
  QHBoxLayout *layoutWidth = new QHBoxLayout();
  layoutMain->addLayout(layoutWidth);
  QLabel *labelWidth = new QLabel("Width:");
  layoutWidth->addWidget(labelWidth);
  _formWidth = new QSpinBox();
  layoutWidth->addWidget(_formWidth);
  _formWidth->setMaximum(10);
  _formWidth->setValue(1);

  // Buttons
  QHBoxLayout* layoutButtons = new QHBoxLayout();
  layoutMain->addLayout( layoutButtons );
  QPushButton *ok =  new QPushButton("Ok");
  layoutButtons->addWidget( ok );
  connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
  QPushButton *cancel = new QPushButton("Cancel");
  layoutButtons->addWidget( cancel );
  connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));

  // Initialization
  plotSelectedSlot( _formPlot->currentIndex() );
  curveSelectedSlot( _formCurve->currentIndex() );

  if ( dialog->exec() ){
    // Dialog executed properly
    if ( _formPlot->currentIndex() < 0 || _formCurve->currentIndex() < 0 )
      return 2;

    // Prepare curve QPen from selections
    QPen pen;
    pen.setColor( _color );
//    switch ( _formColor->currentIndex() ){
//    case 0:
//      pen.setColor( Qt::black );
//      break;
//    case 1:
//      pen.setColor( Qt::darkCyan );
//      break;
//    case 2:
//      pen.setColor( Qt::darkMagenta );
//      break;
//    case 3:
//      pen.setColor( Qt::darkYellow );
//      break;
//    case 4:
//      pen.setColor( Qt::darkGray );
//      break;
//    case 5:
//      pen.setColor( Qt::darkBlue );
//      break;
//    case 6:
//      pen.setColor( Qt::darkGreen );
//      break;
//    case 7:
//      pen.setColor( Qt::darkRed );
//      break;
//    default:
//      pen.setColor( Qt::black );
//      break;
//    }
    pen.setWidth( _formWidth->value() );
    switch ( _formStyle->currentIndex() ){
    case 0:
      pen.setStyle( Qt::SolidLine );
      break;
    case 1:
      pen.setStyle( Qt::DashLine );
      break;
    case 2:
      pen.setStyle( Qt::DashDotDotLine );
      break;
    case 3:
      pen.setStyle( Qt::DotLine );
      break;
    default:
      pen.setStyle( Qt::SolidLine );
      break;
    }

    // Update output arguments
    *_plotId = _formPlot->currentIndex();
    *_curveId = _formCurve->currentIndex();
    *_curveName = _formName->text();
    *_curvePen = pen;

    return 0;
  } else {
    // Dialog cancelled or not executed correctly
    return 1;
  }

  return 0;
}

void CurvePropertiesDialog::plotSelectedSlot(int plotId){

  if ( plotId < 0 )
    return;

  _formCurve->blockSignals(true);  // Disable _formCurve signals

  // Remove all previous items from the list
  _formCurve->clear();
  // Retrieve currently selected plot
  QwtPlot* plot = _plots[plotId];
  // Add items according to currently selected plot
  for( int k = 0 ; k != plot->itemList(QwtPlotItem::Rtti_PlotCurve).size() ; ++k )
    _formCurve->addItem(
          plot->itemList(QwtPlotItem::Rtti_PlotCurve).at(k)->title().text() );

  // Initialize according to new _formCurve current index
  curveSelectedSlot( _formCurve->currentIndex() );

  _formCurve->blockSignals(false); // Reenable _formCurve signals
  return;
}

void CurvePropertiesDialog::curveSelectedSlot(int curveId){

  if ( curveId < 0 )
    return;

  // Retrieve currently selected curve
  QwtPlot* plot = _plots.at(_formPlot->currentIndex());
  QwtPlotCurve* curve = static_cast<QwtPlotCurve*>(
        plot->itemList( QwtPlotItem::Rtti_PlotCurve ).at( curveId ) );

  // Set _formName
  _formName->setText( curve->title().text() );

  // Set _color
  _setColor( curve->pen().color() );

//  // Set _formColor
//  if( curve->pen().color()==Qt::black )
//    _formColor->setCurrentIndex(0);
//  else if( curve->pen().color()==Qt::darkCyan )
//    _formColor->setCurrentIndex(1);
//  else if( curve->pen().color()==Qt::darkMagenta )
//    _formColor->setCurrentIndex(2);
//  else if( curve->pen().color()==Qt::darkYellow )
//    _formColor->setCurrentIndex(3);
//  else if( curve->pen().color()==Qt::darkGray )
//    _formColor->setCurrentIndex(4);
//  else if( curve->pen().color()==Qt::darkBlue )
//    _formColor->setCurrentIndex(5);
//  else if( curve->pen().color()==Qt::darkGreen )
//    _formColor->setCurrentIndex(6);
//  else if( curve->pen().color()==Qt::darkRed )
//    _formColor->setCurrentIndex(7);

  // Set _formWidth
  _formWidth->setValue(curve->pen().width());

  // Set _formStyle
  if( curve->pen().style()==Qt::SolidLine )
    _formStyle->setCurrentIndex(0);
  else if( curve->pen().style()==Qt::DashLine )
    _formStyle->setCurrentIndex(1);
  else if( curve->pen().style()==Qt::DashDotDotLine )
    _formStyle->setCurrentIndex(2);
  else if( curve->pen().style()==Qt::DotLine )
    _formStyle->setCurrentIndex(3);

  return;
}

void CurvePropertiesDialog::chooseColorSlot(){
  QColor newColor = QColorDialog::getColor( _color );
  if ( !newColor.isValid() )
    return;
  _setColor( newColor );
  return;
}

void CurvePropertiesDialog::_setColor( QColor const& color ){
  _color = color;
  int const THRESHOLD = 105;
  int BackgroundDelta =   (_color.red() * 0.299)
                        + (_color.green() * 0.587)
                        + (_color.blue() * 0.114);
  QColor textColor = QColor((255- BackgroundDelta < THRESHOLD) ? Qt::black : Qt::white);

  QString const COLOR_STYLE("QPushButton { background-color : %1; color : %2; }");
  _colorButton->setStyleSheet( COLOR_STYLE.arg(color.name()).arg(textColor.name()) );
}
