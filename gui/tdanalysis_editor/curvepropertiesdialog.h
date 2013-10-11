/*!
\file curvepropertiesdialog.h

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef CURVEPROPERTIESDIALOG_H
#define CURVEPROPERTIESDIALOG_H

#include <QObject>
#include <QColor>
class QComboBox;
class QLineEdit;
class QSpinBox;
class QPen;
class QwtPlot;
class QPushButton;

#include <vector>

namespace elabtsaot{

class CurvePropertiesDialog : public QObject {

  Q_OBJECT

 public:

  CurvePropertiesDialog( std::vector<QwtPlot*> const& plots,
                         int* plotId, int* curveId,
                         QString* curveName,
                         QPen* curvePen,
                         QWidget* parent = 0 );
  int exec();

 public slots:

  void plotSelectedSlot(int k);
  void curveSelectedSlot(int k);
  void chooseColorSlot();

 private:

  void _setColor( QColor const& color );

  // Output (pointer) arguments
  std::vector<QwtPlot*> _plots;
  int* _plotId;
  int* _curveId;
  QString* _curveName;
  QPen* _curvePen;

  QColor _color; // Internal argument

  // GUI
  QComboBox* _formPlot;
  QComboBox* _formCurve;
  QLineEdit* _formName;
  QPushButton* _colorButton;
//  QComboBox* _formColor;
  QComboBox* _formStyle;
  QSpinBox* _formWidth;

};

} // end of namespace elabtsaot

#endif // CURVEPROPERTIESDIALOG_H
