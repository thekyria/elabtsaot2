/*!
\file powersystem.h
\brief Definition file for class Powersystem

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef POWERSYSTEM_H
#define POWERSYSTEM_H

#include "bus.h"
#include "branch.h"
#include "generator.h"
#include "load.h"

#include <iostream>
#include <set>
#include <string>
#include <vector>
#include <complex>

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/bimap.hpp>

//! Definition of an (uint-uint) bimap
typedef boost::bimap<unsigned int,unsigned int> UintUintBimap;

namespace elabtsaot{

//! Network status enumeration
enum PowerSystemStatus {
  PWSSTATUS_INIT  = 0,   //!< Powersystem initialized (just created)
  PWSSTATUS_VALID = 1,   //!< Powersystem validated (checked for topological
                         //!< consistency)
  PWSSTATUS_LF    = 2    //!< Power flow solved successfully for the powersystem
};

class Powersystem {

 public:

  Powersystem( std::string const& name, double baseS, double baseF );

  std::string serialize() const;
  int logPowerFlowResults(std::ostream& ostr = std::cout) const;
  int logPowerFlowResults(std::string const& filename) const;
  double getMaxX() const;

  void clear();
  void addBus(Bus const& newBus);
  void addBranch(Branch const& newBranch);
  void addGen(Generator const& newGen);
  void addLoad(Load const& newLoad);

  int deleteBus(unsigned int busExtId, bool recursive = false);
  int deleteBranch(unsigned int brExtId);
  int deleteGen(unsigned int genExtId);
  int deleteLoad(unsigned int loadExtId);

  int validate();

  int status() const;                    //!< getter for _status
  std::string name() const;              //!< getter for _name
  std::string description() const;       //!< getter for _description
  double baseS() const;                  //!< getter for _baseS
  double baseF() const;                  //!< getter for _baseF
  int slackBusExtId() const;             //!< getter for _slackBusExtId
  int slackGenExtId() const;             //!< getter for _slackGenExtId
  std::vector<std::set<size_t> > getBusBrMap() const;
  std::vector<std::set<size_t> > getBusGenMap() const;
  std::vector<std::set<size_t> > getBusLoadMap() const;

  std::vector<Bus> const& getBusSet() const; //!< gets the Bus set
  std::vector<Generator> const& getGenSet() const; //!< gets the Gen set
  std::vector<Branch> const& getBrSet() const; //!< gets the Branch set
  std::vector<Load> const& getLoadSet()const; //!< gets the Load set

  size_t getBusSet_size() const;   //!< gets the size of _busSet
  size_t getBrSet_size() const;    //!< gets the size of _brSet
  size_t getGenSet_size() const;   //!< gets the size of _genSet
  size_t getLoadSet_size() const;  //!< gets the size of _loadSet

  int getBus_extId(size_t intId) const;
  int getBr_extId(size_t intId) const;
  int getGen_extId(size_t intId) const;
  int getLoad_extId(size_t intId) const;

  int getBus_intId(unsigned int extId) const;
  int getBr_intId(unsigned int extId) const;
  int getGen_intId(unsigned int extId) const;
  int getLoad_intId(unsigned int extId) const;

  int getBus(unsigned int busExtId, Bus*& pBusReturned);
  int getBus(unsigned int busExtId, Bus const*& pBusReturned) const;
  int getBranch(unsigned int brExtId, Branch*& pBrReturned);
  int getBranch(unsigned int brExtId, Branch const*& pBrReturned) const;
  int getGenerator(unsigned int genExtId, Generator*& pGenReturned);
  int getGenerator( unsigned int genExtId, Generator const*& pGenReturned) const;
  int getLoad(unsigned int loadExtId, Load*& pLoadReturned);
  int getLoad(unsigned int loadExtId, Load const*& pLoadReturned) const;

  Bus* getBus(size_t busIntId);
  Bus const* getBus(size_t busIntId) const;
  Branch* getBranch(size_t brIntId);
  Branch const* getBranch(size_t brIntId) const;
  Generator* getGenerator(size_t genIntId);
  Generator const* getGenerator(size_t genIntId) const;
  Load* getLoad(size_t loadIntId);
  Load const* getLoad(size_t loadIntId) const;


  void set_name(std::string const& val);          //!< setter for _name
  void set_description(std::string const& val);   //!< setter for _description

  int set_baseS(double val);               //!< setter for _baseS
  int set_baseF(double val);               //!< setter for _baseF

  int set_slackBusExtId(int val);          //!< setter for _slackBusExtId
  int set_slackGenExtId(int val);          //!< setter for _slackGenExtId

  void set_status(int val);

 private:

  void _rebuildBusIdBimap();
  void _rebuildBrIdBimap();
  void _rebuildGenIdBimap();
  void _rebuildLoadIdBimap();
  void _rebuildBusBrMap();
  void _rebuildBusGenMap();
  void _rebuildBusLoadMap();

  int _status;
  std::string _name;
  std::string _description;

  double _baseS;
  double _baseF;

  int _slackBusExtId;
  int _slackGenExtId;

  std::vector<Bus> _busSet;
  std::vector<Branch> _brSet;
  std::vector<Generator> _genSet;
  std::vector<Load> _loadSet;

  UintUintBimap _busIdBimap;      // bimaps storing extId's <-> intId's
  UintUintBimap _brIdBimap;
  UintUintBimap _genIdBimap;
  UintUintBimap _loadIdBimap;


  std::vector<std::set<size_t> > _busBrMap;
  std::vector<std::set<size_t> > _busGenMap;
  std::vector<std::set<size_t> > _busLoadMap;
};

} // end of namespace elabtsaot

#endif // POWERSYSTEM_H
