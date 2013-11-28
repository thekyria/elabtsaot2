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

namespace elabtsaot{

//! Class representing a node of the hardware emulator
/*!
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

  //! Constructor - all node switches are open
  Node();
  //! Destructor - does nothing :)
  virtual ~Node(){}

  // --- functions ---
  //! Outputs the member values to std::cout
  int display() const;
  //! Resets the node - all node switches are opened
  int reset( bool complete );

  // --- getters ---
  /*! \name Getters */
  //@{
  double real_adc_offset_corr() const;          //!< getter for _real_adc_offset_corr
  double real_adc_gain_corr() const;            //!< getter for _real_adc_gain_corr
  bool real_sw_voltage() const;                 //!< getter for _real_sw_voltage
  bool real_sw_current() const;                 //!< getter for _real_sw_current
  bool real_sw_current_shunt() const;           //!< getter for _real_sw_current_shunt
  bool real_sw_resistance() const;              //!< getter for _real_sw_resistance
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

  double imag_adc_offset_corr() const;          //!< getter for _imag_adc_offset_corr
  double imag_adc_gain_corr() const;            //!< getter for _imag_adc_gain_corr
  bool imag_sw_voltage() const;                 //!< getter for _imag_sw_voltage
  bool imag_sw_current() const;                 //!< getter for _imag_sw_current
  bool imag_sw_current_shunt() const;           //!< getter for _imag_sw_current_shunt
  bool imag_sw_resistance() const;              //!< getter for _imag_sw_resistance
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
  //@}

  // --- setters ---
  /*! @name Setters */
  //@{
  void set_real_adc_offset_corr( double val ); //!< setter for _real_adc_offset_corr
  void set_real_adc_gain_corr( double val ); //!< setter for _real_adc_gain_corr
  void set_real_sw_voltage(bool val); //!< setter for _real_sw_voltage
  void set_real_sw_current(bool val); //!< setter for _real_sw_current
  void set_real_sw_current_shunt(bool val); //!< setter for _real_sw_current_shunt
  void set_real_sw_resistance(bool val); //!< setter for _real_sw_resistance
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

  void set_imag_adc_offset_corr( double val ); //!< setter for _imag_adc_offset_corr
  void set_imag_adc_gain_corr( double val ); //!< setter for _imag_adc_gain_corr
  void set_imag_sw_voltage(bool val); //!< setter for _imag_sw_voltage
  void set_imag_sw_current(bool val); //!< setter for _imag_sw_current
  void set_imag_sw_current_shunt(bool val); //!< setter for _imag_sw_current_shunt
  void set_imag_sw_resistance(bool val); //!< setter for _imag_sw_resistance
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
  //@}

 private:

  // -------------- Real part --------------
  //! Correction on the offset value of the adc of the real part
  /*!
    This means that the final adc offset value that will be communicated to the
    emulator hardware for the real part of every atom of a slice is going to be:
    - Slice::_got_offset + Node::_real_adc_offset_corr
  */
  double _real_adc_offset_corr;
  //! Correction on the gain value of the adc of the real part
  /*!
    This means that the final adc offset value that will be communicated to the
    emulator hardware for the real part of every atom of a slice is going to be:
    - Slice::_got_gain + Node::_real_adc_gain_corr
  */
  double _real_adc_gain_corr;
  //! Switch to apply voltage to the node (normally open)
  bool _real_sw_voltage;              // Switch to apply voltage to the node(no)
  //! Switch to inject a current to the node
  bool _real_sw_current;              // Switch to inject a current to the node
  //! Calibration switch (no)
  bool _real_sw_current_shunt;        // Calibration switch (normally open)
  //! Switch to connect a resistance to GND
  bool _real_sw_resistance;           // Switch to connect a resistance to GND
  //! Potentiometer of the current source
  Potentiometer _real_pot_current;    // Potentiometer of the current source
  //! Resistance between node and GND
  Potentiometer _real_pot_resistance; // Resistance between node and GND
  // -------------- Imag part --------------
  //! Correction on the offset value of the adc of the imag part
  /*!
    This means that the final adc offset value that will be communicated to the
    emulator hardware for the imag part of every atom of a slice is going to be:
    - Slice::_got_offset + Node::_imag_adc_offset_corr
  */
  double _imag_adc_offset_corr;
  //! Correction on the gain value of the adc of the imag part
  /*!
    This means that the final adc offset value that will be communicated to the
    emulator hardware for the imag part of every atom of a slice is going to be:
    - Slice::_got_gain + Node::_imag_adc_gain_corr
  */
  double _imag_adc_gain_corr;
  //! Switch to apply voltage to the node (normally open)
  bool _imag_sw_voltage;            // Switch to apply voltage to the node(no)
  //! Switch to inject a current to the node
  bool _imag_sw_current;            // Switch to inject a current to the node
  //! Calibration switch (no)
  bool _imag_sw_current_shunt;      // Calibration switch (normally open)
  //! Switch to connect a resistance to GND
  bool _imag_sw_resistance;         // Switch to connect a resistance to GND
  //! Potentiometer of the current source
  Potentiometer _imag_pot_current;    // Potentiometer of the current source
  //! Resistance between node and GND
  Potentiometer _imag_pot_resistance; // Resistance between node and GND
};

} // end of namespace elabtsaot

#endif // NODE_H
