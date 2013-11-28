/*!
\file slicedigital.h
\brief Definition file for class SliceDigital

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef SLICEDIGITAL_H
#define SLICEDIGITAL_H

#include "gpfpqpipeline.h"
#include "gpfslackpipeline.h"
#include "tdgeneratorpipeline.h"
#include "tdconstzloadpipeline.h"
#include "tdconstiloadpipeline.h"
#include "tdconstploadpipeline.h"

#define MAX_VERATOMCOUNT 4 //!< Default vertical number of atoms
#define MAX_HORATOMCOUNT 6 //!< Default horizontal number of atoms

enum NodeInjectionType{
  NODE_NOINJECTION,
  NODE_VINJECTION,
  NODE_IINJECTION
};

namespace elabtsaot{

class SliceDigital{

 public:

  SliceDigital();
  void reset();
  int remove(size_t ver, size_t hor); //!< Remove all (ver,hor) pipeline entries

  // PF specific pipelines
  GPFPQPipeline pipe_GPFPQ;
  GPFSlackPipeline pipe_GPFslack;

  // TD specific pipelines
  TDGeneratorPipeline pipe_TDgen;
  TDConstZLoadPipeline pipe_TDzload;
  TDConstILoadPipeline pipe_TDiload;
  TDConstPLoadPipeline pipe_TDpload;

  NodeInjectionType injectionTypes[MAX_VERATOMCOUNT][MAX_HORATOMCOUNT];
  double VInjections[MAX_VERATOMCOUNT][MAX_HORATOMCOUNT];
  double IInjections[MAX_VERATOMCOUNT][MAX_HORATOMCOUNT];
};

} // end of namespace elabtsaot

#endif // SLICEDIGITAL_H
