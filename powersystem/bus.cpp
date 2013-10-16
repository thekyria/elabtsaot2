
#include "bus.h"
using namespace elabtsaot;

//#include <string>
using std::string;
#include <sstream>
using std::stringstream;

Bus::Bus() : extId(0), name("defbus"), Gsh(0), Bsh(0), baseKV(1), V(1), theta(0), P(0), Q(0) {}

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
