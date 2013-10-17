/*!
\file generator.h
\brief Definition file for class Generator

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef GENERATOR_H
#define GENERATOR_H

#include <string>
#include <complex>

namespace elabtsaot{

/*!
  Enum value      Order Description (for d- and q-axes)
  --------------------------------------------------------
  GENMODEL_0p0    II    "classical" model; d:constant flux linkages; q:-
  GENMODEL_1p0    III   d:field circuit; q:-
  GENMODEL_1p1    IV    d:field circuit; q:one damper
  GENMODEL_2p1t1  V     d:field circuit and one damper; q:one damper; typ1 as per PSAT
  GENMODEL_2p1t2  V     d:field circuit and one damper; q:one damper; typ2 as per PSAT
  GENMODEL_2p2    VI    d:field circuit and one damper; q:two damper
  GENMODEL_3p2    VII   d:field circuit and two damper; q:two damper
  GENMODEL_3p3    VIII  d:field circuit and two damper; q:three damper
*/
enum GeneratorModel{
  GENMODEL_0p0 = 2,
  GENMODEL_1p0,
  GENMODEL_1p1 = 4,
  GENMODEL_2p1t1,
  GENMODEL_2p1t2,
  GENMODEL_2p2,
  GENMODEL_3p2,
  GENMODEL_3p3
};

/* Static     : concern the power flow
 * Dynamic    : concern the time-domain simulation
 * Variable   : time-variable
 * Parameters : innate properties of the object; they dont change over time
 */
class Generator {

 public:

  Generator();
  std::string serialize() const; //! Serializes the contents of the powersystem branch into an std::string
  void updateVariables();

  // ----- Static parameters -----
  unsigned int extId; //! External id of the generator
  bool status; //! Enable status of the generator [pu]
  std::string name; //! Name of the generator
  int busExtId; //! External id of the bus the generator is connected to

  // ----- Static variables -----
  double Pgen; //! Active power (P) produced by the generator [pu]
  double Qgen; //! Reactive power (Q) produced by the generator [pu]
  std::complex<double> Vss; //! Steady state voltage (complex) at the terminals of the generator [pu]

  // ----- Dynamic parameters -----
  int model; //! model of the generator
  double xl; //! Leakage reactance (Potier) [pu]
  double ra; //! Armature resistance [pu]
  double xd; //! Synchronous reactance - direct axis [pu]
  double xd_1; //! Transient reactance - direct axis [pu]
  double xd_2; //! Sub-transient reactance - direct axis [pu]
  double Td0_1; //! Open-circuit transient time constant - direct axis [s]
  double Td0_2; //! Open-circuit sub-transient time constant - direct axis [s]
  double xq; //! Synchronous reactance - quadrature axis [pu]
  double xq_1; //! Transient reactance - quadrature axis [pu]
  double xq_2; //! Sub-transient reactance - quadrature axis [pu]
  double Tq0_1; //! Open-circuit transient time constant - quadrature axis [s]
  double Tq0_2; //! Open-circuit sub-transient time constant - quadrature axis [s]
  double M; //! Mechanical start time M[s]=2xH, H[MWs/MVA] (stored energy at rated speed in MW x s) / (MVA rating)
  double D; //! Damping coefficient

  // ----- Getters for dynamic variables -----
  double Ess() const;
  double deltass() const;

 private:

  // ----- Dynamic variables -----
  double _Ess; //! Steady state internal voltage magnitude for classical model [pu]
  double _deltass; //! Steady state internal voltage angle; [rad] against reference at the slack bus
};

} // end of namespace elabtsaot

#endif // GENERATOR_H
