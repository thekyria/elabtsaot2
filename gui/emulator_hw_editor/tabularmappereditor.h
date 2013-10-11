/*!
\file tabularmappereditor.h
\brief Definition file for class TabularMapperEditor

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Fragkiskos Koufogiannis, fkoufis at gmail dot com
\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef TABULARMAPPEREDITOR_H
#define TABULARMAPPEREDITOR_H

#include <QSplitter>
#include <QModelIndex>

namespace elabtsaot{

class PwsMapperModel;

class TabularMapperView;

class TabularMapperEditor : public QSplitter{

  Q_OBJECT

 public:

  TabularMapperEditor( PwsMapperModel const* mmd,
                       bool editable,
                       QWidget* parent = 0);

 public slots:

  void viewClicked(int type, QModelIndex index);
  void updt();
  void clearSelection();

 signals:

  void componentSelected(int type, unsigned int extId);

 private:

  PwsMapperModel const* _mmd;
  bool _editable;
  TabularMapperView* busView;
  TabularMapperView* branchView;
  TabularMapperView* genView;
  TabularMapperView* loadView;

};

} // end of namespace elabtsaot

#endif // TABULARMAPPEREDITOR_H
