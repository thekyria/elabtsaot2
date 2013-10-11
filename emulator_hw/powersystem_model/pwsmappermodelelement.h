/*!
\file pwsmappermodelelement.h
\brief Definition file for class PwsMapperModelElement

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
\author Fragkiskos Koufogiannis, fkoufis at gmail dot com
*/

#ifndef PWSMAPPERMODELELEMENT_H
#define PWSMAPPERMODELELEMENT_H

#include "pwsmodelelement.h"

namespace elabtsaot{

class PwsMapperModelElement : public PwsModelElement {

 public:

  PwsMapperModelElement(int type = -1, int extId = 0);

  bool mapped;       // Is component mapped on emulator?
  int tab;           // If yes, this is its slice placement,
  int row;           // its row mapping,
  int col;           // and its column mapping

};

} // end of namespace elabtsaot

#endif // PWSMAPPERMODELELEMENT_H
