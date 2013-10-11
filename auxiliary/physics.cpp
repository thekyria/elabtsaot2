
#include "physics.h"
using namespace elabtsaot;

//#include <string>
using std::string;

string physics::variableToString( int variable ){
  switch ( variable ){
  case TIME: return "time";
  case ANGLE: return "angle";
  case ANGULAR_SPEED: return "angular speed";
  case VOLTAGE: return "voltage";
  case POWER: return "power";
  case CURRENT: return "current";
  default: return "unknown";
  }
}

int physics::defaultUnitOfVariable( int variable ){
  switch ( variable ){
  case TIME: return SECOND;
  case ANGLE: return RAD;
  case ANGULAR_SPEED: return PU;
  case VOLTAGE: return PU;
  case POWER: return PU;
  case CURRENT: return PU;
  default: return UNKNOWN_VARIABLE;
  }
}

string physics::unitToString( int unit ){
  switch ( unit ){
  case SECOND: return "second";
  case RAD: return "rad";
  case RAD_PER_SECOND: return "rad/second";
  case VOLT: return "volt";
  case WATT: return "watt";
  case VAR: return "var";
  case AMPERE: return "ampere";
  case PU: return "pu";
  default: return "unknown";
  }
}
