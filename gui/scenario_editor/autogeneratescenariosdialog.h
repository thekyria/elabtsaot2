
#ifndef AUTOGENERATESCENARIOSDIALOG_H
#define AUTOGENERATESCENARIOSDIALOG_H

#include <QObject>
class QDoubleSpinBox;

namespace elabtsaot{

class AutoGenerateScenariosDialog : public QObject {

  Q_OBJECT

 public:

  AutoGenerateScenariosDialog( double& scenarioStopTime,
                               bool& trip,
                               bool& ignoreLowZBranches,
                               double& faultLocation,
                               double& faultStart,
                               double& faultStop,
                               QWidget* parent );
  int exec();

 public slots:

  void sceStopChanged(double val);
  void faultStopChanged(double val);
  void randomFaultLocationChecked(bool checked);
  void randomFaultStartChecked(bool checked);
  void randomFaultStopChecked(bool checked);

 private:

  double& _scenarioStopTime;
  bool& _trip;
  bool& _ignoreLowZBranches;
  double& _faultLocation;
  double& _faultStart;
  double& _faultStop;

  QDoubleSpinBox* sceStopForm;
  QDoubleSpinBox* faultLocationForm;
  QDoubleSpinBox* faultStartForm;
  QDoubleSpinBox* faultStopForm;

};

} // end of namespace elabtsaot

#endif // AUTOGENERATESCENARIOSDIALOG_H
