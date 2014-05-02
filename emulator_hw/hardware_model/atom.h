/*!
\file atom.h
\brief Definition file for class Atom

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef ATOM_H
#define ATOM_H

#include "node.h"
#include "emulatorbranch.h"
#include "dac.h"

#include <vector>
#include <stdlib.h>

namespace elabtsaot{

/*! Enumeration for the positions of branches around the node of the atom
  EMBRPOS_
    UL   U   UR
       \ | /
     L - N - R
       / | \
    DL   D   DR  */
enum EmbrPosition {
  EMBRPOS_U  = 0,    //!< denotes U  EmulatorBranch of the atom
  EMBRPOS_UR = 1,    //!< denotes UR EmulatorBranch of the atom
  EMBRPOS_R  = 2,    //!< denotes R  EmulatorBranch of the atom
  EMBRPOS_DR = 3,    //!< denotes DR EmulatorBranch of the atom
  EMBRPOS_D  = 4,    //!< denotes D  EmulatorBranch of the atom
  EMBRPOS_DL = 5,    //!< denotes DL EmulatorBranch of the atom
  EMBRPOS_L  = 6,    //!< denotes L  EmulatorBranch of the atom
  EMBRPOS_UL = 7     //!< denotes UL EmulatorBranch of the atom
};

/*! Class representing an atom of the hardware emulator
  An atom is the building blog of the analog part of a hardware emulator slice.
  Atoms consist of nodes -measuerement and injection points to the analog grid-
  surrounded by emulator branches -physically connecting nodes of adjacent atoms
  of the analog grid-.

  Atom nodes are physically connected with nodes of adjacent atoms by the
  emulator branches.

  Atom representation - the node appears as the central dot 'o' and the emulator
  branches as dashes '-, |, /' surrounding the node.
      |  /
      | /
      |/
   ---o---

  The analog grid has a real and an imaginary part.

  \sa Node, EmulatorBranch

  \warning This class is part of the emulator_hw classes that provide a software
  representation of the hardware emulator developed at ELAB. As such, changes in
  hardware are reflected in the software classes. For more information on the
  hardware of the project contact: guillaume dot lanz at epfl dot ch (hw team
  leader)

  \author thekyria
  \date May 2014
*/
class Atom{

 public:

  Atom();
  int reset( bool complete );

  size_t getEmbrCount() const; //!< number of emulator branches physically present on the Atom
  void calibrate(Atom const& cal_am);
  //! Get the minimum OVER the maximum achievable R of potentiometers in the atom (embrs and node)
  double getMinMaxAchievableR() const; // TODO: do that only for "used" resistors

  void set_embr_exist(size_t pos, bool val);
  std::vector<bool> embr_exist() const;
  bool embr_exist(size_t pos) const;

  Node node; //!< node of the atom
  inline EmulatorBranch const& embr(size_t pos, bool real) const{return real ?embr_real[pos]:embr_imag[pos];}
  std::vector<EmulatorBranch> embr_real; //!< Branches of real part of the grid
  std::vector<EmulatorBranch> embr_imag; //!< Branches of imag part of the grid

 private:

  std::vector<bool> _embr_exist; //!< whether branches (real and im) physically exist
};

} // end of namespace elabtsaot

#endif // ATOM_H
