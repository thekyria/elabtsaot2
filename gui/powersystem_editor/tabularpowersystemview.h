/*!
\file tabularpowersystemview.h
\brief Definition file for class TabularPowersystemView

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Fragkiskos Koufogiannis, fkoufis at gmail dot com
\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef TABULARPOWERSYSTEMVIEW_H
#define TABULARPOWERSYSTEMVIEW_H

#include <QSplitter>
#include <QModelIndex>
class QLabel;
class QTableView;

namespace elabtsaot{

class TabularPowersystemInterface;

class Powersystem;

// A customized widget to present powersystem elements in tabular view
class TabularPowersystemView : public QSplitter{

  Q_OBJECT

 public:

  // ------------ functions ------------
  TabularPowersystemView( int type,
                          Powersystem* pws,
                          bool editable,
                          QWidget *parent = 0);
  void mouseDoubleClickEvent(QMouseEvent *);
  void updt();
  void clearSelection();
  unsigned int getComponentExtId(QModelIndex const& index);

 public slots:

  void collapseView();
  void expandView();
  void viewClickedSlot(QModelIndex index);

signals:

  void viewClicked(int type, QModelIndex index);

 private:

  // ------------ variables ------------
  unsigned int _type;
  QTableView* _view;
  TabularPowersystemInterface* _model;

};

} // end of namespace elabtsaot

#endif // TABULARPOWERSYSTEMVIEW_H
