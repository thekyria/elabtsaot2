/*!
\file tdconstzloadpipeline.h
\brief Definition file for class TDConstZLoadPipeline

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef TDCONSTZLOADPIPELINE_H
#define TDCONSTZLOADPIPELINE_H

#include <complex>

#include "pipeline.h" // includes <string>, <vector>, <utility>

namespace elabtsaot{

class Load;

//! Constant impedance load pipeline class
/*!
  Class for the constant impedance load calculation pipelines of the digital part
  (FPGA) of a hardware emulator slice.

  \sa Pipeline, ConstPLoadPipeline, ConstILoadPipeline, GeneratorPipeline

  \warning This class is part of the emulator_hw classes that provide a software
  representation of the hardware emulator developed at ELAB. As such, changes in
  hardware are reflected in the software classes. For more information on the
  hardware of the project contact: guillaume dot lanz at epfl dot ch (hw team
  leader)

  \author thekyria
  \date June 2011
*/
class TDConstZLoadPipeline : public Pipeline {

 public:

  //! Constructor
  /*! \param element_capacity Capacity of the pipeline; default MAXZLOADS
      \param ver_dim Vertical dimension of the emulator the pipeline is
                     instantiated for
      \param hor_dim Horizontal dimension of the emulator the pipeline is
                     instantiated for */
  TDConstZLoadPipeline(size_t element_capacity, size_t ver_dim, size_t hor_dim);
  virtual ~TDConstZLoadPipeline(){}
  void reset();          //!< Resets the pipeline

  //! An element is inserted at the pipeline
  int insert_element(size_t ver_pos, size_t hor_pos, Load const& el, bool overwrite);
  //! An element is removed from the pipeline
  int remove_element(size_t ver_pos, size_t hor_pos);

  std::vector<std::complex<double> > Yconst; //!< constant complex impedance power of the load [pu]
  std::vector<std::complex<double> > I0; //!< Initial current flow [p.u.]

};

} // end of namespace elabtsaot

#endif // TDCONSTZLOADPIPELINE_H
