/*!
\file powersystemeditor.h
\brief Definition file for class PowersystemEditor

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Fragkiskos Koufogiannis, fkoufis at gmail dot com
\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef POWERSYSTEMEDITOR_H
#define POWERSYSTEMEDITOR_H

#include <QSplitter>
class QComboBox;

#include <vector>

namespace elabtsaot{

class Powersystem;
class PwsModel;
class PwsSchematicModel;

class TabularPowersystemEditor;
class SchematicEditor;

class PowersystemEditor : public QSplitter {

  Q_OBJECT

 public:

  PowersystemEditor( Powersystem* pws,
                     PwsModel* const pwsModel0,
                     QWidget* parent = 0 );
  ~PowersystemEditor();
  //! Initialize the powersystem editor to incorporate any _pws changes
  int init();
  //! Initialize the powersystem editor to a new schematic model
  int init( PwsSchematicModel* smd );
  //! Update the GUI according to _pws and _smd
  void updt();

  PwsSchematicModel* smd();
  PwsSchematicModel const* smd() const;

 public slots:

  void resetSchematicSlot();
  void clearPowersystem();
  void removeSelectedElements();
  void editElement();
  void addBusSlot();
  void addBranchSlot();
  void addGeneratorSlot();
  void addLoadSlot();
  void setSlackGen();
  void validatePowersystem();

 private:

  Powersystem* _pws;
  PwsSchematicModel* _smd;
  PwsModel* const _pwsModel0; // other PwsModel associated with the power system

  // GUI widgets
  TabularPowersystemEditor* _tbl;
  SchematicEditor* _sch;

};

} // end of namespace elabtsaot

#endif // POWERSYSTEMEDITOR_H
