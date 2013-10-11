/*!
\file scenario.h

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef SCENARIO_H
#define SCENARIO_H

#include <vector>
#include <string>

#include "event.h"

namespace elabtsaot{

class Powersystem;

class Scenario{

 public:

  Scenario(unsigned int extId = 0,
           std::string name = "",
           std::string description = "",
           bool status = true);

  std::string serialize() const;
  void display() const;
  void reset();
  void update();
  void reduce();  // removes duplicates, invalid events, no-effect events, etc.
  void sort_t();  // sorts events in increasing time order
  bool checkPwsCompatibility( Powersystem const& pws ) const;
  bool checkCCTCompatibility() const;
  bool checkTSACompatibility() const;

  int insertEvent(Event const& ev);
  int editEvent(unsigned id, Event const& ev);
  int removeEvent(unsigned id);
  Event getEvent(unsigned id) const;
  Event* getRawEvent(unsigned id);
  std::vector<Event> getEventSet() const;
  size_t getEventSetSize() const;

  // setters
  void set_extId(unsigned int val);
  void set_name(std::string const& val);
  void set_description(std::string const& val);
  int set_startTime(double val);
  int set_stopTime(double val);
  void set_status(bool val);

  // getters
  unsigned int extId() const;
  std::string name() const;
  std::string description() const;
  double startTime() const;
  double stopTime() const;
  bool status() const;

 private:

  unsigned int _extId;
  std::string _name;
  std::string _description;
  double _startTime;          //!< start time of simulation of the scenario [s]
  double _stopTime;           //!< stop time of simulation of the scenario [s]
  bool _status;
  std::vector<Event> _eventSet;

};

} // end of namespace elabtsaot

#endif // SCENARIO_H
