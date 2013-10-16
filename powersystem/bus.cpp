
#include "bus.h"
using namespace elabtsaot;

//#include <string>
using std::string;
#include <sstream>
using std::stringstream;

string Bus::serialize() const{
  stringstream ss;
  ss << extId << " ";
  ss << "_name:" << name << ":_name ";
  ss << Gsh << " ";
  ss << Bsh << " ";
  ss << baseKV << " ";
  ss << V << " ";
  ss << theta << " ";
  ss << P << " ";
  ss << Q << " ";
  return ss.str();
}
