/*!
\file emulatorbranch.h
\brief Definition file for class EmulatorBranch

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef EMULATORBRANCH_H
#define EMULATORBRANCH_H

#include "potentiometer.h"

namespace elabtsaot{

//! Class representing a branch of the hardware emulator
/*! Emulator branches are an integral part of atoms. They physically connect the
  node of one atom to the node of another adjacent atom in the analog grid.
  \warning This class is part of the emulator_hw classes that provide a software
  representation of the hardware emulator developed at ELAB. As such, changes in
  hardware are reflected in the software classes. For more information on the
  hardware of the project contact: guillaume dot lanz at epfl dot ch (hw team
  leader)

  \author thekyria
  \date Sep 2013 */
class EmulatorBranch{

 public:

  EmulatorBranch(); //!< Default constructor
  int reset( bool complete );

  /*! Getters of private members of the class */
  //@{
  bool sw_sc() const;                    //!< getter for _sw_sc
  bool sw_mid() const;                   //!< getter for _sw_mid

  double pot_near_rab() const;           //!< getter for _pot_near._rab
  double pot_near_rw() const;            //!< getter for _pot_near._rw
  double pot_near_r() const;             //!< getter for _pot_near._r
  double pot_near_getRMin() const;
  double pot_near_getRMax() const;       //!< wrapper for _pot_near.getRMax
  unsigned int pot_near_tap() const;     //!< getter for _pot_near._tap
  unsigned int pot_near_tap_max() const; //!< getter for _pot_near._tap_max
  bool pot_near_sw() const;              //!< getter for _pot_near._sw
  bool pot_near_swA() const;             //!< getter for _pot_near._swA

  double pot_far_rab() const;            //!< getter for _pot_far._rab
  double pot_far_rw() const;             //!< getter for _pot_far._rw
  double pot_far_r() const;              //!< getter for _pot_far._r
  double pot_far_getRMin() const;
  double pot_far_getRMax() const;        //!< wrapper for _pot_far.getRMax
  unsigned int pot_far_tap() const;      //!< getter for _pot_far._tap
  unsigned int pot_far_tap_max() const;  //!< getter for _pot_far._tap_max
  bool pot_far_sw() const;               //!< getter for _pot_far._sw
  bool pot_far_swA() const;              //!< getter for _pot_far._swA
  //@}

  /*! Setters of private members of the class */
  //@{
  void set_sw_sc( bool isShortCircuited); //!< setter for _sw_sc
  void set_sw_mid( bool isGrounded); //!< setter for _sw_mid

  int set_pot_near_rab( double val, bool updateTap ); //!< setter for _pot_near._rab
  int set_pot_near_rw( double val, bool updateTap ); //!< setter for _pot_near._rw
  int set_pot_near_r( double pot_near_r, double* p_r_mismatch ); //!< setter for _pot_near._r
  int set_pot_near_tap( unsigned int val ); //!< setter for _pot_near._tap
  void set_pot_near_sw( bool val ); //!< setter for _pot_near._sw
  int set_pot_near_swA( bool val, bool updateTap ); //!< setter for _pot_near._swA

  int set_pot_far_rab( double val, bool updateTap ); //!< setter for _pot_far._rab
  int set_pot_far_rw( double val, bool updateTap ); //!< setter for _pot_far._rw
  int set_pot_far_r( double pot_far_r, double* p_r_mismatch ); //!< setter for _pot_far._r
  int set_pot_far_tap( unsigned int val ); //!< setter for _pot_far._tap
  void set_pot_far_sw( bool val ); //!< setter for _pot_far._s
  int set_pot_far_swA( bool val, bool updateTap ); //!< setter for _pot_far._swA
  //@}

 private:

  bool _sw_sc; //!< Switch that short circuits the branch
  bool _sw_mid; //!< Switch that grounds the midpoint of the branch
  Potentiometer _pot_near; //!< Potentiometer at near end of the branch
  Potentiometer _pot_far; //!< Potentiometer at far end of the branch
};

} // end of namespace elabtsaot

#endif // EMULATORBRANCH_H
