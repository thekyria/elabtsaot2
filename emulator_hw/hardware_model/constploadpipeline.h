/*!
\file constploadpipeline.h
\brief Definition file for class ConstPLoadPipeline

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef CONSTPLOADPIPELINE_H
#define CONSTPLOADPIPELINE_H

#include "pipeline.h" // includes <string>, <vector>, <utility>

namespace elabtsaot{

class Load;

//! Constant power load pipeline class
/*!
  Class for the constant power load calculation pipelines of the digital part
  (FPGA) of a hardware emulator slice.

  \sa Pipeline, ConstILoadPipeline, ConstZLoadPipeline, GeneratorPipeline

  \warning This class is part of the emulator_hw classes that provide a software
  representation of the hardware emulator developed at ELAB. As such, changes in
  hardware are reflected in the software classes. For more information on the
  hardware of the project contact: guillaume dot lanz at epfl dot ch (hw team
  leader)

  \author thekyria
  \date June 2011
*/
class ConstPLoadPipeline : public Pipeline {

 public:

  //! Constructor
  /*! \param element_capacity Capacity of the pipeline; default MAXPLOADS
      \param ver_dim Vertical dimension of the emulator the pipeline is
                     instantiated for
      \param hor_dim Horizontal dimension of the emulator the pipeline is
                     instantiated for */
  ConstPLoadPipeline(size_t element_capacity, size_t ver_dim, size_t hor_dim);
  virtual ~ConstPLoadPipeline(){}
  int reset();          //!< Resets the pipeline

  //! An element is inserted at the pipeline
  int insert_element(size_t ver_pos, size_t hor_pos, Load const& el, bool overwrite);
  //! An element is removed from the pipeline
  int remove_element(size_t ver_pos,
                     size_t hor_pos);

  /*! \name Getters */
  //@{
  std::vector<Load const*> elPtrs() const;
  std::vector<double> P() const;  //!< getter for _P
  std::vector<double> Q() const;  //!< getter for _Q
  //@}

  /*! \name Setters */
  //@{
  int set_P(size_t pos, double val);
  int set_Q(size_t pos, double val);
  //@}

 private:

  std::vector<Load const*> _elPtrs; //!< ptrs to loads mapped to the pipeline
  std::vector<double> _P;           //!< Constant active power (P) of the load in [pu]
  std::vector<double> _Q;           //!< Constant reactive power (Q) of the load in [pu]

};

} // end of namespace elabtsaot

#endif // CONSTPLOADPIPELINE_H
