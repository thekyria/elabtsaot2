/*!
\file branch.h
\brief Definition file for class Branch

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef BRANCH_H
#define BRANCH_H

#include <string>
#include <complex>

namespace elabtsaot{

class Branch {

 public:

  Branch();
  std::string serialize() const;

  // ----- Static parameters -----
  unsigned int extId; //!< External id of the branch
  bool status; //!< Enable status of the branch
  int fromBusExtId; //!< External id of the bus at the from side of the branch
  int toBusExtId; //!< External id of the bus at the to side of the branch
  double R; //!< Resistance of the pi-model of the branch [pu]
  double X; //!< Reactance of the pi-model of the branch [pu]
  double Bfrom; //!< Susceptance to ground at the from end of the pi-model of the branch [pu]
  double Bto; //!< Susceptance to ground at the to end of the pi-model of the branch [pu]
  double Gfrom; //!< Conductance to ground at the from end of the pi-model of the branch [pu]
  double Gto; //!< Conductance to ground at the to end of the pi-model of the branch [pu]
  /*! Ratio of the transformer of the branch
    The transformer is considered at the from end of the branch.
    If Xratio == 0 then the branch is not a transformer */
  double Xratio;
  /*! Phase shift of the transformer of the branch [rad]
    The transformer is considered at the from end of the branch
    If _Xshift > 0 then V_from (voltage at the from end of the branch) leads
    V_to (voltage at the to end of the branch). */
  double Xshift;

};

} // end of namespace elabtsaot

#endif // BRANCH_H
