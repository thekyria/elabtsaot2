/*!
\file physics.h

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef PHYSICS_H
#define PHYSICS_H

#include <string>

namespace elabtsaot{

namespace physics{

enum Variable{
  TIME,
  ANGLE,
  ANGULAR_SPEED,
  VOLTAGE,
  POWER,
  CURRENT,
  UNKNOWN_VARIABLE
};

enum Unit{
  SECOND,
  RAD,
  RAD_PER_SECOND,
  VOLT,
  WATT,
  VAR,
  AMPERE,
  PU,
  UNKNOWN_UNIT
};

std::string variableToString( int variable );
int defaultUnitOfVariable( int variable );
std::string unitToString( int unit );

} // end of namespace physics

} // end of namespace elabtsaot

#endif // PHYSICS_H
