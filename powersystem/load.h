/*!
\file load.h
\brief Definition file for class Load

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef LOAD_H
#define LOAD_H

#include <string>
#include <complex>

namespace elabtsaot{

//! Load type enumeration
enum LoadType{
  LOADTYPE_CONSTP = 0,      //!< Constant power (S/P) load
  LOADTYPE_CONSTI = 1,      //!< Constant current (I) load
  LOADTYPE_CONSTZ = 2,      //!< Constant impedance (Z) load
  LOADTYPE_OTHER = 3        //!< Other type of load
};

//!  Load class
/*! Class representing a load of the real world power system. The load is
    considered a source of power demand from the power system.

    \todo Include dynamic data for the load (const P/I/Z), see Kundur p.272. Also
          see Thesis: Chan - Power system load modelling
    \todo What about frequency fss? */
class Load {

 public:
  Load();
  /*! //! Constructor
    \param extId external id of the load to be created
    \param name name of the load
    \param type type of the load according to respective enum
    \param busExtId external id of the bus where the load is going to be at
    \param pdemand active power (P) demanded by the load (withdrawn from the
                   power system)
    \param qdemand reactive power (Q) demanded by the load (withdrawn from the
                   power system)
    \param status enable status of the load; 0: offline, 1: online */
  Load(unsigned int extId,
       std::string const& name,
       unsigned int busExtId,
       double pdemand, double qdemand,
       bool status);
  virtual ~Load(){} //!< Destructor

  //! Serializes the contents of the powersystem branch into an std::string
  std::string serialize() const;
  //! Outputs the member values of Load to std::cout
  void display() const;

  /*! \name Getters */
  //@{
  unsigned int extId() const;             //!< getter for _extId
  std::string name() const;               //!< getter for _name
  //! Pseudo-getter for Load::_type
  /*! \return type of the load based on the values of _v_exp_a, _v_exp_b */
  unsigned int type() const;
  unsigned int busExtId() const;          //!< getter for _busExtId
  double pdemand() const;                 //!< getter for _pdemand
  double qdemand() const;                 //!< getter for _qdemand
  double v_exp_a() const;                 //!< getter for _v_exp_a
  double v_exp_b() const;                 //!< getter for _v_exp_b
  double k_p_f() const;                   //!< getter for _k_p_f
  double k_q_f() const;                   //!< getter for _k_q_f
  bool status() const;                    //!< getter for _status
  std::complex<double> Uss() const;       //!< getter for _Uss
  //@}

  /*! \name Setters */
  //@{
  void set_extId(unsigned int val);    //!< setter for _extId
  void set_name(std::string const& val);      //!< setter for _name
  //! Pseudo-setter for Load::_type
  /*! Sets _v_exp_a, _v_exp_b with respect to the provided load type argument
      \param val type of the load according to LoadType enum
      \sa LoadType, _v_exp_a, _v_exp_b */
  void set_type(unsigned int val);
  void set_busExtId(unsigned int val); //!< setter for _busExtId
  void set_pdemand(double val);        //!< setter for _pdemand
  void set_qdemand(double val);        //!< setter for _qdemand
  void set_v_exp_a(double val);        //!< setter for _v_exp_a
  void set_v_exp_b(double val);        //!< setter for _v_exp_b
  void set_k_p_f(double val);          //!< setter for _k_p_f
  void set_k_q_f(double val);          //!< setter for _k_q_f
  void set_status(bool val);           //!< setter for _status
  void set_Uss(std::complex<double> const& val);   //!< setter for _Uss
  //@}

 private:

  unsigned int _extId; //! External index of the load
  std::string _name; //! Name of the load
  unsigned int _busExtId; //! External id of the bus the load is connected to
  //! Active power (P) demanded by the load
  /*! In [pu]; Sign convention:
    _pdemand > 0 : is power DEMANDED by the load bus */
  double _pdemand;
  //! Reactive power (Q) demanded by the load
  /*! In [pu]; Sign convention:
    _qdemand > 0 : is power DEMANDED by the load bus */
  double _qdemand;

  /*! \name Enhanced static load model
      voltage dependency according to the exponential model and frequency
      dependency:
        pdemand_dyn= _pdemand * (v_current/real(Uss))^_v_exp_a * (1 + _k_p_f*Df)
        qdemand_dyn= _qdemand * (v_current/real(Uss))^_v_exp_b * (1 + _k_q_f*Df)

      for the exponents of voltage dependency it holds:
        _v_exp_a, _v_exp_b = 0  => constant power load
        _v_exp_a, _v_exp_b = 1  => constant current load
        _v_exp_a, _v_exp_b = 2  => constant impedance load
      for composite loads, the values depend on the aggregate characteristics of
      load components */
  double _v_exp_a; //! Exponent for the active power voltage dependency
  double _v_exp_b; //! Exponent for the reactive power voltage dependency
  double _k_p_f;   //! Factor for the active power frequency dependency
  double _k_q_f;   //! Factor for the reactive power frequency dependency
  //@}
  //! Enable status of the branch
  /*! 0: offline, 1: online */
  bool _status;
  /*! \name Variables that result from the loadflow */
  //@{
  //! Steady state voltage (complex) at the terminals of the load [pu]
  /*! Corresponds to the voltage of the bus that the load is connected to */
  std::complex<double> _Uss;
  //@}
};

} // end of namespace elabtsaot

#endif // LOAD_H
