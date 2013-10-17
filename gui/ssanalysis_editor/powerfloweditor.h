
#ifndef POWERFLOWEDITOR_H
#define POWERFLOWEDITOR_H

#include <QSplitter>

namespace elabtsaot{

class Powersystem;
class SSEngine;
class TabularPowersystemEditor;

class PowerFlowEditor : public QSplitter {

  Q_OBJECT

 public:

  PowerFlowEditor( Powersystem*& pws, SSEngine* const& sse, QWidget* parent = 0);
  ~PowerFlowEditor(){}

 public slots:

  void updt();
  void powerFlowPowersystem() const;
  void logPowerFlowResults();

 private:

  static int logPowerFlowDialog(bool* toConsole, QString& filename);

  Powersystem*& _pws;
  SSEngine* const& _sse;
  TabularPowersystemEditor* _tbl;

};

} // end of namespace elabtsaot

#endif // POWERFLOWEDITOR_H
