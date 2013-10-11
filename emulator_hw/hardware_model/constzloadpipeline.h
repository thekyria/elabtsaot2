/*!
\file constzloadpipeline.h
\brief Definition file for class ConstZLoadPipeline

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef CONSTZLOADPIPELINE_H
#define CONSTZLOADPIPELINE_H

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
class ConstZLoadPipeline : public Pipeline {

 public:

  //! Constructor
  /*! \param element_capacity Capacity of the pipeline; default MAXZLOADS
      \param ver_dim Vertical dimension of the emulator the pipeline is
                     instantiated for
      \param hor_dim Horizontal dimension of the emulator the pipeline is
                     instantiated for */
  ConstZLoadPipeline(size_t element_capacity, size_t ver_dim, size_t hor_dim);
  virtual ~ConstZLoadPipeline(){}
  int reset();          //!< Resets the pipeline

  //! An element is inserted at the pipeline
  int insert_element(size_t ver_pos, size_t hor_pos, Load const& el, bool overwrite);
  //! An element is removed from the pipeline
  int remove_element(size_t ver_pos, size_t hor_pos);

  /*! \name Getters */
  //@{
  std::vector<Load const*> elPtrs() const;
  std::vector<double> real_Y() const;  //!< getter for _real_Y
  std::vector<double> imag_Y() const;  //!< getter for _imag_Y
  //@}

  /*! \name Setters */
  //@{
  int set_real_Y(size_t pos, double val);
  int set_imag_Y(size_t pos, double val);
  //@}

 private:

  std::vector<Load const*> _elPtrs; //!< ptrs to loads mapped to the pipeline
  std::vector<double> _real_Y; //!< Real part of the load admittance [p.u.]
  std::vector<double> _imag_Y; //!< Imag part of the load admittance [p.u.]

};

} // end of namespace elabtsaot

#endif // CONSTZLOADPIPELINE_H
