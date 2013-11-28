
#include "slicedigital.h"
using namespace elabtsaot;

#define MAXPQNODES 18
#define MAXSLACKNODES 1
#define MAXGENERATORS 8           //!< Default max number of gens in pipeline
#define MAXZLOADS 24              //!< Default max number of zloads in pipeline
#define MAXILOADS 24              //!< Default max number of iloads in pipeline
#define MAXPLOADS 24              //!< Default max number of ploads in pipeline

SliceDigital::SliceDigital():
    pipe_GPFPQ(MAXPQNODES, MAX_VERATOMCOUNT, MAX_HORATOMCOUNT),
    pipe_GPFslack(MAXSLACKNODES, MAX_VERATOMCOUNT, MAX_HORATOMCOUNT),
    pipe_TDgen(MAXGENERATORS, MAX_VERATOMCOUNT, MAX_HORATOMCOUNT),
    pipe_TDzload(MAXILOADS, MAX_VERATOMCOUNT, MAX_HORATOMCOUNT),
    pipe_TDiload(MAXILOADS, MAX_VERATOMCOUNT, MAX_HORATOMCOUNT),
    pipe_TDpload(MAXPLOADS, MAX_VERATOMCOUNT, MAX_HORATOMCOUNT) {
  for (size_t k(0);k!=MAX_VERATOMCOUNT;++k){
    for (size_t m(0);m!=MAX_HORATOMCOUNT;++m){
      injectionTypes[k][m]=NODE_NOINJECTION;
      VInjections[k][m]=0.;
      IInjections[k][m]=0.;
    }
  }
}

void SliceDigital::reset(){
  // Reset PF pipelines
  pipe_GPFPQ.reset();
  pipe_GPFslack.reset();
  // Reset TD pipelines
  pipe_TDgen.reset();
  pipe_TDzload.reset();
  pipe_TDiload.reset();
  pipe_TDpload.reset();

  // Reset node injections
  for (size_t k(0);k!=MAX_VERATOMCOUNT;++k){
    for (size_t m(0);m!=MAX_HORATOMCOUNT;++m){
      injectionTypes[k][m]=NODE_NOINJECTION;
      VInjections[k][m]=0.;
      IInjections[k][m]=0.;
    }
  }
}

int SliceDigital::remove(size_t ver, size_t hor){
  int ans = 0;
  // remove PF pipelines
  ans |= pipe_GPFPQ.remove_element(ver, hor);
  ans |= pipe_GPFslack.remove_element(ver, hor);
  // remove TD pipelines
  ans |= pipe_TDgen.remove_element(ver, hor);
  ans |= pipe_TDzload.remove_element(ver, hor);
  ans |= pipe_TDiload.remove_element(ver, hor);
  ans |= pipe_TDpload.remove_element(ver, hor);
  // reset node injection
  injectionTypes[ver][hor]=NODE_NOINJECTION;
  VInjections[ver][hor]=0.;
  IInjections[ver][hor]=0.;
  return ans;
}
