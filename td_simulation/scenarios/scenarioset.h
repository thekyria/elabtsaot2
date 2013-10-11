/*!
\file scenarioset.h

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef SCENARIOSET_H
#define SCENARIOSET_H

#include <vector>
#include <string>

#include "scenario.h"

namespace elabtsaot{

class ScenarioSet {

 public:

  ScenarioSet();
  void reset();
  std::string serialize();
  void display();

  void createScenariosPerBranch( Powersystem const* pws,
                                 double scenarioStopTime,
                                 bool trip,
                                 bool ignoreLowZBrnahces,
                                 double faultLocation,
                                 double faultStart,
                                 double faultStop );

  // Setters & getter of private members
  void set_id( size_t val );
  void set_name( std::string const& val );
  void set_description( std::string const& val);
  unsigned int extId() const;
  std::string name() const;
  std::string description() const;

  // Facilitator function
  int deleteScenario( unsigned int sceId );

  std::vector<Scenario> scenarios;

 private:

  unsigned int _extId;
  std::string _name;
  std::string _description;

};

} // end of namespace elabtsaot

#endif // SCENARIOSET_H
