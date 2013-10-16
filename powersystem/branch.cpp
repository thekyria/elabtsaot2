
#include "branch.h"
using namespace elabtsaot;

//#include <string>
using std::string;
#include <sstream>
using std::stringstream;

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
  ss << Ifrom << " ";
  ss << Ito << " ";
  ss << Sfrom << " ";
  ss << Sto << " ";

  return ss.str();
}
