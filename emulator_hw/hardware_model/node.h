/*!
\file node.h
\brief Definition file for class Node

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef NODE_H
#define NODE_H

#include "potentiometer.h"

#define NODEDAC_RES 12
#define NODEDAC_MAXOUT 5.0
#define NODEADC_RES 12

namespace elabtsaot{

/*! Class representing a node of the hardware emulator
  Nodes are an integral part of atoms. They represent the connection point of
  the ADC and DAC drivers interfacing the analog with the digital part of an
  emulator slice. Practically at a node occur all measurements and injections in
  the analog grid.

  Nodes are physically connect node of adjacent atoms with the
  emulator branches.

  Atom representation - the node appears as the central dot 'o' and the emulator
  branches as dashes '-, |, /' surrounding the node.
      |  /
      | /
      |/
   ---o---

  \sa Atom, EmulatorBranch

  \warning This class is part of the emulator_hw classes that provide a software
  representation of the hardware emulator developed at ELAB. As such, changes in
  hardware are reflected in the software classes. For more information on the
  hardware of the project contact: guillaume dot lanz at epfl dot ch (hw team
  leader)

  \author thekyria
  \date June 2011
*/
class Node{

 public:

  Node(); //!< Constructor - all node switches are open
  int reset(bool complete); //!< Resets the node - all node switches are opened

  // --- getters ---
  /*! \name Getters */
  //@{
  double real_pot_current_rab() const;          //!< getter for _real_pot_current._rab
  double real_pot_current_rw() const;           //!< getter for _real_pot_current._rw
  double real_pot_current_r() const;            //!< getter for _real_pot_current._r
  unsigned int real_pot_current_tap() const;    //!< getter for _real_pot_current._tap
  bool real_pot_current_sw() const;             //!< getter for _real_pot_current._sw
  bool real_pot_current_swA() const;            //!< getter for _real_pot_current._swA
  double real_pot_current_getRMax() const;
  double real_pot_resistance_rab() const;       //!< getter for _real_pot_resistance._rab
  double real_pot_resistance_rw() const;        //!< getter for _real_pot_resistance._rw
  double real_pot_resistance_r() const;         //!< getter for _real_pot_resistance._r
  unsigned int real_pot_resistance_tap() const; //!< getter for _real_pot_resistance._tap
  bool real_pot_resistance_sw() const;          //!< getter for _real_pot_resistance._sw
  bool real_pot_resistance_swA() const;         //!< getter for _real_pot_resistance._swA
  double real_pot_resistance_getRMax() const;

  double imag_pot_current_rab() const;          //!< getter for _imag_pot_current._rab
  double imag_pot_current_rw() const;           //!< getter for _imag_pot_current._rw
  double imag_pot_current_r() const;            //!< getter for _imag_pot_current._r
  unsigned int imag_pot_current_tap() const;    //!< getter for _imag_pot_current._tap
  bool imag_pot_current_sw() const;             //!< getter for _imag_pot_current._sw
  bool imag_pot_current_swA() const;            //!< getter for _imag_pot_current._swA
  double imag_pot_current_getRMax() const;
  double imag_pot_resistance_rab() const;       //!< getter for _imag_pot_resistance._rab
  double imag_pot_resistance_rw() const;        //!< getter for _imag_pot_resistance._rw
  double imag_pot_resistance_r() const;         //!< getter for _imag_pot_resistance._r
  unsigned int imag_pot_resistance_tap() const; //!< getter for _imag_pot_resistance._tap
  bool imag_pot_resistance_sw() const;          //!< getter for _imag_pot_resistance._sw
  bool imag_pot_resistance_swA() const;         //!< getter for _imag_pot_resistance._swA
  double imag_pot_resistance_getRMax() const;

  inline double pot_current_rab(bool real) const{
    return real ? real_pot_current_rab() : imag_pot_current_rab();
  }
  inline double pot_current_rw(bool real) const{
    return real ? real_pot_current_rw() : imag_pot_current_rw();
  }
  inline double pot_current_r(bool real) const{
    return real ? real_pot_current_r() : imag_pot_current_r();
  }
  inline unsigned int pot_current_tap(bool real) const{
    return real ? real_pot_current_tap() : imag_pot_current_tap();
  }
  inline bool pot_current_sw(bool real) const{
    return real ? real_pot_current_sw() : imag_pot_current_sw();
  }
  inline bool pot_current_swA(bool real) const{
    return real ? real_pot_current_swA() : imag_pot_current_swA();
  }
  inline double pot_current_getRMax(bool real) const{
    return real ? real_pot_current_getRMax() : imag_pot_current_getRMax();
  }
  inline double pot_resistance_rab(bool real) const{
    return real ? real_pot_resistance_rab() : imag_pot_resistance_rab();
  }
  inline double pot_resistance_rw(bool real) const{
    return real ? real_pot_resistance_rw() : imag_pot_resistance_rw();
  }
  inline double pot_resistance_r(bool real) const{
    return real ? real_pot_resistance_r() : imag_pot_resistance_r();
  }
  inline unsigned int pot_resistance_tap(bool real) const{
    return real ? real_pot_resistance_tap() : imag_pot_resistance_tap();
  }
  inline bool pot_resistance_sw(bool real) const{
    return real ? real_pot_resistance_sw() : imag_pot_resistance_sw();
  }
  inline bool pot_resistance_swA(bool real) const{
    return real ? real_pot_resistance_swA() :imag_pot_resistance_swA();
  }
  inline double pot_resistance_getRMax(bool real) const{
    return real ? real_pot_resistance_getRMax() : imag_pot_resistance_getRMax();
  }
  //@}

  // --- setters ---
  /*! @name Setters */
  //@{
  int set_real_pot_current_rab(double val, bool updateTap); //!< setter for _real_pot_current._rab
  int set_real_pot_current_rw(double val, bool updateTap); //!< setter for _real_pot_current._rw
  int set_real_pot_current_r(double val); //!< setter for _real_pot_current._r
  int set_real_pot_current_tap(unsigned int val); //!< setter for _real_pot_current._tap
  void set_real_pot_current_sw(bool val); //!< setter for _real_pot_current._sw
  int set_real_pot_current_swA(bool val, bool updateTap); //!< setter for _real_pot_current._swA
  int set_real_pot_resistance_rab(double val, bool updateTap); //!< setter for _real_pot_resistance._rab
  int set_real_pot_resistance_rw(double val, bool updateTap); //!< setter for _real_pot_resistance._rw
  int set_real_pot_resistance_r(double val); //!< setter for _real_pot_resistance._r
  int set_real_pot_resistance_tap(unsigned int val); //!< setter for _real_pot_resistance._tap
  void set_real_pot_resistance_sw(bool val); //!< setter for _real_pot_resistance._sw
  int set_real_pot_resistance_swA(bool val, bool updateTap); //!< setter for _real_pot_resistance._swA

  int set_imag_pot_current_rab(double val, bool updateTap); //!< setter for _imag_pot_current._rab
  int set_imag_pot_current_rw(double val, bool updateTap); //!< setter for _imag_pot_current._rw
  int set_imag_pot_current_r(double val); //!< setter for _imag_pot_current._r
  int set_imag_pot_current_tap(unsigned int val); //!< setter for _imag_pot_current._tap
  void set_imag_pot_current_sw(bool val); //!< setter for _imag_pot_current._sw
  int set_imag_pot_current_swA(bool val, bool updateTap); //!< setter for _imag_pot_current._swA
  int set_imag_pot_resistance_rab(double val, bool updateTap); //!< setter for _imag_pot_resistance._rab
  int set_imag_pot_resistance_rw(double val, bool updateTap); //!< setter for _imag_pot_resistance._rw
  int set_imag_pot_resistance_r(double val); //!< setter for _imag_pot_resistance._r
  int set_imag_pot_resistance_tap(unsigned int val); //!< setter for _imag_pot_resistance._tap
  void set_imag_pot_resistance_sw(bool val); //!< setter for _imag_pot_resistance._sw
  int set_imag_pot_resistance_swA(bool val, bool updateTap); //!< setter for _imag_pot_resistance._swA

  inline int set_pot_current_rab(double val, bool updateTap, bool real){
    return real ? set_real_pot_current_rab(val, updateTap) : set_imag_pot_current_rab(val, updateTap);
  }
  inline int set_pot_current_rw(double val, bool updateTap, bool real){
    return real ? set_real_pot_current_rw(val, updateTap) : set_imag_pot_current_rw(val, updateTap);
  }
  inline int set_pot_current_r(double val, bool real){
    return real ? set_real_pot_current_r(val) : set_imag_pot_current_r(val);
  }
  inline int set_pot_current_tap(unsigned int val, bool real){
    return real ? set_real_pot_current_tap(val) : set_imag_pot_current_tap(val);
  }
  inline void set_pot_current_sw(bool val, bool real){
    return real ? set_real_pot_current_sw(val) : set_imag_pot_current_sw(val);
  }
  inline int set_pot_current_swA(bool val, bool updateTap, bool real){
    return real ? set_real_pot_current_swA(val, updateTap) : set_imag_pot_current_swA(val, updateTap);
  }
  inline int set_pot_resistance_rab(double val, bool updateTap, bool real){
    return real ? set_real_pot_resistance_rab(val, updateTap) : set_imag_pot_resistance_rab(val, updateTap);
  }
  inline int set_pot_resistance_rw(double val, bool updateTap, bool real){
    return real ? set_real_pot_resistance_rw(val, updateTap) : set_imag_pot_resistance_rw(val, updateTap);
  }
  inline int set_pot_resistance_r(double val, bool real){
    return real ? set_real_pot_resistance_r(val) : set_imag_pot_resistance_r(val);
  }
  inline int set_pot_resistance_tap(unsigned int val, bool real){
    return real ? set_real_pot_resistance_tap(val) : set_imag_pot_resistance_tap(val);
  }
  inline void set_pot_resistance_sw(bool val, bool real){
    return real ? set_real_pot_resistance_sw(val) : set_imag_pot_resistance_sw(val);
  }
  inline int set_pot_resistance_swA(bool val, bool updateTap, bool real){
    return real ? set_real_pot_resistance_swA(val, updateTap) : set_imag_pot_resistance_swA(val, updateTap);
  }
  //@}

  // -------------- Real part --------------
  double real_adc_offset_corr;
  double real_adc_gain_corr;
  double real_dac_offset_corr;
  double real_dac_gain_corr;
  bool real_sw_voltage;              //!< Switch to apply voltage to the node (normally open)
  bool real_sw_current;              //!< Switch to inject a current to the node
  bool real_sw_current_shunt;        //!< Calibration switch (normally open)
  bool real_sw_resistance;           //!< Switch to connect a resistance to GND
  // -------------- Imag part --------------
  double imag_adc_offset_corr;
  double imag_adc_gain_corr;
  double imag_dac_offset_corr;
  double imag_dac_gain_corr;
  bool imag_sw_voltage;              //!< Switch to apply voltage to the node (normally open)
  bool imag_sw_current;              //!< Switch to inject a current to the node
  bool imag_sw_current_shunt;        //!< Calibration switch (normally open)
  bool imag_sw_resistance;           //!< Switch to connect a resistance to GND

 private:

  // -------------- Real part --------------
  Potentiometer _real_pot_current;    //!< Potentiometer of the current source
  Potentiometer _real_pot_resistance; //!< Resistance between node and GND
  // -------------- Imag part --------------
  Potentiometer _imag_pot_current;    //!< Potentiometer of the current source
  Potentiometer _imag_pot_resistance; //!< Resistance between node and GND
};

} // end of namespace elabtsaot

#endif // NODE_H
