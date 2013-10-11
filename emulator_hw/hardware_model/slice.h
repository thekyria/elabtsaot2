/*!
\file slice.h
\brief Definition file for struct Slice

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef SLICE_H
#define SLICE_H


#include "sliceanalog.h"
#include "slicedigital.h"

namespace elabtsaot{

struct Slice{

  int reset(int sliceIndex, bool complete);
  int reset(bool complete);

  SliceAnalog ana;
  SliceDigital dig;
  int sliceIndex; //!< slice order in structure; slice 0 is the bottom (master)

};

} // end of namespace elabtsaot

#endif // SLICE_H
