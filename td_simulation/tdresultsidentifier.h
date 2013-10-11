/*!
\file tdresultsidentifier.h

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef TDRESULTSIDENTIFIER_H
#define TDRESULTSIDENTIFIER_H

#include <string>

namespace elabtsaot{

enum TDResultsElement{
  TDRESULTS_OTHER = -1,
  TDRESULTS_BUS = 0,
  TDRESULTS_BRANCH = 1,
  TDRESULTS_GENERATOR = 2,
  TDRESULTS_LOAD = 3
};

enum TDResultsVariable{
  TDRESULTS_TIME,
  TDRESULTS_ANGLE,
  TDRESULTS_SPEED,
  TDRESULTS_ACTIVE_POWER,
  TDRESULTS_REACTIVE_POWER,
  TDRESULTS_MECHANICAL_POWER,
  TDRESULTS_ACCELERATING_POWER,
  TDRESULTS_VOLTAGE_REAL,
  TDRESULTS_VOLTAGE_IMAG,
  TDRESULTS_VOLTAGE_Q_TR,
  TDRESULTS_VOLTAGE_D_TR,
  TDRESULTS_CURRENT_REAL,
  TDRESULTS_CURRENT_IMAG
};

struct TDResultIdentifier{
  int elementType;    // TDResultsElement
  int variable;       // TDResultsVariable
  unsigned int extId;

  static bool isValid( int elementType, int variable,
                       std::string* name = 0, int* unit = 0);
  static std::string elementTypeToString( int elementType );
  static int variableType( int variable ); // returns physics::variable
  static std::string variableToString( int variable );
  static std::string defaultName( TDResultIdentifier tdri );
  static std::string defaultName( int elementType, int variable );
  static std::string defaultUnits( TDResultIdentifier tdri );
  static std::string defaultUnits( int elementType, int variable );
};

} // end of namespace elabtsaot

#endif // TDRESULTSIDENTIFIER_H
