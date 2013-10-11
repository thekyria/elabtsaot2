/*!
\file event.h

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef EVENT_H
#define EVENT_H

#include <string>

namespace elabtsaot{

enum EventElementType{
  EVENT_ELEMENT_TYPE_BUS = 0,
  EVENT_ELEMENT_TYPE_BRANCH = 1,
  EVENT_ELEMENT_TYPE_GEN = 2,
  EVENT_ELEMENT_TYPE_LOAD = 3,
  EVENT_ELEMENT_TYPE_OTHER = 4
};

enum EventEventType{
  EVENT_EVENTTYPE_BUSFAULT = 0,
  EVENT_EVENTTYPE_BRFAULT = 0,
  EVENT_EVENTTYPE_BRTRIP = 1,
  EVENT_EVENTTYPE_BRSHORT = 2,
  EVENT_EVENTTYPE_GENTRIP = 0,
  EVENT_EVENTTYPE_GENPCHANGE = 1,
  EVENT_EVENTTYPE_GENQCHANGE = 2,
  EVENT_EVENTTYPE_GENVCHANGE = 3,
  EVENT_EVENTTYPE_LOADTRIP = 0,
  EVENT_EVENTTYPE_LOADPCHANGE = 1,
  EVENT_EVENTTYPE_LOADQCHANGE = 2
};

class Event {

 public:

  Event(std::string const& name="",
        double time = 0,
        unsigned int element_type = 0,
        unsigned int element_extId = 0,
        unsigned int event_type = 0,
        bool bool_arg = false,
        double double_arg = 0.0,
        double double_arg_1 = 0.0,
        double double_arg_2 = 0.0,
        unsigned int uint = 0);
  virtual ~Event(){}

  std::string serialize() const;
  void display() const;

  // -- setters
  void set_name(std::string const& val);
  int set_time(double val);
  void set_description(std::string const& val);
  void set_status(bool val);
  void set_element_type(unsigned int val);
  void set_element_extId(unsigned int val);
  void set_event_type(unsigned int val);
  void set_bool_arg(bool val);
  void set_double_arg(double val);
  void set_double_arg_1(double val);
  void set_double_arg_2(double val);
  void set_uint_arg(unsigned int val);

  // -- getters
  std::string name() const;
  double time() const;
  std::string description() const;
  bool status() const;
  unsigned int element_type() const;
  unsigned int element_extId() const;
  unsigned int event_type() const;
  bool bool_arg() const;
  double double_arg() const;
  double double_arg_1() const;
  double double_arg_2() const;
  unsigned int uint_arg() const;

 private:

  std::string _name;                 // the name of the event
  std::string _description;          // short description of the event
  double _time;                 // the time at which the event occurs [s]
  bool _status;                 // if true then the event is taken into account
  unsigned int _element_type;   // the type of the power system element the
                                // event occurs at
  unsigned int _element_extId;  // external id of the element
  unsigned int _event_type;     // For the following see table hereunder
  bool _bool_arg;               // --
  double _double_arg;           // --
  double _double_arg_1;         // 3ph fault rsh
  double _double_arg_2;         // 3ph fault xsh
  unsigned int _uint_arg;       // --

  /*
    Event codes table
  -------------------------------------------------------------------------------------------
  element_type  event_type    bool_arg(true)  double_arg  double_arg_1 double_arg_2 uint_arg
  -------------------------------------------------------------------------------------------
  0: bus        0: 3ph fault  fault on        -           rsh          xsh          -

  1: branch     0: 3ph fault  fault on        % of line   rsh          xsh          -
                1: trip       trip on         -           -            -            -
                2: short      short on        -           -            -            -

  2: generator  0: trip       trip on         -           -            -            -
                1: p change   -               new p val   -            -            -
                2: q change   -               new q val   -            -            -
                3: vset chang -               new v val   -            -            -

  3: load       0: trip       trip on         -           -            -            -
                1: p change   -               new p val   -            -            -
                2: q change   -               new q val   -            -            -
  -------------------------------------------------------------------------------------------

  e.g
  Event(0.1, 1, 12, 2, true, 0.25);
   denotes a 3ph fault applied at branch #12 at 0.25 secs

  */

};

} // end of namespace elabtsaot

#endif // EVENT_H
