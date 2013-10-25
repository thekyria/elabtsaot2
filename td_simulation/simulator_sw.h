/*!
\file simulator_sw.h

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef SIMULATOR_SW_H
#define SIMULATOR_SW_H

#include "tdengine.h"
#include "powersystem.h"
#include "scenario.h"

#include <complex>
#include <vector>

#include <boost/numeric/ublas/lu.hpp>
#include <boost/numeric/ublas/matrix.hpp>
namespace ublas = boost::numeric::ublas;

namespace elabtsaot{

class SSEngine;
class Logger;

class Simulator_sw : public TDEngine {

 public:

  Simulator_sw( Powersystem const* pws,
                SSEngine const* const& sse,
                Logger* log );
  virtual ~Simulator_sw(){}
  int init( Powersystem const* pws = NULL );

 private:

  bool do_isEngineCompatible(Scenario const& sce) const;
  int do_simulate( Scenario const& sce, TDResults& res );
  Powersystem const* do_getPws() const;

  int _calculateAugmentedYMatrix( std::vector<std::complex<double> > const& Ubus,
                                  std::vector<std::complex<double> > const& Ubus0,
                                  ublas::matrix<std::complex<double>,ublas::column_major>& augY );
  void _solveNetwork( ublas::matrix<std::complex<double>,ublas::column_major> const& LUaugY,
                      ublas::permutation_matrix<size_t> const& pmatrix,
                      std::vector<std::vector<double> > const& Xgen,
                      std::vector<size_t> const& genBusIntId,
                      std::vector<std::complex<double> >& Ubus );
  void _calculateMachineCurrents( std::vector<std::vector<double> > const& Xgen,
                                  std::vector<std::complex<double> > const& Ubus,
                                  std::vector<size_t> const& genBusIntId,
                                  std::vector<double>& Iq,
                                  std::vector<double>& Id,
                                  std::vector<double>& Pel );
  void _calculateGeneratorDynamics( std::vector<std::vector<double> > const& Xgen,
                                    std::vector<double> const& Pel,
                                    std::vector<double> const& Efd,
                                    std::vector<double> const& Iq,
                                    std::vector<double> const& Id,
                                    std::vector<std::vector<double> >& dXgen);
  int _rungeKutta( ublas::matrix<std::complex<double>,ublas::column_major> const& LUaugY,
                   ublas::permutation_matrix<size_t> const& pmatrix,
                   std::vector<size_t> const& genBusIntId,
                   double stepSize,
                   std::vector<std::vector<double> >& Xgen,
                   std::vector<double>& Pe,
                   std::vector<double>& Efd,
                   std::vector<double>& Iq,
                   std::vector<double>& Id,
                   std::vector<std::complex<double> >& Ubus );
  int _storeTDResults(TDResults& res );

  int _parseBusFault(Event& event);
  int _parseBrFault(Event& event);
  int _parseGenFault(Event& event);
  int _parseLoadFault(Event& event);

  SSEngine const* const& _sse;
  Powersystem _pwsLocal;

  // Results
  std::vector<double> _time;
  std::vector<std::vector<double> > _angles;
  std::vector<std::vector<double> > _speeds;
  std::vector<std::vector<double> > _eq_tr;
  std::vector<std::vector<double> > _ed_tr;
  std::vector<std::vector<double> > _powers;
  std::vector<std::vector<std::complex<double> > > _voltages;
  std::vector<std::vector<std::complex<double> > > _currents;

  // Store of old data before fault for bus
  std::vector<int> _busintid;
  std::vector<double> _oldgsh;
  std::vector<double> _oldbsh;
  // Store of old data before fault for brach
  std::vector<int> _br_frombus_intid;
  std::vector<int> _br_tobus_intid;
  std::vector<double> _br_frombus_oldgsh;
  std::vector<double> _br_frombus_oldbsh;
  std::vector<double> _br_tobus_oldgsh;
  std::vector<double> _br_tobus_oldbsh;
  // Store of old data before fault on location of branch
  std::vector<unsigned int> _br_branchoffault_extid;
  std::vector<unsigned int> _br_faultbus_extid;

  size_t _busCount;
  size_t _brCount;
  size_t _genCount;
  size_t _loadCount;
  std::vector<size_t> _genBusIntId;
  std::vector<size_t> _loadBusIntId;
  std::vector<int> _genModel;

};

} // end of namespace elabtsaot

#endif // SIMULATOR_SW_H
