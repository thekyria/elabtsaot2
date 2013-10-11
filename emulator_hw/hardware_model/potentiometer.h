/*!
\file potentiometer.h
\brief Definition file for class Potentiometer

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef POTENTIOMETER_H
#define POTENTIOMETER_H

#ifndef POTENTIOMETER_SWA
//! Default status of the terminal A pin switch
#define POTENTIOMETER_SWA false
#endif // POTENTIOMETER_SWA

#ifndef POTENTIOMETER_RAB
//! Resistance from A to B terminal [Ohm] of the potentiometer used in the
//! analog part of the hardware emulator
#define POTENTIOMETER_RAB 10000
#endif // POTENTIOMETER_RAB

#ifndef POTENTIOMETER_RW
//! Resistance of the wiper [Ohm] of the potentiometer used in the analog part
//! of the hardware emulator
#define POTENTIOMETER_RW 75
#endif // POTENTIOMETER_RW

#ifndef POTENTIOMETER_RESOLUTION
//! Resolution [bits] of the potentiometer used in the analog part of the
//! hardware emulator
#define POTENTIOMETER_RESOLUTION 8
#endif // POTENTIOMETER_RESOLUTION

#ifndef POTENTIOMETER_DEFTAPVALUE
//! Default tap value of the potentiometer used in the analog part of the
//! hardware emulator
#define POTENTIOMETER_DEFTAPVALUE 128
#endif // POTENTIOMETER_DEFTAPVALUE

namespace elabtsaot{

//! Class representing a potentiometer
/*!
  Potentiometer description based on Microchip MCP4351 (see datasheet)
  On our design terminal A is short circuited to wiper terminal W
  So the status of the internal terminal A pin switch determines the
  configuration of the potentiometer circuit:
  With _swA = true (switch closed):

               rab
    -------\/\/\/\/\/\/\/\-----
         (1-n)rab  .  nrab
                  / \
            rw     |              Resulting r = rw//(1-n)rab + nrab
    -----\/\/\/\----              where n = tap/tap_max


  With _swA = false (switch open):

               rab
    --]  [-\/\/\/\/\/\/\/\-----
                   .  nrab
                  / \
            rw     |              Resulting r = rw + n*rab
    -----\/\/\/\----              where n = tap/tap_max

  rw the wiper resistance, rab resistance from terminals A to B

  Practically, after instantiation the only variable element of the
  potentiometer is the tap setting, an so indirectly the resulting resistance.
  There are 257 allowable tap settings [0, 256].

  The datasheet precision of the resistor is +- 20%; for Rab it is: Typ: 10 kOhm

  In order to compensate for manufacturing discrepancies a calibration procedure
  can determine the rab resistor and the rw resistor.

  \warning This class is part of the emulator_hw classes that provide a software
  representation of the hardware emulator developed at ELAB. As such, changes in
  hardware are reflected in the software classes. For more information on the
  hardware of the project contact: guillaume dot lanz at epfl dot ch (hw team
  leader)

  \author thekyria
  \date January 2012
*/
class Potentiometer{

 public:

  //! Constructor - members instantiated to default values
  Potentiometer();
  //! Constructor - potentiometer instantiated according to resistance argument
  explicit Potentiometer(double r);
  //! Constructor - potentiometer instantiated according to tap setting argument
  explicit Potentiometer(unsigned int tap);
  //! Destructor
  virtual ~Potentiometer(){}

  // --- functions ---
  //! Outputs member values to std::cout
  int display() const;
  //! Potentiometer switch closed and tap setting reset to
  //! POTENTIOMETER_DEFTAPVALUE. If the 'complete' flag is set then _rab is
  //! reset back to its default value POTENTIOMETER_RAB
  int reset(bool complete);

  // --- "setters" ---
  //! Sets the _rab resistor
  /*!
    The rab resistance coincides the maximum achievable resistance of the
    potentiometer. This function is normally used after calibration.
    \param rab the new rab resistance of the potentiometer to take into account
               during calculations
    \param updateTap when set then the tap setting is updated accordingly in
                     order to preserve the resistance value _r of the
                     potentiometer, before and after calibration
  */
  int set_rab(double rab, bool updateTap);
  int set_rw(double rw, bool updateTap);
  //! Sets tap value implicitly, to match a required resistance
  /*!
    \param r resistance to match
    \param p_rMismatch pointer to output argument that stores the mismatch
                       between the resistance value asked for and the closest
                       one achieved by a valid tap setting
    \param force when set then the tap of the potentiometer is set to a value
                 that tries to match the asked for resistance 'r' as much as
                 possible. This means that if an 'r' greater than the max.
                 achievable resistance _rab is asked for, then the tap will
                 saturate to max (trying to get as close to asked for 'r' as
                 possible.

    \return Int exit code; 0 if ok.

    \warning A non-zero value is returned when the asked for resistance 'r' is
             out of limits. In the latter case, if the 'force' flag is set, then
             the tap value is changed as explained here bove. STILL though a
             non-zero exit code is returned!
  */
  int set_r(double r, double* p_rMismatch = 0, bool force = true);
  //! Sets tap value explicitly
  int set_tap(unsigned int tap);
  //! Sets status of the internal switch of the potentiometer
  void set_sw(bool status);
  //! Sets status of the terminal A pin switch of the potentiometer
  int set_swA(bool status, bool updateTap);

  /*! \name Getters
    Getters of private members of the class
  */
  //@{
  // --- getters ---
  double rab() const;              //!< getter for _rab
  double rw() const;               //!< getter for _rw
  unsigned int resolution() const; //!< getter for _resolution
  double r() const;                //!< getter for _r
  unsigned int tap() const;        //!< getter for _tap
  unsigned int tap_max() const;    //!< getter for _tap_max
  bool sw() const;                 //!< getter for _sw
  bool swA() const;                //!< getter for _swA
  double getRMax() const;          //!< returns the maximum achievable r

  //@}

  //! Static calculator function; returns the equivalent resistance of two
  //! parallel resistors
  /*!
    \param r1 resistance of the first resistor in parallel in Ohms
    \param r2 resistance of the second resistor in parallel in Ohms
    \return resistance of the parallel combination of r1 & r2
  */
  static double parallel_r( double r1 , double r2 );
  //! Static calculator function; returns resulting resistance of the
  //! potentiometer for the given tap setting
  /*!
    \param tap tap setting of the potentiometer
    \param rab resistance of the A-B resistor of the potenetiometer in Ohms
    \param rw resistance of the wiper of the potenetiometer in Ohms
    \param resolution resolution of the potentiometer in bits; from which
           tap_max is calculated as 2^resolution (including the last value)
    \return negative value if arguments invalid
  */
  static double r_from_tap(unsigned int tap,
                           double rab = POTENTIOMETER_RAB,
                           double rw = POTENTIOMETER_RW,
                           bool swA = POTENTIOMETER_SWA,
                           unsigned int resolution = POTENTIOMETER_RESOLUTION);
  //! Static calculator function; returns required tap setting of the
  //! potentiometer in order to get the given resistance
  /*!
    \param r resistance required from he potentiometer - to be matched by tap
           setting
    \param rab resistance of the A-B resistor of the potenetiometer in Ohms
    \param rw resistance of the wiper of the potentiometer in Ohms
    \param resolution resolution of the potentiometer in bits; from which
           tap_max is calculated as 2^resolution (including the last value)
    \return negative value if arguments invalid
  */
  static int tap_from_r( double r,
                         double rab = POTENTIOMETER_RAB,
                         double rw = POTENTIOMETER_RW,
                         bool swA = POTENTIOMETER_SWA,
                         unsigned int resolution = POTENTIOMETER_RESOLUTION );

 private:

  //! Resistance of the resistor network between terminals A and B
  double _rab;                            // Resistance of the resistor network
                                          // between terminals A and B
  //! Resistance of the wiper
  double _rw;                             // Resistance of the wiper
  //! Resolution of the potentiometer [number of bits]
  unsigned int _resolution;               // Resolution of the potentiom. [bits]
  //! Resistance of the potentiometer [Ohm]
  double _r;                              // Resistance of the potentiom. [Ohm]
  //! Current tap value of the potentiometer; allowed values [0, _tap_max]
  unsigned int _tap;                      // Tap value of the potentiometer
  //! Maximum allowable tap value
  unsigned int _tap_max;                  // Maximum tap value
  //! Internal switch; closed if true
  bool _sw;                               // Internal "switch"
  //! Terminal A pin switch; closed if true
  bool _swA;
  //! Array containing the trend of rw values vs tap setting
  static double const _rw_trend[257];

};

} // end of namespace elabtsaot

#endif // POTENTIOMETER_H
