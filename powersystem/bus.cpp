
#include "bus.h"
using namespace elabtsaot;

//#include <string>
using std::string;
#include <iostream>
using std::cout;
using std::cin;
using std::endl;
#include <sstream>
using std::stringstream;

Bus::Bus(unsigned int extId, string const& name,
         double baseKV, double Vss, double thss) :
    _extId(extId), _name(name),
    _gsh(0), _bsh(0),
    _baseKV(baseKV),
    _Vss(Vss), _thss(thss), _Pss(0), _Qss(0) {}

string Bus::serialize() const{

  stringstream ss;

  ss << _extId << " ";
  ss << "_name:" << _name << ":_name ";
  ss << _gsh << " ";
  ss << _bsh << " ";
  ss << _baseKV << " ";
  ss << _Vmax << " ";
  ss << _Vmin << " ";
  ss << _Vss << " ";
  ss << _thss << " ";
  ss << _Pss << " ";
  ss << _Qss << " ";

  return ss.str();
}

void Bus::display() const{
  cout << "#" << _extId << " " << _name << endl;
  cout << "\t" << _gsh;
  cout << "\t" << _bsh;
  cout << "\t" << _baseKV;
  cout << "\t" << _Vmax;
  cout << "\t" << _Vmin;

  cout << "\t" << _Vss;
  cout << "\t" << _thss << endl;
}

// --- getters ---
unsigned int Bus::extId() const{ return _extId; }
string Bus::name() const{ return _name; }
double Bus::gsh() const{ return _gsh; }
double Bus::bsh() const{ return _bsh; }
double Bus::baseKV() const{ return _baseKV; }
double Bus::Vmax() const{ return _Vmax; }
double Bus::Vmin() const{ return _Vmin; }

double Bus::Vss() const{ return _Vss; }
double Bus::thss() const{ return _thss; }
double Bus::Pss() const{ return _Pss; }
double Bus::Qss() const{ return _Qss; }

// --- setters ---
void Bus::set_extId(unsigned int val){ _extId = val; }
void Bus::set_name(string const& val){ _name = val; }
void Bus::set_gsh(double val){ _gsh = val; }
void Bus::set_bsh(double val){ _bsh = val; }
void Bus::set_baseKV(double val){ _baseKV = val; }
void Bus::set_Vmax(double val){ _Vmax = val; }
void Bus::set_Vmin(double val){ _Vmin = val; }

void Bus::set_Vss(double val){ _Vss = val; }
void Bus::set_thss(double val){ _thss = val; }
void Bus::set_Pss(double val){ _Pss = val; }
void Bus::set_Qss(double val){ _Qss = val; }
