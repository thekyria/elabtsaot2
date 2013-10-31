/*!
\file pqpipeline.h
\brief Definition file for class PQPipeline

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef PQPIPELINE_H
#define PQPIPELINE_H

#include <complex>

#include "pipeline.h" // includes <string>, <vector>, <utility>

namespace elabtsaot{

class Bus;

class PQPipeline : public Pipeline {

 public:

  PQPipeline(size_t element_capacity, size_t ver_dim, size_t hor_dim);
  virtual ~PQPipeline(){}
  void reset();

  int insert_element(size_t ver_pos, size_t hor_pos, Bus const& el, bool overwrite);
  int remove_element(size_t ver_pos, size_t hor_pos);

  std::vector<std::complex<double> > Sset; //!< P & Q setpoints [pu]
  std::vector<std::complex<double> > I0; //!< Initial current flow [p.u.]

};

} // end of namespace elabtsaot

#endif // PQPIPELINE_H
