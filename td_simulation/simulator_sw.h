/*!
\file simulator_sw.h

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Lilis Georgios, georgios.lilis at epfl dot ch, Elab EPFL
\author Theodoros Kyriakidis, thekyria at gmail dot com, Elab EPFL
*/

#ifndef SIMULATOR_SW_H
#define SIMULATOR_SW_H

#include "tdengine.h"
#include "powersystem.h"
#include "scenario.h"

#include <vector>
#include <complex>
typedef std::complex<double> complex;

#include <boost/numeric/ublas/lu.hpp>
#include <boost/numeric/ublas/matrix.hpp>
namespace ublas = boost::numeric::ublas;

namespace elabtsaot{

class Logger;

class Simulator_sw : public TDEngine {

 public:

  Simulator_sw(Powersystem const* pws, Logger* log);
  int init(Powersystem const* pws);

 private:

  bool do_isEngineCompatible(Scenario const& sce) const;
  int do_simulate(Scenario const& sce, TDResults& res);
  Powersystem const* do_getPws() const;

  // Grid related functions
  void _augmentYForLoads(ublas::matrix<complex,ublas::column_major>& augY,
                         ublas::vector<complex> const& Vbus) const;
  void _augmentYForGenerators(ublas::matrix<complex,ublas::column_major>& augY) const;
  ublas::vector<complex> _calculateGridVoltages (ublas::matrix<complex,ublas::column_major> const& LUaugY,
                                                 ublas::permutation_matrix<size_t> const& PaugY,
                                                 ublas::vector<complex> const& I) const;

  // Generator related functions
  ublas::vector<complex> _calculateNortonCurrents(std::vector<std::vector<double> > const& Xgen) const;
  std::vector<double> _calculateGeneratorPowers(ublas::vector<complex> const& Vbus,
                                                std::vector<std::vector<double> > const& Xgen) const;
  // -- 0p0
  void _init0p0(complex Sgen, complex Vbus_, double ra, double xd_1, double baseF,
                double& delta, double& omega, double& EQDm_1, double& Pel_) const;
  complex _calculateCurrent_0p0(complex Vbus_, double EQDm_1, double delta, double ra, double xd_1) const;
  double _calculateActivePower_0p0(complex Vbus_, complex IQD) const;
  complex _calculateNortonCurrent_0p0(double EQDm_1, double delta, double ra, double xd_1) const;
  void _dynamics_0p0(complex Vbus_, double EQDm_1, double delta, double omega,
                     double Pmech, double M, double D, double ra, double xd_1,
                     double& ddelta, double& domega) const;
  // -- 1p1
  void _init1p1(complex Sgen, complex Vbus_, double ra, double xq, double xd, double xq_1, double xd_1, double baseF,
                double& delta, double& omega, double& Eq_1, double& Ed_1, double& Efd, double& Pel_) const;
  complex _calculateCurrent_1p1(complex Vbus_, double delta, double Eq_1, double Ed_1, double ra, double xd_1, double xq_1) const;
  double _calculateActivePower_1p1(complex Vbus_, double delta, complex Iqd) const;
  complex _calculateNortonCurrent_1p1(double Eq_1, double Ed_1, double delta, double ra, double xq_1) const;
  void _dynamics_1p1(complex Vbus_, double delta, double omega, double Eq_1, double Ed_1, double Efd,
                     double Pmech, double M, double D, double ra, double xd, double xq, double xd_1, double xq_1, double Td0_1, double Tq0_1,
                     double& ddelta, double& domega, double& dEq_1, double& dEd_1) const;

  // Integration related functions
  std::vector<std::vector<double> > _calculateGeneratorDynamics(
                                    std::vector<std::vector<double> > const& Xgen,
                                    std::vector<double> const& Efd,
                                    ublas::vector<complex> const& Vbus) const;
  std::vector<std::vector<double> > _rungeKutta(ublas::matrix<complex,ublas::column_major> const& LUaugY,
                                                ublas::permutation_matrix<size_t> const& PaugY,
                                                std::vector<std::vector<double> > const& Xgen,
                                                std::vector<double> const& Efd,
                                                double stepSize) const;

  void _storeTDResults(std::vector<double> const& time_store,
                       std::vector<std::vector<double> > const& angles_store,
                       std::vector<std::vector<double> > const& speeds_store,
                       std::vector<std::vector<double> > const& eq_tr_store,
                       std::vector<std::vector<double> > const& ed_tr_store,
                       std::vector<std::vector<double> > const& Pel_store,
                       std::vector<std::vector<complex> > const& Vbus_store,
                       std::vector<std::vector<complex> > const& Ibus_store,
                       TDResults& res) const;
  void _parseBusFault(Event& event);
  int _parseBrFault(Event& event);

  Powersystem _pwsLocal;

  // Store of old data before fault for bus
  std::vector<int> _busintid;
  std::vector<double> _oldgsh;
  std::vector<double> _oldbsh;
  // Store of old data before fault for branch
  std::vector<int> _br_frombus_intid;
  std::vector<int> _br_tobus_intid;
  std::vector<double> _br_frombus_oldgsh;
  std::vector<double> _br_frombus_oldbsh;
  std::vector<double> _br_tobus_oldgsh;
  std::vector<double> _br_tobus_oldbsh;
  // Store of old data before fault on location of branch
  std::vector<unsigned int> _br_branchoffault_extid;
  std::vector<unsigned int> _br_faultbus_extid;
};

} // end of namespace elabtsaot

#endif // SIMULATOR_SW_H
