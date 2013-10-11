/*!
\file sanalysiseditor.h

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef SSANALYSISEDITOR_H
#define SSANALYSISEDITOR_H

#include <QSplitter>
#include <QComboBox>

namespace elabtsaot{

class Powersystem;
class SSEngine;
class MoteurRenard;
class MoteurFengtian;

class LoadflowEditor;

class SSAnalysisEditor : public QSplitter {

  Q_OBJECT

 public:

  SSAnalysisEditor( Powersystem*& pws,
                    SSEngine*& sse,
                    MoteurRenard* sse_mrn,
                    MoteurFengtian* sse_fen,
                    QWidget* parent = 0 );
  ~SSAnalysisEditor(){}

 public slots:

  void updt();
  void SSEngineSelectionSlot(int);
  void SSsetOptionsSlot();

 private:

  SSEngine*& _sse;
  MoteurRenard* _sse_mrn;
  MoteurFengtian* _sse_fen;

  // GUI widgets
  LoadflowEditor* loadflowEditor;
  QComboBox* ssEngineSelectBox;

};

} // end of namespace elabtsaot

#endif // SSANALYSISEDITOR_H
