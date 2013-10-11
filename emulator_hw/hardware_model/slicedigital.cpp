
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
  pipe_PQ(MAXPQNODES, VERTICALNUMBEROFATOMS, HORIZONTALNUMBEROFATOMS),
  pipe_slack(MAXSLACKNODES, VERTICALNUMBEROFATOMS, HORIZONTALNUMBEROFATOMS),
  pipe_gen(MAXGENERATORS, VERTICALNUMBEROFATOMS, HORIZONTALNUMBEROFATOMS),
  pipe_zload(MAXILOADS, VERTICALNUMBEROFATOMS, HORIZONTALNUMBEROFATOMS),
  pipe_iload(MAXILOADS, VERTICALNUMBEROFATOMS, HORIZONTALNUMBEROFATOMS),
  pipe_pload(MAXPLOADS, VERTICALNUMBEROFATOMS, HORIZONTALNUMBEROFATOMS) {}

void SliceDigital::reset(){
  // Reset PF pipelines
  pipe_PQ.reset();
  pipe_slack.reset();
  // Reset TD pipelines
  pipe_gen.reset();
  pipe_zload.reset();
  pipe_iload.reset();
  pipe_pload.reset();
}

int SliceDigital::remove(size_t ver, size_t hor){
  int ans = 0;
  // remove PF pipelines
//  ans |= pipe_PQ.remove_element(ver, hor);
//  ans |= pipe_slack.remove_element(ver, hor);
  // remove TD pipelines
  ans |= pipe_gen.remove_element(ver, hor);
  ans |= pipe_zload.remove_element(ver, hor);
  ans |= pipe_iload.remove_element(ver, hor);
  ans |= pipe_pload.remove_element(ver, hor);
  return ans;
}
