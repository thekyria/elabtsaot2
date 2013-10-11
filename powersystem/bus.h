/*!
\file bus.h
\brief Definition file for class Bus

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef BUS_H
#define BUS_H

#include <string>

namespace elabtsaot{

//!  Bus class
/*!
  Class representing a bus of the real world power system.
*/
class Bus {

 public:

  //! Constructor
  /*!
    \param extId external id of the bus to be created
    \param name name of the bus
    \param baseKV base voltage at the bus; given in [KV]
    \param Vss steady state voltage magnitude at the bus; given in [pu]
    \param thss steady state voltage angle at the bus; given in [rad]
  */
  Bus(unsigned int extId = 0,
      std::string const& name = "defbus",
      double baseKV = 11.0,
      double Vss = 1.0,
      double thss = 0.0);
  //! Destructor
  virtual ~Bus(){}

  //! Serializes the contents of the powersystem bus into an std::string
  std::string serialize() const;
  //! Outputs the members values of Bus to std::cout
  void display() const;

  /*! @name Getters
    Getters of private members of the class
  */
  //@{
  unsigned int extId() const;              //!< getter for _extId
  std::string name() const;                //!< getter for _name
  double gsh() const;                      //!< getter for _gsh
  double bsh() const;                      //!< getter for _bsh
  double baseKV() const;                   //!< getter for _baseKV
  double Vmax() const;                     //!< getter for _Vmax
  double Vmin() const;                     //!< getter for _Vmin

  double Vss() const;                      //!< getter for _Vss
  double thss() const;                     //!< getter for _thss
  double Pss() const;                      //!< getter for _Pss
  double Qss() const;                      //!< getter for _Qss
  //@}

  /*! @name Setters
    Setters of private members of the class
  */
  //@{
  // --- setters ---
  void set_extId(unsigned int val); //!< setter for _extId
  void set_name(std::string const& val);   //!< setter for _name
  void set_gsh(double val);         //!< setter for _gsh
  void set_bsh(double val);         //!< setter for _bsh
  void set_baseKV(double val);      //!< setter for _baseKV
  void set_Vmax(double val);        //!< setter for _Vmax
  void set_Vmin(double val);        //!< setter for _Vmin

  void set_Vss(double val);         //!< setter for _Vss
  void set_thss(double val);        //!< setter for _thss
  void set_Pss(double val);         //!< setter for _Pss
  void set_Qss(double val);         //!< setter for _Qss
  //@}

 private:
  //! External id of the bus
  /*!
    Unique identifier of the given bus in a power system
  */
  unsigned int _extId;    // bus external int index
  //! Name of the bus
  std::string _name;
  //! Shunt conductance to ground at bus
  /*!
    Given in [pu]
  */
  double _gsh;            // shunt conductance [p.u.]
  //! Shunt susceptance to ground at bus
  /*!
    Given in [pu]
  */
  double _bsh;            // shunt susceptance [p.u.]
  //! Voltage base at the bus
  /*!
    Given in [kV]
  */
  double _baseKV;         // base voltage [kV]
  //! Maximum allowable bus voltage magnitude
  /*!
    Given in [pu]
  */
  double _Vmax;           // maximum allowable bus voltage magnitude [p.u.]
  //! Minimum allowable bus voltage magnitude
  /*!
    Given in [pu]
  */
  double _Vmin;           // minimum allowable bus voltage magnitude [p.u.]

  // Variables that result from the loadflow
  //! Steady state voltage magnitude at bus
  /*!
    Given in [pu].
    This is a loadflow result, and is filled in after loadflow has been
    sucessfully solved in the power system that the bus partakes in.
  */
  double  _Vss;            // steady state voltage magnitude [p.u.]
  //! Steady state voltage angle at bus
  /*!
    Given in [rad].
    This is a loadflow result, and is filled in after loadflow has been
    sucessfully solved in the power system that the bus partakes in.
  */
  double _thss;           // steady state angle norm [rad]
  //! Steady state active power (P) generated at bus
  /*!
    Given in [pu].
    This is a loadflow result, and is filled in after loadflow has been
    sucessfully solved in the power system that the bus partakes in.
  */
  double _Pss;            // steady state active power at bus [p.u.]
  //! Steady state reactive power (Q) generated at bus
  /*!
    Given in [pu].
    This is a loadflow result, and is filled in after loadflow has been
    sucessfully solved in the power system that the bus partakes in.
  */
  double _Qss;            // steady state reactive power at bus [p.u.]
};

} // end of namespace elabtsaot

#endif // BUS_H
