
#include "slice.h"
using namespace elabtsaot;

int Slice::reset( int sliceIndex, bool complete ){
  this->sliceIndex = sliceIndex;
  return reset(complete);
}

int Slice::reset( bool complete ){
  if (int ans = ana.reset(complete))
    return ans;
  dig.reset();
  return 0;
}
