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
class Bus {

 public:

  //! Serializes the contents of the powersystem bus into an std::string
  std::string serialize() const;

  unsigned int extId; //! External id of the bus
  std::string name; //! Name of the bus
  double gsh; //! Shunt conductance to ground at bus [pu]
  double bsh; //! Shunt susceptance to ground at bus [pu]
  double baseKV; //! Voltage base at the bus [kV]

  // Variables that result from the loadflow
  double V; //! Steady state voltage magnitude at bus [pu]
  double theta; //! Steady state voltage angle at bus [rad]
  double P; //! Steady state active power (P) generated at bus [pu]
  double Q; //! Steady state reactive power (Q) generated at bus [pu]
};

} // end of namespace elabtsaot

#endif // BUS_H
