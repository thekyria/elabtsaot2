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

//! Bus type enumeration
enum BusType{
  BUSTYPE_SLACK = 0,   //!< Slack bus
  BUSTYPE_PV = 1,      //!< PV bus: P known, V magnitude known
  BUSTYPE_PQ = 2       //!< PQ bus: P known, Q known
};

class Bus{

 public:

  Bus();
  std::string serialize() const; //!< Serializes the contents of the powersystem bus into an std::string

  // ----- Static parameters -----
  unsigned int extId; //!< External id of the bus
  std::string name; //!< Name of the bus
  int type; //!< according to BusType
  double Gsh; //!< Shunt conductance to ground at bus [pu]
  double Bsh; //!< Shunt susceptance to ground at bus [pu]
  double baseKV; //!< Voltage base at the bus [kV]

  // ----- Static variables -----
  double P; //!< Steady state active power (P) generated at bus [pu]
  double Q; //!< Steady state reactive power (Q) generated at bus [pu]
  double V; //!< Steady state voltage magnitude at bus [pu]
  double theta; //!< Steady state voltage angle at bus [rad]
};

} // end of namespace elabtsaot

#endif // BUS_H
