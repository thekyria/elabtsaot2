
#include "tdanalysiseditordialogs.h"
using namespace elabtsaot;

#include "tdresults.h"
#include "tdresultsplotproperties.h"
#include "emulator.h"

#include <QDialog>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QIcon>
#include <QSpinBox>
#include <QString>
#include <QLineEdit>
#include <QStringList>
#include <QTableWidget>
#include <QHeaderView>
#include <QGroupBox>
#include <QTextEdit>

#include <qwt_plot.h>

//#include <vector>
using std::vector;
//#include <map>
using std::map;

int TDAnalysisEditorDialogs::setTDEngineTimeStepDialog( double* timeStep ){

  QDialog dialog;
  dialog.setWindowTitle("Simulation options");
  QVBoxLayout* layout = new QVBoxLayout;
  dialog.setLayout(layout);

  QHBoxLayout* stepll = new QHBoxLayout;
  layout->addLayout(stepll);
  QLabel* stepl= new QLabel("Stepsize");
  stepll->addWidget(stepl);
  QDoubleSpinBox* step = new QDoubleSpinBox;
  stepll->addWidget(step);
  step->setDecimals(8);
  step->setRange(0.000001,1.0);
  step->setSingleStep(0.001);
  step->setValue(*timeStep);

  QHBoxLayout okcancel;
  layout->addLayout(&okcancel);
  QPushButton* okb = new QPushButton("OK");
  okcancel.addWidget(okb);
  dialog.connect(okb, SIGNAL(clicked()), &dialog, SLOT(accept()));
  QPushButton* cancelb = new QPushButton("Cancel");
  okcancel.addWidget(cancelb);
  dialog.connect(cancelb, SIGNAL(clicked()), &dialog, SLOT(reject()));

  if (dialog.exec()) {
  // Dialog executed properly
    *timeStep = step->value();
    return 0;
  } else {
    return 1;
  }

  return 0;
}

int TDAnalysisEditorDialogs::selectPlotDialog( vector<QwtPlot*> const& plots,
                                             int* selectedPlot ){
  // Dialog to enter parameters of the new plot
  QDialog dialog;
  dialog.setWindowTitle("Select Plot");
  QVBoxLayout layoutMain;
  dialog.setLayout(&layoutMain);
  // name of plot
  QHBoxLayout layoutName;
  layoutMain.addLayout(&layoutName);
  QLabel labelName("Plot:");
  layoutName.addWidget(&labelName);
  QComboBox formName;
  layoutName.addWidget(&formName);
  for ( size_t k = 0 ; k != plots.size() ; ++k )
    formName.addItem( plots[k]->title().text() );

  // Buttons
  QHBoxLayout layoutButtons;
  layoutMain.addLayout( &layoutButtons );
  QPushButton ok("Ok");
  layoutButtons.addWidget( &ok );
  dialog.connect(&ok, SIGNAL(clicked()), &dialog, SLOT(accept()));
  QPushButton cancel("Cancel");
  layoutButtons.addWidget( &cancel );
  dialog.connect(&cancel, SIGNAL(clicked()), &dialog, SLOT(reject()));

  if( dialog.exec() ){
    // Dialog executed properly
    if ( formName.currentIndex() >= 0 ){
      // Valid item selected in the combobox
      *selectedPlot = formName.currentIndex();
      return 0;
    } else {
      // Invalid selection
      return 1;
    }
  } else {
    // Dialog cancelled or not executed correctly
    return 1;
  }

  return 0;
}

int TDAnalysisEditorDialogs::selectPlotDialog( vector<QwtPlot*> const& plots,
                map<QwtPlot*, TDResultsPlotProperties> const& plotPropertiesMap,
                TDResultsPlotProperties const& curveTdrpp,
                int* selectedPlot ){

  // Dialog to enter parameters of the new plot
  QDialog dialog;
  dialog.setWindowTitle("Select Plot");
  QVBoxLayout layoutMain;
  dialog.setLayout(&layoutMain);
  // name of plot
  QHBoxLayout layoutName;
  layoutMain.addLayout(&layoutName);
  QLabel labelName("Plot:");
  layoutName.addWidget(&labelName);
  QComboBox formName;
  layoutName.addWidget(&formName);
  // Populate comboBox only with plot names of plots combatible with the
  // requested curve tdrpp
  for ( size_t k = 0 ; k != plots.size() ; ++k ){
    TDResultsPlotProperties plotTdrpp = plotPropertiesMap.at(plots.at(k));
    if ( plotTdrpp.variable == curveTdrpp.variable )
      formName.addItem( plots[k]->title().text(), k );
  }

  // Buttons
  QHBoxLayout layoutButtons;
  layoutMain.addLayout( &layoutButtons );
  QPushButton ok("Ok");
  layoutButtons.addWidget( &ok );
  dialog.connect(&ok, SIGNAL(clicked()), &dialog, SLOT(accept()));
  QPushButton cancel("Cancel");
  layoutButtons.addWidget( &cancel );
  dialog.connect(&cancel, SIGNAL(clicked()), &dialog, SLOT(reject()));

  if( dialog.exec() ){
    // Dialog executed properly
    if ( formName.currentIndex() >= 0 ){
      // Valid item selected in the combobox
      bool ok;
      *selectedPlot = formName.itemData( formName.currentIndex() ).toInt(&ok);
      if ( !ok ) return 2;
      return 0;
    } else {
      // Invalid selection
      return 1;
    }
  } else {
    // Dialog cancelled or not executed correctly
    return 1;
  }

  return 0;
}
