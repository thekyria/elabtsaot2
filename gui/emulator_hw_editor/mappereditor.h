/*!
\file mappereditor.h
\brief Definition file for class MapperEditor

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Fragkiskos Koufogiannis, fkoufis at gmail dot com
\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef MAPPEREDITOR_H
#define MAPPEREDITOR_H

#include <QSplitter>
class QToolBar;
class QAction;

#include <vector>

namespace elabtsaot{

class Emulator;

class TabularMapperEditor;
class MapperEmulatorEditor;

class MapperEditor : public QSplitter{

  Q_OBJECT

 public:

  MapperEditor( Emulator* emu, QWidget* parent);
  int init();

 public slots:

  void updt();
  void resetMappingSlot();
  void autoMappingSlot();
  void validateMappingSlot();
  void unmapComponentSlot();
  void componentSelectedSlot(int type, unsigned int extId);
  void mappingPositionClickedSlot(int mapper_tab,
                                  int mapper_row,
                                  int mapper_col );
 private:

  Emulator* _emu;

  TabularMapperEditor* tbl;
  MapperEmulatorEditor* mee;

  std::vector<std::pair<int, unsigned int> > _selected;

};

} // end of namespace elabtsaot

#endif // MAPPEREDITOR_H
