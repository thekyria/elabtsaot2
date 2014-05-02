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

  \todo change functions XXX...(..., bool real = true)

  \author thekyria
  \date Apr 2013
*/
class Atom{

 public:

  Atom(DAC const* real_voltage_ref,
       DAC const* imag_voltage_ref);
  int reset( bool complete );

  size_t getEmbrCount() const; //!< number of emulator branches physically present on the Atom
  //! Get the minimum OVER the maximum achievable R of potentiometers in the atom (embrs and node)
  double getMinMaxAchievableR() const; // TODO: do that only for "used" resistors
  void calibrate(Atom const& cal_am);



  /*****************************************************************************
  ****** SETTERS ***************************************************************
  *****************************************************************************/

  //////////////////////////////////////////////////////////////////////////////
  ////BRANCH RELATED ///////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  void set_embr_exist(size_t pos, bool val);

  void set_embr_real_sw_sc(size_t pos, bool val); //!< wrapper for _embr_real[pos].set_sw_sc(val)
  void set_embr_real_sw_mid(size_t pos, bool val); //!< wrapper for _embr_real[pos].set_sw_mid(val)
  int set_embr_real_pot_near_rab(size_t pos, double val, bool updateTap ); //!< wrapper for _embr_real[pos].set_pot_near_rab(val, updateTap)
  int set_embr_real_pot_near_rw( size_t pos, double val, bool updateTap ); //!< wrapper for _embr_real[pos].set_pot_near_rw(val, updateTap)
  int set_embr_real_pot_near_r(size_t pos, double val); //!< wrapper for _embr_real[pos].set_pot_near_r(val)
  int set_embr_real_pot_near_tap(size_t pos, unsigned int val); //!< wrapper for _embr_real[pos].set_pot_near_tap(val)
  void set_embr_real_pot_near_sw(size_t pos, bool val); //!< wrapper for _embr_real[pos].set_pot_near_sw(val)
  int set_embr_real_pot_near_swA(size_t pos, bool val, bool updateTap ); //!< wrapper for _embr_real[pos].set_pot_near_swA(val)
  int set_embr_real_pot_far_rab(size_t pos, double val, bool updateTap ); //!< wrapper for _embr_real[pos].set_pot_far_rab(val, updateTap)
  int set_embr_real_pot_far_rw( size_t pos, double val, bool updateTap ); //!< wrapper for _embr_real[pos].set_pot_far_rw(val, updateTap)
  int set_embr_real_pot_far_r(size_t pos, double val); //!< wrapper for _embr_real[pos].set_pot_far_r(val)
  int set_embr_real_pot_far_tap(size_t pos, unsigned int val); //!< wrapper for _embr_real[pos].set_pot_far_tap(val)
  void set_embr_real_pot_far_sw(size_t pos, bool val); //!< wrapper for _embr_real[pos].set_pot_far_sw(val)
  int set_embr_real_pot_far_swA(size_t pos, bool val, bool updateTap ); //!< wrapper for _embr_real[pos].set_pot_far_swA(val)

  void set_embr_imag_sw_sc(size_t pos, bool val); //!< wrapper for _embr_imag[pos].set_sw_sc(val)
  void set_embr_imag_sw_mid(size_t pos, bool val); //!< wrapper for _embr_imag[pos].set_sw_mid(val)
  int set_embr_imag_pot_near_rab(size_t pos, double val, bool updateTap ); //!< wrapper for _embr_imag[pos].set_pot_near_rab(val, updateTap)
  int set_embr_imag_pot_near_rw( size_t pos, double val, bool updateTap ); //!< wrapper for _embr_imag[pos].set_pot_near_rw(val, updateTap)
  int set_embr_imag_pot_near_r(size_t pos, double val); //!< wrapper for _embr_imag[pos].set_pot_near_r(val)
  int set_embr_imag_pot_near_tap(size_t pos, unsigned int val); //!< wrapper for _embr_imag[pos].set_pot_near_tap(val)
  void set_embr_imag_pot_near_sw(size_t pos, bool val); //!< wrapper for _embr_imag[pos].set_pot_near_sw(val)
  int set_embr_imag_pot_near_swA(size_t pos, bool val, bool updateTap ); //!< wrapper for _embr_imag[pos].set_pot_near_swA(val)
  int set_embr_imag_pot_far_rab(size_t pos, double val, bool updateTap ); //!< wrapper for _embr_imag[pos].set_pot_far_rab(val, updateTap)
  int set_embr_imag_pot_far_rw( size_t pos, double val, bool updateTap ); //!< wrapper for _embr_imag[pos].set_pot_far_rw(val, updateTap)
  int set_embr_imag_pot_far_r(size_t pos, double val); //!< wrapper for _embr_imag[pos].set_pot_far_r(val)
  int set_embr_imag_pot_far_tap(size_t pos, unsigned int val); //!< wrapper for _embr_imag[pos].set_pot_far_tap(val)
  void set_embr_imag_pot_far_sw(size_t pos, bool val); //!< wrapper for _embr_imag[pos].set_pot_far_sw(val)
  int set_embr_imag_pot_far_swA(size_t pos, bool val, bool updateTap ); //!< wrapper for _embr_imag[pos].set_pot_far_swA(val)

  // Inline embr setters with bool real selector
  inline void set_embr_sw_sc(size_t pos, bool val, bool real ){
      return real ? set_embr_real_sw_sc(pos,val) : set_embr_imag_sw_sc(pos,val);
  }
  inline void set_embr_sw_mid(size_t pos, bool val, bool real ){
      return real?set_embr_real_sw_mid(pos,val) : set_embr_imag_sw_mid(pos,val);
  }
  inline int set_embr_pot_near_rab(size_t pos, double val, bool updateTap, bool real ){
      return real ? set_embr_real_pot_near_rab( pos, val, updateTap ) :
                    set_embr_imag_pot_near_rab( pos, val, updateTap );
  }
  inline int set_embr_pot_near_rw( size_t pos, double val, bool updateTap, bool real ){
      return real ? set_embr_real_pot_near_rw( pos, val, updateTap ) :
                    set_embr_imag_pot_near_rw( pos, val, updateTap );
  }
  inline int set_embr_pot_near_r(size_t pos, double val, bool real ){
      return real ? set_embr_real_pot_near_r(pos,val) :
                    set_embr_imag_pot_near_r(pos,val);
  }
  inline int set_embr_pot_near_tap( size_t pos, unsigned int val, bool real ){
      return real ? set_embr_real_pot_near_tap(pos,val) :
                    set_embr_imag_pot_near_tap(pos,val);
  }
  inline void set_embr_pot_near_sw(size_t pos, bool val, bool real ){
      return real ? set_embr_real_pot_near_sw(pos,val) :
                    set_embr_imag_pot_near_sw(pos,val);
  }
  inline int set_embr_pot_near_swA(size_t pos, bool val, bool updateTap, bool real ){
      return real ? set_embr_real_pot_near_swA(pos,val,updateTap) :
                    set_embr_imag_pot_near_swA(pos,val,updateTap);
  }

  inline int set_embr_pot_far_rab(size_t pos, double val, bool updateTap, bool real ){
      return real ? set_embr_real_pot_far_rab( pos, val, updateTap ) :
                    set_embr_imag_pot_far_rab( pos, val, updateTap );
  }
  inline int set_embr_pot_far_rw( size_t pos, double val, bool updateTap, bool real ){
      return real ? set_embr_real_pot_far_rw( pos, val, updateTap ) :
                    set_embr_imag_pot_far_rw( pos, val, updateTap );
  }
  inline int set_embr_pot_far_r(size_t pos, double val, bool real ){
      return real ? set_embr_real_pot_far_r(pos,val) :
                    set_embr_imag_pot_far_r(pos,val);
  }
  inline int set_embr_pot_far_tap(size_t pos, unsigned int val, bool real){
      return real ? set_embr_real_pot_far_tap(pos,val) :
                    set_embr_imag_pot_far_tap(pos,val);
  }
  inline void set_embr_pot_far_sw(size_t pos, bool val, bool real ){
      return real ? set_embr_real_pot_far_sw(pos,val) :
                    set_embr_imag_pot_far_sw(pos,val);
  }
  inline int set_embr_pot_far_swA(size_t pos, bool val, bool updateTap, bool real ){
      return real ? set_embr_real_pot_far_swA(pos,val,updateTap) :
                    set_embr_imag_pot_far_swA(pos,val,updateTap);
  }



  /*****************************************************************************
  ****** GETTERS ***************************************************************
  *****************************************************************************/

  //////////////////////////////////////////////////////////////////////////////
  ////BRANCH RELATED ///////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  EmulatorBranch const& embr_real(size_t pos) const; //!< Getter for embr_real[pos]
  EmulatorBranch const& embr_imag(size_t pos) const; //!< Getter for embr_imag[pos]
  // Inline for the above
  inline EmulatorBranch const& embr( size_t pos, bool real ) const{
      return real ? embr_real(pos) : embr_imag(pos);
  }

  std::vector<bool> embr_exist() const;
  bool embr_exist(size_t pos) const;

  bool embr_real_sw_sc(size_t pos) const; //!< wrapper for _embr_real[pos].sw_sc
  bool embr_real_sw_mid(size_t pos) const; //!< wrapper for _embr_real[pos].sw_mid
  double embr_real_pot_near_rab(size_t pos) const; //!< wrapper for _embr_real[pos].pot_near_rab
  double embr_real_pot_near_rw(size_t pos) const; //!< wrapper for _embr_real[pos].pot_near_rw
  double embr_real_pot_near_r(size_t pos) const; //!< wrapper for _embr_real[pos].pot_near_r
  unsigned int embr_real_pot_near_tap(size_t pos) const; //!< wrapper for _embr_real[pos].pot_near_tap
  bool embr_real_pot_near_sw(size_t pos) const; //!< wrapper for _embr_real[pos].pot_near_sw
  bool embr_real_pot_near_swA(size_t pos) const; //!< wrapper for _embr_real[pos].pot_near_swA
  double embr_real_pot_far_rab(size_t pos) const; //!< wrapper for _embr_real[pos].pot_far_rab
  double embr_real_pot_far_rw(size_t pos) const; //!< wrapper for _embr_real[pos].pot_far_rw
  double embr_real_pot_far_r(size_t pos) const; //!< wrapper for _embr_real[pos].pot_far_r
  unsigned int embr_real_pot_far_tap(size_t pos) const; //!< wrapper for _embr_real[pos].pot_far_tap
  bool embr_real_pot_far_sw(size_t pos) const; //!< wrapper for _embr_real[pos].pot_far_sw
  bool embr_real_pot_far_swA(size_t pos) const; //!< wrapper for _embr_real[pos].pot_far_swA

  bool embr_imag_sw_sc(size_t pos) const; //!< wrapper for _embr_real[pos].sw_sc
  bool embr_imag_sw_mid(size_t pos) const; //!< wrapper for _embr_real[pos].sw_mid
  double embr_imag_pot_near_rab(size_t pos) const; //!< wrapper for _embr_real[pos].pot_near_rab
  double embr_imag_pot_near_rw(size_t pos) const; //!< wrapper for _embr_real[pos].pot_near_rw
  double embr_imag_pot_near_r(size_t pos) const; //!< wrapper for _embr_real[pos].pot_near_r
  unsigned int embr_imag_pot_near_tap(size_t pos) const; //!< wrapper for _embr_real[pos].pot_near_tap
  bool embr_imag_pot_near_sw(size_t pos) const; //!< wrapper for _embr_real[pos].pot_near_sw
  bool embr_imag_pot_near_swA(size_t pos) const; //!< wrapper for _embr_real[pos].pot_near_swA
  double embr_imag_pot_far_rab(size_t pos) const; //!< wrapper for _embr_real[pos].pot_far_rab
  double embr_imag_pot_far_rw(size_t pos) const; //!< wrapper for _embr_real[pos].pot_far_rw
  double embr_imag_pot_far_r(size_t pos) const; //!< wrapper for _embr_real[pos].pot_far_r
  unsigned int embr_imag_pot_far_tap(size_t pos) const; //!< wrapper for _embr_real[pos].pot_far_tap
  bool embr_imag_pot_far_sw(size_t pos) const; //!< wrapper for _embr_real[pos].pot_far_sw
  bool embr_imag_pot_far_swA(size_t pos) const; //!< wrapper for _embr_real[pos].pot_far_swA

  // Inline embr getters with bool real selector
  inline bool embr_sw_sc(size_t pos, bool real) const{
      return real ? embr_real_sw_sc(pos) : embr_imag_sw_sc(pos);
  }
  inline bool embr_sw_mid(size_t pos, bool real) const{
      return real ? embr_real_sw_mid(pos) : embr_imag_sw_mid(pos);
  }
  inline double embr_pot_near_rab(size_t pos, bool real) const{
      return real ? embr_real_pot_near_rab(pos) : embr_imag_pot_near_rab(pos);
  }
  inline double embr_pot_near_rw(size_t pos, bool real) const{
      return real ? embr_real_pot_near_rw(pos) : embr_imag_pot_near_rw(pos);
  }
  inline double embr_pot_near_r(size_t pos, bool real) const{
      return real ? embr_real_pot_near_r(pos) : embr_imag_pot_near_r(pos);
  }
  inline unsigned int embr_pot_near_tap(size_t pos, bool real ) const{
      return real ? embr_real_pot_near_tap(pos):embr_imag_pot_near_tap(pos);
  }
  inline bool embr_pot_near_sw(size_t pos, bool real ) const{
      return real ? embr_real_pot_near_sw(pos) : embr_imag_pot_near_sw(pos);
  }
  inline bool embr_pot_near_swA(size_t pos, bool real ) const{
      return real ? embr_real_pot_near_swA(pos) : embr_imag_pot_near_swA(pos);
  }
  inline double embr_pot_far_rab(size_t pos, bool real ) const{
      return real ? embr_real_pot_far_rab(pos) : embr_imag_pot_far_rab(pos);
  }
  inline double embr_pot_far_rw( size_t pos, bool real ) const{
      return real ? embr_real_pot_far_rw(pos) : embr_imag_pot_far_rw(pos);
  }
  inline double embr_pot_far_r(size_t pos, bool real ) const{
      return real ? embr_real_pot_far_r(pos) : embr_imag_pot_far_r(pos);
  }
  inline unsigned int embr_pot_far_tap(size_t pos, bool real ) const{
      return real ? embr_real_pot_far_tap(pos) : embr_imag_pot_far_tap(pos);
  }
  inline bool embr_pot_far_sw(size_t pos, bool real ) const{
      return real ? embr_real_pot_far_sw(pos) : embr_imag_pot_far_sw(pos);
  }
  inline bool embr_pot_far_swA( size_t pos, bool real ) const{
      return real ? embr_real_pot_far_swA(pos) : embr_imag_pot_far_swA(pos);
  }

  Node node;       //!< node of the atom

 private:

  std::vector<bool> _embr_exist; //!< whether branches (real and im) physically exist
  std::vector<EmulatorBranch> _embr_real; //!< Branches of real part of the grid
  std::vector<EmulatorBranch> _embr_imag; //!< Branches of imag part of the grid

  DAC const* _real_voltage_ref;       //!< pointer to the real Vref of the slice
  DAC const* _imag_voltage_ref;       //!< pointer to the imag Vref of the slice
};

} // end of namespace elabtsaot

#endif // ATOM_H
