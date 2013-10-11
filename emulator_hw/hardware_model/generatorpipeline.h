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

  /*! \name Getters */
  //@{
  std::vector<double> xd_1_inverse() const;  //!< getter for _xd_1_inverse
  std::vector<Generator const*> elPtrs() const;
  std::vector<std::complex<double> > i_ss() const;    //!< getter for _i_ss
  std::vector<double> p_mechanical() const;  //!< getter for _p_mechanical
  std::vector<double> gain_1() const;        //!< getter for _gain_1
  std::vector<double> gain_2() const;        //!< getter for _gain_2
  std::vector<double> gain_3() const;        //!< getter for _gain_3
  std::vector<double> gain_4() const;        //!< getter for _gain_4
  std::vector<double> gain_5() const;        //!< getter for _gain_5
  std::vector<double> gain_6() const;        //!< getter for _gain_6
  std::vector<double> pa_0() const;          //!< getter for _pa_0
  std::vector<double> omega_0() const;       //!< getter for _omega_0
  std::vector<double> delta_0() const;       //!< getter for _delta_0
  //@}

  /*! \name Setters  members of the class. */
  //@{
  int set_xd_1_inverse(size_t pos, double val);
  int set_i_0(size_t pos, std::complex<double> val);
  int set_p_mechanical(size_t pos, double val);
  int set_gain_1(size_t pos, double val);
  int set_gain_2(size_t pos, double val);
  int set_gain_3(size_t pos, double val);
  int set_gain_4(size_t pos, double val);
  int set_gain_5(size_t pos, double val);
  int set_gain_6(size_t pos, double val);
  int set_pa_0(size_t pos, double val);
  int set_omega_0(size_t pos, double  val);
  int set_delta_0(size_t pos, double val);
  //@}

 private:

  std::vector<Generator const*> _elPtrs; //!< pointers to gens mapped to the pipeline
  std::vector<double> _xd_1_inverse; //!< Inverse of the transient reactance [p.u.]
  std::vector<std::complex<double> > _i_ss; //!< Initial current flow [p.u.]
  std::vector<double> _p_mechanical; //!< Mechanical power [p.u.]
  std::vector<double> _gain_1;       //!< (freq [Hz])/(inertia const H [kW/kVA])
  std::vector<double> _gain_2;       //!< (internal voltage E [p.u.])/(xd' [p.u.])
  std::vector<double> _gain_3;       //!< by default = -gain_2
  std::vector<double> _gain_4;       //!< by default = 1
  std::vector<double> _gain_5;       //!< by default = -gain_4
  std::vector<double> _gain_6;       //!< by default = 2
  std::vector<double> _pa_0;         //! S.s. P accelerating [p.u.]; default 0
  std::vector<double> _omega_0;      //! S.s. relative omega [rad/s]; default 0
  std::vector<double> _delta_0;      //! S.s. delta [rad]; from loadflow

};

} // end of namespace elabtsaot

#endif // GENERATORPIPELINE_H
