/*!
\file generator.h
\brief Definition file for class Generator

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef GENERATOR_H
#define GENERATOR_H

#include <string>
#include <complex>

namespace elabtsaot{

/*!
  Enum value      Order Description (for d- and q-axes)
  --------------------------------------------------------
  GENMODEL_0p0    II    "classical" model; d:constant flux linkages; q:-
  GENMODEL_1p0    III   d:field circuit; q:-
  GENMODEL_1p1    IV    d:field circuit; q:one damper
  GENMODEL_2p1t1  V     d:field circuit and one damper; q:one damper; typ1 as per PSAT
  GENMODEL_2p1t2  V     d:field circuit and one damper; q:one damper; typ2 as per PSAT
  GENMODEL_2p2    VI    d:field circuit and one damper; q:two damper
  GENMODEL_3p2    VII   d:field circuit and two damper; q:two damper
  GENMODEL_3p3    VIII  d:field circuit and two damper; q:three damper
*/
enum GeneratorModel{
  GENMODEL_0p0 = 2,
  GENMODEL_1p0,
  GENMODEL_1p1 = 4,
  GENMODEL_2p1t1,
  GENMODEL_2p1t2,
  GENMODEL_2p2,
  GENMODEL_3p2,
  GENMODEL_3p3
};

//!  Generator class
/*!
  Class representing a generator of the real world power system.

  \todo Include members for startup/shutdown costs, ramp rates, avr, pss, etc..
*/
class Generator {

 public:

  //! Constructor
  /*!
    \param extId external id of the generator to be created
    \param name name of the generator
    \param busExtId external id of the bus where the generator is connected to
    \param voltageSetpoint target voltage for the bus where the generator is
                           connected to [pu]
    \param pgen currently generated active power [pu]
    \param qgen currently generated reactive power [pu]
    \param pmin minimum active power generation capacity [pu]
    \param pmax maximum active power generation capacity [pu]
    \param qmin minimum reactive power generation capacity [pu]
    \param qmax maximum reactive power generation capacity [pu]
    \param xd_1 direct axis transient reactance [pu]
    \param M mechanical starting t (M = 2H, 2xinertia const)[s]
    \param status enable status of the generator; 0: offline, 1: online
  */
  Generator(unsigned int extId = 0,
            std::string const& name = "defgen",
            unsigned int busExtId = 0,
            double voltageSetpoint = 0.0,
            double pgen = 0.0,
            double qgen = 0.0,
            double pmin = 0.0,
            double pmax = 10.0,
            double qmin = -10.0,
            double qmax = 10.0,
            double xd_1 = 0.3,
            double M = 2.0,
            bool status = false);          // explicit constructor
  //! Destructor
  virtual ~Generator();                           // destructor

  //! Serializes the contents of the powersystem branch into an std::string
  std::string serialize() const;
  //! Outputs the member values of Generator to std::cout
  void display() const;

  /*! \name Getters
    Getters of private members of the class
  */
  //@{
  // --- getters ---
  unsigned int extId() const;        //!< getter for _extId
  std::string name() const;          //!< getter for _name
  unsigned int busExtId() const;     //!< getter for _busExtId
  double voltageSetpoint() const;    //!< getter for _voltageSetpoint
  bool avr() const;                  //!< getter for _avr
  double pgen() const;               //!< getter for _pgen
  double qgen() const;               //!< getter for _qgen
  double pmin() const;               //!< getter for _pmin
  double pmax() const;               //!< getter for _pmax
  double qmin() const;               //!< getter for _qmin
  double qmax() const;               //!< getter for _qmax
  bool status() const;               //!< getter for _status

  int model() const;                 //!< getter for _model
  double fss() const;                //!< getter for _fss
  double xl() const;                 //!< getter for _xl
  double ra() const;                 //!< getter for _ra
  double xd() const;                 //!< getter for _xd
  double xd_1() const;               //!< getter for _xd_1
  double xd_2() const;               //!< getter for _xd_2
  double Td0_1() const;              //!< getter for _Td0_1
  double Td0_2() const;              //!< getter for _Td0_2
  double xq() const;                 //!< getter for _xq
  double xq_1() const;               //!< getter for _xq_1
  double xq_2() const;               //!< getter for _xq_2
  double Tq0_1() const;              //!< getter for _Tq0_1
  double Tq0_2() const;              //!< getter for _Tq0_2
  double M() const;                  //!< getter for _M
  double D() const;                  //!< getter for _D

  double Ess() const;                //!< getter for _Ess
  double deltass() const;            //!< getter for _deltass
  std::complex<double> Uss() const;  //!< getter for _Uss
  //@}

  /*! @name Setters
    Setters of private members of the class
  */
  //@{
  // --- setters ---
  int set_extId(unsigned int val);       //!< setter for _extId
  int set_name(std::string const& val);  //!< setter for _name
  int set_busExtId(unsigned int val);    //!< setter for _busExtId
  int set_avr(bool val);                 //!< setter for _avr
  int set_voltageSetpoint(double val);   //!< setter for _voltageSetpoint
  int set_pgen(double val);              //!< setter for _pgen
  int set_qgen(double val);              //!< setter for _qgen
  int set_pmin(double val);              //!< setter for _pmin
  int set_pmax(double val);              //!< setter for _pmax
  int set_qmin(double val);              //!< setter for _qmin
  int set_qmax(double val);              //!< setter for _qmax
  int set_status(bool val);              //!< setter for _status

  int set_model(int val);                //!< setter for _model
  int set_fss(double val);               //!< setter for _fss
  int set_xl(double val);                //!< setter for _xl
  int set_ra(double val);                //!< setter for _ra
  int set_xd(double val);                //!< setter for _xd
  int set_xd_1(double val);              //!< setter for _xd_1
  int set_xd_2(double val);              //!< setter for _xd_2
  int set_Td0_1(double val);             //!< setter for _Td0_1
  int set_Td0_2(double val);             //!< setter for _Td0_2
  int set_xq(double val);                //!< setter for _xq
  int set_xq_1(double val);              //!< setter for _xq_1
  int set_xq_2(double val);              //!< setter for _xq_2
  int set_Tq0_1(double val);             //!< setter for _Tq0_1
  int set_Tq0_2(double val);             //!< setter for _Tq0_2
  int set_M(double val);                 //!< setter for _M
  int set_D(double val);                 //!< setter for _D

  int set_Ess(double val);               //!< setter for _Ess
  int set_deltass(double val);           //!< setter for _deltass
  int set_Uss(std::complex<double> const& val); //!< setter for _Uss
  //@}

 private:
  //! External id of the generator
  /*!
    Unique identifier of the given generator in a power system
  */
  unsigned int _extId;
  //! Name of the generator
  /*! */
  std::string _name;
  //! External id of the bus the generator is connected to
  /*! */
  unsigned int _busExtId;
  //! Automatic Voltage Regulation (AVR) flag
  /*!
    True if generator has automatic voltage regulation capabilities. If so it
    can maintain the voltage of the bus it is connected to at the level
    specified by private variable _voltageSetpoint.

    \sa _voltageSetpoint
  */
  bool _avr;
  //! Voltage setpoint for the bus the generator is connected to and controlling
  /*!
    Given in [pu]; This voltage will be enforced to the bus by the generator
    only if the generator is AVR enabled - see private variable _avr.

    \sa _avr
  */
  double _voltageSetpoint;
  //! Active power (P) produced by the generator
  /*!
    In [pu]

    \sa _pmin, pmax
  */
  double _pgen;
  //! Reactive power (Q) produced by the generator
  /*!
    In [pu]

    \sa _qmin, _qmax
  */
  double _qgen;
  //! Minimum active power (P) capacity of the generator
  /*!
    In [pu]

    \sa _pgen, _pmax
  */
  double _pmin;
  //! Maximum active power (P) capacity of the generator
  /*!
    In [pu]

    \sa _pgen, _pmin
  */
  double  _pmax;
  //! Minimum reactive power (Q) capacity of the generator
  /*!
    In [pu]

    \sa _qgen, _qmax
  */
  double _qmin;
  //! Maximum reactive power (Q) capacity of the generator
  /*!
    In [pu]

    \sa _qgen, _qmin
  */
  double _qmax;
  //! Enable status of the branch
  /*!
    0: offline, 1: online
  */
  bool _status;

  /*! @name Dynamic parameters
  */
  //@{
  // dynamic data
  //! Steady state nominal frequency
  /*!
    In [Hz]
  */
  double _fss;
  //! Name of the model of the generator
  int _model;
  //! Leakage reactance (Potier)
  /*!
    In [pu]
  */
  double _xl;
  //! Armature resistance
  /*!
    In [pu]
  */
  double _ra;
  //! Synchronous reactance - direct axis
  /*!
    In [pu]
  */
  double _xd;
  //! Transient reactance - direct axis
  /*!
    In [pu]
  */
  double _xd_1;
  //! Sub-transient reactance - direct axis
  /*!
    In [pu]
  */
  double _xd_2;
  //! Open-circuit transient time constant - direct axis
  /*!
    In [s]
  */
  double _Td0_1;
  //! Open-circuit sub-transient time constant - direct axis
  /*!
    In [s]
  */
  double _Td0_2;
  //! Synchronous reactance - quadrature axis
  /*!
    In [pu]
  */
  double _xq;
  //! Transient reactance - quadrature axis
  /*!
    In [pu]
  */
  double _xq_1;
  //! Sub-transient reactance - quadrature axis
  /*!
    In [pu]
  */
  double _xq_2;
  //! Open-circuit transient time constant - quadrature axis
  /*!
    In [s]
  */
  double _Tq0_1;
  //! Open-circuit sub-transient time constant - quadrature axis
  /*!
    In [s]
  */
  double _Tq0_2;
  //! Mechanical starting time M
  /*!
    M [s] = 2 x H, 2 x inertia constant
    Inertia constant H [MWs/MVA]:
      (stored energy at rated speed in MW x s) / (MVA rating)
  */
  double _M;
  //! Damping coefficient
  /*!
    In [pu]
  */
  double _D;
  //@}

  /*! @name Variables that result from the loadflow */
  //@{
  //! Steady state internal voltage magnitude for classical model
  /*!
    In [pu]
  */
  double _Ess;
  //! Steady state internal voltage angle
  /*!
    In [rad] with respect to angle reference at the slack bus
  */
  double _deltass;
  //! Steady state voltage (complex) at the terminals of the generator
  /*!
    In [pu]. Corresponds to the voltage of the bus that the gen is connected to.
  */
  std::complex<double> _Uss;
  //@}
};

} // end of namespace elabtsaot

#endif // GENERATOR_H
