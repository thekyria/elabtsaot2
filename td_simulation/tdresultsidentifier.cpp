
#include "tdresultsidentifier.h"
using namespace elabtsaot;

#include "auxiliary.h"
#include "physics.h"

//#include <string>
using std::string;

bool TDResultIdentifier::isValid( int elementType, int variable,
                                  string* elementVariableName, int* units ){

  switch( elementType ){

  case TDRESULTS_OTHER:
    switch ( variable ){

    case TDRESULTS_TIME:
      if ( elementVariableName )
        *elementVariableName = "time";
      if ( units )
        *units = physics::SECOND;
      return true;

    default:
      return false;
    }

  case TDRESULTS_BUS:
    switch ( variable ){

    case TDRESULTS_ACTIVE_POWER:
      if ( elementVariableName )
        *elementVariableName = "active power injected at bus";
      if ( units )
        *units = physics::PU;
      return true;

    case TDRESULTS_REACTIVE_POWER:
      if ( elementVariableName )
        *elementVariableName = "reactive power injected at bus";
      if ( units )
        *units = physics::PU;
      return true;

    case TDRESULTS_VOLTAGE_REAL:
      if ( elementVariableName )
        *elementVariableName = "voltage (real) at bus";
      if ( units )
        *units = physics::PU;
      return true;

    case TDRESULTS_VOLTAGE_IMAG:
      if ( elementVariableName )
        *elementVariableName = "voltage (imag) at bus";
      if ( units )
        *units = physics::PU;
      return true;

    case TDRESULTS_CURRENT_REAL:
      if ( elementVariableName )
        *elementVariableName = "current (real) injected at bus";
      if ( units )
        *units = physics::PU;
      return true;

    case TDRESULTS_CURRENT_IMAG:
      if ( elementVariableName )
        *elementVariableName = "current (imag) injected at bus";
      if ( units )
        *units = physics::PU;
      return true;

    default:
      return false;
    }

  case TDRESULTS_BRANCH:
    switch ( variable ){

    case TDRESULTS_ACTIVE_POWER:
      if ( elementVariableName )
        *elementVariableName = "active power flow (from to to) in branch";
      if ( units )
        *units = physics::PU;
      return true;

    case TDRESULTS_REACTIVE_POWER:
      if ( elementVariableName )
        *elementVariableName = "reactive power flow (from to to) in branch";
      if ( units )
        *units = physics::PU;
      return true;

    case TDRESULTS_CURRENT_REAL:
      if ( elementVariableName )
        *elementVariableName = "current flow (real, from to to) in branch";
      if ( units )
        *units = physics::PU;
      return true;

    case TDRESULTS_CURRENT_IMAG:
      if ( elementVariableName )
        *elementVariableName = "current flow (imag, from to to) in branch";
      if ( units ) *units = physics::PU;
      return true;

    default:
      return false;
    }

  case TDRESULTS_GENERATOR:
    switch ( variable ){

    case TDRESULTS_ANGLE:
      if ( elementVariableName )
        *elementVariableName = "internal angle of generator";
      if ( units )
        *units = physics::RAD;
      return true;

    case TDRESULTS_SPEED:
      if ( elementVariableName )
        *elementVariableName = "electrical speed of generator";
      if ( units )
        *units = physics::PU;
      return true;

    case TDRESULTS_ACTIVE_POWER:
      if ( elementVariableName )
        *elementVariableName = "active power injected by generator";
      if ( units )
        *units = physics::PU;
      return true;

    case TDRESULTS_REACTIVE_POWER:
      if ( elementVariableName )
        *elementVariableName = "reactive power injected by generator";
      if ( units )
        *units = physics::PU;
      return true;

    case TDRESULTS_MECHANICAL_POWER:
      if ( elementVariableName )
        *elementVariableName = "mechanical power of the generator turbine ";
      if ( units )
        *units = physics::PU;
      return true;

    case TDRESULTS_ACCELERATING_POWER:
      if ( elementVariableName )
        *elementVariableName = "accelerating power of generator";
      if ( units )
        *units = physics::PU;
      return true;

    case TDRESULTS_VOLTAGE_REAL:
      if ( elementVariableName )
        *elementVariableName = "voltage (real) at the bus of generator";
      if ( units )
        *units = physics::PU;
      return true;

    case TDRESULTS_VOLTAGE_IMAG:
      if ( elementVariableName )
        *elementVariableName = "voltage (imag) at the bus of generator";
      if ( units )
        *units = physics::PU;
      return true;

    case TDRESULTS_VOLTAGE_Q_TR:
      if ( elementVariableName )
        *elementVariableName = "q-axis voltage behind transient reactance";
      if ( units )
        *units = physics::PU;
      return true;

    case TDRESULTS_VOLTAGE_D_TR:
      if ( elementVariableName )
        *elementVariableName = "d-axis voltage behind transient reactance";
      if ( units )
        *units = physics::PU;
      return true;

    case TDRESULTS_CURRENT_REAL:
      if ( elementVariableName )
        *elementVariableName = "current (real) injected by generator";
      if ( units ) *units = physics::PU;
      return true;

    case TDRESULTS_CURRENT_IMAG:
      if ( elementVariableName )
        *elementVariableName = "current (imag) injected by generator";
      if ( units )
        *units = physics::PU;
      return true;

    default:
      return false;
    }

  case TDRESULTS_LOAD:
    switch ( variable ){

    case TDRESULTS_ACTIVE_POWER:
      if ( elementVariableName )
        *elementVariableName = "active power demanded by load";
      if ( units )
        *units = physics::PU;
      return true;

    case TDRESULTS_REACTIVE_POWER:
      if ( elementVariableName )
        *elementVariableName = "reactive power demanded by load";
      if ( units )
        *units = physics::PU;
      return true;

    case TDRESULTS_VOLTAGE_REAL:
      if ( elementVariableName )
        *elementVariableName = "voltage (real) at the bus of load";
      if ( units ) *units = physics::PU;
      return true;

    case TDRESULTS_VOLTAGE_IMAG:
      if ( elementVariableName )
        *elementVariableName = "voltage (imag) at the bus of load";
      if ( units ) *units = physics::PU;
      return true;

    case TDRESULTS_CURRENT_REAL:
      if ( elementVariableName )
        *elementVariableName = "current (real) injected at the bus of load";
      if ( units ) *units = physics::PU;
      return true;

    case TDRESULTS_CURRENT_IMAG:
      if ( elementVariableName )
        *elementVariableName = "current (imag) injected at the bus of load";
      if ( units ) *units = physics::PU;
      return true;

    default:
      return false;
    }

    break;

  default:
    return false;
  }
}

string TDResultIdentifier::elementTypeToString( int val ){
  switch ( val ){
  case TDRESULTS_OTHER:
    return "other";
  case TDRESULTS_BUS:
    return "bus";
  case TDRESULTS_BRANCH:
    return "branch";
  case TDRESULTS_GENERATOR:
    return "generator";
  case TDRESULTS_LOAD:
    return "load";
  default:
    return "unknown element type";
  }
}

int TDResultIdentifier::variableType( int variable ){
  switch ( variable ){
  case TDRESULTS_TIME:
    return physics::TIME;
  case TDRESULTS_ANGLE:
    return physics::ANGLE;
  case TDRESULTS_SPEED:
    return physics::ANGULAR_SPEED;
  case TDRESULTS_ACTIVE_POWER:
  case TDRESULTS_REACTIVE_POWER:
  case TDRESULTS_MECHANICAL_POWER:
  case TDRESULTS_ACCELERATING_POWER:
    return physics::POWER;
  case TDRESULTS_VOLTAGE_REAL:
  case TDRESULTS_VOLTAGE_IMAG:
  case TDRESULTS_VOLTAGE_Q_TR:
  case TDRESULTS_VOLTAGE_D_TR:
    return physics::VOLTAGE;
  case TDRESULTS_CURRENT_REAL:
  case TDRESULTS_CURRENT_IMAG:
    return physics::CURRENT;
  default:
    return physics::UNKNOWN_VARIABLE;
  }
}

string TDResultIdentifier::variableToString( int variable ){
  switch ( variable ){
  case TDRESULTS_TIME:
    return "time";
  case TDRESULTS_ANGLE:
    return "angle";
  case TDRESULTS_SPEED:
    return "speed";
  case TDRESULTS_ACTIVE_POWER:
    return "active power";
  case TDRESULTS_REACTIVE_POWER:
    return "reactive power";
  case TDRESULTS_MECHANICAL_POWER:
    return "mechanical power";
  case TDRESULTS_ACCELERATING_POWER:
    return "accelerating power";
  case TDRESULTS_VOLTAGE_REAL:
    return "voltage real";
  case TDRESULTS_VOLTAGE_IMAG:
    return "voltage imag";
  case TDRESULTS_VOLTAGE_Q_TR:
    return "q-axis transient voltage";
  case TDRESULTS_VOLTAGE_D_TR:
    return "d-axis transient voltage";
  case TDRESULTS_CURRENT_REAL:
    return "current real";
  case TDRESULTS_CURRENT_IMAG:
    return "current imag";
  default:
    return "unknown variable type";
  }
}

string TDResultIdentifier::defaultName( TDResultIdentifier tdri ){

  string name;
  int units;
  if (isValid( tdri.elementType, tdri.variable, &name, &units ) )
    if ( tdri.elementType != TDRESULTS_OTHER )
      return (name + " " + auxiliary::to_string( tdri.extId ));
    else
      return name;
  else
    return "";
}

string TDResultIdentifier::defaultName( int elementType,
                                        int variable ){
  string name;
  int units;
  if ( isValid( elementType, variable, &name, &units ))
    return name;
  else
    return "";
}

string TDResultIdentifier::defaultUnits( TDResultIdentifier tdri ){

  string name;
  int units;
  if (isValid( tdri.elementType, tdri.variable, &name, &units ) )
    return physics::unitToString( units );
  else
    return "";
}

string TDResultIdentifier::defaultUnits( int elementType,
                                         int variable ){
  string name;
  int units;
  if ( isValid( elementType, variable, &name, &units ))
    return physics::unitToString( units );
  else
    return "";
}
