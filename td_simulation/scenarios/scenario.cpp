
#include "scenario.h"
using namespace elabtsaot;

#include "event.h"
#include "powersystem.h"

//#include <vector>
using std::vector;
//#include <string>
using std::string;
#include <algorithm>
using std::swap;
#include <iostream>
using std::cout;
using std::cin;
using std::endl;
#include <sstream>
using std::stringstream;

#ifndef SCE_DEFSTOPTIME
#define SCE_DEFSTOPTIME 5.0
#endif // SCE_DEFSTOPTIME
#define DBL_EPSILON 0.0000001
#define DOUBLE_EQ(x,v) (((v - DBL_EPSILON) < x) && (x <( v + DBL_EPSILON)))

Scenario::Scenario(unsigned int extId,
                   string name,
                   string description,
                   bool status) :
    _extId(extId), _name(name), _description(description),
    _startTime(0), _stopTime(SCE_DEFSTOPTIME),
    _status(status) {}

string Scenario::serialize() const{

  stringstream ss;

  ss << _extId << " ";
  ss << "_name:" << _name << ":_name ";
  ss << "_description:" << _description << ":_description ";
  ss << _startTime << " ";
  ss << _stopTime << " ";
  ss << _status << " ";

  ss << "_eventSet.size=" << _eventSet.size() << " ";
  for ( size_t k = 0 ; k != _eventSet.size() ; ++k )
    ss << "_eventSet[" << k << "]:" << _eventSet[k].serialize()
       << ":_eventSet[" << k << "] ";

  return ss.str();
}

void Scenario::display() const{
  cout << endl;
  cout << "--------------Scenario details ---------------" << endl;
//  cout << "----------------------------------------------" << endl;
  cout << "External id: " << _extId << endl;
  cout << _name << ": " << _description << endl;
  cout << "Start-stop time: " << _startTime << "-" << _stopTime << " [s]"<<endl;
  cout << "status: " << _status << endl;
  cout << "--- Events ------------------------------------------ " << endl;
  for (size_t k = 0 ; k != _eventSet.size() ; ++k ){
    getEvent(k).display();
  }
  cout << "----------------------------------------------------- " << endl;
}

void Scenario::reset(){
  _eventSet.clear();

  _name = "";
  _description = "";
  _startTime = 0;
  _stopTime = SCE_DEFSTOPTIME;
  _status = true;
}

void Scenario::reduce(){
  // TODO!
  return;
}

void Scenario::sort_t(){
  size_t n = _eventSet.size();
  for ( size_t k = 0; k < n ; ++k )
   // back through the area bringing smallest remaining element to position k
    for ( size_t m = n-1 ; m > k ; --m ){
      if ( _eventSet[m-1].time() > _eventSet[m].time() )
        swap( _eventSet[m-1] , _eventSet[m] );}
  return;
}

bool Scenario::checkPwsCompatibility( Powersystem const& pws ) const{

  // for every event in the scenario set make sure that the element refered to
  // exists in the powersystem
  for ( size_t k = 0 ; k != _eventSet.size() ; ++k ){
    switch ( _eventSet[k].element_type() ){

    case EVENT_ELEMENT_TYPE_BUS:
      if ( pws.getBus_intId( _eventSet[k].element_extId() ) < 0 )
        return false;
      break;

    case EVENT_ELEMENT_TYPE_BRANCH:
      if ( pws.getBr_intId( _eventSet[k].element_extId() ) < 0 )
        return false;
      break;

    case EVENT_ELEMENT_TYPE_GEN:
      if ( pws.getGen_intId( _eventSet[k].element_extId() ) < 0 )
        return false;
      break;

    case EVENT_ELEMENT_TYPE_LOAD:
      if ( pws.getLoad_intId( _eventSet[k].element_extId() ) < 0 )
        return false;
      break;

    default:
      // invalid element_type
      return false;
    }
  }

  return true;
}

bool Scenario::checkCCTCompatibility() const{
  size_t k;
  // A reduced event set is assumed in checking for CCT compatibility!

  /* Scenarios that are CCT compatible must have one of the following forms:
  - br 3ph fault ON + same br 3ph fault OFF
  - br 3ph fault ON + same br 3ph fault OFF (later) + trip same br (at OFF time)

  TODO: add also bus 3ph faults
  */
  vector<Event> brFaultOnEvents;
  vector<Event> brFaultOffEvents;
  vector<Event> brTripEvents;
  vector<Event> otherActiveEvents;
  for ( k = 0 ; k != _eventSet.size() ; ++k ){
    // Skip inactive events
    if ( !_eventSet[k].status() ) continue;

    // Collect branch 3ph fault on events
    if (    _eventSet[k].element_type() == EVENT_ELEMENT_TYPE_BRANCH
         && _eventSet[k].event_type() == EVENT_EVENTTYPE_BRFAULT
         && _eventSet[k].bool_arg() )
      brFaultOnEvents.push_back( _eventSet[k] );

    // Collect branch 3ph fault off events
    else if (    _eventSet[k].element_type() == EVENT_ELEMENT_TYPE_BRANCH
              && _eventSet[k].event_type() == EVENT_EVENTTYPE_BRFAULT
              && !_eventSet[k].bool_arg() )
      brFaultOffEvents.push_back( _eventSet[k] );

    // Collect branch trip events
    else if (    _eventSet[k].element_type() == EVENT_ELEMENT_TYPE_BRANCH
              && _eventSet[k].event_type() == EVENT_EVENTTYPE_BRTRIP
              && _eventSet[k].bool_arg() )
      brTripEvents.push_back( _eventSet[k] );

    // Collect all other events
    else
      otherActiveEvents.push_back( _eventSet[k] );
  }

  // No other events should  be active apart from the ones consisting the 3ph
  // fault (+trip) on-off cycle
  if ( otherActiveEvents.size() > 0 )
    return false;

  // Only one fault on and one fault off event is allowed
  if ( brFaultOnEvents.size() != 1 || brFaultOffEvents.size() != 1 )
    return false;
  // The fault on and the fault off events should be on the same branch
  if ( brFaultOnEvents[0].element_extId() != brFaultOffEvents[0].element_extId() )
    return false;

  // Only one branch trip event is allowed
  if ( brTripEvents.size() > 1 )
    return false;
  // And the branch trip (if any) should be on the same branch as the fault
  // events
  if ( brTripEvents.size() > 0 )
      if (    brTripEvents[0].element_extId()
           != brFaultOnEvents[0].element_extId() )
        return false;

  // If the scenario passed all the tests then it is CCT compatible
  return true;
}

bool Scenario::checkTSACompatibility() const{

  size_t k;
  bool _TSACompatible = false;
  // ----------------------- Check TSA compatibility --------------------------
  // A scenario in order to be compatible with TSA analysis, it must contain a
  // any number of NON 3-phase fault events (even 0)
  // 1: a branch/bus 3-phase fault ON event
  // any number of NON 3-phase fault events (even 0)
  // 2: an OFF event for the same fault event as 1
  // any number of NON 3-phase fault events (even 0)
  unsigned int element_type, event_type, element_extId;
//  if ( _eventSet.size() > 1 )
//    _TSACompatible = true;
//  else
//    _TSACompatible = false;

  // Search for the first fault on event
  bool fault_on_found = false;
  for ( k = 0 ; k != _eventSet.size() ; ++k ){
    // Bus fault ON event
    if ( (_eventSet[k].element_type() == EVENT_ELEMENT_TYPE_BUS) &&
         (_eventSet[k].event_type() == EVENT_EVENTTYPE_BUSFAULT) &&
         (_eventSet[k].bool_arg() == true) ){
      element_type = _eventSet[k].element_type(); // = EVENT_ELEMENT_TYPE_BUS;
      event_type = _eventSet[k].event_type();
      element_extId = _eventSet[k].element_extId();
      fault_on_found = true;
      break;
    }
    // Branch fault ON event
    if ( (_eventSet[k].element_type() == EVENT_ELEMENT_TYPE_BRANCH) &&
         (_eventSet[k].event_type() == EVENT_EVENTTYPE_BRFAULT) &&
         (_eventSet[k].bool_arg() == true) ){
      element_type = _eventSet[k].element_type(); // = EVENT_ELEMENT_TYPE_BUS;
      event_type = _eventSet[k].event_type();
      element_extId = _eventSet[k].element_extId();
      fault_on_found = true;
      break;
    }
  }
  // Search for the fault OFF event that corresponds to the previously found
  // fault ON event; if a different fault event is encountered then scenario is
  // flagged as non-TSA compatible
  bool fault_off_found = false;
  for ( ; k != _eventSet.size() ; ++k ){
    // Bus fault ON
    if ( (_eventSet[k].element_type() == EVENT_ELEMENT_TYPE_BUS) &&
         (_eventSet[k].event_type() == EVENT_EVENTTYPE_BUSFAULT) &&
         (_eventSet[k].bool_arg() == true) ){
      // If orig fault of type bus and @ orig bus then ok, else break
      if ( (element_type  == EVENT_ELEMENT_TYPE_BUS) &&
           (element_extId == _eventSet[k].element_extId()) ){
        continue;
      } else {
        break;
      }
    }
    // Branch fault ON
    if ( (_eventSet[k].element_type() == EVENT_ELEMENT_TYPE_BRANCH) &&
         (_eventSet[k].event_type() == EVENT_EVENTTYPE_BRFAULT) &&
         (_eventSet[k].bool_arg() == true)) {
      // If orig fault of type branch and @ orig branch then ok, else break
      if ( (element_type  == EVENT_ELEMENT_TYPE_BRANCH) &&
           (element_extId == _eventSet[k].element_extId()) ){
        continue;
      } else {
        break;
      }
    }

    // Bus fault off
    if ( (_eventSet[k].element_type() == EVENT_ELEMENT_TYPE_BUS) &&
         (_eventSet[k].event_type() == EVENT_EVENTTYPE_BUSFAULT) &&
         (_eventSet[k].bool_arg() == false) ){
      // If orig fault of type bus and @ orig bus then fault_off_found
      if ( (element_type  == EVENT_ELEMENT_TYPE_BUS) &&
           (element_extId == _eventSet[k].element_extId()) ){
        fault_off_found = true;
        break;
      } else {
        continue;
      }
    }
    // Branch fault off
    if ( (_eventSet[k].element_type() == EVENT_ELEMENT_TYPE_BRANCH) &&
         (_eventSet[k].event_type() == EVENT_EVENTTYPE_BRFAULT) &&
         (_eventSet[k].bool_arg() == false) ){
      // If orig fault of type bus and @ orig bus then fault_off_found
      if ( (element_type  == EVENT_ELEMENT_TYPE_BRANCH) &&
           (element_extId == _eventSet[k].element_extId()) ){
        fault_off_found = true;
        break;
      } else {
        continue;
      }
    }
  }
  // Check that the remainder of the events are non-3phase fault events
  bool no_more_faults = true;
  for ( ; k != _eventSet.size() ; ++k ){
    // Bus fault ON
    if ( (_eventSet[k].element_type() == EVENT_ELEMENT_TYPE_BUS) &&
         (_eventSet[k].event_type() == EVENT_EVENTTYPE_BUSFAULT) &&
         (_eventSet[k].bool_arg() == true) ){
      no_more_faults = false;
      break;
    }
    // Branch fault ON
    if ( (_eventSet[k].element_type() == EVENT_ELEMENT_TYPE_BRANCH) &&
         (_eventSet[k].event_type() == EVENT_EVENTTYPE_BRFAULT) &&
         (_eventSet[k].bool_arg() == true) ){
      no_more_faults = false;
      break;
    }
  }
  _TSACompatible = fault_on_found && fault_off_found && no_more_faults;
  return _TSACompatible;
}

int Scenario::insertEvent(Event const& ev){
  _eventSet.push_back(ev);
  return 0;
}

int Scenario::editEvent(unsigned id, Event const& ev){
  if ( id >= _eventSet.size() )
    return 1;
  _eventSet[id] = ev;
  return 0;
}

int Scenario::removeEvent(size_t id){
  if ( id >= _eventSet.size() )
    return 1;

  _eventSet.erase( _eventSet.begin() + id );
  return 0;
}
Event* Scenario::getRawEvent(unsigned id){ return &_eventSet[id]; }
Event Scenario::getEvent(unsigned id) const{ return _eventSet[id]; }
vector<Event> Scenario::getEventSet() const{
  return _eventSet;
}
size_t Scenario::getEventSetSize() const{
  return _eventSet.size();
}

// setters
void Scenario::set_extId(unsigned int val){ _extId = val; }
void Scenario::set_name(string const& val){ _name = val; }
void Scenario::set_description(string const& val){ _description = val; }
int Scenario::set_startTime(double val){
  if ( val < 0 )
    return 1;
  if ( val > _stopTime )
    _stopTime = val;
  _startTime = val;
  return 0;
}
int Scenario::set_stopTime(double val){
  if ( val < 0 )
    return 1;
  if ( val < _startTime )
    _startTime = val;
  _stopTime = val;
  return 0;
}
void Scenario::set_status(bool val){ _status = val; }

// getters
unsigned int Scenario::extId() const{ return _extId; }
string Scenario::name() const{ return _name; }
string Scenario::description() const{ return _description; }
double Scenario::startTime() const{ return _startTime; }
double Scenario::stopTime() const{ return _stopTime; }
bool Scenario::status() const{ return _status; }
