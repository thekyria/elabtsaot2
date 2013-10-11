/*!
\file deletecurvedialog.h

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef DELETECURVEDIALOG_H
#define DELETECURVEDIALOG_H

#include <QObject>
class QComboBox;
class QwtPlot;

#include <vector>

namespace elabtsaot{

class DeleteCurveDialog : public QObject {

  Q_OBJECT

 public:

  DeleteCurveDialog( std::vector<QwtPlot*> const& plots,
                     int* plotId, int* curveId,
                     QWidget* parent = 0 );
  int exec();

 public slots:

  void plotSelectedSlot(int k);

 private:

  std::vector<QwtPlot*> _plots;
  int* _plotId;
  int* _curveId;

  QComboBox *_formPlot;
  QComboBox *_formCurve;

};

} // end of namespace elabtsaot

#endif // DELETECURVEDIALOG_H
