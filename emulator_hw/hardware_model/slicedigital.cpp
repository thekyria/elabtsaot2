
#include "slicedigital.h"
using namespace elabtsaot;

#define HORIZONTALNUMBEROFATOMS 6 //!< Default horizontal number of atoms
#define VERTICALNUMBEROFATOMS 4   //!< Default vertical number of atoms
#define MAXPQNODES 24
#define MAXSLACKNODES 1
#define MAXGENERATORS 8           //!< Default max number of gens in pipeline
#define MAXZLOADS 24              //!< Default max number of zloads in pipeline
#define MAXILOADS 24              //!< Default max number of iloads in pipeline
#define MAXPLOADS 24              //!< Default max number of ploads in pipeline

SliceDigital::SliceDigital():
  pipe_PFPQ(MAXPQNODES, VERTICALNUMBEROFATOMS, HORIZONTALNUMBEROFATOMS),
  pipe_PFslack(MAXSLACKNODES, VERTICALNUMBEROFATOMS, HORIZONTALNUMBEROFATOMS),
  pipe_TDgen(MAXGENERATORS, VERTICALNUMBEROFATOMS, HORIZONTALNUMBEROFATOMS),
  pipe_TDzload(MAXILOADS, VERTICALNUMBEROFATOMS, HORIZONTALNUMBEROFATOMS),
  pipe_TDiload(MAXILOADS, VERTICALNUMBEROFATOMS, HORIZONTALNUMBEROFATOMS),
  pipe_TDpload(MAXPLOADS, VERTICALNUMBEROFATOMS, HORIZONTALNUMBEROFATOMS) {}

void SliceDigital::reset(){
  // Reset PF pipelines
  pipe_PFPQ.reset();
  pipe_PFslack.reset();
  // Reset TD pipelines
  pipe_TDgen.reset();
  pipe_TDzload.reset();
  pipe_TDiload.reset();
  pipe_TDpload.reset();
}

int SliceDigital::remove(size_t ver, size_t hor){
  int ans = 0;
  // remove PF pipelines
  ans |= pipe_PFPQ.remove_element(ver, hor);
  ans |= pipe_PFslack.remove_element(ver, hor);
  // remove TD pipelines
  ans |= pipe_TDgen.remove_element(ver, hor);
  ans |= pipe_TDzload.remove_element(ver, hor);
  ans |= pipe_TDiload.remove_element(ver, hor);
  ans |= pipe_TDpload.remove_element(ver, hor);
  return ans;
}
