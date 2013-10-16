
#include "powersystem.h"
using namespace elabtsaot;

#include "precisiontimer.h"
#include "graph.h"

#include <boost/numeric/ublas/lu.hpp> // for matrix operations
#include <boost/numeric/ublas/io.hpp> // for ublas::matrix '<<'
//#include <boost/numeric/ublas/matrix.hpp> // Required for matrix operations
namespace ublas = boost::numeric::ublas;

//#include <set>
using std::set;
//#include <string>
using std::string;
//#include <vector>
using std::vector;
//#include <iostream>
using std::ostream;
using std::cout;
using std::endl;
using std::ios;
#include <sstream>
using std::stringstream;
#include <fstream>
using std::ofstream;
//#include <complex>
using std::complex;
using std::polar;
using std::conj;
#include <limits>
using std::numeric_limits;
#include <ctime>                      // for time(), ctime()
#include <cmath>                      // for M_PI constant
#define _USE_MATH_DEFINES

//! Auxiliary definition for UintUintBimap
typedef UintUintBimap::value_type UintPair;
//! Auxiliary definition for UintUintBimap
typedef UintUintBimap::const_iterator UintUintBimapConstIt;

//! Bus type enumeration
enum BusType {
  BUSTYPE_UNDEF,         //!< Undefined bus type (possibly only yet)
  BUSTYPE_PQ,            //!< PQ bus type: P and Q injection defined at the bus
  BUSTYPE_PV,            //!< PV bus type: P injection and voltage magnitude
                         //!< defined at the bus
  BUSTYPE_SLACK          //!< Slack bus: Voltage magnitude and angle defined
                         //!< defined at the bus
};

Powersystem::Powersystem( string const& name,
                          double baseS, double baseF) :
    _status(PWSSTATUS_INIT), _name(name),
    _baseS(baseS), _baseF(baseF),
    _slackBusExtId(-1), _slackGenExtId(-1) {}

//! Serializes the contents of the powersystem bus into an std::string
string Powersystem::serialize() const{

  stringstream ss;

  ss << _status << " ";
  ss << "_name:" << _name << ":_name ";
  ss << "_description:" << _description << ":_description ";
  ss << _baseS << " ";
  ss << _baseF << " ";
  ss << _slackBusExtId << " ";
  ss << _slackGenExtId << " ";
  ss << "_busSet.size=" << _busSet.size() << " ";
  for ( size_t k = 0 ; k != _busSet.size() ; ++k )
    ss << "_busSet[" << k << "]:" << _busSet[k].serialize()
       << ":_busSet[" << k << "] ";
  ss << "_brSet.size=" << _brSet.size() << " ";
  for ( size_t k = 0 ; k != _brSet.size() ; ++k )
    ss << "_brSet[" << k << "]:" << _brSet[k].serialize()
       << ":_brSet[" << k << "] ";
  ss << "_genSet.size=" << _genSet.size() << " ";
  for ( size_t k = 0 ; k != _genSet.size() ; ++k )
    ss << "_genSet[" << k << "]:" << _genSet[k].serialize()
       << ":_genSet[" << k << "] ";
  ss << "_loadSet.size=" << _loadSet.size() << " ";
  for ( size_t k = 0 ; k != _loadSet.size() ; ++k )
    ss << "_loadSet[" << k << "]:" << _loadSet[k].serialize()
       << ":_loadSet[" << k << "] ";

  return ss.str();
}

int Powersystem::log_loadflow_results(ostream& ostr) const{

  if ( _status != PWSSTATUS_LF ){
#ifdef VERBOSE_PWR
    ostr << "ERROR: Loadflow not yet performed on the powersystem!" << endl;
    ostr << "Please run loadflow first and then "
         << "invoke Powersystem::log_loadflow_results()" << endl;
#endif // VERBOSE_PWR
    return 1;
  }
  // else{ // ( _status == PWSSTATUS_LF )
  if ( ostr.bad() )
    // Error writing to ostr!
    return 10;

  size_t k;                               // counter
  //size_t n = _busSet.size();              // number of nodes

  ostr.precision(5);        // sets the decimal precision to be used by output
                            // operations at 5 significant digits
  //ostr.width(xxx);        // set field width to xxx BUT works only for the
                            // next insertion operation; no turn around.
  ostr.setf ( ios::fixed, ios::floatfield );  // use fixed-point notation
  //ostr.setf ( ios::right, ios::adjustfield ); // adjust fields right (TODO!)

  ostr << "========================================" ;
  ostr << "=======================================" ;
  ostr << endl;
  ostr << "\tSystem summary" << endl;
  ostr << "========================================" ;
  ostr << "=======================================" ;
  ostr << endl;
  ostr << endl;

  ostr << "Name of the power system: " << this->_name << endl;
  ostr << "Short description: " << this->_description << endl;
  ostr << endl;

  ostr << "Power base:\t\t" << this->_baseS << " MVA" << endl;
  ostr << "Frequency base:\t\t" << this->_baseF << " Hz" << endl;
  ostr << "Index of slack bus:\t" << this->_slackBusExtId << endl;
  ostr << "Index of slack gen:\t" << this->_slackGenExtId << endl;
  ostr << endl;

  ostr << "Number of buses:\t" << this->_busSet.size() << endl;
  ostr << "Number of branches:\t" << this->_brSet.size() << endl;
  ostr << "Number of generators:\t" << this->_genSet.size() << endl;
  ostr << "Number of loads:\t" << this->_loadSet.size() << endl;
  ostr << endl;

  double pgen_capacity_total = 0;
  double qgen_capacity_total_min = 0, qgen_capacity_total_max = 0;
  double pgen_capacity_online = 0;
  double qgen_capacity_online_min = 0, qgen_capacity_online_max = 0;
  double pgen_actual_total = 0, qgen_actual_total = 0;
  double pload_actual_total = 0, qload_actual_total = 0;

  for ( k = 0 ; k != _genSet.size() ; ++k ){
    pgen_capacity_total += _genSet[k].pmax();
    qgen_capacity_total_min += _genSet[k].qmin();
    qgen_capacity_total_max += _genSet[k].qmax();
    if ( _genSet[k].status() ){
      pgen_capacity_online += _genSet[k].pmax();
      qgen_capacity_online_min += _genSet[k].qmin();
      qgen_capacity_online_max += _genSet[k].qmax();
    }
    pgen_actual_total += _genSet[k].pgen();
    qgen_actual_total += _genSet[k].qgen();
  }
  for ( k = 0 ; k != _loadSet.size() ; ++k ){
    pload_actual_total += _loadSet[k].Qdemand;
    qload_actual_total += _loadSet[k].Qdemand;
  }
  ostr << "Generation capacity total:\t";
  ostr << "P = " << pgen_capacity_total << "\t";
  ostr << "Q = " << qgen_capacity_total_min << " ";
  ostr << "to " << qgen_capacity_total_max << endl;
  ostr << "Generation capacity online:\t";
  ostr << "P = " << pgen_capacity_online << "\t";
  ostr << "Q = " << qgen_capacity_online_min << " ";
  ostr << "to " << qgen_capacity_online_max << endl;
  ostr << "Actual generation total:\t";
  ostr << "P = " << pgen_actual_total << "\t";
  ostr << "Q = " << qgen_actual_total << endl;
  ostr << "Actual load total:\t\t";
  ostr << "P = " << pload_actual_total << "\t";
  ostr << "Q = " << qload_actual_total << endl;
  ostr << "Losses total:\t\t\t";
  ostr << "P = " << pgen_actual_total - pload_actual_total << "\t";
  ostr << "Q = " << qgen_actual_total - qload_actual_total << endl;
  ostr << endl;

  ostr << "========================================" ;
  ostr << "=======================================" ;
  ostr << endl;
  ostr << "\tBus Data" << endl;
  ostr << "========================================" ;
  ostr << "=======================================" ;
  ostr << endl;
  ostr << endl;

  ostr.width(5);
  ostr << "Id";
  ostr.width(9);
  ostr << "V";
  ostr.width(9);
  ostr << "theta";
  ostr.width(9);
  ostr << "P";
  ostr.width(9);
  ostr << "Q" << endl;
  for ( k = 0 ; k != _busSet.size() ; ++k ){
    ostr.width(5);
    ostr << _busSet[k].extId;          // bus ext id
    ostr.width(9);
    ostr << _busSet[k].V;            // bus voltage magnitude
    ostr.width(9);
    ostr << _busSet[k].theta;           // bus voltage angle
    ostr.width(9);
    ostr << _busSet[k].P;            // P at bus
    ostr.width(9);
    ostr << _busSet[k].Q;            // Q at bus
    ostr << endl;
  }
  ostr << endl;

  ostr << "========================================" ;
  ostr << "=======================================" ;
  ostr << endl;
  ostr << "\tBranch Data (* denotes online)" << endl;
  ostr << "========================================" ;
  ostr << "=======================================" ;
  ostr << endl;
  ostr << endl;

  // TODO: fix I flows
  ostr.width(5);
  ostr << "Id";
  ostr.width(5);
  ostr << "from";
  ostr.width(5);
  ostr << "to";
  ostr.width(9);
  ostr << "P_f";
  ostr.width(9);
  ostr << "Q_f";
  ostr.width(9);
  ostr << "P_t";
  ostr.width(9);
  ostr << "Q_t";
  ostr.width(9);
  ostr << "I_f";
  ostr.width(9);
  ostr << "I_t";
  ostr << endl;
  for ( k = 0 ; k != _brSet.size() ; ++k ){
    if ( _brSet[k].status )
      ostr << "*";
    else
      ostr << " ";
    ostr.width(4);
    ostr << _brSet[k].extId;        // branch ext id
    ostr.width(5);
    ostr << _brSet[k].fromBusExtId; // branch from bus ext id
    ostr.width(5);
    ostr << _brSet[k].toBusExtId;   // branch to bus ext id
    ostr.width(9);
    ostr << _brSet[k].Sfrom.real(); // active power flow at from side
    ostr.width(9);
    ostr << _brSet[k].Sfrom.imag(); // reactive power flow at from side
    ostr.width(9);
    ostr << _brSet[k].Sto.real();   // active power flow at to side
    ostr.width(9);
    ostr << _brSet[k].Sto.imag();   // reactive power flow at to side
    ostr.width(9);
    ostr << _brSet[k].Ifrom;        // current flow at from side
    ostr.width(9);
    ostr << _brSet[k].Ito;          // current flow at to side
    ostr << endl;
  }
  ostr << endl;

  ostr << "========================================" ;
  ostr << "=======================================" ;
  ostr << endl;
  ostr << "\tGenerator Data (* denotes online)" << endl;
  ostr << "========================================" ;
  ostr << "=======================================" ;
  ostr << endl;
  ostr << endl;

  ostr.width(5);
  ostr << "Id";
  ostr.width(9);
  ostr << "P";
  ostr.width(9);
  ostr << "Q";
  ostr.width(9);
  ostr << "E";
  ostr.width(9);
  ostr << "delta";
  ostr << endl;
  for ( k = 0 ; k != _genSet.size() ; ++k ){
    if ( _genSet[k].status() )
      ostr << "*";
    else
      ostr << " ";
    ostr.width(4);
    ostr << _genSet[k].extId();
    ostr.width(9);
    ostr << _genSet[k].pgen();
    ostr.width(9);
    ostr << _genSet[k].qgen();
    ostr.width(9);
    ostr << _genSet[k].Ess();
    ostr.width(9);
    ostr << _genSet[k].deltass();
    ostr << endl;
  }
  ostr << endl;

  return 0;
}

int Powersystem::log_loadflow_results(string const& filename) const{

  if ( _status != PWSSTATUS_LF ){
    return 1;
  }
  // else{ // ( _status == PWSSTATUS_LF )

  // Convert string fname to char* a
  char *a=new char[filename.size()+1];
  a[filename.size()]=0;
  memcpy(a,filename.c_str(),filename.size());

  int ans = 0;
  ofstream ofstr;
  ofstr.open( a , std::ios::trunc);
  if ( ofstr.is_open() ){
    ans = log_loadflow_results(ofstr);
    ofstr.close();
  } else{
  // Error opening the file
    return 2;
  }

  return ans;
}

double Powersystem::getMaxX() const{
  double maxX = -1;

  // Check branch set
  for ( size_t k = 0 ; k != _brSet.size() ; ++k )
    if ( _brSet[k].X > maxX )
      maxX = _brSet[k].X;

  // Check generator set
  for ( size_t k = 0 ; k != _genSet.size(); ++k )
    if ( _genSet[k].xd_1() > maxX )
      maxX = _genSet[k].xd_1();

  return maxX;
}

//// network builder functions
int Powersystem::clear(){
  _status = PWSSTATUS_INIT;

  _slackBusExtId = -1;
  _slackGenExtId = -1;

  _busSet.clear();
  _brSet.clear();
  _genSet.clear();
  _loadSet.clear();

  _busIdBimap.clear();
  _brIdBimap.clear();
  _genIdBimap.clear();
  _loadIdBimap.clear();

  _busBrMap.clear();
  _busGenMap.clear();
  _busLoadMap.clear();

  return 0;
}

int Powersystem::addBus(Bus const& newBus){

  // Check whether extId of newBus exists in _busSet
  if(_busIdBimap.left.find(newBus.extId) != _busIdBimap.left.end()){
    // extId of newBus already exists in Powersystem
    return 1;
  }

  // Insert newBus into the _busSet
  _busSet.push_back(newBus);

  // Update _busIdBimap & _busXXMap's
  _rebuildBusIdBimap();
  _rebuildBusBrMap();
  _rebuildBusGenMap();
  _rebuildBusLoadMap();

  _status = PWSSTATUS_INIT;

  return 0;
}

int Powersystem::addBranch(Branch const& newBranch){

  // Check whether extId of newBranch exists in _brSet
  if(_brIdBimap.left.find(newBranch.extId) != _brIdBimap.left.end()){
    // extId of newBranch already exists in Powersystem
    return 1;
  }

  // Check whether newBranch fromBusExtId and toBusExtId exist in busSet
  if(_busIdBimap.left.find(newBranch.fromBusExtId)==_busIdBimap.left.end() ||
     _busIdBimap.left.find(newBranch.toBusExtId)==_busIdBimap.left.end() ){
    // from/to-BusExtId's not found in busSet, so addBranch fails as newBranch
    // would be floating
    return 2;
  }

  // Assert that newBranch fromBusExtId <> toBusExtId
  if(newBranch.fromBusExtId==newBranch.toBusExtId){
    return 3;
  }  

  // Insert newBranch into the _busSet
  _brSet.push_back(newBranch);

  // Update _brIdBimap & _busBrMap
  _rebuildBrIdBimap();
  _rebuildBusBrMap();

  _status = PWSSTATUS_INIT;

  return 0;
}

int Powersystem::addGen(Generator const& newGen){

  // Check whether extId of newGen exists in _genSet
  if(_genIdBimap.left.find(newGen.extId()) != _genIdBimap.left.end()){
    // extId of newGen already exists in Powersystem
    return 1;
  }
  
  // Check whether newGen busExtId exists in busSet
  if(_busIdBimap.left.find(newGen.busExtId())==_busIdBimap.left.end() ){
    // busExtId not found in busSet, so addGen fails as newGen would be floating
    return 2;
  }

  // Check whether voltageSetpoint of newGen is different from the
  // voltageSetpoint of generators already present at the bus
  size_t n = _genSet.size();
  size_t k;

  for(k = 0; k != n; ++k){
      if ( (_genSet[k].busExtId() == newGen.busExtId()) &&
           (_genSet[k].voltageSetpoint() != newGen.voltageSetpoint()) ){
        // there is a gen in the powersystem at the bus of newGen with different
        // voltageSetpoint (illegal)
        return 3;
      }
  }

  // Insert newGen into the _genSet
  _genSet.push_back(newGen);

  // Update _genIdBimap & _busGenMap
  _rebuildGenIdBimap();
  _rebuildBusGenMap();

  _status = PWSSTATUS_INIT;
  return 0;
}

int Powersystem::addLoad(Load const& newLoad){

  // Check whether extId of newLoad exists in _loadSet
  if(_loadIdBimap.left.find(newLoad.extId) != _loadIdBimap.left.end()){
    // extId of newLoad already exists in Powersystem
    return 1;
  }
  
  // Check whether newLoad busExtId exists in busSet
  if(_busIdBimap.left.find(newLoad.busExtId)==_busIdBimap.left.end() ){
    // busExtId not found in busSet so addLoad fails as newLoad would be floatin
    return 2;
  }

  // Insert newLoad into the _loadSet
  _loadSet.push_back(newLoad);

  // Update _loadIdBimap & _busLoadMap
  _rebuildLoadIdBimap();
  _rebuildBusLoadMap();

  _status = PWSSTATUS_INIT;

  return 0;
}

// TODO: Adapt deleteBus() to use _busGenMap, _busBrMap, _busLoadMap
int Powersystem::deleteBus(unsigned int busExtId, bool recursive){

  // Check whether busExtId exists in _busSet
  if(_busIdBimap.left.find(busExtId) == _busIdBimap.left.end()){
    // busExtId not found, cannot delete a non-existent bus!
    return 1;
  }

  if(recursive){
    // If flag recursive is set, then the bus is deleted alongside all elements
    // (branches, loads, gens) that are connected to it
    for(vector<Branch>::iterator i = _brSet.begin(); i != _brSet.end(); ++i)
      if (    (i->fromBusExtId == static_cast<int>(busExtId))
           || (i->toBusExtId   == static_cast<int>(busExtId)) ){
        deleteBranch(i->extId);
        --i;
      }
    for(vector<Generator>::iterator i = _genSet.begin(); i != _genSet.end(); ++i)
      if ( i->busExtId() == static_cast<int>(busExtId) ){
        deleteGen(i->extId());
        --i;
      }
    for(vector<Load>::iterator i = _loadSet.begin(); i != _loadSet.end(); ++i)
      if (i->busExtId == static_cast<int>(busExtId)){
        deleteLoad(i->extId);
        --i;
      }
  } else{
    // If the flag is not set, and network elements (branches, loads, gens) are
    // connected to the bus to be deleted, then deleteBus fails
    for(vector<Branch>::iterator i = _brSet.begin(); i != _brSet.end(); ++i)
      if (    (i->fromBusExtId == static_cast<int>(busExtId))
           || (i->toBusExtId   == static_cast<int>(busExtId)) )
        return 2;
    for(vector<Generator>::iterator i = _genSet.begin(); i != _genSet.end(); ++i)
      if ( i->busExtId() == static_cast<int>(busExtId) )
        return 3;
    for(vector<Load>::iterator i = _loadSet.begin(); i != _loadSet.end(); ++i)
      if ( i->busExtId == static_cast<int>(busExtId) )
        return 4;
  }

  // If the bus to be deleted was the slackBus, reset _slackBusExtId to -1
  if( busExtId == static_cast<unsigned int>(_slackBusExtId) )
    _slackBusExtId = -1;

  // Delete busIntId'th element of the _busSet
  size_t busIntId = _busIdBimap.left.at(busExtId);
  _busSet.erase(_busSet.begin()+busIntId);

  // Update _busIdBimap & _busXXMap's
  _rebuildBusIdBimap();
  _rebuildBusBrMap();
  _rebuildBusGenMap();
  _rebuildBusLoadMap();

  _status = PWSSTATUS_INIT;

  return 0;
}

int Powersystem::deleteBranch(unsigned int brExtId){

  // Check whether brExtId exists in _brSet
  if( _brIdBimap.left.find(brExtId) == _brIdBimap.left.end()){
    // brExtId not found, cannot delete a non-existent branch!
    return 1;
  }

  // Delete brIntId'th element of the _brSet
  size_t brIntId = _brIdBimap.left.at(brExtId);
  _brSet.erase(_brSet.begin()+brIntId);

  // Update _brIdBimap & _busBrMap
  _rebuildBrIdBimap();
  _rebuildBusBrMap();

  _status = PWSSTATUS_INIT;

  return 0;
}

int Powersystem::deleteGen(unsigned int genExtId){

  // Check whether genExtId exists in _genSet
  if( _genIdBimap.left.find(genExtId) == _genIdBimap.left.end()){
    // genExtId not found, cannot delete a non-existent generator!
    return 1;
  }

  // If the gen to be deleted was the slackGen, reset _slackGenExtId to -1
  if( genExtId == static_cast<unsigned int>(_slackGenExtId) )
    _slackGenExtId = -1;

  // Delete genIntId'th element of the vector
  size_t genIntId = _genIdBimap.left.at(genExtId);
  _genSet.erase(_genSet.begin()+genIntId);

  // Update _genIdBimap & _busGenMap
  _rebuildGenIdBimap();
  _rebuildBusGenMap();

  _status = PWSSTATUS_INIT;

  return 0;
}

int Powersystem::deleteLoad(unsigned int loadExtId){

  // Check whether loadExtId exists in _loadSet
  if( _loadIdBimap.left.find(loadExtId) == _loadIdBimap.left.end()){
    // loadExtId not found, cannot delete a non-existent load!
    return 1;
  }

  // Delete loadIntId'th element of the vector
  size_t loadIntId = _loadIdBimap.left.at(loadExtId);
  _loadSet.erase(_loadSet.begin()+loadIntId);

  // Update _loadIdBimap & _busLoadMap
  _rebuildLoadIdBimap();
  _rebuildBusLoadMap();

  _status = PWSSTATUS_INIT;

  return 0;
}

int Powersystem::validate(){

  size_t k, m; // counters

  // ***** Validate bus set *****
  // Validate that all bus extIds are unique
  set<unsigned int> busExtIds;
  for ( k = 0 ; k != _busSet.size() ; ++k ){
    if ( busExtIds.find(_busSet[k].extId) != busExtIds.end() ){
      return 1;
    } else {
      busExtIds.insert( _busSet[k].extId );
    }
  }

  // Validate that the slack bus has been set
  if ( _slackBusExtId < 0 )

  // Validate that the slack bus is an existing bus
  if ( busExtIds.find( static_cast<unsigned int>(_slackBusExtId) )
        == busExtIds.end() ){
    return 2;
  }

  // Rebuild busIdBimap
  _rebuildBusIdBimap();

  // Rebuild bus-element maps
  _rebuildBusBrMap();
  _rebuildBusGenMap();
  _rebuildBusLoadMap();

  // ***** Validate branch set *****
  // Validate that all branch extIds are unique
  set<unsigned int> brExtIds;
  for ( k = 0 ; k != _brSet.size() ; ++k ){
    if ( brExtIds.find(_brSet[k].extId) != brExtIds.end() ){
      return 3;
    } else {
      brExtIds.insert( _brSet[k].extId );
    }
    // Validate that all branches point to existing buses
    if ( ( busExtIds.find(_brSet[k].fromBusExtId) == busExtIds.end() ) ||
         ( busExtIds.find(_brSet[k].toBusExtId) == busExtIds.end() ) ){
      return 4;
    }
    // Assert that for every branch fromBusExtId <> toBusExtId
    if ( _brSet[k].fromBusExtId == _brSet[k].toBusExtId ){
      return 5;
    }
  }
  // Rebuild brIdBimap
  _rebuildBrIdBimap();

  // ***** Islands (connected components) in the power system topology *****
  int num = graph::countIslands( *this );
  // Validate that there is only one island
  if (num > 1){
    return 12;
  }

  // ***** Validate gen set *****
  set<unsigned int> genExtIds;
  bool slackBusHasGen = false;
  for ( k = 0 ; k != _genSet.size() ; ++k ){
    // Validate that all gen extIds are unique
    if ( genExtIds.find(_genSet[k].extId()) != genExtIds.end() ){
      return 6;
    } else {
      genExtIds.insert( _genSet[k].extId() );
    }
    // Validate that all generators point to existing buses
    if ( busExtIds.find( _genSet[k].busExtId() ) == busExtIds.end() ){
      return 7;
    }
    // Make sure that slack bus has at least one online gen
    if ( (_genSet[k].busExtId() == static_cast<unsigned int>(_slackBusExtId))
         && (_genSet[k].status()) )
      slackBusHasGen = true;
    // Assert that the slack gen is at the slack bus
    if ( (_genSet[k].extId() == static_cast<unsigned int>(_slackGenExtId) )
         && (_genSet[k].busExtId() != static_cast<unsigned int>(_slackBusExtId))
        ){
      return 71;
    }
  }
  if ( !slackBusHasGen ){
    return 8;
  }
  // Validate that the slack generator is an existing generator
  if ( genExtIds.find(_slackGenExtId) == genExtIds.end() ){
    return 81;
  }

  // Validate that no 2 gens in _genSet are at the same bus but have different
  // voltage setpoints
  size_t n = _genSet.size();
  unsigned int busExtId;
  double voltageSetpoint;
  for(k = 0; k != n-1; ++k){
    busExtId = _genSet[k].busExtId();
    voltageSetpoint = _genSet[k].voltageSetpoint();
    for (m = k+1; m != n ; ++m)
      if ( (_genSet[m].busExtId() == busExtId) &&
           (_genSet[m].voltageSetpoint() != voltageSetpoint) ){
        // there are two gens (k, m) of the _genSet, both at bus busExtId but
        // with different voltageSetpoints (illegal)
        return 9;
      }
  }

  // Rebuild genIdBimap
  _rebuildGenIdBimap();

  // ***** Validate load set *****
  set<unsigned int> loadExtIds;
  for ( k = 0 ; k != _loadSet.size() ; ++k ){
    // Validate that all load extIds are unique
    if ( loadExtIds.find(_loadSet[k].extId) != loadExtIds.end() ){
      return 10;
    } else {
      loadExtIds.insert( _loadSet[k].extId );
    }

    // Validate that all loads point to existing buses
    if ( busExtIds.find( _loadSet[k].busExtId ) == busExtIds.end() ){
      return 11;
    }
  }

  // Rebuild loadIdBimap
  _rebuildLoadIdBimap();

  this->_status = PWSSTATUS_VALID;
  return 0;
}

// --- getters ---
int Powersystem::status() const{ return _status; }
string Powersystem::name() const{ return _name; }
string Powersystem::description() const{ return _description; }
double Powersystem::baseS() const{ return _baseS; }
double Powersystem::baseF() const{ return _baseF; }
int Powersystem::slackBusExtId() const{ return _slackBusExtId; }
int Powersystem::slackGenExtId() const{ return _slackGenExtId; }
vector<set<size_t> > Powersystem::getBusBrMap() const{ return _busBrMap; }
vector<set<size_t> > Powersystem::getBusGenMap() const{ return _busGenMap; }
vector<set<size_t> > Powersystem::getBusLoadMap() const{ return _busLoadMap; }

vector<Bus> const& Powersystem::getBusSet() const{return _busSet;}
vector<Generator> const& Powersystem::getGenSet() const{return _genSet;}
vector<Branch> const& Powersystem::getBrSet() const{return _brSet;}
vector<Load> const& Powersystem::getLoadSet() const{return _loadSet;}

size_t Powersystem::getBusSet_size() const{ return _busSet.size(); }
size_t Powersystem::getBrSet_size() const{ return _brSet.size(); }
size_t Powersystem::getGenSet_size() const{ return _genSet.size(); }
size_t Powersystem::getLoadSet_size() const{ return _loadSet.size(); }

int Powersystem::getBus_extId(size_t intId) const{
  if ( intId >= _busSet.size() ){
    // Element not found!
    return -1;
  } else{
    // Element found; return its external index
    return _busSet[intId].extId;
  }
}
int Powersystem::getBr_extId(size_t intId) const{
  if ( intId >= _brSet.size() ){
    // Element not found!
    return -1;
  } else{
    // Element found; return its external index
    return _brSet[intId].extId;
  }
}
int Powersystem::getGen_extId(size_t intId) const{
  if ( intId >= _genSet.size() ){
    // Element not found!
    return -1;
  } else{
    // Element found; return its external index
    return _genSet[intId].extId();
  }
}
int Powersystem::getLoad_extId(size_t intId) const{
  if ( intId >= _loadSet.size() ){
    // Element not found!
    return -1;
  } else{
    // Element found; return its external index
    return _loadSet[intId].extId;
  }
}

int Powersystem::getBus_intId(unsigned int extId) const{
  if(_busIdBimap.left.find( extId ) == _busIdBimap.left.end()){
    // Element not found!
    return -1;
  } else{
    // Element found; return its internal index
    return _busIdBimap.left.at( extId );
  }
}
int Powersystem::getBr_intId(unsigned int extId) const{
  if(_brIdBimap.left.find( extId ) == _brIdBimap.left.end()){
    // Element not found!
    return -1;
  } else{
    // Element found; return its internal index
    return _brIdBimap.left.at( extId );
  }
}
int Powersystem::getGen_intId(unsigned int extId) const{
  if(_genIdBimap.left.find( extId ) == _genIdBimap.left.end()){
    // Element not found!
    return -1;
  } else{
    // Element found; return its internal index
    return _genIdBimap.left.at( extId );
  }
}
int Powersystem::getLoad_intId(unsigned int extId) const{
  if(_loadIdBimap.left.find( extId ) == _loadIdBimap.left.end()){
    // Element not found!
    return -1;
  } else{
    // Element found; return its internal index
    return _loadIdBimap.left.at( extId );
  }
}

int Powersystem::getBus(unsigned int busExtId, Bus*& pBusReturned ){

  // Check whether busExtId exists in _busSet
  if(_busIdBimap.left.find(busExtId) == _busIdBimap.left.end()){
    // busExtId not found, cannot get a non-existent bus!
    return 1;
  }

  size_t busIntId = _busIdBimap.left.at( busExtId );

  pBusReturned = &(_busSet[busIntId]);

  return 0;
}

int Powersystem::getBus( unsigned int busExtId,
                         Bus const*& pBusReturned ) const{

  // Check whether busExtId exists in _busSet
  if(_busIdBimap.left.find(busExtId) == _busIdBimap.left.end()){
    // busExtId not found, cannot get a non-existent bus!
    return 1;
  }

  size_t busIntId = _busIdBimap.left.at( busExtId );

  pBusReturned = &(_busSet[busIntId]);

  return 0;
}

int Powersystem::getBranch(unsigned int brExtId, Branch*& pBrReturned){

  // Check whether brExtId exists in _brSet
  if( _brIdBimap.left.find(brExtId) == _brIdBimap.left.end()){
    // brExtId not found, cannot get a non-existent branch!
    return 1;
  }

  size_t brIntId = _brIdBimap.left.at( brExtId );

  pBrReturned = &(_brSet[brIntId]);

  return 0;
}

int Powersystem::getBranch(unsigned int brExtId,
                           Branch const*& pBrReturned) const{

  // Check whether brExtId exists in _brSet
  if( _brIdBimap.left.find(brExtId) == _brIdBimap.left.end()){
    // brExtId not found, cannot get a non-existent branch!
    return 1;
  }

  size_t brIntId = _brIdBimap.left.at( brExtId );

  pBrReturned = &(_brSet[brIntId]);

  return 0;
}

int Powersystem::getGenerator(unsigned int genExtId,
                              Generator*& pGenReturned){

  // Check whether genExtId exists in _genSet
  if( _genIdBimap.left.find(genExtId) == _genIdBimap.left.end()){
    // genExtId not found, cannot get a non-existent generator!
    return 1;
  }

  size_t genIntId = _genIdBimap.left.at( genExtId );

  pGenReturned = &(_genSet[genIntId]);

  return 0;
}

int Powersystem::getGenerator( unsigned int genExtId,
                               Generator const*& pGenReturned) const{

  // Check whether genExtId exists in _genSet
  if( _genIdBimap.left.find(genExtId) == _genIdBimap.left.end()){
    // genExtId not found, cannot get a non-existent generator!
    return 1;
  }

  size_t genIntId = _genIdBimap.left.at( genExtId );

  pGenReturned = &(_genSet[genIntId]);

  return 0;
}

int Powersystem::getLoad(unsigned int loadExtId, Load*& pLoadReturned){

  // Check whether loadExtId exists in _loadSet
  if( _loadIdBimap.left.find(loadExtId) == _loadIdBimap.left.end()){
    // loadExtId not found, get modify a non-existent load!
    return 1;
  }

  size_t loadIntId = _loadIdBimap.left.at( loadExtId );

  pLoadReturned = &(_loadSet[loadIntId]);

  return 0;
}

int Powersystem::getLoad(unsigned int loadExtId,
                         Load const*& pLoadReturned) const{

  // Check whether loadExtId exists in _loadSet
  if( _loadIdBimap.left.find(loadExtId) == _loadIdBimap.left.end()){
    // loadExtId not found, get modify a non-existent load!
    return 1;
  }

  size_t loadIntId = _loadIdBimap.left.at( loadExtId );

  pLoadReturned = &(_loadSet[loadIntId]);

  return 0;
}

Bus* Powersystem::getBus(size_t busIntId){
//  return static_cast<Bus&>( _busSet[busIntId] );
  if( busIntId >= _busSet.size() )
    return 0;
  else
    return &_busSet[busIntId];
}

Bus const* Powersystem::getBus(size_t busIntId) const{
//  return static_cast<Bus const&>( _busSet[busIntId] );
  if( busIntId >= _busSet.size() )
    return 0;
  else
    return &_busSet[busIntId];
}

Branch* Powersystem::getBranch(size_t brIntId){
//  return static_cast<Branch&>( _brSet[brIntId] );
  if( brIntId >= _brSet.size() )
    return 0;
  else
    return &_brSet[brIntId];
}

Branch const* Powersystem::getBranch(size_t brIntId) const{
//  return static_cast<Branch const&>( _brSet[brIntId] );
  if( brIntId >= _brSet.size() )
    return 0;
  else
    return &_brSet[brIntId];
}

Generator* Powersystem::getGenerator(size_t genIntId){
//  return static_cast<Generator&>( _genSet[genIntId] );
  if( genIntId >= _genSet.size() )
    return 0;
  else
    return &_genSet[genIntId];
}

Generator const* Powersystem::getGenerator(size_t genIntId) const{
//  return static_cast<Generator const&>( _genSet[genIntId] );
  if( genIntId >= _genSet.size() )
    return 0;
  else
    return &_genSet[genIntId];
}

Load* Powersystem::getLoad(size_t loadIntId){
//  return static_cast<Load&>( _loadSet[loadIntId] );
  if( loadIntId >= _loadSet.size() )
    return 0;
  else
    return &_loadSet[loadIntId];
}

Load const* Powersystem::getLoad(size_t loadIntId) const{
//  return static_cast<Load const&>( _loadSet[loadIntId] );
  if( loadIntId >= _loadSet.size() )
    return 0;
  else
    return &_loadSet[loadIntId];
}

// --- setters ---
void Powersystem::set_name(string const& val){ _name = val; }
void Powersystem::set_description(string const& val){ _description = val; }
int Powersystem::set_baseS(double val){
  _baseS = val;
  return 0;
}
int Powersystem::set_baseF(double val){
  _baseF = val;
  return 0;
}
int Powersystem::set_slackBusExtId(int val){
//  // Argument consistency: val should exist in _busSet of the Powersystem
//  if(_busIdBimap.left.find(val) == _busIdBimap.left.end())
//    return 1;

  _slackBusExtId = val;
  return 0;
}
int Powersystem::set_slackGenExtId(int val){
//  // Argument consistency: val should exist in _busSet of the Powersystem
//  if(_genIdBimap.left.find(val) == _genIdBimap.left.end())
//    return 1;

  _slackGenExtId = val;
  return 0;
}
void Powersystem::set_status(int val){ _status = val; }

/////////////////// PRIVATE MEMBERS ///////////////////

void Powersystem::_rebuildBusIdBimap(){
  // Rebuild _busIdBimap bimap; first: extId - second: intId
  _busIdBimap.clear();
  for(vector<Bus>::iterator i = _busSet.begin(); i != _busSet.end(); ++i){
    _busIdBimap.insert(UintPair(i->extId,_busIdBimap.size()));
  }
}

void Powersystem::_rebuildBrIdBimap(){
  // Rebuild _brIdBimap bimap; first: extId - second: intId
  _brIdBimap.clear();
  for(vector<Branch>::iterator i = _brSet.begin(); i != _brSet.end(); ++i){
    _brIdBimap.insert(UintPair(i->extId,_brIdBimap.size()));
  }
}

void Powersystem::_rebuildGenIdBimap(){
  // Rebuild _genIdBimap bimap; first: extId - second: intId
  _genIdBimap.clear();
  for(vector<Generator>::iterator i = _genSet.begin(); i != _genSet.end(); ++i){
    _genIdBimap.insert(UintPair(i->extId(),_genIdBimap.size()));
  }
}

void Powersystem::_rebuildLoadIdBimap(){
  // Rebuild _loadIdBimap bimap; first: extId - second: intId
  _loadIdBimap.clear();
  for(vector<Load>::iterator i = _loadSet.begin(); i != _loadSet.end(); ++i){
    _loadIdBimap.insert(UintPair(i->extId,_loadIdBimap.size()));
  }
}

void Powersystem::_rebuildBusBrMap(){
  _busBrMap.clear();
  _busBrMap.resize(_busSet.size());

  size_t fromBusIntId, toBusIntId;
  for ( size_t k = 0 ; k != _brSet.size() ; ++k ){
    // Add branch to _busBrMap for fromBus
    fromBusIntId = _busIdBimap.left.at( _brSet[k].fromBusExtId );
    _busBrMap[fromBusIntId].insert(k);

    // Add branch to _busBrMap for toBus
    toBusIntId = _busIdBimap.left.at( _brSet[k].toBusExtId );
    _busBrMap[toBusIntId].insert(k);
  }

  return;
}

void Powersystem::_rebuildBusGenMap(){
  _busGenMap.clear();
  _busGenMap.resize(_busSet.size());

  size_t busIntId;
  for ( size_t k = 0 ; k != _genSet.size() ; ++k ){
    busIntId = _busIdBimap.left.at( _genSet[k].busExtId() );
    _busGenMap[busIntId].insert(k);
  }

  return;
}

void Powersystem::_rebuildBusLoadMap(){
  _busLoadMap.clear();
  _busLoadMap.resize(_busSet.size());

  size_t busIntId;
  for ( size_t k = 0 ; k != _loadSet.size() ; ++k ){
    busIntId = _busIdBimap.left.at( _loadSet[k].busExtId );
    _busLoadMap[busIntId].insert(k);
  }

  return;
}
