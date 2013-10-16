
#include "load.h"
using namespace elabtsaot;

//#include <string>
using std::string;
//#include <complex>
using std::complex;
#include <sstream>
using std::stringstream;

Load::Load() :
    extId(0), busExtId(-1), Pdemand(0), Qdemand(0),
    Vexpa(0), Vexpb(0), kpf(0), kqf(0), // Corresponding to pload
    Vss(complex<double>(1,0)) {}

string Load::serialize() const{
  stringstream ss;
  ss << extId << " ";
  ss << busExtId << " ";
  ss << Pdemand << " ";
  ss << Qdemand << " ";
  ss << Vexpa << " ";
  ss << Vexpb << " ";
  ss << kpf << " ";
  ss << kqf << " ";
  ss << Vss << " ";
  return ss.str();
}

unsigned int Load::type() const{
  if ( Vexpa == 0 && Vexpb == 0 )
    return LOADTYPE_CONSTP;
  if ( Vexpa == 1 && Vexpb == 1 )
    return LOADTYPE_CONSTI;
  if ( Vexpa == 2 && Vexpb == 2 )
    return LOADTYPE_CONSTZ;
  return LOADTYPE_OTHER;
}
void Load::set_type(unsigned int val){
  if (val == LOADTYPE_CONSTP){
    Vexpa = 0;
    Vexpb = 0;
  } else if (val == LOADTYPE_CONSTI){
    Vexpa = 1;
    Vexpb = 1;
  } else if (val == LOADTYPE_CONSTZ){
    Vexpa = 2;
    Vexpb = 2;
  }
}
