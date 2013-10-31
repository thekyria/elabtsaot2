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

#include "generatorpipeline.h"
#include "constzloadpipeline.h"
#include "constiloadpipeline.h"
#include "constploadpipeline.h"
#include "pqpipeline.h"
#include "slackpipeline.h"

namespace elabtsaot{

class SliceDigital{

 public:

  SliceDigital();
  void reset();
  int remove(size_t ver, size_t hor); //!< Remove all (ver,hor) pipeline entries

  // PF specific pipelines
  PQPipeline pipe_PFPQ;
  SlackPipeline pipe_PFslack;

  // TD specific pipelines
  GeneratorPipeline pipe_TDgen;
  ConstZLoadPipeline pipe_TDzload;
  ConstILoadPipeline pipe_TDiload;
  ConstPLoadPipeline pipe_TDpload;
};

} // end of namespace elabtsaot

#endif // SLICEDIGITAL_H
