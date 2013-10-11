/*!
\file tdresultsplotproperties.h

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef TDRESULTSPLOTPROPERTIES_H
#define TDRESULTSPLOTPROPERTIES_H

#include <string>
//#include <set>
//using std::set;

#include "physics.h"

namespace elabtsaot{

struct TDResultsPlotProperties{

  TDResultsPlotProperties() :
      variable(physics::ANGLE),
      units(physics::variableToString(
        physics::defaultUnitOfVariable(physics::ANGLE))){}
  TDResultsPlotProperties( int variable_ ) :
      variable(variable_),
      units(physics::variableToString(
        physics::defaultUnitOfVariable(variable_))){}

//  std::string name;        // Name of the plot
  int variable;       // physics::variable; the variable plotted in different
                      // curves in the plot
  std::string units;       // Units for different curves in the plot
//  std::set<std::string> units;  // Units for different curves in the plot // TODO

};

} // end of namespace elabtsaot

#endif // TDRESULTSPLOTPROPERTIES_H
