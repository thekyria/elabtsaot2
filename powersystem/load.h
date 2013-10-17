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

/*! Class representing a load of the real world power system. The load is
    considered a source of power demand from the power system.

    \todo Include dynamic data for the load (const P/I/Z), see Kundur p.272. Also
          see Thesis: Chan - Power system load modelling */
class Load {

 public:

  Load();
  std::string serialize() const;  //!< Serializes the contents of the powersystem branch into an std::string
  void setType(unsigned int val); //!< Pseudo-setter for "Load::_type"; sets Vexpa, Vexpb
  unsigned int type() const;      //!< Pseudo-getter for "Load::_type"; based on Vexpa, Vexpb

  // ----- Static parameters -----
  unsigned int extId; //!< External index of the load
  int busExtId; //!< External id of the bus the load is connected to

  // ----- Static variables -----
  double Pdemand; //!< Active power (P) demanded by the load [pu]; >0 for power DEMANDED
  double Qdemand; //!< Reactive power (Q) demanded by the load [pu]; >0 for power DEMANDED
  std::complex<double> Vss; //!< Steady state voltage (complex) at the terminals of the load [pu]

  // ----- Dynamic parameters -----
  /*! Enhanced static load model
    voltage dependency according to the exponential model and frequency
    dependency:
      pdemand_dyn= Pdemand * (Vcurrent/real(Vss))^Vexpa * (1 + kpf*Df)
      qdemand_dyn= Qdemand * (Vcurrent/real(Vss))^Vexpb * (1 + kqf*Df)

    for the exponents of voltage dependency it holds:
      Vexpa, Vexpb = 0  => constant power load
      Vexpa, Vexpb = 1  => constant current load
      Vexpa, Vexpb = 2  => constant impedance load
    for composite loads, the values depend on the aggregate characteristics of
    load components */
  double Vexpa; //!< Exponent for the active power voltage dependency
  double Vexpb; //!< Exponent for the reactive power voltage dependency
  double kpf;   //!< Factor for the active power frequency dependency
  double kqf;   //!< Factor for the reactive power frequency dependency
};

} // end of namespace elabtsaot

#endif // LOAD_H
