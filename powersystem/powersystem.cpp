
#include "powersystem.h"
using namespace elabtsaot;

#include "graph.h"

using std::set;
using std::string;
using std::vector;
#include <sstream>
using std::stringstream;
#include <ctime> // for time(), ctime()
#include <cmath> // for M_PI constant
#define _USE_MATH_DEFINES

typedef UintUintBimap::value_type     UintPair; //!< Auxiliary definition for UintUintBimap
typedef UintUintBimap::const_iterator UintUintBimapConstIt; //!< Auxiliary definition for UintUintBimap

//! Bus type enumeration
enum BusType{
  BUSTYPE_UNDEF, //!< Undefined bus type (possibly only yet)
  BUSTYPE_PQ,    //!< PQ bus type: P and Q injection defined at the bus
  BUSTYPE_PV,    //!< PV bus type: P injection and voltage magnitude defined at the bus
  BUSTYPE_SLACK  //!< Slack bus: Voltage magnitude and angle defined defined at the bus
};

Powersystem::Powersystem(string const& name, double baseS, double baseF) :
    name(name), baseS(baseS), baseF(baseF), _status(PWSSTATUS_INIT) {}

string Powersystem::serialize() const{

  stringstream ss;

  ss << _status << " ";
  ss << "_name:" << name << ":_name ";
  ss << baseS << " ";
  ss << baseF << " ";
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

double Powersystem::getMaxX() const{
  double maxX = -1;
  // Check branch set
  for ( size_t k = 0 ; k != _brSet.size() ; ++k )
    if ( _brSet[k].X > maxX )
      maxX = _brSet[k].X;
  // Check generator set
  for ( size_t k = 0 ; k != _genSet.size(); ++k )
    if ( _genSet[k].xd_1 > maxX )
      maxX = _genSet[k].xd_1;
  return maxX;
}

double Powersystem::getMaxXTimesTap() const{
  double maxXtimesTap = -1;
  // Check branch set
  for ( size_t k = 0 ; k != _brSet.size() ; ++k ){
    Branch const* branch = &_brSet[k];
    double tap = branch->Xratio;
    if (tap==0.) tap=1.; // assuming that (tap==0) => denotes line
    double tempMaxXtimesTap = branch->X*tap;
    if ( tempMaxXtimesTap > maxXtimesTap )
      maxXtimesTap = tempMaxXtimesTap;
  }
  return maxXtimesTap;
}

void Powersystem::clear(){
  _status = PWSSTATUS_INIT;

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
}

void Powersystem::addBus(Bus const& newBus){

  // Insert newBus into the _busSet
  _busSet.push_back(newBus);

  // Update _busIdBimap & _busXXMap's
  _rebuildBusIdBimap();
  _rebuildBusBrMap();
  _rebuildBusGenMap();
  _rebuildBusLoadMap();

  _status = PWSSTATUS_INIT;
}

void Powersystem::addBranch(Branch const& newBranch){

  // Insert newBranch into the _busSet
  _brSet.push_back(newBranch);

  // Update _brIdBimap & _busBrMap
  _rebuildBrIdBimap();
  _rebuildBusBrMap();

  _status = PWSSTATUS_INIT;
}

void Powersystem::addGen(Generator const& newGen){

  // Insert newGen into the _genSet
  _genSet.push_back(newGen);

  // Update _genIdBimap & _busGenMap
  _rebuildGenIdBimap();
  _rebuildBusGenMap();

  _status = PWSSTATUS_INIT;
}

void Powersystem::addLoad(Load const& newLoad){

  // Insert newLoad into the _loadSet
  _loadSet.push_back(newLoad);

  // Update _loadIdBimap & _busLoadMap
  _rebuildLoadIdBimap();
  _rebuildBusLoadMap();

  _status = PWSSTATUS_INIT;
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
      if ( i->busExtId == static_cast<int>(busExtId) ){
        deleteGen(i->extId);
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
      if ( i->busExtId == static_cast<int>(busExtId) )
        return 3;
    for(vector<Load>::iterator i = _loadSet.begin(); i != _loadSet.end(); ++i)
      if ( i->busExtId == static_cast<int>(busExtId) )
        return 4;
  }

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
  if (_genIdBimap.left.find(genExtId) == _genIdBimap.left.end())
    // genExtId not found, cannot delete a non-existent generator!
    return 1;

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

  size_t k; // counters

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
  for ( k = 0 ; k != _genSet.size() ; ++k ){
    // Validate that all gen extIds are unique
    if ( genExtIds.find(_genSet[k].extId) != genExtIds.end() )
      return 6;
    else
      genExtIds.insert( _genSet[k].extId );
    // Validate that all generators point to existing buses
    if ( busExtIds.find( _genSet[k].busExtId ) == busExtIds.end() )
      return 7;
  }
  _rebuildGenIdBimap(); // Rebuild genIdBimap

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
  _rebuildLoadIdBimap(); // Rebuild loadIdBimap

  this->_status = PWSSTATUS_VALID;
  return 0;
}

size_t Powersystem::getBusCount() const{ return _busSet.size(); }
size_t Powersystem::getBranchCount() const{ return _brSet.size(); }
size_t Powersystem::getGenCount() const{ return _genSet.size(); }
size_t Powersystem::getLoadCount() const{ return _loadSet.size(); }
unsigned int Powersystem::getBus_extId(size_t intId) const{ return _busSet[intId].extId; }
unsigned int Powersystem::getBr_extId(size_t intId) const{ return _brSet[intId].extId; }
unsigned int Powersystem::getGen_extId(size_t intId) const{ return _genSet[intId].extId; }
unsigned int Powersystem::getLoad_extId(size_t intId) const{ return _loadSet[intId].extId; }

set<size_t> Powersystem::getBusBrMap(size_t busIntId) const{ return _busBrMap[busIntId]; }
set<size_t> Powersystem::getBusGenMap(size_t busIntId) const{ return _busGenMap[busIntId]; }
set<size_t> Powersystem::getBusLoadMap(size_t busIntId) const{ return _busLoadMap[busIntId]; }


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
  if(_busIdBimap.left.find(busExtId) == _busIdBimap.left.end())
    // busExtId not found, cannot get a non-existent bus!
    return 1;
  size_t busIntId = _busIdBimap.left.at( busExtId );
  pBusReturned = &(_busSet[busIntId]);
  return 0;
}
int Powersystem::getBus( unsigned int busExtId, Bus const*& pBusReturned ) const{
  // Check whether busExtId exists in _busSet
  if(_busIdBimap.left.find(busExtId) == _busIdBimap.left.end())
    // busExtId not found, cannot get a non-existent bus!
    return 1;
  size_t busIntId = _busIdBimap.left.at( busExtId );
  pBusReturned = &(_busSet[busIntId]);
  return 0;
}
Bus* Powersystem::getBus(size_t busIntId){ return &_busSet[busIntId]; }
Bus const* Powersystem::getBus(size_t busIntId) const{ return &_busSet[busIntId]; }

int Powersystem::getBranch(unsigned int brExtId, Branch*& pBrReturned){
  // Check whether brExtId exists in _brSet
  if( _brIdBimap.left.find(brExtId) == _brIdBimap.left.end())
    // brExtId not found, cannot get a non-existent branch!
    return 1;
  size_t brIntId = _brIdBimap.left.at( brExtId );
  pBrReturned = &(_brSet[brIntId]);
  return 0;
}
int Powersystem::getBranch(unsigned int brExtId, Branch const*& pBrReturned) const{
  // Check whether brExtId exists in _brSet
  if( _brIdBimap.left.find(brExtId) == _brIdBimap.left.end())
    // brExtId not found, cannot get a non-existent branch!
    return 1;
  size_t brIntId = _brIdBimap.left.at( brExtId );
  pBrReturned = &(_brSet[brIntId]);
  return 0;
}
Branch* Powersystem::getBranch(size_t brIntId){ return &_brSet[brIntId]; }
Branch const* Powersystem::getBranch(size_t brIntId) const{ return &_brSet[brIntId]; }

int Powersystem::getGenerator(unsigned int genExtId, Generator*& pGenReturned){
  // Check whether genExtId exists in _genSet
  if( _genIdBimap.left.find(genExtId) == _genIdBimap.left.end())
    // genExtId not found, cannot get a non-existent generator!
    return 1;
  size_t genIntId = _genIdBimap.left.at( genExtId );
  pGenReturned = &(_genSet[genIntId]);
  return 0;
}
int Powersystem::getGenerator( unsigned int genExtId, Generator const*& pGenReturned) const{
  // Check whether genExtId exists in _genSet
  if( _genIdBimap.left.find(genExtId) == _genIdBimap.left.end())
    // genExtId not found, cannot get a non-existent generator!
    return 1;
  size_t genIntId = _genIdBimap.left.at( genExtId );
  pGenReturned = &(_genSet[genIntId]);
  return 0;
}
Generator* Powersystem::getGenerator(size_t genIntId){ return &_genSet[genIntId]; }
Generator const* Powersystem::getGenerator(size_t genIntId) const{ return &_genSet[genIntId]; }

int Powersystem::getLoad(unsigned int loadExtId, Load*& pLoadReturned){
  // Check whether loadExtId exists in _loadSet
  if( _loadIdBimap.left.find(loadExtId) == _loadIdBimap.left.end())
    // loadExtId not found, get modify a non-existent load!
    return 1;
  size_t loadIntId = _loadIdBimap.left.at( loadExtId );
  pLoadReturned = &(_loadSet[loadIntId]);
  return 0;
}
int Powersystem::getLoad(unsigned int loadExtId, Load const*& pLoadReturned) const{
  // Check whether loadExtId exists in _loadSet
  if( _loadIdBimap.left.find(loadExtId) == _loadIdBimap.left.end())
    // loadExtId not found, get modify a non-existent load!
    return 1;
  size_t loadIntId = _loadIdBimap.left.at( loadExtId );
  pLoadReturned = &(_loadSet[loadIntId]);
  return 0;
}
Load* Powersystem::getLoad(size_t loadIntId){ return &_loadSet[loadIntId]; }
Load const* Powersystem::getLoad(size_t loadIntId) const{ return &_loadSet[loadIntId]; }

void Powersystem::set_status(int val){ _status = val; }
int Powersystem::status() const{ return _status; }

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
    _genIdBimap.insert(UintPair(i->extId,_genIdBimap.size()));
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
}
void Powersystem::_rebuildBusGenMap(){
  _busGenMap.clear();
  _busGenMap.resize(_busSet.size());
  size_t busIntId;
  for ( size_t k = 0 ; k != _genSet.size() ; ++k ){
    busIntId = _busIdBimap.left.at( _genSet[k].busExtId );
    _busGenMap[busIntId].insert(k);
  }
}
void Powersystem::_rebuildBusLoadMap(){
  _busLoadMap.clear();
  _busLoadMap.resize(_busSet.size());
  size_t busIntId;
  for ( size_t k = 0 ; k != _loadSet.size() ; ++k ){
    busIntId = _busIdBimap.left.at( _loadSet[k].busExtId );
    _busLoadMap[busIntId].insert(k);
  }
}
