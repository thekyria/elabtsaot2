
#include "scenarioset.h"
using namespace elabtsaot;

#include "powersystem.h"
#include "auxiliary.h"

//#include <vector>
using std::vector;
//#include <string>
using std::string;
#include <iostream>
using std::cout;
using std::endl;
#include <sstream>
using std::stringstream;
#include <complex>
using std::complex;

ScenarioSet::ScenarioSet() :
    _extId(0), _name("Default scenario set"), _description("") {}

void ScenarioSet::reset(){
  scenarios.clear();
  _extId = 0;
  _name = "Default scenario set";
  _description = "";
}

string ScenarioSet::serialize(){
  stringstream ss;

  ss << _extId << " ";
  ss << "_name:" << _name << ":_name ";
  ss << "_description:" << _description << ":_description ";

  return ss.str();
}

void ScenarioSet::display(){
  cout << "Scenario set: " << _name << " (id=" << _extId << ")" << endl;
  cout << " " << _description << endl;

  for ( size_t k = 0 ; k != scenarios.size() ; ++k ){
    cout << "Scenario #" << k << endl;
    scenarios[k].display();
  }
  return;
}

#define SCE_LOWZTHRESHOLD 0.0001
#define SCS_MNRNDFLTSTOP 0.200
#define SCS_MXRNDFLTSTOP 0.700

void ScenarioSet::createScenariosPerBranch( Powersystem const* pws,
                                            double scenarioStopTime,
                                            bool trip,
                                            bool ignoreLowZBrnahces,
                                            double faultLocation,
                                            double faultStart,
                                            double faultStop ){
  for ( size_t k = 0 ; k != pws->getBranchCount() ; ++k ){

    // Retrieve branch
    const Branch* br( pws->getBranch(k) );
    unsigned int brExtId = br->extId;

    // Create scenarios only for ON branches
    if (!br->status)  continue;

    // Check whether impedance of the branch is tool low (<SCE_LOWZTHRESHOLD)
    complex<double> branchZ( br->R, br->X );
    if ( ignoreLowZBrnahces )
      if ( std::abs(branchZ) < SCE_LOWZTHRESHOLD )
        continue;

    // Use faultLocation/Duration arguments unless out-of-bounds (eg. negative);
    // otherwise get randoms
    double tempFaultLocation = (faultLocation < 0.0 || faultLocation > 1.0 ) ?
          auxiliary::getRandom(double(0.0),double(1.0)) : faultLocation;
    double tempFaultStop = (faultStop < 0.0) ?
          auxiliary::getRandom(SCS_MNRNDFLTSTOP,SCS_MXRNDFLTSTOP) : faultStop;
    double tempFaultStart = (faultStart < 0.0) ?
          auxiliary::getRandom(0.0, tempFaultStop) : faultStart;


    Scenario tempSce( k,
                      "Scenario for branch " + auxiliary::to_string(brExtId),
                      "[" + auxiliary::to_string(tempFaultStart) + ","
                          + auxiliary::to_string(tempFaultStop) + "] "
                        + "3ph fault branch "
                        + auxiliary::to_string(brExtId)
                        + (trip ? " with " : " without ") + "branch trip" );
    tempSce.set_stopTime(scenarioStopTime);
    Event evOn( "fault on", tempFaultStart, EVENT_ELEMENT_TYPE_BRANCH, brExtId,
                EVENT_EVENTTYPE_BRFAULT, true, tempFaultLocation);
    Event evOff( "fault off", tempFaultStop, EVENT_ELEMENT_TYPE_BRANCH, brExtId,
                 EVENT_EVENTTYPE_BRFAULT, false, tempFaultLocation );
    tempSce.insertEvent( evOn );
    tempSce.insertEvent( evOff );
    if ( trip ){
      Event evTrip( "trip", tempFaultStop, EVENT_ELEMENT_TYPE_BRANCH, brExtId,
                    EVENT_EVENTTYPE_BRTRIP, true );
      tempSce.insertEvent( evTrip );
    }
    scenarios.push_back(tempSce);
  }

  return;
}

void ScenarioSet::set_id( unsigned int val ){ _extId = val; }
void ScenarioSet::set_name( string const& val ){ _name = val; }
void ScenarioSet::set_description( string const& val){ _description = val; }

unsigned int ScenarioSet::extId() const{ return _extId; }
string ScenarioSet::name() const{ return _name; }
string ScenarioSet::description() const{ return _description; }

int ScenarioSet::deleteScenario( size_t sceId ){
  if ( sceId >= scenarios.size() )
    return 1;

  scenarios.erase( scenarios.begin() + sceId );
  return 0;
}
