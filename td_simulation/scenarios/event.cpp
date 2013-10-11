
#include "event.h"
using namespace elabtsaot;

//#include <string>
using std::string;
#include <iostream>
using std::cout;
using std::cin;
using std::endl;
#include <sstream>
using std::stringstream;

Event::Event(string const& name,
             double time,
             unsigned int element_type,
             unsigned int element_extId,
             unsigned int event_type,
             bool bool_arg,
             double double_arg,
             double double_arg_1,
             double double_arg_2,
             unsigned int uint_arg) :
    _name(name), _description(""), _time(time), _status(true),
    _element_type(element_type), _element_extId(element_extId),
    _event_type(event_type),
    _bool_arg(bool_arg), _double_arg(double_arg), _double_arg_1(double_arg_1),
    _double_arg_2(double_arg_2), _uint_arg(uint_arg) {}

string Event::serialize() const{

  stringstream ss;

  ss << "_name:" << _name << ":_name ";
  ss << _time << " ";
  ss << "_description:" << _description << "_description ";
  ss << _status << " ";
  ss << _element_type << " ";
  ss << _element_extId << " ";
  ss << _event_type << " ";
  ss << _bool_arg << " ";
  ss << _double_arg << " ";
  ss << _double_arg_1 << " ";
  ss << _double_arg_2 << " ";
  ss << _uint_arg << " ";

  return ss.str();
}

void Event::display() const{
  cout << "Name: " << _name << "; ";
  cout << "Description: " << _description << endl;
  cout << "@time " << _time << " at element : ";
  cout << _element_extId << " (type " << _element_type << ")" << endl;
  cout << "Event args: " << _event_type << " "
                         << _bool_arg << " "
                         << _double_arg << " "
                         << _double_arg_1 << " "
                         << _double_arg_2 << " "
                         << _uint_arg << " status: "<< _status<< endl;
}

// -- setters
void Event::set_name(string const& val){ _name = val;}
int Event::set_time(double val){
  if ( val < 0 )
    return 1;
  _time = val;
  return 0;
}
void Event::set_description(string const& val){ _description = val; }
void Event::set_status(bool val){ _status = val; }
void Event::set_element_type(unsigned int val){ _element_type = val;}
void Event::set_element_extId(unsigned int val){ _element_extId = val; }
void Event::set_event_type(unsigned int val){ _event_type = val; }
void Event::set_bool_arg(bool val){ _bool_arg = val; }
void Event::set_double_arg(double val){ _double_arg = val; }
void Event::set_double_arg_2(double val){ _double_arg_1 = val; }
void Event::set_double_arg_1(double val){ _double_arg_2 = val; }
void Event::set_uint_arg(unsigned int val){ _uint_arg = val; }

// -- getters
string Event::name() const{ return _name; }
double Event::time() const{ return _time; }
string Event::description() const{ return _description; }
bool Event::status() const{ return _status; }
unsigned int Event::element_type() const{ return _element_type; }
unsigned int Event::element_extId() const{ return _element_extId; }
unsigned int Event::event_type() const{ return _event_type; }
bool Event::bool_arg() const{ return _bool_arg; }
double Event::double_arg() const{ return _double_arg; }
double Event::double_arg_2() const{ return _double_arg_1; }
double Event::double_arg_1() const{ return _double_arg_2; }
unsigned int Event::uint_arg() const{ return _uint_arg; }
