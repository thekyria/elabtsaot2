/*!
\file tabularmapperview.h
\brief Definition file for class TabularMapperView

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Fragkiskos Koufogiannis, fkoufis at gmail dot com
\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef TABULARMAPPERVIEW_H
#define TABULARMAPPERVIEW_H

#include <QSplitter>
#include <QModelIndex>
class QLabel;
class QTableView;

namespace elabtsaot{

class TabularMapperInterface;
class PwsMapperModel;

//! A customized widget to present components in tabular view
class TabularMapperView : public QSplitter{

  Q_OBJECT

 public:

  // ------------ functions ------------
  TabularMapperView(int type,
                    PwsMapperModel const* mmd,
                    bool editable,
                    QWidget *parent = 0);
  void updt();
  void clearSelection();
  unsigned int getComponentExtId(QModelIndex const& index);

 public slots:

  void viewClickedSlot(QModelIndex index);

 signals:

  void viewClicked(int type, QModelIndex index);

 private:

  // ------------ variables ------------
  int _type;
  QTableView* _view;
  TabularMapperInterface* _model;

};

} // end of namespace elabtsaot

#endif // TABULARMAPPERVIEW_H
