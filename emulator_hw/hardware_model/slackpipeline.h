/*!
\file slackpipeline.h
\brief Definition file for class SlackPipeline

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef SLACKPIPELINE_H
#define SLACKPIPELINE_H

#include "pipeline.h" // includes <string>, <vector>, <utility>

namespace elabtsaot{

//! Constant current load pipeline class
class SlackPipeline : public Pipeline {

 public:

  SlackPipeline(size_t element_capacity, size_t ver_dim, size_t hor_dim);
  // TODO

};

} // end of namespace elabtsaot

#endif // SLACKPIPELINE_H
