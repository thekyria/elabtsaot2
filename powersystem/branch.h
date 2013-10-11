/*!
\file branch.h
\brief Definition file for class Branch

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef BRANCH_H
#define BRANCH_H

#include <string>
#include <complex>

#ifndef DEFBRANCHX
//! Default reactance value [pu] for power system branches
#define DEFBRANCHX 0.1
#endif // DEFBRANCHX

namespace elabtsaot{

//!  Branch class
/*!
  Class representing a branch of the real world power system. A branch can be a
  transmission line or a transformer. No provision yet available for HVDC lines.

  \author thekyria
*/
class Branch {

 public:

  //! Constructor
  /*!
    \param extId external id of the branch to be created
    \param fromBusExtId external id of the bus at the from end of the branch
    \param toBusExtId external id of the bus at the to end of the branch
    \param name name of the branch
    \param r resistance of the pi-model of the branch
    \param x reactance of the pi-model of the branch
    \param b susceptance of the pi-model of the branch; b is divided between
             from and to ends of the branch
    \param status enable status of the branch; 0: offline, 1: online
  */
  Branch(unsigned int extId = 0,
         unsigned int fromBusExtId = 0,
         unsigned int toBusExtId = 0,
         std::string const& name = "defbranch",
         double r = 0.0,
         double x = DEFBRANCHX,
         double b = 0.0,
         bool status = true);

  //! Serializes the contents of the powersystem branch into an std::string
  std::string serialize() const;
  //! Outputs the member values of Branch to std::cout
  void display() const;

  // transformer controls
  //! Validation function for transformer variables
  /*!
    Checks that all _Xratio_xx and _Xshift_xx variables respect some rules
    \return Int exit code; 0 if ok.
    \sa Xparams_update(), Xratio_tap_change(), Xshift_tap_change()
  */
  int Xparams_validate() const;
  //! Update function for transformer variables
  /*!
    Updates transfrormer related variables _Xratio_xx and _Xshift_xx
    \return Int exit code; 0 if ok.
    \sa Xparams_validate(), Xratio_tap_change(), Xshift_tap_change()
  */
  int Xparams_update();
  //! Tap setting change function (single-step) for tranformer ratio
  /*!
    Changes _Xratio_tap by one to the direction defined by input argument up.
    Updates the value of _Xratio according to the new tap setting.
    \param up bool value that denotes the change direction; true for up, false
              for down
    \return Int exit code; 0 if ok.
    \sa Xparams_validate(), Xparams_update()
  */
  int Xratio_tap_change(bool up);
  //! Tap setting change function (multi-step) for tranformer ratio
  /*!
    Changes _Xratio_tap by the amount defined by input argument val (+/-).
    Updates the value of _Xratio according to the new tap setting.
    \param val sets the amount of step changes to _Xratio_tap setting; can be
               positive of negative
    \return Int exit code; 0 if ok.
    \sa Xparams_validate(), Xparams_update()
  */
  int Xratio_tap_change(int val);
  //! Tap setting change function (single-step) for tranformer phase shift
  /*!
    Changes _Xshift_tap by one to the direction defined by input argument up.
    Updates the value of _Xshift according to the new tap setting.
    \param up bool value that denotes the change direction; true for up, false
              for down
    \return Int exit code; 0 if ok.
    \sa Xparams_validate(), Xparams_update()
  */
  int Xshift_tap_change(bool up);
  //! Tap setting change function (multi-step) for tranformer phase shift
  /*!
    Changes _Xshift_tap by the amount defined by input argument val (+/-).
    Updates the value of _Xshift according to the new tap setting.
    \param val sets the amount of step changes to _Xshift_tap setting; can be
               positive of negative
    \return Int exit code; 0 if ok.
    \sa Xparams_validate(), Xparams_update()
  */
  int Xshift_tap_change(int val);

  /*! \name Getters
    Getters of private members of the class
  */
  //@{
  unsigned int extId() const;               //!< getter for _extId
  unsigned int fromBusExtId() const;        //!< getter for _fromBusExtId
  unsigned int toBusExtId() const;          //!< getter for _toBusExtId
  std::string name() const;                 //!< getter for _name
  double r() const;                         //!< getter for _r
  double x() const;                         //!< getter for _x
  double b() const;                         //!< getter for b =_b_from+_b_to
  double b_from() const;                    //!< getter for _b_from
  double b_to() const;                      //!< getter for _b_to
  double g() const;                         //!< getter for g =_g_from+_g_to
  double g_from() const;                    //!< getter for _g_from
  double g_to() const;                      //!< getter for _g_to
  double c_series_x() const;                //!< getter for _c_series_x

  double Xratio() const;                    //!< getter for _Xratio
  unsigned int Xratio_tap() const;          //!< getter for _Xratio_tap
  double Xratio_base() const;               //!< getter for _Xratio_base
  unsigned int Xratio_tap_min() const;      //!< getter for _Xratio_tap_min
  unsigned int Xratio_tap_max() const;      //!< getter for _Xratio_tap_max
  double Xratio_tap_step() const;           //!< getter for _Xratio_tap_step
  double Xshift() const;                    //!< getter for _Xshift
  unsigned int Xshift_tap() const;          //!< getter for _Xshift_tap
  double Xshift_base() const;               //!< getter for _Xshift_base
  unsigned int Xshift_tap_min() const;      //!< getter for _Xshift_tap_min
  unsigned int Xshift_tap_max() const;      //!< getter for _Xshift_tap_max
  double Xshift_tap_step() const;           //!< getter for _Xshift_tap_step

  bool status() const;                      //!< getter for _status

  std::complex<double> ifrom() const;       //!< getter for _ifrom
  std::complex<double> ito() const;         //!< getter for _ito
  std::complex<double> sfrom() const;       //!< getter for _sfrom
  std::complex<double> sto() const;         //!< getter for _sto
  //@}

  /*! \name Setters
    Setters of private members of the class
  */
  //@{
  void set_extId(unsigned int val);         //!< setter for _extId
  void set_fromBusExtId(unsigned int val);  //!< setter for _fromBusExtId
  void set_toBusExtId(unsigned int val);    //!< setter for _toBusExtId
  void set_name(std::string const& val);    //!< setter for _name
  void set_r(double val);                   //!< setter for _r
  void set_x(double val);                   //!< setter for _x
  void set_b(double val);                   //!< setter for _b_from & _b_to
  void set_b_from(double val);              //!< setter for _b_from
  void set_b_to(double val);                //!< setter for _b_to
  void set_g(double val);                   //!< setter for _g_from & _g_to
  void set_g_from(double val);              //!< setter for _g_from
  void set_g_to(double val);                //!< setter for _g_to
  void set_c_series_x(double val);          //!< setter for _c_series_x

  void set_Xratio(double val);              //!< setter for _Xratio
  int set_Xratio_tap(unsigned int val);     //!< setter for _Xratio_tap
  int set_Xratio_base(double val);          //!< setter for _Xratio_base
  int set_Xratio_tap_min(unsigned int val); //!< setter for _Xratio_tap_min
  int set_Xratio_tap_max(unsigned int val); //!< setter for _Xratio_tap_max
  int set_Xratio_tap_step(double val);      //!< setter for _Xratio_tap_step
  void set_Xshift(double val);              //!< setter for _Xshift
  int set_Xshift_tap(unsigned int val);     //!< setter for _Xshift_tap
  int set_Xshift_base(double val);          //!< setter for _Xshift_base
  int set_Xshift_tap_min(unsigned int val); //!< setter for _Xshift_tap_min
  int set_Xshift_tap_max(unsigned int val); //!< setter for _Xshift_tap_max
  int set_Xshift_tap_step(double val);      //!< setter for _Xshift_tap_step

  void set_status(bool val);                //!< setter for _status

  void set_ifrom(std::complex<double> const& val); //!< setter for _ifrom
  void set_ito(std::complex<double> const& val);   //!< setter for _ito
  void set_sfrom(std::complex<double> const& val); //!< setter for _sfrom
  void set_sto(std::complex<double> const& val);   //!< setter for _sto
  //@}

 private:

  //! External id of the branch
  /*!
    Unique identifier of the given branch in a power system
  */
  unsigned int _extId;        // branch (external) integer index
  //! External id of the bus at the from side of the branch
  /*!
    External id of the bus is the unique identifier of the bus at the from end
    of the branch among buses
  */
  unsigned int _fromBusExtId; // fromBus (external) integer index
  //! External id of the bus at the to side of the branch
  /*!
    External id of the bus is the unique identifier of the bus at the to end
    of the branch among buses
  */
  unsigned int _toBusExtId;   // toBus (external) integer index
  //! Name of the branch
  /*! */
  std::string _name;
  //! Resistance of the pi-model of the branch
  /*!
    Given in [pu]
  */
  double _r;                  // total resistance [p.u.]
  //! Reactance of the pi-model of the branch
  /*!
    Given in [pu]
  */
  double _x;                  // total reactance [p.u.]
  //! Susceptance to ground at the from end of the pi-model of the branch
  /*!
    Given in [pu]
  */
  double _b_from;             // susceptance to ground at from end [p.u.]
  //! Susceptance to ground at the to end of the pi-model of the branch
  /*!
    Given in [pu]
  */
  double _b_to;               // susceptance to ground at to end [p.u.]
  //! Conductance to ground at the from end of the pi-model of the branch
  /*!
    Given in [pu]
  */
  double _g_from;             // conductance to ground at from end [p.u.]
  //! Conductance to ground at the to end of the pi-model of the branch
  /*!
    Given in [pu]
  */
  double _g_to;               // conductance to ground at to end [p.u.]
  //! Series compensation capacitance of the branch
  /*!
    Given in [pu]
  */
  double _c_series_x;         // series capacitor (in series with r & x) [p.u.]
                              // normally negative (as X_c = - 1/wC)

  /*! @name Transformer variables
    See also Kundur - Power System Stability and Control p. 235

    \todo Refine/review setters
  */
  //@{
  //! Ratio of the transformer of the branch
  /*!
    The transformer is considered at the from end of the branch.
    If Xratio == 0 then the branch is not a transformer
  */
  double _Xratio;             // transformer ratio on the primary side
                              // 0 for lines; <>0 for transformers:
                              // 1 for NR, <>1 for ONR Xformers
  //! Tap setting of the transformer ratio
  /*!
    Discrete setting between a minimum and a maximum tap setting value.
    _Xratio_tap_min <= _Xratio_tap <= _Xratio_tap_max
    Also it holds:
      _Xratio = _Xratio_base + _Xratio_tap*_Xratio_tap_step
  */
  unsigned int _Xratio_tap;   // transformer ratio tap setting [min-0-max]
  //! Base value of the transformer ratio of the branch
  /*!
    This is the ratio value of the transformer when the tap setting is 0;
    eg. 1.2
  */
  double _Xratio_base;        // transformer ratio at tap setting = 0; eg. 1.2
  //! Minimum value of the tap setting of the transformer of the branch
  /*!
    Discrete value;
    eg. -5
  */
  unsigned int _Xratio_tap_min; // transformer ratio min tap setting; eg. -5
  //! Maximum value of the tap setting of the transformer of the branch
  /*!
    Discrete value;
    eg. +5
  */
  unsigned int _Xratio_tap_max; // transformer ratio max tap setting; eg. +5
  //! Ratio value corresponding to one tap setting step
  /*!
    Change to the transformer ratio when tap setting changes by one;
    eg. 0.05
  */
  double _Xratio_tap_step;    // transformer ratio tap step value; eg. 0.05

  //! Phase shift of the transformer of the branch
  /*!
    The transformer is considered at the from end of the branch. Given in [rad].
    If _Xshift > 0 then V_from (voltage at the from end of the branch) leads
    V_to (voltage at the to end of the branch).
  */
  double _Xshift;             // transformer phase shift [rad]:
                              // >0 when V_from leads V_to
  //! Tap setting of the transformer phase shift
  /*!
    Discrete setting between a minimum and a maximum tap setting value.
    _Xshift_tap_min <= _Xshift_tap <= _Xshift_tap_max
    Also it holds:
      _Xshift = _Xshift_base + _Xshift_tap*_Xshift_tap_step
  */
  unsigned int _Xshift_tap;   // transformer phase shift tap setting [min-0-max]
  //! Base value of the transformer phase shift of the branch
  /*!
    This is the phase shift value of the transformer when the tap setting is 0;
    eg. 0.5 rad (~ +29 degrees)
  */
  double _Xshift_base;        // transformer phase shift at tap setting=0 [rad]
  //! Minimum value of the phase shift setting of the transformer of the branch
  /*!
    Discrete value;
    eg. -5
  */
  unsigned int _Xshift_tap_min; // transformer phase shift min tap setting
  //! Maximum value of the phase shift setting of the transformer of the branch
  /*!
    Discrete value;
    eg. +5
  */
  unsigned int _Xshift_tap_max; // transformer phase shift max tap setting
  //! Phase shift value corresponding to one tap setting step
  /*!
    Change to the transformer phase shift when tap setting changes by one;
    eg. 0.01
  */
  double _Xshift_tap_step;    // transformer phase shift step value [rad]

  //! Enable status of the branch
  /*!
    0: offline, 1: online
  */
  bool _status;               // 1: online 0: offline

  // loadflow result variables
  //! Steady state current flow at the from end of the branch
  /*!
    Given in [pu].
    This is a loadflow result, and is filled in after loadflow has been
    sucessfully solved in the power system that the branch partakes in.
  */
  std::complex<double> _ifrom;
  //! Steady state current flow at the to end of the branch
  /*!
    Given in [pu].
    This is a loadflow result, and is filled in after loadflow has been
    sucessfully solved in the power system that the branch partakes in.
  */
  std::complex<double> _ito;
  //! Steady state apparent power (S) flow at the from end of the branch
  /*!
    Given in [pu].
    This is a loadflow result, and is filled in after loadflow has been
    sucessfully solved in the power system that the branch partakes in.
  */
  std::complex<double> _sfrom;
  //! Steady state apparent power (S) flow at the to end of the branch
  /*!
    Given in [pu].
    This is a loadflow result, and is filled in after loadflow has been
    sucessfully solved in the power system that the branch partakes in.
  */
  std::complex<double> _sto;

};

} // end of namespace elabtsaot

#endif // BRANCH_H
