
#include "sliceanalog.h"
using namespace elabtsaot;

#include <boost/numeric/ublas/operation.hpp> // for axpy_prod
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/banded.hpp> // for diagonal_matrix

using ublas::vector;
#include <limits>
using std::numeric_limits;
#include <iostream>
using std::cout;
using std::endl;

#define HORIZONTALNUMBEROFATOMS 6 //!< Default horizontal number of atoms
#define VERTICALNUMBEROFATOMS 4   //!< Default vertical number of atoms
#define DEFGOTGAIN 1.25           //!< Default GOT gain level [Volt]
#define DEFGOTOFFSET 2.5          //!< Default GOT offset [Volt]

SliceAnalog::SliceAnalog() :
  _atomSet(VERTICALNUMBEROFATOMS, std::vector<Atom>(HORIZONTALNUMBEROFATOMS,
          Atom(&_real_voltage_ref,&_imag_voltage_ref))),
  _real_voltage_ref( DAC_DEF_TAP, DAC_DEF_RESOLUTION, DAC_DEF_OUTMIN, DAC_DEF_OUTMAX ),
  _imag_voltage_ref( DAC_DEF_TAP, DAC_DEF_RESOLUTION, DAC_DEF_OUTMIN, DAC_DEF_OUTMAX ) {}

int SliceAnalog::reset(bool complete){
  int ans = 0;
  // ----- Initialize atom set -----
  // Add left side connections of the slice
  if ( _atomSet.size() > 1 )
    for ( size_t k = 0; k != _atomSet.size()-1 ; ++k)
      _atomSet[k][0].set_embr_exist( EMBRPOS_L , true );
  // Add down side connections of the slice
  if ( _atomSet[0].size() > 2)
    for ( size_t k = 1; k != _atomSet[0].size()-1 ; ++k)
      _atomSet[0][k].set_embr_exist( EMBRPOS_D , true );
  // Initialize atom members
  for ( size_t k = 0 ; k != _atomSet.size() ; ++k )
    for ( size_t m = 0 ; m != _atomSet[k].size() ; ++m )
      ans |= _atomSet[k][m].reset( complete );

  // ----- Initialize private members -----
  _got_gain = static_cast<double>(DEFGOTGAIN);
  _got_offset = static_cast<double>(DEFGOTOFFSET);
  // Perform value-wise reset to the voltage reference DACs
  // (alternative is tap-wise reset)
  ans |= _real_voltage_ref.reset( false, complete );
  ans |= _imag_voltage_ref.reset( false, complete );
  return ans;
}

void SliceAnalog::size(size_t& ver, size_t& hor) const{
  ver = _atomSet.size();
  if (_atomSet.size()==0) hor = 0;
  else                    hor = _atomSet[0].size();
}

double SliceAnalog::getMinMaxAchievableR() const{
  double minMaxR = numeric_limits<double>::max();
  for ( unsigned k = 0 ; k != _atomSet.size() ; ++k )
    for ( unsigned m = 0 ; m != _atomSet[k].size() ; ++m )
      if ( _atomSet[k][m].getMinMaxAchievableR() < minMaxR )
        minMaxR = _atomSet[k][m].getMinMaxAchievableR();
  return minMaxR;
}
using namespace ublas;

void SliceAnalog::buildG(matrix<double,column_major>& G, bool real) const{
  // directed incidence matrix
  matrix<int,column_major> Cd;
  buildCd(Cd);
  cout << "{DEBUG} Cd: " << Cd << endl;
  // branch conductance vector
  vector<double> Gbr;
  buildGbr(Gbr,real);
  cout << "{DEBUG} Gbr: " << Gbr << endl;
  // branch status vector
  vector<int> brStatus;
  buildBrStatus(brStatus,real);
  cout << "{DEBUG} brStatus: " << brStatus << endl;

  Gbr = element_prod(Gbr,brStatus); // status-updated branch conductance
  diagonal_matrix<double,column_major> diagGbr(Gbr.size(),Gbr.data()); // diagonalized br conductance
  matrix<double,column_major> Gtemp;
  axpy_prod(diagGbr,Cd,Gtemp,true); // Gtemp = diag(Gbr)*Cd
  Cd = trans(Cd); // now Cd hold transpose of Cd
  axpy_prod(Cd,Gtemp,G,true); // G = Cd'*Gtemp = Cd'*diag(Gbr)*Cd
  cout << "{DEBUG} G: " << G << endl;
}
void SliceAnalog::buildCd(matrix<int,column_major>& Cd) const{

  size_t ver, hor;
  this->size(ver,hor);
  Cd.resize(6*              ver*hor,   1 + ver*hor + hor-2 +  ver-2 +   hor +    ver-1);
  //        br no per node  node no   gnd  atoms     bot.ext  left.ext  top.ext  right.ext
  Cd.clear();

  /* ---------------------------------------
  Node numbering
  ------------------------------------------
        32   33    34    35    36    37
        |  /  |  /  |  /  |  /  |  /  |  /
        | /   | /   | /   | /   | /   | /
        |/___ |/___ |/___ |/___ |/___ |/___
        19   20    21    22    23    24
        |  /  |  /  |  /  |  /  |  /  |  /
        | /   | /   | /   | /   | /   | /
  31___ |/___ |/___ |/___ |/___ |/___ |/___40
        13   14    15    16    17    18
        |  /  |  /  |  /  |  /  |  /  |  /
        | /   | /   | /   | /   | /   | /
  30___ |/___ |/___ |/___ |/___ |/___ |/___39
        7     8     9    10    11    12
        |  /  |  /  |  /  |  /  |  /  |  /
        | /   | /   | /   | /   | /   | /
  29___ |/___ |/___ |/___ |/___ |/___ |/___38
        1     2     3     4     5     6
              |     |     |     |
              |     |     |     |
              |     |     |     |
              25    26    27    28
       *
       |
      ---
       -  0  (GND)

  ------------------------------------------
  Branch numbering
  ------------------------------------------
  For node N [0-23], base number for the branches is BN = 6xN and the following
  branches are indexed by the respective offsets

                      |     /
                      |    /
                     +1   +2
                      |  /
                      | /
                      |/
         ---- +5 ---- N ---- +3 ------
                      |\
                      | \
                     +4  +0
                      |   |
                      |  ---
                      |   -

  All branches are considered with the FROM side at node N. */

  // --- Node 1 ---
  Cd((1-1)*6+0, 1) = 1; Cd((1-1)*6+0, 0) = -1;
  Cd((1-1)*6+1, 1) = 1; Cd((1-1)*6+1, 7) = -1;
  Cd((1-1)*6+2, 1) = 1; Cd((1-1)*6+2, 8) = -1;
  Cd((1-1)*6+3, 1) = 1; Cd((1-1)*6+3, 2) = -1;
//  Cd((1-1)*6+4, 1) = 1; Cd((1-1)*6+4, X) = -1;
  Cd((1-1)*6+5, 1) = 1; Cd((1-1)*6+5, 29) = -1;

  // --- Node 2 ---
  Cd((2-1)*6+0, 2) = 1; Cd((2-1)*6+0, 0) = -1;
  Cd((2-1)*6+1, 2) = 1; Cd((2-1)*6+1, 8) = -1;
  Cd((2-1)*6+2, 2) = 1; Cd((2-1)*6+2, 9) = -1;
  Cd((2-1)*6+3, 2) = 1; Cd((2-1)*6+3, 3) = -1;
  Cd((2-1)*6+4, 2) = 1; Cd((2-1)*6+4, 25) = -1;
//  Cd((2-1)*6+5, 2) = 1; Cd((2-1)*6+5, X) = -1;

  // --- Node 3 ---
  Cd((3-1)*6+0, 3) = 1; Cd((3-1)*6+0, 0) = -1;
  Cd((3-1)*6+1, 3) = 1; Cd((3-1)*6+1, 9) = -1;
  Cd((3-1)*6+2, 3) = 1; Cd((3-1)*6+2, 10) = -1;
  Cd((3-1)*6+3, 3) = 1; Cd((3-1)*6+3, 4) = -1;
  Cd((3-1)*6+4, 3) = 1; Cd((3-1)*6+4, 26) = -1;
//  Cd((3-1)*6+5, 3) = 1; Cd((3-1)*6+5, X) = -1;

  // --- Node 4 ---
  Cd((4-1)*6+0, 4) = 1; Cd((4-1)*6+0, 0) = -1;
  Cd((4-1)*6+1, 4) = 1; Cd((4-1)*6+1, 10) = -1;
  Cd((4-1)*6+2, 4) = 1; Cd((4-1)*6+2, 11) = -1;
  Cd((4-1)*6+3, 4) = 1; Cd((4-1)*6+3, 5) = -1;
  Cd((4-1)*6+4, 4) = 1; Cd((4-1)*6+4, 27) = -1;
//  Cd((4-1)*6+5, 4) = 1; Cd((4-1)*6+5, X) = -1;

  // --- Node 5 ---
  Cd((5-1)*6+0, 5) = 1; Cd((5-1)*6+0, 0) = -1;
  Cd((5-1)*6+1, 5) = 1; Cd((5-1)*6+1, 11) = -1;
  Cd((5-1)*6+2, 5) = 1; Cd((5-1)*6+2, 12) = -1;
  Cd((5-1)*6+3, 5) = 1; Cd((5-1)*6+3, 6) = -1;
  Cd((5-1)*6+4, 5) = 1; Cd((5-1)*6+4, 28) = -1;
//  Cd((5-1)*6+5, 5) = 1; Cd((5-1)*6+5, X) = -1;

  // --- Node 6 ---
  Cd((6-1)*6+0, 6) = 1; Cd((6-1)*6+0, 0) = -1;
  Cd((6-1)*6+1, 6) = 1; Cd((6-1)*6+1, 12) = -1;
  Cd((6-1)*6+2, 6) = 1; Cd((6-1)*6+2, 39) = -1;
  Cd((6-1)*6+3, 6) = 1; Cd((6-1)*6+3, 38) = -1;
//  Cd((6-1)*6+4, 6) = 1; Cd((6-1)*6+4, X) = -1;
//  Cd((6-1)*6+5, 6) = 1; Cd((6-1)*6+5, X) = -1;

  // --- Node 7 ---
  Cd((7-1)*6+0, 7) = 1; Cd((7-1)*6+0, 0) = -1;
  Cd((7-1)*6+1, 7) = 1; Cd((7-1)*6+1, 13) = -1;
  Cd((7-1)*6+2, 7) = 1; Cd((7-1)*6+2, 14) = -1;
  Cd((7-1)*6+3, 7) = 1; Cd((7-1)*6+3, 8) = -1;
//  Cd((7-1)*6+4, 7) = 1; Cd((7-1)*6+4, X) = -1;
  Cd((7-1)*6+5, 7) = 1; Cd((7-1)*6+5, 30) = -1;

  // --- Node 8 ---
  Cd((8-1)*6+0, 8) = 1; Cd((8-1)*6+0, 0) = -1;
  Cd((8-1)*6+1, 8) = 1; Cd((8-1)*6+1, 14) = -1;
  Cd((8-1)*6+2, 8) = 1; Cd((8-1)*6+2, 15) = -1;
  Cd((8-1)*6+3, 8) = 1; Cd((8-1)*6+3, 9) = -1;
//  Cd((8-1)*6+4, 8) = 1; Cd((8-1)*6+4, X) = -1;
//  Cd((8-1)*6+5, 8) = 1; Cd((8-1)*6+5, X) = -1;

  // --- Node 9 ---
  Cd((9-1)*6+0, 9) = 1; Cd((9-1)*6+0, 0) = -1;
  Cd((9-1)*6+1, 9) = 1; Cd((9-1)*6+1, 15) = -1;
  Cd((9-1)*6+2, 9) = 1; Cd((9-1)*6+2, 16) = -1;
  Cd((9-1)*6+3, 9) = 1; Cd((9-1)*6+3, 10) = -1;
//  Cd((9-1)*6+4, 9) = 1; Cd((9-1)*6+4, X) = -1;
//  Cd((9-1)*6+5, 9) = 1; Cd((9-1)*6+5, X) = -1;

  // --- Node 10 ---
  Cd((10-1)*6+0, 10) = 1; Cd((10-1)*6+0, 0) = -1;
  Cd((10-1)*6+1, 10) = 1; Cd((10-1)*6+1, 16) = -1;
  Cd((10-1)*6+2, 10) = 1; Cd((10-1)*6+2, 17) = -1;
  Cd((10-1)*6+3, 10) = 1; Cd((10-1)*6+3, 11) = -1;
//  Cd((10-1)*6+4, 10) = 1; Cd((10-1)*6+4, X) = -1;
//  Cd((10-1)*6+5, 10) = 1; Cd((10-1)*6+5, X) = -1;

  // --- Node 11 ---
  Cd((11-1)*6+0, 11) = 1; Cd((11-1)*6+0, 0) = -1;
  Cd((11-1)*6+1, 11) = 1; Cd((11-1)*6+1, 17) = -1;
  Cd((11-1)*6+2, 11) = 1; Cd((11-1)*6+2, 18) = -1;
  Cd((11-1)*6+3, 11) = 1; Cd((11-1)*6+3, 12) = -1;
//  Cd((11-1)*6+4, 11) = 1; Cd((11-1)*6+4, X) = -1;
//  Cd((11-1)*6+5, 11) = 1; Cd((11-1)*6+5, X) = -1;

  // --- Node 12 ---
  Cd((12-1)*6+0, 12) = 1; Cd((12-1)*6+0, 0) = -1;
  Cd((12-1)*6+1, 12) = 1; Cd((12-1)*6+1, 18) = -1;
  Cd((12-1)*6+2, 12) = 1; Cd((12-1)*6+2, 40) = -1;
  Cd((12-1)*6+3, 12) = 1; Cd((12-1)*6+3, 39) = -1;
//  Cd((12-1)*6+4, 12) = 1; Cd((12-1)*6+4, X) = -1;
//  Cd((12-1)*6+5, 12) = 1; Cd((12-1)*6+5, X) = -1;

  // --- Node 13 ---
  Cd((13-1)*6+0, 13) = 1; Cd((13-1)*6+0, 0) = -1;
  Cd((13-1)*6+1, 13) = 1; Cd((13-1)*6+1, 19) = -1;
  Cd((13-1)*6+2, 13) = 1; Cd((13-1)*6+2, 20) = -1;
  Cd((13-1)*6+3, 13) = 1; Cd((13-1)*6+3, 14) = -1;
//  Cd((13-1)*6+4, 13) = 1; Cd((13-1)*6+4, X) = -1;
  Cd((13-1)*6+5, 13) = 1; Cd((13-1)*6+5, 31) = -1;

  // --- Node 14 ---
  Cd((14-1)*6+0, 14) = 1; Cd((14-1)*6+0, 0) = -1;
  Cd((14-1)*6+1, 14) = 1; Cd((14-1)*6+1, 20) = -1;
  Cd((14-1)*6+2, 14) = 1; Cd((14-1)*6+2, 21) = -1;
  Cd((14-1)*6+3, 14) = 1; Cd((14-1)*6+3, 15) = -1;
//  Cd((14-1)*6+4, 14) = 1; Cd((14-1)*6+4, X) = -1;
//  Cd((14-1)*6+5, 14) = 1; Cd((14-1)*6+5, X) = -1;

  // --- Node 15 ---
  Cd((15-1)*6+0, 15) = 1; Cd((15-1)*6+0, 0) = -1;
  Cd((15-1)*6+1, 15) = 1; Cd((15-1)*6+1, 21) = -1;
  Cd((15-1)*6+2, 15) = 1; Cd((15-1)*6+2, 22) = -1;
  Cd((15-1)*6+3, 15) = 1; Cd((15-1)*6+3, 16) = -1;
//  Cd((15-1)*6+4, 15) = 1; Cd((15-1)*6+4, X) = -1;
//  Cd((15-1)*6+5, 15) = 1; Cd((15-1)*6+5, X) = -1;

  // --- Node 16 ---
  Cd((16-1)*6+0, 16) = 1; Cd((16-1)*6+0, 0) = -1;
  Cd((16-1)*6+1, 16) = 1; Cd((16-1)*6+1, 22) = -1;
  Cd((16-1)*6+2, 16) = 1; Cd((16-1)*6+2, 23) = -1;
  Cd((16-1)*6+3, 16) = 1; Cd((16-1)*6+3, 17) = -1;
//  Cd((16-1)*6+4, 16) = 1; Cd((16-1)*6+4, X) = -1;
//  Cd((16-1)*6+5, 16) = 1; Cd((16-1)*6+5, X) = -1;

  // --- Node 17 ---
  Cd((17-1)*6+0, 17) = 1; Cd((17-1)*6+0, 0) = -1;
  Cd((17-1)*6+1, 17) = 1; Cd((17-1)*6+1, 23) = -1;
  Cd((17-1)*6+2, 17) = 1; Cd((17-1)*6+2, 24) = -1;
  Cd((17-1)*6+3, 17) = 1; Cd((17-1)*6+3, 18) = -1;
//  Cd((17-1)*6+4, 17) = 1; Cd((17-1)*6+4, X) = -1;
//  Cd((17-1)*6+5, 17) = 1; Cd((17-1)*6+5, X) = -1;

  // --- Node 18 ---
  Cd((18-1)*6+0, 18) = 1; Cd((18-1)*6+0, 0) = -1;
  Cd((18-1)*6+1, 18) = 1; Cd((18-1)*6+1, 24) = -1;
//  Cd((18-1)*6+2, 18) = 1; Cd((18-1)*6+2, X) = -1;
  Cd((18-1)*6+3, 18) = 1; Cd((18-1)*6+3, 40) = -1;
//  Cd((18-1)*6+4, 18) = 1; Cd((18-1)*6+4, X) = -1;
//  Cd((18-1)*6+5, 18) = 1; Cd((18-1)*6+5, X) = -1;

  // --- Node 19 ---
  Cd((19-1)*6+0, 19) = 1; Cd((19-1)*6+0, 0) = -1;
  Cd((19-1)*6+1, 19) = 1; Cd((19-1)*6+1, 32) = -1;
  Cd((19-1)*6+2, 19) = 1; Cd((19-1)*6+2, 33) = -1;
  Cd((19-1)*6+3, 19) = 1; Cd((19-1)*6+3, 20) = -1;
//  Cd((19-1)*6+4, 19) = 1; Cd((19-1)*6+4, X) = -1;
//  Cd((19-1)*6+5, 19) = 1; Cd((19-1)*6+5, X) = -1;

  // --- Node 20 ---
  Cd((20-1)*6+0, 20) = 1; Cd((20-1)*6+0, 0) = -1;
  Cd((20-1)*6+1, 20) = 1; Cd((20-1)*6+1, 33) = -1;
  Cd((20-1)*6+2, 20) = 1; Cd((20-1)*6+2, 34) = -1;
  Cd((20-1)*6+3, 20) = 1; Cd((20-1)*6+3, 21) = -1;
//  Cd((20-1)*6+4, 20) = 1; Cd((20-1)*6+4, X) = -1;
//  Cd((20-1)*6+5, 20) = 1; Cd((20-1)*6+5, X) = -1;

  // --- Node 21 ---
  Cd((21-1)*6+0, 21) = 1; Cd((21-1)*6+0, 0) = -1;
  Cd((21-1)*6+1, 21) = 1; Cd((21-1)*6+1, 34) = -1;
  Cd((21-1)*6+2, 21) = 1; Cd((21-1)*6+2, 35) = -1;
  Cd((21-1)*6+3, 21) = 1; Cd((21-1)*6+3, 22) = -1;
//  Cd((21-1)*6+4, 21) = 1; Cd((21-1)*6+4, X) = -1;
//  Cd((21-1)*6+5, 21) = 1; Cd((21-1)*6+5, X) = -1;

  // --- Node 22 ---
  Cd((22-1)*6+0, 22) = 1; Cd((22-1)*6+0, 0) = -1;
  Cd((22-1)*6+1, 22) = 1; Cd((22-1)*6+1, 35) = -1;
  Cd((22-1)*6+2, 22) = 1; Cd((22-1)*6+2, 36) = -1;
  Cd((22-1)*6+3, 22) = 1; Cd((22-1)*6+3, 23) = -1;
//  Cd((22-1)*6+4, 22) = 1; Cd((22-1)*6+4, X) = -1;
//  Cd((22-1)*6+5, 22) = 1; Cd((22-1)*6+5, X) = -1;

  // --- Node 23 ---
  Cd((23-1)*6+0, 23) = 1; Cd((23-1)*6+0, 0) = -1;
  Cd((23-1)*6+1, 23) = 1; Cd((23-1)*6+1, 36) = -1;
  Cd((23-1)*6+2, 23) = 1; Cd((23-1)*6+2, 37) = -1;
  Cd((23-1)*6+3, 23) = 1; Cd((23-1)*6+3, 24) = -1;
//  Cd((23-1)*6+4, 23) = 1; Cd((23-1)*6+4, X) = -1;
//  Cd((23-1)*6+5, 23) = 1; Cd((23-1)*6+5, X) = -1;

  // --- Node 24 ---
  Cd((24-1)*6+0, 24) = 1; Cd((24-1)*6+0, 0) = -1;
  Cd((24-1)*6+1, 24) = 1; Cd((24-1)*6+1, 37) = -1;
//  Cd((24-1)*6+2, 24) = 1; Cd((24-1)*6+2, X) = -1;
//  Cd((24-1)*6+3, 24) = 1; Cd((24-1)*6+3, X) = -1;
//  Cd((24-1)*6+4, 24) = 1; Cd((24-1)*6+4, X) = -1;
//  Cd((24-1)*6+5, 24) = 1; Cd((24-1)*6+5, X) = -1;

}

void SliceAnalog::buildGbr(vector<double>& Gbr, bool real) const{
  size_t ver, hor;
  this->size(ver,hor);
  Gbr.resize(6*ver*hor);
  Gbr.clear();

  /* ------------------------------------------
  Branch numbering
  ------------------------------------------
  For node N [0-23], base number for the branches is BN = 6xN and the following
  branches are indexed by the respective offsets

                      |     /
                      |    /
                     +1   +2
                      |  /
                      | /
                      |/
         ---- +5 ---- N ---- +3 ------
                      |\
                      | \
                     +4  +0
                      |   |
                      |  ---
                      |   -

  All branches are considered with the FROM side at node N. */

  Atom const* atom;
  // --- Node 1 ---
  atom = &_atomSet[0][0];
  Gbr((1-1)*6+0) = atom->node_pot_resistance_r(real);
  Gbr((1-1)*6+1) = atom->embr_pot_near_r(EMBRPOS_U,real) + atom->embr_pot_far_r(EMBRPOS_U,real);
  Gbr((1-1)*6+2) = atom->embr_pot_near_r(EMBRPOS_UR,real) + atom->embr_pot_far_r(EMBRPOS_UR,real);
  Gbr((1-1)*6+3) = atom->embr_pot_near_r(EMBRPOS_R,real) + atom->embr_pot_far_r(EMBRPOS_R,real);
//  Gbr((1-1)*6+4) = atom->embr_pot_near_r(EMBRPOS_D,real) + atom->embr_pot_far_r(EMBRPOS_D,real);
  Gbr((1-1)*6+5) = atom->embr_pot_near_r(EMBRPOS_L,real) + atom->embr_pot_far_r(EMBRPOS_L,real);

  // --- Node 2 ---
  atom = &_atomSet[0][1];
  Gbr((2-1)*6+0) = atom->node_pot_resistance_r(real);
  Gbr((2-1)*6+1) = atom->embr_pot_near_r(EMBRPOS_U,real) + atom->embr_pot_far_r(EMBRPOS_U,real);
  Gbr((2-1)*6+2) = atom->embr_pot_near_r(EMBRPOS_UR,real) + atom->embr_pot_far_r(EMBRPOS_UR,real);
  Gbr((2-1)*6+3) = atom->embr_pot_near_r(EMBRPOS_R,real) + atom->embr_pot_far_r(EMBRPOS_R,real);
  Gbr((2-1)*6+4) = atom->embr_pot_near_r(EMBRPOS_D,real) + atom->embr_pot_far_r(EMBRPOS_D,real);
//  Gbr((2-1)*6+5) = atom->embr_pot_near_r(EMBRPOS_L,real) + atom->embr_pot_far_r(EMBRPOS_L,real);

  // --- Node 3 ---
  atom = &_atomSet[0][2];
  Gbr((3-1)*6+0) = atom->node_pot_resistance_r(real);
  Gbr((3-1)*6+1) = atom->embr_pot_near_r(EMBRPOS_U,real) + atom->embr_pot_far_r(EMBRPOS_U,real);
  Gbr((3-1)*6+2) = atom->embr_pot_near_r(EMBRPOS_UR,real) + atom->embr_pot_far_r(EMBRPOS_UR,real);
  Gbr((3-1)*6+3) = atom->embr_pot_near_r(EMBRPOS_R,real) + atom->embr_pot_far_r(EMBRPOS_R,real);
  Gbr((3-1)*6+4) = atom->embr_pot_near_r(EMBRPOS_D,real) + atom->embr_pot_far_r(EMBRPOS_D,real);
//  Gbr((3-1)*6+5) = atom->embr_pot_near_r(EMBRPOS_L,real) + atom->embr_pot_far_r(EMBRPOS_L,real);

  // --- Node 4 ---
  atom = &_atomSet[0][3];
  Gbr((4-1)*6+0) = atom->node_pot_resistance_r(real);
  Gbr((4-1)*6+1) = atom->embr_pot_near_r(EMBRPOS_U,real) + atom->embr_pot_far_r(EMBRPOS_U,real);
  Gbr((4-1)*6+2) = atom->embr_pot_near_r(EMBRPOS_UR,real) + atom->embr_pot_far_r(EMBRPOS_UR,real);
  Gbr((4-1)*6+3) = atom->embr_pot_near_r(EMBRPOS_R,real) + atom->embr_pot_far_r(EMBRPOS_R,real);
  Gbr((4-1)*6+4) = atom->embr_pot_near_r(EMBRPOS_D,real) + atom->embr_pot_far_r(EMBRPOS_D,real);
//  Gbr((4-1)*6+5) = atom->embr_pot_near_r(EMBRPOS_L,real) + atom->embr_pot_far_r(EMBRPOS_L,real);

  // --- Node 5 ---
  atom = &_atomSet[0][4];
  Gbr((5-1)*6+0) = atom->node_pot_resistance_r(real);
  Gbr((5-1)*6+1) = atom->embr_pot_near_r(EMBRPOS_U,real) + atom->embr_pot_far_r(EMBRPOS_U,real);
  Gbr((5-1)*6+2) = atom->embr_pot_near_r(EMBRPOS_UR,real) + atom->embr_pot_far_r(EMBRPOS_UR,real);
  Gbr((5-1)*6+3) = atom->embr_pot_near_r(EMBRPOS_R,real) + atom->embr_pot_far_r(EMBRPOS_R,real);
  Gbr((5-1)*6+4) = atom->embr_pot_near_r(EMBRPOS_D,real) + atom->embr_pot_far_r(EMBRPOS_D,real);
//  Gbr((5-1)*6+5) = atom->embr_pot_near_r(EMBRPOS_L,real) + atom->embr_pot_far_r(EMBRPOS_L,real);

  // --- Node 6 ---
  atom = &_atomSet[0][5];
  Gbr((6-1)*6+0) = atom->node_pot_resistance_r(real);
  Gbr((6-1)*6+1) = atom->embr_pot_near_r(EMBRPOS_U,real) + atom->embr_pot_far_r(EMBRPOS_U,real);
  Gbr((6-1)*6+2) = atom->embr_pot_near_r(EMBRPOS_UR,real) + atom->embr_pot_far_r(EMBRPOS_UR,real);
  Gbr((6-1)*6+3) = atom->embr_pot_near_r(EMBRPOS_R,real) + atom->embr_pot_far_r(EMBRPOS_R,real);
  Gbr((6-1)*6+4) = atom->embr_pot_near_r(EMBRPOS_D,real) + atom->embr_pot_far_r(EMBRPOS_D,real);
//  Gbr((6-1)*6+5) = atom->embr_pot_near_r(EMBRPOS_L,real) + atom->embr_pot_far_r(EMBRPOS_L,real);

  // --------------

  // --- Node 7 ---
  atom = &_atomSet[1][0];
  Gbr((7-1)*6+0) = atom->node_pot_resistance_r(real);
  Gbr((7-1)*6+1) = atom->embr_pot_near_r(EMBRPOS_U,real) + atom->embr_pot_far_r(EMBRPOS_U,real);
  Gbr((7-1)*6+2) = atom->embr_pot_near_r(EMBRPOS_UR,real) + atom->embr_pot_far_r(EMBRPOS_UR,real);
  Gbr((7-1)*6+3) = atom->embr_pot_near_r(EMBRPOS_R,real) + atom->embr_pot_far_r(EMBRPOS_R,real);
//  Gbr((7-1)*6+4) = atom->embr_pot_near_r(EMBRPOS_D,real) + atom->embr_pot_far_r(EMBRPOS_D,real);
  Gbr((7-1)*6+5) = atom->embr_pot_near_r(EMBRPOS_L,real) + atom->embr_pot_far_r(EMBRPOS_L,real);

  // --- Node 8 ---
  atom = &_atomSet[1][1];
  Gbr((8-1)*6+0) = atom->node_pot_resistance_r(real);
  Gbr((8-1)*6+1) = atom->embr_pot_near_r(EMBRPOS_U,real) + atom->embr_pot_far_r(EMBRPOS_U,real);
  Gbr((8-1)*6+2) = atom->embr_pot_near_r(EMBRPOS_UR,real) + atom->embr_pot_far_r(EMBRPOS_UR,real);
  Gbr((8-1)*6+3) = atom->embr_pot_near_r(EMBRPOS_R,real) + atom->embr_pot_far_r(EMBRPOS_R,real);
//  Gbr((8-1)*6+4) = atom->embr_pot_near_r(EMBRPOS_D,real) + atom->embr_pot_far_r(EMBRPOS_D,real);
//  Gbr((8-1)*6+5) = atom->embr_pot_near_r(EMBRPOS_L,real) + atom->embr_pot_far_r(EMBRPOS_L,real);

  // --- Node 9 ---
  atom = &_atomSet[1][2];
  Gbr((9-1)*6+0) = atom->node_pot_resistance_r(real);
  Gbr((9-1)*6+1) = atom->embr_pot_near_r(EMBRPOS_U,real) + atom->embr_pot_far_r(EMBRPOS_U,real);
  Gbr((9-1)*6+2) = atom->embr_pot_near_r(EMBRPOS_UR,real) + atom->embr_pot_far_r(EMBRPOS_UR,real);
  Gbr((9-1)*6+3) = atom->embr_pot_near_r(EMBRPOS_R,real) + atom->embr_pot_far_r(EMBRPOS_R,real);
//  Gbr((9-1)*6+4) = atom->embr_pot_near_r(EMBRPOS_D,real) + atom->embr_pot_far_r(EMBRPOS_D,real);
//  Gbr((9-1)*6+5) = atom->embr_pot_near_r(EMBRPOS_L,real) + atom->embr_pot_far_r(EMBRPOS_L,real);

  // --- Node 10 ---
  atom = &_atomSet[1][3];
  Gbr((10-1)*6+0) = atom->node_pot_resistance_r(real);
  Gbr((10-1)*6+1) = atom->embr_pot_near_r(EMBRPOS_U,real) + atom->embr_pot_far_r(EMBRPOS_U,real);
  Gbr((10-1)*6+2) = atom->embr_pot_near_r(EMBRPOS_UR,real) + atom->embr_pot_far_r(EMBRPOS_UR,real);
  Gbr((10-1)*6+3) = atom->embr_pot_near_r(EMBRPOS_R,real) + atom->embr_pot_far_r(EMBRPOS_R,real);
//  Gbr((10-1)*6+4) = atom->embr_pot_near_r(EMBRPOS_D,real) + atom->embr_pot_far_r(EMBRPOS_D,real);
//  Gbr((10-1)*6+5) = atom->embr_pot_near_r(EMBRPOS_L,real) + atom->embr_pot_far_r(EMBRPOS_L,real);

  // --- Node 11 ---
  atom = &_atomSet[1][4];
  Gbr((11-1)*6+0) = atom->node_pot_resistance_r(real);
  Gbr((11-1)*6+1) = atom->embr_pot_near_r(EMBRPOS_U,real) + atom->embr_pot_far_r(EMBRPOS_U,real);
  Gbr((11-1)*6+2) = atom->embr_pot_near_r(EMBRPOS_UR,real) + atom->embr_pot_far_r(EMBRPOS_UR,real);
  Gbr((11-1)*6+3) = atom->embr_pot_near_r(EMBRPOS_R,real) + atom->embr_pot_far_r(EMBRPOS_R,real);
//  Gbr((11-1)*6+4) = atom->embr_pot_near_r(EMBRPOS_D,real) + atom->embr_pot_far_r(EMBRPOS_D,real);
//  Gbr((11-1)*6+5) = atom->embr_pot_near_r(EMBRPOS_L,real) + atom->embr_pot_far_r(EMBRPOS_L,real);

  // --- Node 12 ---
  atom = &_atomSet[1][5];
  Gbr((12-1)*6+0) = atom->node_pot_resistance_r(real);
  Gbr((12-1)*6+1) = atom->embr_pot_near_r(EMBRPOS_U,real) + atom->embr_pot_far_r(EMBRPOS_U,real);
  Gbr((12-1)*6+2) = atom->embr_pot_near_r(EMBRPOS_UR,real) + atom->embr_pot_far_r(EMBRPOS_UR,real);
  Gbr((12-1)*6+3) = atom->embr_pot_near_r(EMBRPOS_R,real) + atom->embr_pot_far_r(EMBRPOS_R,real);
//  Gbr((12-1)*6+4) = atom->embr_pot_near_r(EMBRPOS_D,real) + atom->embr_pot_far_r(EMBRPOS_D,real);
//  Gbr((12-1)*6+5) = atom->embr_pot_near_r(EMBRPOS_L,real) + atom->embr_pot_far_r(EMBRPOS_L,real);

  // --------------

  // --- Node 13 ---
  atom = &_atomSet[2][0];
  Gbr((13-1)*6+0) = atom->node_pot_resistance_r(real);
  Gbr((13-1)*6+1) = atom->embr_pot_near_r(EMBRPOS_U,real) + atom->embr_pot_far_r(EMBRPOS_U,real);
  Gbr((13-1)*6+2) = atom->embr_pot_near_r(EMBRPOS_UR,real) + atom->embr_pot_far_r(EMBRPOS_UR,real);
  Gbr((13-1)*6+3) = atom->embr_pot_near_r(EMBRPOS_R,real) + atom->embr_pot_far_r(EMBRPOS_R,real);
//  Gbr((13-1)*6+4) = atom->embr_pot_near_r(EMBRPOS_D,real) + atom->embr_pot_far_r(EMBRPOS_D,real);
  Gbr((13-1)*6+5) = atom->embr_pot_near_r(EMBRPOS_L,real) + atom->embr_pot_far_r(EMBRPOS_L,real);

  // --- Node 14 ---
  atom = &_atomSet[2][1];
  Gbr((14-1)*6+0) = atom->node_pot_resistance_r(real);
  Gbr((14-1)*6+1) = atom->embr_pot_near_r(EMBRPOS_U,real) + atom->embr_pot_far_r(EMBRPOS_U,real);
  Gbr((14-1)*6+2) = atom->embr_pot_near_r(EMBRPOS_UR,real) + atom->embr_pot_far_r(EMBRPOS_UR,real);
  Gbr((14-1)*6+3) = atom->embr_pot_near_r(EMBRPOS_R,real) + atom->embr_pot_far_r(EMBRPOS_R,real);
//  Gbr((14-1)*6+4) = atom->embr_pot_near_r(EMBRPOS_D,real) + atom->embr_pot_far_r(EMBRPOS_D,real);
//  Gbr((14-1)*6+5) = atom->embr_pot_near_r(EMBRPOS_L,real) + atom->embr_pot_far_r(EMBRPOS_L,real);

  // --- Node 15 ---
  atom = &_atomSet[2][2];
  Gbr((15-1)*6+0) = atom->node_pot_resistance_r(real);
  Gbr((15-1)*6+1) = atom->embr_pot_near_r(EMBRPOS_U,real) + atom->embr_pot_far_r(EMBRPOS_U,real);
  Gbr((15-1)*6+2) = atom->embr_pot_near_r(EMBRPOS_UR,real) + atom->embr_pot_far_r(EMBRPOS_UR,real);
  Gbr((15-1)*6+3) = atom->embr_pot_near_r(EMBRPOS_R,real) + atom->embr_pot_far_r(EMBRPOS_R,real);
//  Gbr((15-1)*6+4) = atom->embr_pot_near_r(EMBRPOS_D,real) + atom->embr_pot_far_r(EMBRPOS_D,real);
//  Gbr((15-1)*6+5) = atom->embr_pot_near_r(EMBRPOS_L,real) + atom->embr_pot_far_r(EMBRPOS_L,real);

  // --- Node 16 ---
  atom = &_atomSet[2][3];
  Gbr((16-1)*6+0) = atom->node_pot_resistance_r(real);
  Gbr((16-1)*6+1) = atom->embr_pot_near_r(EMBRPOS_U,real) + atom->embr_pot_far_r(EMBRPOS_U,real);
  Gbr((16-1)*6+2) = atom->embr_pot_near_r(EMBRPOS_UR,real) + atom->embr_pot_far_r(EMBRPOS_UR,real);
  Gbr((16-1)*6+3) = atom->embr_pot_near_r(EMBRPOS_R,real) + atom->embr_pot_far_r(EMBRPOS_R,real);
//  Gbr((16-1)*6+4) = atom->embr_pot_near_r(EMBRPOS_D,real) + atom->embr_pot_far_r(EMBRPOS_D,real);
//  Gbr((16-1)*6+5) = atom->embr_pot_near_r(EMBRPOS_L,real) + atom->embr_pot_far_r(EMBRPOS_L,real);

  // --- Node 17 ---
  atom = &_atomSet[2][4];
  Gbr((17-1)*6+0) = atom->node_pot_resistance_r(real);
  Gbr((17-1)*6+1) = atom->embr_pot_near_r(EMBRPOS_U,real) + atom->embr_pot_far_r(EMBRPOS_U,real);
  Gbr((17-1)*6+2) = atom->embr_pot_near_r(EMBRPOS_UR,real) + atom->embr_pot_far_r(EMBRPOS_UR,real);
  Gbr((17-1)*6+3) = atom->embr_pot_near_r(EMBRPOS_R,real) + atom->embr_pot_far_r(EMBRPOS_R,real);
//  Gbr((17-1)*6+4) = atom->embr_pot_near_r(EMBRPOS_D,real) + atom->embr_pot_far_r(EMBRPOS_D,real);
//  Gbr((17-1)*6+5) = atom->embr_pot_near_r(EMBRPOS_L,real) + atom->embr_pot_far_r(EMBRPOS_L,real);

  // --- Node 18 ---
  atom = &_atomSet[2][5];
  Gbr((18-1)*6+0) = atom->node_pot_resistance_r(real);
  Gbr((18-1)*6+1) = atom->embr_pot_near_r(EMBRPOS_U,real) + atom->embr_pot_far_r(EMBRPOS_U,real);
//  Gbr((18-1)*6+2) = atom->embr_pot_near_r(EMBRPOS_UR,real) + atom->embr_pot_far_r(EMBRPOS_UR,real);
  Gbr((18-1)*6+3) = atom->embr_pot_near_r(EMBRPOS_R,real) + atom->embr_pot_far_r(EMBRPOS_R,real);
//  Gbr((18-1)*6+4) = atom->embr_pot_near_r(EMBRPOS_D,real) + atom->embr_pot_far_r(EMBRPOS_D,real);
//  Gbr((18-1)*6+5) = atom->embr_pot_near_r(EMBRPOS_L,real) + atom->embr_pot_far_r(EMBRPOS_L,real);

  // --------------

  // --- Node 19 ---
  atom = &_atomSet[3][0];
  Gbr((19-1)*6+0) = atom->node_pot_resistance_r(real);
  Gbr((19-1)*6+1) = atom->embr_pot_near_r(EMBRPOS_U,real) + atom->embr_pot_far_r(EMBRPOS_U,real);
  Gbr((19-1)*6+2) = atom->embr_pot_near_r(EMBRPOS_UR,real) + atom->embr_pot_far_r(EMBRPOS_UR,real);
  Gbr((19-1)*6+3) = atom->embr_pot_near_r(EMBRPOS_R,real) + atom->embr_pot_far_r(EMBRPOS_R,real);
//  Gbr((19-1)*6+4) = atom->embr_pot_near_r(EMBRPOS_D,real) + atom->embr_pot_far_r(EMBRPOS_D,real);
//  Gbr((19-1)*6+5) = atom->embr_pot_near_r(EMBRPOS_L,real) + atom->embr_pot_far_r(EMBRPOS_L,real);

  // --- Node 20 ---
  atom = &_atomSet[3][1];
  Gbr((20-1)*6+0) = atom->node_pot_resistance_r(real);
  Gbr((20-1)*6+1) = atom->embr_pot_near_r(EMBRPOS_U,real) + atom->embr_pot_far_r(EMBRPOS_U,real);
  Gbr((20-1)*6+2) = atom->embr_pot_near_r(EMBRPOS_UR,real) + atom->embr_pot_far_r(EMBRPOS_UR,real);
  Gbr((20-1)*6+3) = atom->embr_pot_near_r(EMBRPOS_R,real) + atom->embr_pot_far_r(EMBRPOS_R,real);
//  Gbr((20-1)*6+4) = atom->embr_pot_near_r(EMBRPOS_D,real) + atom->embr_pot_far_r(EMBRPOS_D,real);
//  Gbr((20-1)*6+5) = atom->embr_pot_near_r(EMBRPOS_L,real) + atom->embr_pot_far_r(EMBRPOS_L,real);

  // --- Node 21 ---
  atom = &_atomSet[3][2];
  Gbr((21-1)*6+0) = atom->node_pot_resistance_r(real);
  Gbr((21-1)*6+1) = atom->embr_pot_near_r(EMBRPOS_U,real) + atom->embr_pot_far_r(EMBRPOS_U,real);
  Gbr((21-1)*6+2) = atom->embr_pot_near_r(EMBRPOS_UR,real) + atom->embr_pot_far_r(EMBRPOS_UR,real);
  Gbr((21-1)*6+3) = atom->embr_pot_near_r(EMBRPOS_R,real) + atom->embr_pot_far_r(EMBRPOS_R,real);
//  Gbr((21-1)*6+4) = atom->embr_pot_near_r(EMBRPOS_D,real) + atom->embr_pot_far_r(EMBRPOS_D,real);
//  Gbr((21-1)*6+5) = atom->embr_pot_near_r(EMBRPOS_L,real) + atom->embr_pot_far_r(EMBRPOS_L,real);

  // --- Node 22 ---
  atom = &_atomSet[3][3];
  Gbr((22-1)*6+0) = atom->node_pot_resistance_r(real);
  Gbr((22-1)*6+1) = atom->embr_pot_near_r(EMBRPOS_U,real) + atom->embr_pot_far_r(EMBRPOS_U,real);
  Gbr((22-1)*6+2) = atom->embr_pot_near_r(EMBRPOS_UR,real) + atom->embr_pot_far_r(EMBRPOS_UR,real);
  Gbr((22-1)*6+3) = atom->embr_pot_near_r(EMBRPOS_R,real) + atom->embr_pot_far_r(EMBRPOS_R,real);
//  Gbr((22-1)*6+4) = atom->embr_pot_near_r(EMBRPOS_D,real) + atom->embr_pot_far_r(EMBRPOS_D,real);
//  Gbr((22-1)*6+5) = atom->embr_pot_near_r(EMBRPOS_L,real) + atom->embr_pot_far_r(EMBRPOS_L,real);

  // --- Node 23 ---
  atom = &_atomSet[3][4];
  Gbr((23-1)*6+0) = atom->node_pot_resistance_r(real);
  Gbr((23-1)*6+1) = atom->embr_pot_near_r(EMBRPOS_U,real) + atom->embr_pot_far_r(EMBRPOS_U,real);
  Gbr((23-1)*6+2) = atom->embr_pot_near_r(EMBRPOS_UR,real) + atom->embr_pot_far_r(EMBRPOS_UR,real);
  Gbr((23-1)*6+3) = atom->embr_pot_near_r(EMBRPOS_R,real) + atom->embr_pot_far_r(EMBRPOS_R,real);
//  Gbr((23-1)*6+4) = atom->embr_pot_near_r(EMBRPOS_D,real) + atom->embr_pot_far_r(EMBRPOS_D,real);
//  Gbr((23-1)*6+5) = atom->embr_pot_near_r(EMBRPOS_L,real) + atom->embr_pot_far_r(EMBRPOS_L,real);

  // --- Node 24 ---
  atom = &_atomSet[3][5];
  Gbr((24-1)*6+0) = atom->node_pot_resistance_r(real);
  Gbr((24-1)*6+1) = atom->embr_pot_near_r(EMBRPOS_U,real) + atom->embr_pot_far_r(EMBRPOS_U,real);
//  Gbr((24-1)*6+2) = atom->embr_pot_near_r(EMBRPOS_UR,real) + atom->embr_pot_far_r(EMBRPOS_UR,real);
//  Gbr((24-1)*6+3) = atom->embr_pot_near_r(EMBRPOS_R,real) + atom->embr_pot_far_r(EMBRPOS_R,real);
//  Gbr((24-1)*6+4) = atom->embr_pot_near_r(EMBRPOS_D,real) + atom->embr_pot_far_r(EMBRPOS_D,real);
//  Gbr((24-1)*6+5) = atom->embr_pot_near_r(EMBRPOS_L,real) + atom->embr_pot_far_r(EMBRPOS_L,real);
}

void SliceAnalog::buildBrStatus(vector<int>& brStatus, bool real) const{
  size_t ver, hor;
  this->size(ver,hor);
  brStatus.resize(6*ver*hor);
  brStatus.clear();

  /* ------------------------------------------
  Branch numbering
  ------------------------------------------
  For node N [0-23], base number for the branches is BN = 6xN and the following
  branches are indexed by the respective offsets

                      |     /
                      |    /
                     +1   +2
                      |  /
                      | /
                      |/
         ---- +5 ---- N ---- +3 ------
                      |\
                      | \
                     +4  +0
                      |   |
                      |  ---
                      |   -

  All branches are considered with the FROM side at node N. */

  Atom const* atom;
  // --- Node 1 ---
  atom = &_atomSet[0][0];
  brStatus((1-1)*6+0) = atom->node_pot_resistance_sw(real)&atom->node_sw_resistance(real)?1:0;
  brStatus((1-1)*6+1) = atom->embr_pot_near_sw(EMBRPOS_U,real)|atom->embr_pot_far_sw(EMBRPOS_U,real)?1:0;
  brStatus((1-1)*6+2) = atom->embr_pot_near_sw(EMBRPOS_UR,real)|atom->embr_pot_far_sw(EMBRPOS_UR,real)?1:0;
  brStatus((1-1)*6+3) = atom->embr_pot_near_sw(EMBRPOS_R,real)|atom->embr_pot_far_sw(EMBRPOS_R,real)?1:0;
//  brStatus((1-1)*6+4) = atom->embr_pot_near_sw(EMBRPOS_D,real)|atom->embr_pot_far_sw(EMBRPOS_D,real)?1:0;
  brStatus((1-1)*6+5) = atom->embr_pot_near_sw(EMBRPOS_L,real)|atom->embr_pot_far_sw(EMBRPOS_L,real)?1:0;

  // --- Node 2 ---
  atom = &_atomSet[0][1];
  brStatus((2-1)*6+0) = atom->node_pot_resistance_sw(real)&atom->node_sw_resistance(real)?1:0;
  brStatus((2-1)*6+1) = atom->embr_pot_near_sw(EMBRPOS_U,real)|atom->embr_pot_far_sw(EMBRPOS_U,real)?1:0;
  brStatus((2-1)*6+2) = atom->embr_pot_near_sw(EMBRPOS_UR,real)|atom->embr_pot_far_sw(EMBRPOS_UR,real)?1:0;
  brStatus((2-1)*6+3) = atom->embr_pot_near_sw(EMBRPOS_R,real)|atom->embr_pot_far_sw(EMBRPOS_R,real)?1:0;
  brStatus((2-1)*6+4) = atom->embr_pot_near_sw(EMBRPOS_D,real)|atom->embr_pot_far_sw(EMBRPOS_D,real)?1:0;
//  brStatus((2-1)*6+5) = atom->embr_pot_near_sw(EMBRPOS_L,real)|atom->embr_pot_far_sw(EMBRPOS_L,real)?1:0;

  // --- Node 3 ---
  atom = &_atomSet[0][2];
  brStatus((3-1)*6+0) = atom->node_pot_resistance_sw(real)&atom->node_sw_resistance(real)?1:0;
  brStatus((3-1)*6+1) = atom->embr_pot_near_sw(EMBRPOS_U,real)|atom->embr_pot_far_sw(EMBRPOS_U,real)?1:0;
  brStatus((3-1)*6+2) = atom->embr_pot_near_sw(EMBRPOS_UR,real)|atom->embr_pot_far_sw(EMBRPOS_UR,real)?1:0;
  brStatus((3-1)*6+3) = atom->embr_pot_near_sw(EMBRPOS_R,real)|atom->embr_pot_far_sw(EMBRPOS_R,real)?1:0;
  brStatus((3-1)*6+4) = atom->embr_pot_near_sw(EMBRPOS_D,real)|atom->embr_pot_far_sw(EMBRPOS_D,real)?1:0;
//  brStatus((3-1)*6+5) = atom->embr_pot_near_sw(EMBRPOS_L,real)|atom->embr_pot_far_sw(EMBRPOS_L,real)?1:0;

  // --- Node 4 ---
  atom = &_atomSet[0][3];
  brStatus((4-1)*6+0) = atom->node_pot_resistance_sw(real)&atom->node_sw_resistance(real)?1:0;
  brStatus((4-1)*6+1) = atom->embr_pot_near_sw(EMBRPOS_U,real)|atom->embr_pot_far_sw(EMBRPOS_U,real)?1:0;
  brStatus((4-1)*6+2) = atom->embr_pot_near_sw(EMBRPOS_UR,real)|atom->embr_pot_far_sw(EMBRPOS_UR,real)?1:0;
  brStatus((4-1)*6+3) = atom->embr_pot_near_sw(EMBRPOS_R,real)|atom->embr_pot_far_sw(EMBRPOS_R,real)?1:0;
  brStatus((4-1)*6+4) = atom->embr_pot_near_sw(EMBRPOS_D,real)|atom->embr_pot_far_sw(EMBRPOS_D,real)?1:0;
//  brStatus((4-1)*6+5) = atom->embr_pot_near_sw(EMBRPOS_L,real)|atom->embr_pot_far_sw(EMBRPOS_L,real)?1:0;

  // --- Node 5 ---
  atom = &_atomSet[0][4];
  brStatus((5-1)*6+0) = atom->node_pot_resistance_sw(real)&atom->node_sw_resistance(real)?1:0;
  brStatus((5-1)*6+1) = atom->embr_pot_near_sw(EMBRPOS_U,real)|atom->embr_pot_far_sw(EMBRPOS_U,real)?1:0;
  brStatus((5-1)*6+2) = atom->embr_pot_near_sw(EMBRPOS_UR,real)|atom->embr_pot_far_sw(EMBRPOS_UR,real)?1:0;
  brStatus((5-1)*6+3) = atom->embr_pot_near_sw(EMBRPOS_R,real)|atom->embr_pot_far_sw(EMBRPOS_R,real)?1:0;
  brStatus((5-1)*6+4) = atom->embr_pot_near_sw(EMBRPOS_D,real)|atom->embr_pot_far_sw(EMBRPOS_D,real)?1:0;
//  brStatus((5-1)*6+5) = atom->embr_pot_near_sw(EMBRPOS_L,real)|atom->embr_pot_far_sw(EMBRPOS_L,real)?1:0;

  // --- Node 6 ---
  atom = &_atomSet[0][5];
  brStatus((6-1)*6+0) = atom->node_pot_resistance_sw(real)&atom->node_sw_resistance(real)?1:0;
  brStatus((6-1)*6+1) = atom->embr_pot_near_sw(EMBRPOS_U,real)|atom->embr_pot_far_sw(EMBRPOS_U,real)?1:0;
  brStatus((6-1)*6+2) = atom->embr_pot_near_sw(EMBRPOS_UR,real)|atom->embr_pot_far_sw(EMBRPOS_UR,real)?1:0;
  brStatus((6-1)*6+3) = atom->embr_pot_near_sw(EMBRPOS_R,real)|atom->embr_pot_far_sw(EMBRPOS_R,real)?1:0;
//  brStatus((6-1)*6+4) = atom->embr_pot_near_sw(EMBRPOS_D,real)|atom->embr_pot_far_sw(EMBRPOS_D,real)?1:0;
//  brStatus((6-1)*6+5) = atom->embr_pot_near_sw(EMBRPOS_L,real)|atom->embr_pot_far_sw(EMBRPOS_L,real)?1:0;

  // --------------

  // --- Node 7 ---
  atom = &_atomSet[1][0];
  brStatus((7-1)*6+0) = atom->node_pot_resistance_sw(real)&atom->node_sw_resistance(real)?1:0;
  brStatus((7-1)*6+1) = atom->embr_pot_near_sw(EMBRPOS_U,real)|atom->embr_pot_far_sw(EMBRPOS_U,real)?1:0;
  brStatus((7-1)*6+2) = atom->embr_pot_near_sw(EMBRPOS_UR,real)|atom->embr_pot_far_sw(EMBRPOS_UR,real)?1:0;
  brStatus((7-1)*6+3) = atom->embr_pot_near_sw(EMBRPOS_R,real)|atom->embr_pot_far_sw(EMBRPOS_R,real)?1:0;
//  brStatus((7-1)*6+4) = atom->embr_pot_near_sw(EMBRPOS_D,real)|atom->embr_pot_far_sw(EMBRPOS_D,real)?1:0;
  brStatus((7-1)*6+5) = atom->embr_pot_near_sw(EMBRPOS_L,real)|atom->embr_pot_far_sw(EMBRPOS_L,real)?1:0;

  // --- Node 8 ---
  atom = &_atomSet[1][1];
  brStatus((8-1)*6+0) = atom->node_pot_resistance_sw(real)&atom->node_sw_resistance(real)?1:0;
  brStatus((8-1)*6+1) = atom->embr_pot_near_sw(EMBRPOS_U,real)|atom->embr_pot_far_sw(EMBRPOS_U,real)?1:0;
  brStatus((8-1)*6+2) = atom->embr_pot_near_sw(EMBRPOS_UR,real)|atom->embr_pot_far_sw(EMBRPOS_UR,real)?1:0;
  brStatus((8-1)*6+3) = atom->embr_pot_near_sw(EMBRPOS_R,real)|atom->embr_pot_far_sw(EMBRPOS_R,real)?1:0;
//  brStatus((8-1)*6+4) = atom->embr_pot_near_sw(EMBRPOS_D,real)|atom->embr_pot_far_sw(EMBRPOS_D,real)?1:0;
//  brStatus((8-1)*6+5) = atom->embr_pot_near_sw(EMBRPOS_L,real)|atom->embr_pot_far_sw(EMBRPOS_L,real)?1:0;

  // --- Node 9 ---
  atom = &_atomSet[1][2];
  brStatus((9-1)*6+0) = atom->node_pot_resistance_sw(real)&atom->node_sw_resistance(real)?1:0;
  brStatus((9-1)*6+1) = atom->embr_pot_near_sw(EMBRPOS_U,real)|atom->embr_pot_far_sw(EMBRPOS_U,real)?1:0;
  brStatus((9-1)*6+2) = atom->embr_pot_near_sw(EMBRPOS_UR,real)|atom->embr_pot_far_sw(EMBRPOS_UR,real)?1:0;
  brStatus((9-1)*6+3) = atom->embr_pot_near_sw(EMBRPOS_R,real)|atom->embr_pot_far_sw(EMBRPOS_R,real)?1:0;
//  brStatus((9-1)*6+4) = atom->embr_pot_near_sw(EMBRPOS_D,real)|atom->embr_pot_far_sw(EMBRPOS_D,real)?1:0;
//  brStatus((9-1)*6+5) = atom->embr_pot_near_sw(EMBRPOS_L,real)|atom->embr_pot_far_sw(EMBRPOS_L,real)?1:0;

  // --- Node 10 ---
  atom = &_atomSet[1][3];
  brStatus((10-1)*6+0) = atom->node_pot_resistance_sw(real)&atom->node_sw_resistance(real)?1:0;
  brStatus((10-1)*6+1) = atom->embr_pot_near_sw(EMBRPOS_U,real)|atom->embr_pot_far_sw(EMBRPOS_U,real)?1:0;
  brStatus((10-1)*6+2) = atom->embr_pot_near_sw(EMBRPOS_UR,real)|atom->embr_pot_far_sw(EMBRPOS_UR,real)?1:0;
  brStatus((10-1)*6+3) = atom->embr_pot_near_sw(EMBRPOS_R,real)|atom->embr_pot_far_sw(EMBRPOS_R,real)?1:0;
//  brStatus((10-1)*6+4) = atom->embr_pot_near_sw(EMBRPOS_D,real)|atom->embr_pot_far_sw(EMBRPOS_D,real)?1:0;
//  brStatus((10-1)*6+5) = atom->embr_pot_near_sw(EMBRPOS_L,real)|atom->embr_pot_far_sw(EMBRPOS_L,real)?1:0;

  // --- Node 11 ---
  atom = &_atomSet[1][4];
  brStatus((11-1)*6+0) = atom->node_pot_resistance_sw(real)&atom->node_sw_resistance(real)?1:0;
  brStatus((11-1)*6+1) = atom->embr_pot_near_sw(EMBRPOS_U,real)|atom->embr_pot_far_sw(EMBRPOS_U,real)?1:0;
  brStatus((11-1)*6+2) = atom->embr_pot_near_sw(EMBRPOS_UR,real)|atom->embr_pot_far_sw(EMBRPOS_UR,real)?1:0;
  brStatus((11-1)*6+3) = atom->embr_pot_near_sw(EMBRPOS_R,real)|atom->embr_pot_far_sw(EMBRPOS_R,real)?1:0;
//  brStatus((11-1)*6+4) = atom->embr_pot_near_sw(EMBRPOS_D,real)|atom->embr_pot_far_sw(EMBRPOS_D,real)?1:0;
//  brStatus((11-1)*6+5) = atom->embr_pot_near_sw(EMBRPOS_L,real)|atom->embr_pot_far_sw(EMBRPOS_L,real)?1:0;

  // --- Node 12 ---
  atom = &_atomSet[1][5];
  brStatus((12-1)*6+0) = atom->node_pot_resistance_sw(real)&atom->node_sw_resistance(real)?1:0;
  brStatus((12-1)*6+1) = atom->embr_pot_near_sw(EMBRPOS_U,real)|atom->embr_pot_far_sw(EMBRPOS_U,real)?1:0;
  brStatus((12-1)*6+2) = atom->embr_pot_near_sw(EMBRPOS_UR,real)|atom->embr_pot_far_sw(EMBRPOS_UR,real)?1:0;
  brStatus((12-1)*6+3) = atom->embr_pot_near_sw(EMBRPOS_R,real)|atom->embr_pot_far_sw(EMBRPOS_R,real)?1:0;
//  brStatus((12-1)*6+4) = atom->embr_pot_near_sw(EMBRPOS_D,real)|atom->embr_pot_far_sw(EMBRPOS_D,real)?1:0;
//  brStatus((12-1)*6+5) = atom->embr_pot_near_sw(EMBRPOS_L,real)|atom->embr_pot_far_sw(EMBRPOS_L,real)?1:0;

  // --------------

  // --- Node 13 ---
  atom = &_atomSet[2][0];
  brStatus((13-1)*6+0) = atom->node_pot_resistance_sw(real)&atom->node_sw_resistance(real)?1:0;
  brStatus((13-1)*6+1) = atom->embr_pot_near_sw(EMBRPOS_U,real)|atom->embr_pot_far_sw(EMBRPOS_U,real)?1:0;
  brStatus((13-1)*6+2) = atom->embr_pot_near_sw(EMBRPOS_UR,real)|atom->embr_pot_far_sw(EMBRPOS_UR,real)?1:0;
  brStatus((13-1)*6+3) = atom->embr_pot_near_sw(EMBRPOS_R,real)|atom->embr_pot_far_sw(EMBRPOS_R,real)?1:0;
//  brStatus((13-1)*6+4) = atom->embr_pot_near_sw(EMBRPOS_D,real)|atom->embr_pot_far_sw(EMBRPOS_D,real)?1:0;
  brStatus((13-1)*6+5) = atom->embr_pot_near_sw(EMBRPOS_L,real)|atom->embr_pot_far_sw(EMBRPOS_L,real)?1:0;

  // --- Node 14 ---
  atom = &_atomSet[2][1];
  brStatus((14-1)*6+0) = atom->node_pot_resistance_sw(real)&atom->node_sw_resistance(real)?1:0;
  brStatus((14-1)*6+1) = atom->embr_pot_near_sw(EMBRPOS_U,real)|atom->embr_pot_far_sw(EMBRPOS_U,real)?1:0;
  brStatus((14-1)*6+2) = atom->embr_pot_near_sw(EMBRPOS_UR,real)|atom->embr_pot_far_sw(EMBRPOS_UR,real)?1:0;
  brStatus((14-1)*6+3) = atom->embr_pot_near_sw(EMBRPOS_R,real)|atom->embr_pot_far_sw(EMBRPOS_R,real)?1:0;
//  brStatus((14-1)*6+4) = atom->embr_pot_near_sw(EMBRPOS_D,real)|atom->embr_pot_far_sw(EMBRPOS_D,real)?1:0;
//  brStatus((14-1)*6+5) = atom->embr_pot_near_sw(EMBRPOS_L,real)|atom->embr_pot_far_sw(EMBRPOS_L,real)?1:0;

  // --- Node 15 ---
  atom = &_atomSet[2][2];
  brStatus((15-1)*6+0) = atom->node_pot_resistance_sw(real)&atom->node_sw_resistance(real)?1:0;
  brStatus((15-1)*6+1) = atom->embr_pot_near_sw(EMBRPOS_U,real)|atom->embr_pot_far_sw(EMBRPOS_U,real)?1:0;
  brStatus((15-1)*6+2) = atom->embr_pot_near_sw(EMBRPOS_UR,real)|atom->embr_pot_far_sw(EMBRPOS_UR,real)?1:0;
  brStatus((15-1)*6+3) = atom->embr_pot_near_sw(EMBRPOS_R,real)|atom->embr_pot_far_sw(EMBRPOS_R,real)?1:0;
//  brStatus((15-1)*6+4) = atom->embr_pot_near_sw(EMBRPOS_D,real)|atom->embr_pot_far_sw(EMBRPOS_D,real)?1:0;
//  brStatus((15-1)*6+5) = atom->embr_pot_near_sw(EMBRPOS_L,real)|atom->embr_pot_far_sw(EMBRPOS_L,real)?1:0;

  // --- Node 16 ---
  atom = &_atomSet[2][3];
  brStatus((16-1)*6+0) = atom->node_pot_resistance_sw(real)&atom->node_sw_resistance(real)?1:0;
  brStatus((16-1)*6+1) = atom->embr_pot_near_sw(EMBRPOS_U,real)|atom->embr_pot_far_sw(EMBRPOS_U,real)?1:0;
  brStatus((16-1)*6+2) = atom->embr_pot_near_sw(EMBRPOS_UR,real)|atom->embr_pot_far_sw(EMBRPOS_UR,real)?1:0;
  brStatus((16-1)*6+3) = atom->embr_pot_near_sw(EMBRPOS_R,real)|atom->embr_pot_far_sw(EMBRPOS_R,real)?1:0;
//  brStatus((16-1)*6+4) = atom->embr_pot_near_sw(EMBRPOS_D,real)|atom->embr_pot_far_sw(EMBRPOS_D,real)?1:0;
//  brStatus((16-1)*6+5) = atom->embr_pot_near_sw(EMBRPOS_L,real)|atom->embr_pot_far_sw(EMBRPOS_L,real)?1:0;

  // --- Node 17 ---
  atom = &_atomSet[2][4];
  brStatus((17-1)*6+0) = atom->node_pot_resistance_sw(real)&atom->node_sw_resistance(real)?1:0;
  brStatus((17-1)*6+1) = atom->embr_pot_near_sw(EMBRPOS_U,real)|atom->embr_pot_far_sw(EMBRPOS_U,real)?1:0;
  brStatus((17-1)*6+2) = atom->embr_pot_near_sw(EMBRPOS_UR,real)|atom->embr_pot_far_sw(EMBRPOS_UR,real)?1:0;
  brStatus((17-1)*6+3) = atom->embr_pot_near_sw(EMBRPOS_R,real)|atom->embr_pot_far_sw(EMBRPOS_R,real)?1:0;
//  brStatus((17-1)*6+4) = atom->embr_pot_near_sw(EMBRPOS_D,real)|atom->embr_pot_far_sw(EMBRPOS_D,real)?1:0;
//  brStatus((17-1)*6+5) = atom->embr_pot_near_sw(EMBRPOS_L,real)|atom->embr_pot_far_sw(EMBRPOS_L,real)?1:0;

  // --- Node 18 ---
  atom = &_atomSet[2][5];
  brStatus((18-1)*6+0) = atom->node_pot_resistance_sw(real)&atom->node_sw_resistance(real)?1:0;
  brStatus((18-1)*6+1) = atom->embr_pot_near_sw(EMBRPOS_U,real)|atom->embr_pot_far_sw(EMBRPOS_U,real)?1:0;
//  brStatus((18-1)*6+2) = atom->embr_pot_near_sw(EMBRPOS_UR,real)|atom->embr_pot_far_sw(EMBRPOS_UR,real)?1:0;
  brStatus((18-1)*6+3) = atom->embr_pot_near_sw(EMBRPOS_R,real)|atom->embr_pot_far_sw(EMBRPOS_R,real)?1:0;
//  brStatus((18-1)*6+4) = atom->embr_pot_near_sw(EMBRPOS_D,real)|atom->embr_pot_far_sw(EMBRPOS_D,real)?1:0;
//  brStatus((18-1)*6+5) = atom->embr_pot_near_sw(EMBRPOS_L,real)|atom->embr_pot_far_sw(EMBRPOS_L,real)?1:0;

  // --------------

  // --- Node 19 ---
  atom = &_atomSet[3][0];
  brStatus((19-1)*6+0) = atom->node_pot_resistance_sw(real)&atom->node_sw_resistance(real)?1:0;
  brStatus((19-1)*6+1) = atom->embr_pot_near_sw(EMBRPOS_U,real)|atom->embr_pot_far_sw(EMBRPOS_U,real)?1:0;
  brStatus((19-1)*6+2) = atom->embr_pot_near_sw(EMBRPOS_UR,real)|atom->embr_pot_far_sw(EMBRPOS_UR,real)?1:0;
  brStatus((19-1)*6+3) = atom->embr_pot_near_sw(EMBRPOS_R,real)|atom->embr_pot_far_sw(EMBRPOS_R,real)?1:0;
//  brStatus((19-1)*6+4) = atom->embr_pot_near_sw(EMBRPOS_D,real)|atom->embr_pot_far_sw(EMBRPOS_D,real)?1:0;
//  brStatus((19-1)*6+5) = atom->embr_pot_near_sw(EMBRPOS_L,real)|atom->embr_pot_far_sw(EMBRPOS_L,real)?1:0;

  // --- Node 20 ---
  atom = &_atomSet[3][1];
  brStatus((20-1)*6+0) = atom->node_pot_resistance_sw(real)&atom->node_sw_resistance(real)?1:0;
  brStatus((20-1)*6+1) = atom->embr_pot_near_sw(EMBRPOS_U,real)|atom->embr_pot_far_sw(EMBRPOS_U,real)?1:0;
  brStatus((20-1)*6+2) = atom->embr_pot_near_sw(EMBRPOS_UR,real)|atom->embr_pot_far_sw(EMBRPOS_UR,real)?1:0;
  brStatus((20-1)*6+3) = atom->embr_pot_near_sw(EMBRPOS_R,real)|atom->embr_pot_far_sw(EMBRPOS_R,real)?1:0;
//  brStatus((20-1)*6+4) = atom->embr_pot_near_sw(EMBRPOS_D,real)|atom->embr_pot_far_sw(EMBRPOS_D,real)?1:0;
//  brStatus((20-1)*6+5) = atom->embr_pot_near_sw(EMBRPOS_L,real)|atom->embr_pot_far_sw(EMBRPOS_L,real)?1:0;

  // --- Node 21 ---
  atom = &_atomSet[3][2];
  brStatus((21-1)*6+0) = atom->node_pot_resistance_sw(real)&atom->node_sw_resistance(real)?1:0;
  brStatus((21-1)*6+1) = atom->embr_pot_near_sw(EMBRPOS_U,real)|atom->embr_pot_far_sw(EMBRPOS_U,real)?1:0;
  brStatus((21-1)*6+2) = atom->embr_pot_near_sw(EMBRPOS_UR,real)|atom->embr_pot_far_sw(EMBRPOS_UR,real)?1:0;
  brStatus((21-1)*6+3) = atom->embr_pot_near_sw(EMBRPOS_R,real)|atom->embr_pot_far_sw(EMBRPOS_R,real)?1:0;
//  brStatus((21-1)*6+4) = atom->embr_pot_near_sw(EMBRPOS_D,real)|atom->embr_pot_far_sw(EMBRPOS_D,real)?1:0;
//  brStatus((21-1)*6+5) = atom->embr_pot_near_sw(EMBRPOS_L,real)|atom->embr_pot_far_sw(EMBRPOS_L,real)?1:0;

  // --- Node 22 ---
  atom = &_atomSet[3][3];
  brStatus((22-1)*6+0) = atom->node_pot_resistance_sw(real)&atom->node_sw_resistance(real)?1:0;
  brStatus((22-1)*6+1) = atom->embr_pot_near_sw(EMBRPOS_U,real)|atom->embr_pot_far_sw(EMBRPOS_U,real)?1:0;
  brStatus((22-1)*6+2) = atom->embr_pot_near_sw(EMBRPOS_UR,real)|atom->embr_pot_far_sw(EMBRPOS_UR,real)?1:0;
  brStatus((22-1)*6+3) = atom->embr_pot_near_sw(EMBRPOS_R,real)|atom->embr_pot_far_sw(EMBRPOS_R,real)?1:0;
//  brStatus((22-1)*6+4) = atom->embr_pot_near_sw(EMBRPOS_D,real)|atom->embr_pot_far_sw(EMBRPOS_D,real)?1:0;
//  brStatus((22-1)*6+5) = atom->embr_pot_near_sw(EMBRPOS_L,real)|atom->embr_pot_far_sw(EMBRPOS_L,real)?1:0;

  // --- Node 23 ---
  atom = &_atomSet[3][4];
  brStatus((23-1)*6+0) = atom->node_pot_resistance_sw(real)&atom->node_sw_resistance(real)?1:0;
  brStatus((23-1)*6+1) = atom->embr_pot_near_sw(EMBRPOS_U,real)|atom->embr_pot_far_sw(EMBRPOS_U,real)?1:0;
  brStatus((23-1)*6+2) = atom->embr_pot_near_sw(EMBRPOS_UR,real)|atom->embr_pot_far_sw(EMBRPOS_UR,real)?1:0;
  brStatus((23-1)*6+3) = atom->embr_pot_near_sw(EMBRPOS_R,real)|atom->embr_pot_far_sw(EMBRPOS_R,real)?1:0;
//  brStatus((23-1)*6+4) = atom->embr_pot_near_sw(EMBRPOS_D,real)|atom->embr_pot_far_sw(EMBRPOS_D,real)?1:0;
//  brStatus((23-1)*6+5) = atom->embr_pot_near_sw(EMBRPOS_L,real)|atom->embr_pot_far_sw(EMBRPOS_L,real)?1:0;

  // --- Node 24 ---
  atom = &_atomSet[3][5];
  brStatus((24-1)*6+0) = atom->node_pot_resistance_sw(real)&atom->node_sw_resistance(real)?1:0;
  brStatus((24-1)*6+1) = atom->embr_pot_near_sw(EMBRPOS_U,real)|atom->embr_pot_far_sw(EMBRPOS_U,real)?1:0;
//  brStatus((24-1)*6+2) = atom->embr_pot_near_sw(EMBRPOS_UR,real)|atom->embr_pot_far_sw(EMBRPOS_UR,real)?1:0;
//  brStatus((24-1)*6+3) = atom->embr_pot_near_sw(EMBRPOS_R,real)|atom->embr_pot_far_sw(EMBRPOS_R,real)?1:0;
//  brStatus((24-1)*6+4) = atom->embr_pot_near_sw(EMBRPOS_D,real)|atom->embr_pot_far_sw(EMBRPOS_D,real)?1:0;
//  brStatus((24-1)*6+5) = atom->embr_pot_near_sw(EMBRPOS_L,real)|atom->embr_pot_far_sw(EMBRPOS_L,real)?1:0;
}

void SliceAnalog::calibrate(SliceAnalog const& cal_sl){
  size_t ver, hor;
  this->size(ver,hor);
  for ( size_t m = 0 ; m != ver ; ++m ){
    for ( size_t n = 0 ; n != hor ; ++n ){
      Atom const* cal_am = cal_sl.getAtom(m,n);
      _atomSet[m][n].calibrate(*cal_am);
    } // horizontal atom loop
  } // vertical atom loop
}

int SliceAnalog::nodeCurrentSource(size_t id_ver, size_t id_hor, double seriesR, double shuntR){
  int ans = 0;
  // Connect current source
  _atomSet[id_ver][id_hor].set_node_real_sw_current(true);
  _atomSet[id_ver][id_hor].set_node_imag_sw_current(true);
  // Set series resistance and close internal potentiometer switch
  ans |= _atomSet[id_ver][id_hor].set_node_real_pot_current_r( seriesR );
  ans |= _atomSet[id_ver][id_hor].set_node_imag_pot_current_r( seriesR );
  _atomSet[id_ver][id_hor].set_node_real_pot_current_sw(true);
  _atomSet[id_ver][id_hor].set_node_imag_pot_current_sw(true);


  // Disconnect currenct voltage source, and shunt resistor
  _atomSet[id_ver][id_hor].set_node_real_sw_voltage(false);
  _atomSet[id_ver][id_hor].set_node_imag_sw_voltage(false);
  _atomSet[id_ver][id_hor].set_node_real_sw_current_shunt(false);
  _atomSet[id_ver][id_hor].set_node_imag_sw_current_shunt(false);

  // Connect resistance to ground - if needed
  if (shuntR > 0){
    // Close the resistance to ground switch
    _atomSet[id_ver][id_hor].set_node_real_sw_resistance(true);
    _atomSet[id_ver][id_hor].set_node_imag_sw_resistance(true);
    // Set resistance to ground and close internal potentiometer switch
    ans |= _atomSet[id_ver][id_hor].set_node_real_pot_resistance_r( shuntR );
    ans |= _atomSet[id_ver][id_hor].set_node_imag_pot_resistance_r( shuntR );
    _atomSet[id_ver][id_hor].set_node_real_pot_resistance_sw(true);
    _atomSet[id_ver][id_hor].set_node_imag_pot_resistance_sw(true);
  } else {
    // Open the resistance to ground switch
    _atomSet[id_ver][id_hor].set_node_real_sw_resistance(false);
    _atomSet[id_ver][id_hor].set_node_imag_sw_resistance(false);
    // Open the internal potentiometer switch
    _atomSet[id_ver][id_hor].set_node_real_pot_resistance_sw(false);
    _atomSet[id_ver][id_hor].set_node_imag_pot_resistance_sw(false);
  }
  return ans;
}

int SliceAnalog::nodeVoltageSource(size_t id_ver, size_t id_hor, double shuntR){
  int ans = 0;
  // Connect voltage source
  _atomSet[id_ver][id_hor].set_node_real_sw_voltage(true);
  _atomSet[id_ver][id_hor].set_node_imag_sw_voltage(true);

  // Disconnect current source and open internal potentiometer switch
  _atomSet[id_ver][id_hor].set_node_real_sw_current(false);
  _atomSet[id_ver][id_hor].set_node_imag_sw_current(false);
  _atomSet[id_ver][id_hor].set_node_real_pot_current_sw(false);
  _atomSet[id_ver][id_hor].set_node_imag_pot_current_sw(false);

  // Disconnect current shunt resistor
  _atomSet[id_ver][id_hor].set_node_real_sw_current_shunt(false);
  _atomSet[id_ver][id_hor].set_node_imag_sw_current_shunt(false);

  // Connect resistance to ground - if needed
  if (shuntR > 0){
    // Close the resistance to ground switch
    _atomSet[id_ver][id_hor].set_node_real_sw_resistance(true);
    _atomSet[id_ver][id_hor].set_node_imag_sw_resistance(true);
    // Set resistance to ground and close internal potentiometer switch
    ans |= _atomSet[id_ver][id_hor].set_node_real_pot_resistance_r( shuntR );
    ans |= _atomSet[id_ver][id_hor].set_node_imag_pot_resistance_r( shuntR );
    _atomSet[id_ver][id_hor].set_node_real_pot_resistance_sw(true);
    _atomSet[id_ver][id_hor].set_node_imag_pot_resistance_sw(true);
  } else {
    // Open the resistance to ground switch
    _atomSet[id_ver][id_hor].set_node_real_sw_resistance(false);
    _atomSet[id_ver][id_hor].set_node_imag_sw_resistance(false);
    // Open the internal potentiometer switch
    _atomSet[id_ver][id_hor].set_node_real_pot_resistance_sw(false);
    _atomSet[id_ver][id_hor].set_node_imag_pot_resistance_sw(false);
  }
  return ans;
}

void SliceAnalog::nodeDisconnect(size_t id_ver, size_t id_hor){
  // Open all switches
  _atomSet[id_ver][id_hor].set_node_real_sw_current(false);
  _atomSet[id_ver][id_hor].set_node_imag_sw_current(false);
  _atomSet[id_ver][id_hor].set_node_real_sw_current_shunt(false);
  _atomSet[id_ver][id_hor].set_node_imag_sw_current_shunt(false);
  _atomSet[id_ver][id_hor].set_node_real_sw_voltage(false);
  _atomSet[id_ver][id_hor].set_node_imag_sw_voltage(false);
  _atomSet[id_ver][id_hor].set_node_real_sw_resistance(false);
  _atomSet[id_ver][id_hor].set_node_imag_sw_resistance(false);

  // Open all internal potentiometer switches
  _atomSet[id_ver][id_hor].set_node_real_pot_resistance_sw(false);
  _atomSet[id_ver][id_hor].set_node_imag_pot_resistance_sw(false);
  _atomSet[id_ver][id_hor].set_node_real_pot_current_sw(false);
  _atomSet[id_ver][id_hor].set_node_imag_pot_current_sw(false);
}

int SliceAnalog::embrConnect(size_t id_ver, size_t id_hor, size_t pos,
                             double r_near, double r_far){

  // Input argument validation
  if (id_ver >= _atomSet.size())                 return 20;
  if (id_hor >= _atomSet[id_ver].size())         return 21;
  if (!_atomSet[id_ver][id_hor].embr_exist(pos)) return 22;

  int ans = 0;
  // Open mid gnd switches
  _atomSet[id_ver][id_hor].set_embr_real_sw_mid(pos, false);
  _atomSet[id_ver][id_hor].set_embr_imag_sw_mid(pos, false);

  // Set near-end potentiometer value and close internal switch
  _atomSet[id_ver][id_hor].set_embr_real_pot_near_sw(pos, true);
  _atomSet[id_ver][id_hor].set_embr_imag_pot_near_sw(pos, true);
  ans |= _atomSet[id_ver][id_hor].set_embr_real_pot_near_r(pos, r_near)   !=0?(1<<4):0;
  ans |= _atomSet[id_ver][id_hor].set_embr_imag_pot_near_r(pos, r_near)   !=0?(1<<5):0;

  // Set far-end potentiometer value and close internal switch
  _atomSet[id_ver][id_hor].set_embr_real_pot_far_sw(pos, true);
  _atomSet[id_ver][id_hor].set_embr_imag_pot_far_sw(pos, true);
  ans |= _atomSet[id_ver][id_hor].set_embr_real_pot_far_r(pos, r_far)     !=0?(1<<6):0;
  ans |= _atomSet[id_ver][id_hor].set_embr_imag_pot_far_r(pos, r_far)     !=0?(1<<7):0;

  // Provision for too small values
  if ( (r_near + r_far) <= static_cast<double> (2*POTENTIOMETER_RW) ){
    // branch corresponding to a short circuit ( r_near ~== r_far ~== 0 )
    _atomSet[id_ver][id_hor].set_embr_real_sw_sc(pos, true);
    _atomSet[id_ver][id_hor].set_embr_imag_sw_sc(pos, true);
  } else {
    // branch corresponding to a normal branch
    _atomSet[id_ver][id_hor].set_embr_real_sw_sc(pos, false);
    _atomSet[id_ver][id_hor].set_embr_imag_sw_sc(pos, false);
  }
  return ans;
}

int SliceAnalog::embrDisconnect(size_t id_ver, size_t id_hor, size_t pos){

  // Input argument validation
  if ( id_ver >= _atomSet.size() )                    return 20;
  if ( id_hor >= _atomSet[id_ver].size() )            return 21;
  // Cannot modify an emulator hw branch that does not exist!
  if (!_atomSet[id_ver][id_hor].embr_exist(pos))      return 22;

  int ans = 0;
  // Open short-circuiting switch
  _atomSet[id_ver][id_hor].set_embr_real_sw_sc(pos, false);
  _atomSet[id_ver][id_hor].set_embr_imag_sw_sc(pos, false);
  // Open mid-grounding switch
  _atomSet[id_ver][id_hor].set_embr_real_sw_mid(pos, false);
  _atomSet[id_ver][id_hor].set_embr_imag_sw_mid(pos, false);
  // Open near-end potentiometer internal switches
  _atomSet[id_ver][id_hor].set_embr_real_pot_near_sw(pos, false);
  _atomSet[id_ver][id_hor].set_embr_imag_pot_near_sw(pos, false);
  // Open far-end potentiometer internal switches
  _atomSet[id_ver][id_hor].set_embr_real_pot_far_sw(pos, false);
  _atomSet[id_ver][id_hor].set_embr_imag_pot_far_sw(pos, false);
  return ans;
}

void SliceAnalog::set_got_gain(double val){ _got_gain = val; }
int SliceAnalog::set_got_offset(double val){
  if ( (val < 0) || (val > DAC_DEF_OUTMAX ) )
    // got_offset out of bounds [0 , DEFMAXVVOLT=5] Volt
    return 1;
  _got_offset = val;
  return 0;
}
int SliceAnalog::set_real_voltage_ref_val(double val){return _real_voltage_ref.set_out(val);}
int SliceAnalog::set_real_voltage_ref_tap(unsigned int tap){return _real_voltage_ref.set_tap(tap);}
int SliceAnalog::set_real_voltage_ref_out_min(double val,bool updateTap){return _real_voltage_ref.set_out_min(val,updateTap);}
int SliceAnalog::set_real_voltage_ref_out_max(double val,bool updateTap){return _real_voltage_ref.set_out_max(val,updateTap);}

int SliceAnalog::set_imag_voltage_ref_val(double val){return _imag_voltage_ref.set_out(val);}
int SliceAnalog::set_imag_voltage_ref_tap(unsigned int tap){return _imag_voltage_ref.set_tap(tap);}
int SliceAnalog::set_imag_voltage_ref_out_min(double val,bool updateTap){return _imag_voltage_ref.set_out_min(val,updateTap);}
int SliceAnalog::set_imag_voltage_ref_out_max(double val,bool updateTap){return _imag_voltage_ref.set_out_max(val,updateTap);}

// --- getters ---
Atom const* SliceAnalog::getAtom(size_t ver, size_t hor) const{return dynamic_cast<Atom const*>(&_atomSet[ver][hor]);}
size_t SliceAnalog::getEmbrCount() const{
  size_t ans = 0;
  for ( size_t k = 0 ; k != _atomSet.size() ; ++k )
    for ( size_t m = 0 ; m != _atomSet[k].size() ; ++m )
      ans += _atomSet[k][m].getEmbrCount();
  return ans;
}
double SliceAnalog::got_gain() const{ return _got_gain; }
double SliceAnalog::got_offset() const{ return _got_offset; }
double SliceAnalog::real_voltage_ref_val() const{ return _real_voltage_ref.out(); }
unsigned int SliceAnalog::real_voltage_ref_tap() const{ return _real_voltage_ref.tap(); }
double SliceAnalog::real_voltage_ref_val_min() const{ return _real_voltage_ref.out_min(); }
double SliceAnalog::real_voltage_ref_val_max() const{ return _real_voltage_ref.out_max(); }
unsigned int SliceAnalog::real_voltage_ref_tap_max() const{ return _real_voltage_ref.tap_max(); }
double SliceAnalog::imag_voltage_ref_val() const{ return _imag_voltage_ref.out(); }
unsigned int SliceAnalog::imag_voltage_ref_tap() const{ return _imag_voltage_ref.tap(); }
double SliceAnalog::imag_voltage_ref_val_min() const{ return _imag_voltage_ref.out_min(); }
double SliceAnalog::imag_voltage_ref_val_max() const{ return _imag_voltage_ref.out_max(); }
unsigned int SliceAnalog::imag_voltage_ref_tap_max() const{ return _imag_voltage_ref.tap_max(); }
