
#include "branch.h"
using namespace elabtsaot;

//#include <string>
using std::string;
#include <sstream>
using std::stringstream;

Branch::Branch() : status(true), fromBusExtId(-1), toBusExtId(-1),
  R(0), X(0), Bfrom(0), Bto(0), Gfrom(0), Gto(0), Xratio(0), Xshift(0) {}


string Branch::serialize() const{
  stringstream ss;
  ss << extId << " ";
  ss << status << " ";
  ss << fromBusExtId << " ";
  ss << toBusExtId << " ";
  ss << R << " ";
  ss << X << " ";
  ss << Bfrom << " ";
  ss << Bto << " ";
  ss << Gfrom << " ";
  ss << Gto << " ";
  ss << Xratio << " ";
  ss << Xshift << " ";
  return ss.str();
}
