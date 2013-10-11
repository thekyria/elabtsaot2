
#ifndef LOADFLOWEDITOR_H
#define LOADFLOWEDITOR_H

#include <QSplitter>

namespace elabtsaot{

class Powersystem;
class SSEngine;
class TabularPowersystemEditor;

class LoadflowEditor : public QSplitter {

  Q_OBJECT

 public:

  LoadflowEditor( Powersystem*& pws, SSEngine* const& sse, QWidget* parent = 0);
  ~LoadflowEditor(){}

 public slots:

  void updt();
  void loadflowPowersystem() const;
  void logLoadflowResults();

 private:

  static int logLoadflowDialog(bool* toConsole, QString& filename);

  Powersystem*& _pws;
  SSEngine* const& _sse;
  TabularPowersystemEditor* _tbl;

};

} // end of namespace elabtsaot

#endif // LOADFLOWEDITOR_H
