/*!
\file tabularpowersysteminterface.h
\brief Definition file for class TabularPowersystemInterface

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Fragkiskos Koufogiannis, fkoufis at gmail dot com
\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef TABULARPOWERSYSTEMINTERFACE_H
#define TABULARPOWERSYSTEMINTERFACE_H

#include <QAbstractTableModel>

namespace elabtsaot{

class Powersystem;

// An abstract class to provide interface of the powersystem to tabular view
class TabularPowersystemInterface : public QAbstractTableModel{

  Q_OBJECT

 public:

  //! Constructor
  TabularPowersystemInterface(int type,
                              Powersystem* pws,
                              QObject* parent = 0);
  //! Number of components in the set
  int rowCount(QModelIndex const& index) const;
  //! Number of attributes in each component
  int columnCount(QModelIndex const& index) const;
  //! Retrieve a single attribute of one component
  QVariant data(QModelIndex const& index, int role) const;
  //! Set a single attribute of one component
  bool setData(QModelIndex const& index, QVariant const& value, int role);
  //! The header titles of component's attributes
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  //! All matrices are editable
  Qt::ItemFlags flags(QModelIndex const& index) const;
  //! Append an empty component
  //virtual bool insertRow(int row, QModelIndex const& index);
  //virtual bool removeRow(int row);
  //! Inform model that data has changed
  void update();

 private:

  int _type;         // 0: Bus, 1: Branch, 2: Generator, 3: Load
  Powersystem* _pws; // pointer to the powersystem for which the interface is
                     // implemented

};

} // end of namespace elabtsaot

#endif // TABULARPOWERSYSTEMINTERFACE_H
