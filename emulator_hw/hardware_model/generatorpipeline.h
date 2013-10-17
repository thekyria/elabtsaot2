/*!
\file generatorpipeline.h
\brief Definition file for class GeneratorPipeline

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef GENERATORPIPELINE_H
#define GENERATORPIPELINE_H

#include <complex>

#include "pipeline.h" // includes <string>, <vector>, <utility>

namespace elabtsaot{

class Generator;

//! Generator pipeline class
/*! Class for the generator calculation pipelines of the digital part
  (FPGA) of a hardware emulator slice.

  \sa Pipeline, ConstPLoadPipeline, ConstZLoadPipeline, ConstZLoadPipeline

  \warning This class is part of the emulator_hw classes that provide a software
  representation of the hardware emulator developed at ELAB. As such, changes in
  hardware are reflected in the software classes. For more information on the
  hardware of the project contact: guillaume dot lanz at epfl dot ch (hw team
  leader)

  \author thekyria
  \date June 2011 */
class GeneratorPipeline : public Pipeline{

 public:

  //! Constructor
  /*! \param element_capacity Capacity of the pipeline
      \param ver_dim Vertical dimension of the emulator the pipeline is
                     instantiated for
      \param hor_dim Horizontal dimension of the emulator the pipeline is
                     instantiated for */
  GeneratorPipeline(size_t element_capacity, size_t ver_dim, size_t hor_dim);
  virtual ~GeneratorPipeline(){}

  void display() const; //!< Outputs the member values to std::cout
  int reset();          //!< Resets the pipeline

  //! An element is inserted at the pipeline
  int insert_element(size_t ver_pos, size_t hor_pos, Generator const& el, bool overwrite);
  //! An element is removed from the pipeline
  int remove_element(size_t ver_pos, size_t hor_pos);

  std::vector<double> xd1inverse; //!< Inverse of the transient reactance [p.u.]
  std::vector<std::complex<double> > I0; //!< Initial current flow [p.u.]
  std::vector<double> pMechanical; //!< Mechanical power [p.u.]
  std::vector<double> gain1;       //!< (freq [Hz])/(inertia const H [kW/kVA])
  std::vector<double> gain2;       //!< (internal voltage E [p.u.])/(xd' [p.u.])
  std::vector<double> gain3;       //!< by default = -gain_2
  std::vector<double> gain4;       //!< by default = 1
  std::vector<double> gain5;       //!< by default = -gain_4
  std::vector<double> gain6;       //!< by default = 2
  std::vector<double> pa0;         //! S.s. P accelerating [p.u.]; default 0
  std::vector<double> omega0;      //! S.s. relative omega [rad/s]; default 0
  std::vector<double> delta0;      //! S.s. delta [rad]; from power flow

};

} // end of namespace elabtsaot

#endif // GENERATORPIPELINE_H
