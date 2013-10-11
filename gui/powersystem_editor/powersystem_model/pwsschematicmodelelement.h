/*!
\file pwsschematicmodelelement.h
\brief Definition file for class PwsSchematicModelElement

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
\author Fragkiskos Koufogiannis, fkoufis at gmail dot com
*/

#ifndef PWSCHEMATICMODELELEMENT_H
#define PWSCHEMATICMODELELEMENT_H

#include "pwsmodelelement.h"

namespace elabtsaot{

class PwsSchematicModelElement : public PwsModelElement {

 public:

  PwsSchematicModelElement(int type = -1, int extId = 0);

  float x1, y1;     // The starting coordinates of component on schematic editor
  float x2, y2;     // The ending coordinates of component on schematic editor

};

} // end of namespace elabtsaot

#endif // PWSCHEMATICMODELELEMENT_H
