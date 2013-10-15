/*!
\file mainwindow.h
\brief Definition file for class MainWindow

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Fragkiskos Koufogiannis, fkoufis at gmail dot _com
\author Theodoros Kyriakidis, thekyria at gmail dot _com, Electronics Laboratory
EPFL
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
class QStackedWidget;

namespace elabtsaot{

class Powersystem;
class PwsSchematicModel;
class Emulator;
class SSEngine;
class MoteurRenard;
class MoteurFengtian;
class TDEngine;
class TDEmulator;
class Simulator_sw;
class ScenarioSet;
class TDResultsBank;

class SidePane;
class Console;
class PowersystemEditor;
class SSAnalysisEditor;
class ScenarioEditor;
class TDAnalysisEditor;

class CommunicationEditor;
class CalibrationEditor;
class MapperEditor;
class FitterEditor;
class AuxiliaryEditor;

class MainWindow : public QMainWindow{

  Q_OBJECT

 public:

  MainWindow( Powersystem*& pws,
              Emulator* emu,
              SSEngine*& sse,
              MoteurRenard* sse_mrn,
              MoteurFengtian* sse_fen,
              TDEngine*& tde,
              TDEmulator* tde_hwe,
              Simulator_sw*& tde_swe,
              ScenarioSet* scs,
              TDResultsBank* trb );
  ~MainWindow();

  Console* con();
  ScenarioEditor* sce();

 public slots:

  void showSplash();                //!< Show the splash screen
  void showPowersystem();           //!< Show the network editor
  void showSSAnalysis();            //!< Show the Steady State Analysis editor
  void showScenarios();             //!< Show the scenarios editor
  void showTDAnalysis();            //!< Show the analysis editor

  void showCommunication();         //!< Show the communication editor
  void showCalibration();           //!< Show the calibration editor
  void showMapper();                //!< Show the mapper editor
  void showFitter();                //!< Show the fitter editor
  void showAuxiliary();             //!< Show the auxiliary editor

  void loadSlot();
  void importPowersystemSlot();
  void importSchematicSlot();
  void importMappingSlot();
  void importScenarioSetSlot();
  void exportPowersystemSlot();
  void exportSchematicSlot();
  void exportMappingSlot();
  void exportScenarioSetSlot();

  void usbChangedSlot();
  void emuChangedSlot(bool complete);
  void mmdChangedSlot();

 public:

  //! Pointers to backend classes
  //@{
  Powersystem*& _pws;
  Emulator* _emu;
  ScenarioSet* _scs;
  //@}

 private:

  void importPowersystem( QString const& filename );
  void importSchematic( QString const& filename );
  void importMapping( QString const& filename );
  void importScenarioSet( QString const& filename );

  //! Pointers to frontend classes
  QStackedWidget* topStack;
  SidePane* _sip;             //!< ptr to the sidepane
  Console* _con;              //!< ptr to the console
  PowersystemEditor* _net;    //!< ptr to the network editor
  SSAnalysisEditor* _SSanl;   //!< ptr to the ss analysis editor

  ScenarioEditor* _sce;       //!< ptr to the scenario editor
  TDAnalysisEditor* _TDanl;   //!< ptr to the td analysis editor

  CommunicationEditor* _com;  //!< ptr to the communication editor
  CalibrationEditor* _cal;    //!< ptr to the calibration editor
  MapperEditor* _map;         //!< ptr to the mapper editor
  FitterEditor* _fit;         //!< ptr to the fitter editor
  AuxiliaryEditor* _aux;      //!< ptr to the auxiliary editos

};

} // end of namespace elabtsaot

#endif // MAINWINDOW_H
