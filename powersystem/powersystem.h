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

#include <set>
#include <string>
#include <vector>

#include <boost/bimap.hpp>
typedef boost::bimap<unsigned int,unsigned int> UintUintBimap; //!< (uint-uint) bimap

namespace elabtsaot{

//! Network status enumeration
enum PowerSystemStatus {
  PWSSTATUS_INIT  = 0,   //!< Powersystem initialized (just created)
  PWSSTATUS_VALID = 1,   //!< Powersystem validated (checked for topological consistency)
  PWSSTATUS_PF    = 2    //!< Power flow solved successfully for the powersystem
};

class Powersystem {

 public:

  Powersystem(std::string const& name, double baseS, double baseF);

  std::string serialize() const;
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

  size_t getBusCount() const;   //!< gets the size of _busSet
  size_t getBranchCount() const;    //!< gets the size of _brSet
  size_t getGenCount() const;   //!< gets the size of _genSet
  size_t getLoadCount() const;  //!< gets the size of _loadSet
  unsigned int getBus_extId(size_t intId) const;
  unsigned int getBr_extId(size_t intId) const;
  unsigned int getGen_extId(size_t intId) const;
  unsigned int getLoad_extId(size_t intId) const;
  int getBus_intId(unsigned int extId) const;
  int getBr_intId(unsigned int extId) const;
  int getGen_intId(unsigned int extId) const;
  int getLoad_intId(unsigned int extId) const;
               int getBus(unsigned int busExtId, Bus*& pBusReturned);
               int getBus(unsigned int busExtId, Bus const*& pBusReturned) const;
              Bus* getBus(size_t busIntId);
        Bus const* getBus(size_t busIntId) const;
               int getBranch(unsigned int brExtId, Branch*& pBrReturned);
               int getBranch(unsigned int brExtId, Branch const*& pBrReturned) const;
           Branch* getBranch(size_t brIntId);
     Branch const* getBranch(size_t brIntId) const;
               int getGenerator(unsigned int genExtId, Generator*& pGenReturned);
               int getGenerator( unsigned int genExtId, Generator const*& pGenReturned) const;
        Generator* getGenerator(size_t genIntId);
  Generator const* getGenerator(size_t genIntId) const;
               int getLoad(unsigned int loadExtId, Load*& pLoadReturned);
               int getLoad(unsigned int loadExtId, Load const*& pLoadReturned) const;
             Load* getLoad(size_t loadIntId);
       Load const* getLoad(size_t loadIntId) const;
  std::set<size_t> getBusBrMap(size_t busIntId) const;
  std::set<size_t> getBusGenMap(size_t busIntId) const;
  std::set<size_t> getBusLoadMap(size_t busIntId) const;

  void set_status(int val);
  int status() const;                    //!< getter for _status
  std::string name;
  double baseS;
  double baseF;

 private:

  void _rebuildBusIdBimap();
  void _rebuildBrIdBimap();
  void _rebuildGenIdBimap();
  void _rebuildLoadIdBimap();
  void _rebuildBusBrMap();
  void _rebuildBusGenMap();
  void _rebuildBusLoadMap();

  int _status;
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
