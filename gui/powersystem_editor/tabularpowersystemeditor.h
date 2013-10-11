/*!
\file tabularpowersystemeditor.h
\brief Definition file for class TabularPowerSystemEditor

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Fragkiskos Koufogiannis, fkoufis at gmail dot com
\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef TABULARPOWERSYSTEMEDITOR_H
#define TABULARPOWERSYSTEMEDITOR_H

#include <QSplitter>
#include <QModelIndex>

namespace elabtsaot{

class Powersystem;

class TabularPowersystemView;

class TabularPowersystemEditor : public QSplitter{

  Q_OBJECT

 public:

  TabularPowersystemEditor( Powersystem* pws, bool editable,
                            QWidget* parent = 0);

 public slots:

  void viewClicked(int type, QModelIndex index);
  void updt();
  void expandAll();
  void collapseAll();

 signals:

  void componentSelected(int type, unsigned int extId);

 private:

  TabularPowersystemView* _busView;
  TabularPowersystemView* _branchView;
  TabularPowersystemView* _genView;
  TabularPowersystemView* _loadView;

};

} // end of namespace elabtsaot

#endif // TABULARPOWERSYSTEMEDITOR_H
