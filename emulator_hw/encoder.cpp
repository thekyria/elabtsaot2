
#include "encoder.h"
using namespace elabtsaot;

#include "slice.h"
#include "auxiliary.h"
#include "scenario.h"
#include "emulator.h"
#include "powersystem.h"

//#include <vector>
using std::vector;
//#include <set>
using std::set;
#include <iostream>
using std::cout;
using std::endl;
#include <cmath>            // Required for M_PI constant
#define _USE_MATH_DEFINES
#include <bitset>
using std::bitset;
#include <limits>
using std::numeric_limits;
//#include <complex>
using std::complex;
using std::conj;

using std::pair;

//#define VERBOSE_ENC

int encoder::encodeSlicePF(Slice const& sl, vector<uint32_t>& sliceConf){

  sliceConf.clear();

  int ans = 0;

  // Vector        | vec.add | cyp.add [num.of.words] corresponing func.
  //---------------|---------|-------------------------------------------------
  // ggot_conf     |   0: 47 |   1: 48 [ 48] detail::encode_ggot
  //  none         |  48:338 |  49:339 [291]  none
  // pos_conf      | 339:354 | 340:343 [  4] detail::encode_PFpositions
  //  none         | 343:350 | 344:351 [  8]  none
  // slpos_conf    |     351 |     352 [  1] detail::encode_PFpositions
  //  none         | 352:353 | 353:354 [  2]  none
  // conf_conf     |     354 |     355 [  1] detail::encode_PFauxiliary
  // vref_conf     | 355:356 | 356:357 [  2] detail::encode_vref
  // icar_conf     | 357:380 | 358:381 [ 24] detail::encode_iinit
  // starter_conf  |     381 |     382 [  1] detail::encode_PFauxiliary
  // res_conf      | 382:485 | 383:486 [104] detail::encode_resistors
  // res_tcon_conf | 486:537 | 487:538 [ 52] detail::encode_resistors
  // switches_conf | 538:550 | 539:551 [ 13] detail::encode_switches
  //  none         | 551:553 | 552:554 [  3]  none
  // conf_conf     |     554 |     555 [  1] detail::encode_PFauxiliary
  // pqset_conf    | 555:578 | 556:579 [ 24] detail::encode_PQsetpoints
  // ipol_conf     | 579:602 | 580:603 [ 24] detail::encode_iinit
  //  none         | 603:608 | 604:609 [  6]  none

  vector<uint32_t> ggot_conf;
  vector<uint32_t> none;
  vector<uint32_t> pos_conf;
  vector<uint32_t> slpos_conf;
  vector<uint32_t> conf_conf;
  vector<uint32_t> vref_conf;
  vector<uint32_t> icar_conf;
  vector<uint32_t> starter_conf;
  vector<uint32_t> res_conf;
  vector<uint32_t> res_tcon_conf;
  vector<uint32_t> switches_conf;
  vector<uint32_t> pqset_conf;
  vector<uint32_t> ipol_conf;
  vector<uint32_t> beta_conf;
  vector<uint32_t> opt2_conf;

  ans |= detail::encode_ggot( sl, ggot_conf );
  ans |= detail::encode_PFpositions( sl, pos_conf, slpos_conf );
  ans |= detail::encode_PFauxiliary( conf_conf, starter_conf );
  ans |= detail::encode_vref( sl, vref_conf );
  ans |= detail::encode_iinit( sl, icar_conf, ipol_conf );
  ans |= detail::encode_resistors( sl, res_conf, res_tcon_conf );
  ans |= detail::encode_switches( sl, switches_conf );
  ans |= detail::encode_PQsetpoints( sl, pqset_conf );
  if (ans)
    return ans;

  sliceConf.insert(sliceConf.end(), ggot_conf.begin(), ggot_conf.end() );
  none.resize(291,0);
  sliceConf.insert(sliceConf.end(), none.begin(), none.end() );
  sliceConf.insert(sliceConf.end(), pos_conf.begin(), pos_conf.end() );
  none.resize(8,0);
  sliceConf.insert(sliceConf.end(), none.begin(), none.end() );
  sliceConf.insert(sliceConf.end(), slpos_conf.begin(), slpos_conf.end() );
  none.resize(2,0);
  sliceConf.insert(sliceConf.end(), none.begin(), none.end() );
  sliceConf.insert(sliceConf.end(), conf_conf.begin(), conf_conf.end() );
  sliceConf.insert(sliceConf.end(), vref_conf.begin(), vref_conf.end() );
  sliceConf.insert(sliceConf.end(), icar_conf.begin(), icar_conf.end() );
  sliceConf.insert(sliceConf.end(), starter_conf.begin(), starter_conf.end() );
  sliceConf.insert(sliceConf.end(), res_conf.begin(), res_conf.end() );
  sliceConf.insert(sliceConf.end(), res_tcon_conf.begin(), res_tcon_conf.end() );
  sliceConf.insert(sliceConf.end(), switches_conf.begin(), switches_conf.end() );
  none.resize(3,0);
  sliceConf.insert(sliceConf.end(), none.begin(), none.end() );
  sliceConf.insert(sliceConf.end(), pqset_conf.begin(), pqset_conf.end() );
  sliceConf.insert(sliceConf.end(), ipol_conf.begin(), ipol_conf.end() );
  none.resize(6,0);
  sliceConf.insert(sliceConf.end(), none.begin(), none.end() );

  return 777;
}

int encoder::encodeSliceTD(Slice const& sl , vector<uint32_t>& sliceConf ){

  sliceConf.clear();

  int ans = 0;

  // Vector        | vec.add | cyp.add [num.of.words] corresponing func.
  //---------------|---------|-------------------------------------------------
  // ggot_conf     |   0: 47 |   1: 48 [ 48] detail::encode_generators
  // zgot_conf     |  48: 95 |  49: 96 [ 48] detail::encode_zloads
  // igot_conf     |  96:143 |  97:144 [ 48] detail::encode_iloads
  // gen_conf1     | 144:167 | 145:168 [ 24] detail::encode_generators
  // damp_conf     |     168 |     169 [  1] none (placeholder words)
  // gen_conf2     | 169:216 | 170:217 [ 48] detail::encode_generators
  // tstep_conf    | 217:218 | 218:219 [  2] none (placeholder words)
  // gen_conf3     | 219:282 | 220:283 [ 64] detail::encode_generators
  // zloads_conf   | 283:306 | 284:307 [ 24] detail::encode_zloads
  // ploads_conf   | 307:330 | 308:331 [ 24] detail::encode_ploads
  // spies_conf    | 331:338 | 332:339 [  8] none (placeholder words)
  // pos_conf      | 339:354 | 340:355 [ 16] detail::encode_positions
  // vref_conf     | 355:356 | 356:357 [  2] detail::encode_vref
  // iloads_conf   | 357:380 | 358:381 [ 24] detail::encode_iloads
  // starter_conf  |     381 |     382 [  1] none (placeholder words)
  // res_conf      | 382:485 | 383:486 [104] detail::encode_resistors
  // res_tcon_conf | 486:537 | 487:538 [ 52] detail::encode_resistors
  // switches_conf | 538:550 | 539:551 [ 13] detail::encode_switches
  // pert_conf     | 551:554 | 552:555 [  4] detail::encode_auxiliary
  // command_conf  | 555:559 | 556:560 [  5] none (placeholder words)
  //         total     0:559 |   1:560 [560]
  vector<uint32_t> ggot_conf;
  vector<uint32_t> zgot_conf;
  vector<uint32_t> igot_conf;
  vector<uint32_t> gen_conf1;
  vector<uint32_t> damp_conf(1,1);
  vector<uint32_t> gen_conf2;
  vector<uint32_t> tstep_conf(2,0);
  vector<uint32_t> gen_conf3;
  vector<uint32_t> zloads_conf;
  vector<uint32_t> ploads_conf;
  vector<uint32_t> spies_conf(8,0);
  vector<uint32_t> pos_conf;
  vector<uint32_t> vref_conf;
  vector<uint32_t> iloads_conf;
  vector<uint32_t> starter_conf(1,0);
  vector<uint32_t> res_conf;
  vector<uint32_t> res_tcon_conf;
  vector<uint32_t> switches_conf;
  vector<uint32_t> pert_conf;
  vector<uint32_t> command_conf(5,0);

  ans |= detail::encode_TDgenerators( sl, ggot_conf, gen_conf1, gen_conf2, gen_conf3 );
  ans |= detail::encode_TDzloads( sl, zgot_conf, zloads_conf );
  ans |= detail::encode_TDiloads( sl, igot_conf, iloads_conf );
  ans |= detail::encode_TDploads( sl, ploads_conf );
  ans |= detail::encode_TDpositions( sl, pos_conf );
  ans |= detail::encode_vref( sl, vref_conf );
  ans |= detail::encode_TDauxiliary( sl, pert_conf );
  ans |= detail::encode_resistors( sl, res_conf, res_tcon_conf );
  ans |= detail::encode_switches( sl, switches_conf );
  if (ans)
    return ans;

  sliceConf.insert(sliceConf.end(), ggot_conf.begin(), ggot_conf.end() );
  sliceConf.insert(sliceConf.end(), zgot_conf.begin(), zgot_conf.end() );
  sliceConf.insert(sliceConf.end(), igot_conf.begin(), igot_conf.end() );
  sliceConf.insert(sliceConf.end(), gen_conf1.begin(), gen_conf1.end() );
  sliceConf.insert(sliceConf.end(), damp_conf.begin(), damp_conf.end() );
  sliceConf.insert(sliceConf.end(), gen_conf2.begin(), gen_conf2.end() );
  sliceConf.insert(sliceConf.end(), tstep_conf.begin(), tstep_conf.end() );
  sliceConf.insert(sliceConf.end(), gen_conf3.begin(), gen_conf3.end() );
  sliceConf.insert(sliceConf.end(), zloads_conf.begin(), zloads_conf.end() );
  sliceConf.insert(sliceConf.end(), ploads_conf.begin(), ploads_conf.end() );
  sliceConf.insert(sliceConf.end(), spies_conf.begin(), spies_conf.end() );
  sliceConf.insert(sliceConf.end(), pos_conf.begin(), pos_conf.end() );
  sliceConf.insert(sliceConf.end(), vref_conf.begin(), vref_conf.end() );
  sliceConf.insert(sliceConf.end(), iloads_conf.begin(), iloads_conf.end() );
  sliceConf.insert(sliceConf.end(), starter_conf.begin(), starter_conf.end() );
  sliceConf.insert(sliceConf.end(), res_conf.begin(), res_conf.end() );
  sliceConf.insert(sliceConf.end(), res_tcon_conf.begin(), res_tcon_conf.end() );
  sliceConf.insert(sliceConf.end(), switches_conf.begin(), switches_conf.end() );
  sliceConf.insert(sliceConf.end(), pert_conf.begin(), pert_conf.end() );
  sliceConf.insert(sliceConf.end(), command_conf.begin(), command_conf.end() );

  return 0;
}

// Auxiliary functions
void encoder::stamp_NIOS_confirm(uint32_t& word, uint32_t confirm_stamp){
  word |= confirm_stamp;
}
void encoder::stamp_NIOS_confirm(int32_t& word, uint32_t confirm_stamp){
  word |= confirm_stamp;
}

int encoder::detail::encode_vref( Slice const& sl,
                                  vector<uint32_t>& vref_conf ){
  vref_conf.clear();

  // Retrieve ref DAC tap setting
  int32_t tempVrefReal = sl.ana.real_voltage_ref_tap();
  int32_t tempVrefImag = sl.ana.imag_voltage_ref_tap();

  // Push back to conf vector
  vref_conf.push_back( static_cast<uint32_t>(tempVrefReal) ); // Real Vref Q16.0 [0,5)
  vref_conf.push_back( static_cast<uint32_t>(tempVrefImag) ); // Imag Vref Q16.0 [0,5)

  return 0;
}

int encoder::detail::encode_resistors( Slice const& sl,
                                       vector<uint32_t>& res_conf,
                                       vector<uint32_t>& res_tcon_conf ){

  res_conf.clear();
  res_tcon_conf.clear();
  size_t k, m;
  Atom const* am;
  size_t maxver, maxhor; sl.ana.size(maxver, maxhor);

  vector<vector<uint8_t> > atom_res_values (26, vector<uint8_t> () );
  vector<vector<bitset<8> > > atom_tcon_values (26, vector<bitset<8> > () );
  size_t atom_count = 0;
  for ( k = 0 ; k != maxver ; ++k ){
    for ( m = 0 ; m != maxhor ; ++m ){
      am = sl.ana.getAtom(k,m);

      atom_res_values[atom_count].clear();
      atom_res_values[atom_count].resize(16, 0);
      atom_tcon_values[atom_count].clear();
      atom_tcon_values[atom_count].resize(8, bitset<8> (136ul) );
      // default value 10001000 (= 136ul)

      ///////////////////////////////////
      // Fill in resistor values (8-bit)

      atom_res_values[atom_count][0]  = (uint8_t) am->embr_real_pot_far_tap(EMBRPOS_R);
      atom_res_values[atom_count][1]  = (uint8_t) am->embr_real_pot_near_tap(EMBRPOS_R);

      atom_res_values[atom_count][2]  = (uint8_t) am->embr_real_pot_near_tap(EMBRPOS_U);
      atom_res_values[atom_count][3]  = (uint8_t) am->embr_real_pot_far_tap(EMBRPOS_U);

      atom_res_values[atom_count][4]  = (uint8_t) am->embr_imag_pot_far_tap(EMBRPOS_R);
      atom_res_values[atom_count][5]  = (uint8_t) am->embr_imag_pot_near_tap(EMBRPOS_R);

      atom_res_values[atom_count][6]  = (uint8_t) am->embr_imag_pot_near_tap(EMBRPOS_U);
      atom_res_values[atom_count][7]  = (uint8_t) am->embr_imag_pot_far_tap(EMBRPOS_U);

      atom_res_values[atom_count][8]  = (uint8_t) am->embr_real_pot_near_tap(EMBRPOS_UR);
      atom_res_values[atom_count][9]  = (uint8_t) am->embr_real_pot_far_tap(EMBRPOS_UR);
      atom_res_values[atom_count][10] = (uint8_t) am->embr_imag_pot_near_tap(EMBRPOS_UR);
      atom_res_values[atom_count][11] = (uint8_t) am->embr_imag_pot_far_tap(EMBRPOS_UR);

      atom_res_values[atom_count][12] = (uint8_t) am->node_imag_pot_resistance_tap();
      atom_res_values[atom_count][13] = (uint8_t) am->node_imag_pot_current_tap();
      atom_res_values[atom_count][14] = (uint8_t) am->node_real_pot_resistance_tap();
      atom_res_values[atom_count][15] = (uint8_t) am->node_real_pot_current_tap();

      ////////////////////////////////////////
      // Fill in TCON register values (8-bit)

      // --- IC controlling res1 & res2 & res3 & res4 ---

      // TCON1&2: TCON0 for IC controlling res1 (R0) & res2 (R1)
      // Modify R0B (pos 0) according to res1 status
      if( am->embr_real_pot_far_sw(EMBRPOS_R) ){
        atom_tcon_values[atom_count][0][0] = 1; // R0B
        atom_tcon_values[atom_count][0][1] = 1; // R0W
        // R0A according to res1 swA
        atom_tcon_values[atom_count][0][2] = am->embr_real_pot_far_swA(EMBRPOS_R);
      }
      // Modify R1B (pos 4) according to res2 status
      if( am->embr_real_pot_near_sw(EMBRPOS_R) ){
        atom_tcon_values[atom_count][0][4] = 1; // R1B
        atom_tcon_values[atom_count][0][5] = 1; // R1W
        // R1A according to res2 swA
        atom_tcon_values[atom_count][0][6] = am->embr_real_pot_near_swA(EMBRPOS_R);
      }

      // TCON3&4: TCON1 for IC controlling res3 (R2) & res4 (R3)
      // Modify R2B (pos 0) according to res3 status
      if( am->embr_real_pot_near_sw(EMBRPOS_U) ){
        atom_tcon_values[atom_count][1][0] = 1; // R2B
        atom_tcon_values[atom_count][1][1] = 1; // R2W
        // R2A according to res3 swA
        atom_tcon_values[atom_count][1][2] = am->embr_real_pot_near_swA(EMBRPOS_U);
      }
      // Modify R3B (pos 4) according to res4 status
      if( am->embr_real_pot_far_sw(EMBRPOS_U) ){
        atom_tcon_values[atom_count][1][4] = 1; // R3B
        atom_tcon_values[atom_count][1][5] = 1; // R3W
        // R3A according to res4 swA
        atom_tcon_values[atom_count][1][6] = am->embr_real_pot_far_swA(EMBRPOS_U);
      }

      // --- IC controlling res5 & res6 & res7 & res8 ---

      // TCON5&6: TCON0 for IC controlling res5 (R0) & res6 (R1)
      // Modify R0B (pos 0) according to res5 status
      if( am->embr_imag_pot_far_sw(EMBRPOS_R) ){
        atom_tcon_values[atom_count][2][0] = 1; // R0B
        atom_tcon_values[atom_count][2][1] = 1; // R0W
        // R0A according to res5 swA
        atom_tcon_values[atom_count][2][2] = am->embr_imag_pot_far_swA(EMBRPOS_R);
      }
      // Modify R1B (pos 4) according to res6 status
      if( am->embr_imag_pot_near_sw(EMBRPOS_R) ){
        atom_tcon_values[atom_count][2][4] = 1; // R1B
        atom_tcon_values[atom_count][2][5] = 1; // R1W
        // R1A according to res6 swA
        atom_tcon_values[atom_count][2][6] = am->embr_imag_pot_near_swA(EMBRPOS_R);
      }

      // TCON7&8: TCON1 for IC controlling res7 (R2) & res8 (R3)
      // Modify R2B (pos 0) according to res7 status
      if( am->embr_imag_pot_near_sw(EMBRPOS_U) ){
        atom_tcon_values[atom_count][3][0] = 1; // R2B
        atom_tcon_values[atom_count][3][1] = 1; // R2W
        // R2A according to res7 swA
        atom_tcon_values[atom_count][3][2] = am->embr_imag_pot_near_swA(EMBRPOS_U);
      }
      // Modify R3B (pos 4) according to res8 status
      if ( am->embr_imag_pot_far_sw(EMBRPOS_U) ){
        atom_tcon_values[atom_count][3][4] = 1; // R3B
        atom_tcon_values[atom_count][3][5] = 1; // R3W
        // R3A according to res8 swA
        atom_tcon_values[atom_count][3][6] = am->embr_imag_pot_far_swA(EMBRPOS_U);
      }

      // --- IC controlling res9 & res10 & res11 & res12 ---

      // TCON9&10: TCON0 for IC controlling res9 (R0) & res10 (R1)
      // retain the default value 10001000 (= 136ul)
      // TCON11&12: TCON1 for IC controlling res11 (R2) & res12 (R3)
      // retain the default value 10001000 (= 136ul)

      // Modify R0B (pos 0) according to res9 status
      if( am->embr_real_pot_near_sw(EMBRPOS_UR) ){
        atom_tcon_values[atom_count][4][0] = 1; // R0B
        atom_tcon_values[atom_count][4][1] = 1; // R0W
        // R0A according to res9 swA
        atom_tcon_values[atom_count][4][2] = am->embr_real_pot_near_swA(EMBRPOS_UR);
      }
      // Modify R1B (pos 4) according to res10 status
      if( am->embr_real_pot_far_sw(EMBRPOS_UR) ){
        atom_tcon_values[atom_count][4][4] = 1; // R1B
        atom_tcon_values[atom_count][4][5] = 1; // R1W
        // R1A according to res10 swA
        atom_tcon_values[atom_count][4][6] = am->embr_real_pot_far_swA(EMBRPOS_UR);
      }

      // Modify R2B (pos 0) according to res11 status
      if( am->embr_imag_pot_near_sw(EMBRPOS_UR) ){
        atom_tcon_values[atom_count][5][0] = 1; // R2B
        atom_tcon_values[atom_count][5][1] = 1; // R2W
        // R2A according to res11 swA
        atom_tcon_values[atom_count][5][2] = am->embr_imag_pot_near_swA(EMBRPOS_UR);
      }
      // Modify R3B (pos 4) according to res12 status
      if( am->embr_imag_pot_far_sw(EMBRPOS_UR) ){
        atom_tcon_values[atom_count][5][4] = 1; // R3B
        atom_tcon_values[atom_count][5][5] = 1; // R3W
        // R3A according to res12 swA
        atom_tcon_values[atom_count][5][6] = am->embr_imag_pot_far_swA(EMBRPOS_UR);
      }

      // --- IC controlling res13 & res14 & res15 & res16 ---

      // TCON13&14: TCON0 for IC controlling res13 (R0) & res14 (R1)
      // retain the default value 10001000 (= 136ul)
      // TCON15&16: TCON1 for IC controlling res15 (R2) & res16 (R3)
      // retain the default value 10001000 (= 136ul)

      // FOR THE NODE
      // TCON13&14: TCON0 for IC controlling res13 (R0) & res14 (R1)
      // Modify R0B (pos 0) according to res13 status
      if( am->node_imag_pot_resistance_sw() ){
        atom_tcon_values[atom_count][6][0] = 1; // R0B
        atom_tcon_values[atom_count][6][1] = 1; // R0W
        // R0A according to res13 swA
        atom_tcon_values[atom_count][6][2] = am->node_imag_pot_resistance_swA();
      }
      // Modify R1B (pos 4) according to res14 status
      if( am->node_imag_pot_current_sw() ){
        atom_tcon_values[atom_count][6][4] = 1; // R1B
        atom_tcon_values[atom_count][6][5] = 1; // R1W
        // R1A according to res14 swA
        atom_tcon_values[atom_count][6][6] = am->node_imag_pot_current_swA();
      }

      // TCON15&16: TCON1 for IC controlling res15 (R2) & res16 (R3)
      // Modify R2B (pos 0) according to res15 status
      if( am->node_real_pot_resistance_sw() ){
        atom_tcon_values[atom_count][7][0] = 1; // R2B
        atom_tcon_values[atom_count][7][1] = 1; // R2W
        // R2A according to res15 swA
        atom_tcon_values[atom_count][7][2] = am->node_real_pot_resistance_swA();
      }
      // Modify R3B (pos 4) according to res16 status
      if( am->node_real_pot_current_sw() ){
        atom_tcon_values[atom_count][7][4] = 1; // R3B
        atom_tcon_values[atom_count][7][5] = 1; // R3W
        // R3A according to res16 swA
        atom_tcon_values[atom_count][7][6] = am->node_real_pot_current_swA();
      }

      ++atom_count;
    }
  }
  // atom_count exists the nested loop with value 24

  // atom_count = 24 contains resistor information regarding the extension
  // system at the left side of the slice
  // ---------------------------------------------------------------------------
  atom_res_values[atom_count].clear();
  atom_res_values[atom_count].resize(16, 0);
  atom_tcon_values[atom_count].clear();
  atom_tcon_values[atom_count].resize(8, bitset<8> (136ul) );

  ///////////////////////////////////
  // Fill in resistor values (8-bit)
  am = sl.ana.getAtom(0,0);
  atom_res_values[atom_count][0]  = (uint8_t) am->embr_real_pot_far_tap(EMBRPOS_L);
  atom_res_values[atom_count][1]  = (uint8_t) am->embr_real_pot_near_tap(EMBRPOS_L);
  atom_res_values[atom_count][2]  = (uint8_t) am->embr_imag_pot_far_tap(EMBRPOS_L);
  atom_res_values[atom_count][3]  = (uint8_t) am->embr_imag_pot_near_tap(EMBRPOS_L);

  am = sl.ana.getAtom(1,0);
  atom_res_values[atom_count][4]  = (uint8_t) am->embr_real_pot_near_tap(EMBRPOS_L);
  atom_res_values[atom_count][5]  = (uint8_t) am->embr_real_pot_far_tap(EMBRPOS_L);
  atom_res_values[atom_count][6]  = (uint8_t) am->embr_imag_pot_near_tap(EMBRPOS_L);
  atom_res_values[atom_count][7]  = (uint8_t) am->embr_imag_pot_far_tap(EMBRPOS_L);

  am = sl.ana.getAtom(2,0);
  atom_res_values[atom_count][8]  = (uint8_t) am->embr_real_pot_near_tap(EMBRPOS_L);
  atom_res_values[atom_count][9]  = (uint8_t) am->embr_real_pot_far_tap(EMBRPOS_L);
  atom_res_values[atom_count][10] = (uint8_t) am->embr_imag_pot_near_tap(EMBRPOS_L);
  atom_res_values[atom_count][11] = (uint8_t) am->embr_imag_pot_far_tap(EMBRPOS_L);

  atom_res_values[atom_count][12] = (uint8_t) 0;
  atom_res_values[atom_count][13] = (uint8_t) 0;
  atom_res_values[atom_count][14] = (uint8_t) 0;
  atom_res_values[atom_count][15] = (uint8_t) 0;

  ////////////////////////////////////////
  // Fill in TCON register values (8-bit)

  // --- IC controlling res1 & res2 & res3 & res4 ---

  // TCON1&2: TCON0 for IC controlling res1 (R0) & res2 (R1)
  // retain the default value 10001000 (= 136ul)
  // TCON3&4: TCON1 for IC controlling res3 (R2) & res4 (R3)
  // retain the default value 10001000 (= 136ul)

  am = sl.ana.getAtom(0,0);
  // TCON1&2: TCON0 for IC controlling res1 (R0) & res2 (R1)
  // Modify R0B (pos 0) according to res1 status
  if ( am->embr_real_pot_far_sw(EMBRPOS_L) ){
    atom_tcon_values[atom_count][0][0] = 1; // R0B
    atom_tcon_values[atom_count][0][1] = 1; // R0W
    // R0A according to res1 swA
    atom_tcon_values[atom_count][0][2] = am->embr_real_pot_far_swA(EMBRPOS_L);
  }
  // Modify R1B (pos 4) according to res2 status
  if ( am->embr_real_pot_near_sw(EMBRPOS_L) ){
    atom_tcon_values[atom_count][0][4] = 1; // R1B
    atom_tcon_values[atom_count][0][5] = 1; // R1W
    // R1A according to res2 swA
    atom_tcon_values[atom_count][0][6] = am->embr_real_pot_near_swA(EMBRPOS_L);
  }

  // TCON3&4: TCON1 for IC controlling res3 (R2) & res4 (R3)
  // Modify R2B (pos 0) according to res3 status
  if ( am->embr_imag_pot_far_sw(EMBRPOS_L) ){
    atom_tcon_values[atom_count][1][0] = 1; // R2B
    atom_tcon_values[atom_count][1][1] = 1; // R2W
    // R2A according to res3 swA
    atom_tcon_values[atom_count][1][2] = am->embr_imag_pot_far_swA(EMBRPOS_L);
  }
  // Modify R3B (pos 4) according to res4 status
  if ( am->embr_imag_pot_near_sw(EMBRPOS_L) ){
    atom_tcon_values[atom_count][1][4] = 1; // R3B
    atom_tcon_values[atom_count][1][5] = 1; // R3W
    // R3A according to res4 swA
    atom_tcon_values[atom_count][1][6] = am->embr_imag_pot_near_swA(EMBRPOS_L);
  }

  // --- IC controlling res5 & res6 & res7 & res8 ---

  // TCON5&6: TCON0 for IC controlling res5 (R0) & res6 (R1)
  // retain the default value 10001000 (= 136ul)
  // TCON7&8: TCON1 for IC controlling res7 (R2) & res8 (R3)
  // retain the default value 10001000 (= 136ul)

  am = sl.ana.getAtom(1,0);
  // TCON5&6: TCON0 for IC controlling res5 (R0) & res6 (R1)
  // Modify R0B (pos 0) according to res5 status
  if ( am->embr_real_pot_near_sw(EMBRPOS_L) ){
    atom_tcon_values[atom_count][2][0] = 1; // R0B
    atom_tcon_values[atom_count][2][1] = 1; // R0W
    // R0A according to res5 swA
    atom_tcon_values[atom_count][2][2] = am->embr_real_pot_near_swA(EMBRPOS_L);
  }
  // Modify R1B (pos 4) according to res6 status
  if ( am->embr_real_pot_far_sw(EMBRPOS_L) ){
    atom_tcon_values[atom_count][2][4] = 1; // R1B
    atom_tcon_values[atom_count][2][5] = 1; // R1W
    // R1A according to res6 swA
    atom_tcon_values[atom_count][2][6] = am->embr_real_pot_far_swA(EMBRPOS_L);
  }

  // TCON7&8: TCON1 for IC controlling res7 (R2) & res8 (R3)
  // Modify R2B (pos 0) according to res7 status
  if ( am->embr_imag_pot_near_sw(EMBRPOS_L) ){
    atom_tcon_values[atom_count][3][0] = 1; // R2B
    atom_tcon_values[atom_count][3][1] = 1; // R2W
    // R2A according to res7 swA
    atom_tcon_values[atom_count][3][2] = am->embr_imag_pot_near_swA(EMBRPOS_L);
  }
  // Modify R3B (pos 4) according to res8 status
  if ( am->embr_imag_pot_far_sw(EMBRPOS_L) ){
    atom_tcon_values[atom_count][3][4] = 1; // R3B
    atom_tcon_values[atom_count][3][5] = 1; // R3W
    // R3A according to res8 swA
    atom_tcon_values[atom_count][3][6] = am->embr_imag_pot_far_swA(EMBRPOS_L);
  }

  // --- IC controlling res9 & res10 & res11 & res12 ---

  // TCON9&10: TCON0 for IC controlling res9 (R0) & res10 (R1)
  // retain the default value 10001000 (= 136ul)
  // TCON11&12: TCON1 for IC controlling res11 (R2) & res12 (R3)
  // retain the default value 10001000 (= 136ul)

  am = sl.ana.getAtom(2,0);
  // TCON9&10: TCON0 for IC controlling res9 (R0) & res10 (R1)
  // Modify R0B (pos 0) according to res9 status
  if ( am->embr_real_pot_near_sw(EMBRPOS_L) ){
    atom_tcon_values[atom_count][4][0] = 1; // R0B
    atom_tcon_values[atom_count][4][1] = 1; // R0W
    // R0A according to res9 swA
    atom_tcon_values[atom_count][4][2] = am->embr_real_pot_near_swA(EMBRPOS_L);
  }
  // Modify R1B (pos 4) according to res10 status
  if ( am->embr_real_pot_far_sw(EMBRPOS_L) ){
    atom_tcon_values[atom_count][4][4] = 1; // R1B
    atom_tcon_values[atom_count][4][5] = 1; // R1W
    // R1A according to res10 swA
    atom_tcon_values[atom_count][4][6] = am->embr_real_pot_far_swA(EMBRPOS_L);
  }

  // TCON11&12: TCON1 for IC controlling res11 (R2) & res12 (R3)
  // Modify R2B (pos 0) according to res11 status
  if ( am->embr_imag_pot_near_sw(EMBRPOS_L) ){
    atom_tcon_values[atom_count][5][0] = 1; // R2B
    atom_tcon_values[atom_count][5][1] = 1; // R2W
    // R2A according to res11 swA
    atom_tcon_values[atom_count][5][2] = am->embr_imag_pot_near_swA(EMBRPOS_L);
  }
  // Modify R3B (pos 4) according to res12 status
  if ( am->embr_imag_pot_far_sw(EMBRPOS_L) ){
    atom_tcon_values[atom_count][5][4] = 1; // R3B
    atom_tcon_values[atom_count][5][5] = 1; // R3W
    // R3A according to res12 swA
    atom_tcon_values[atom_count][5][6] = am->embr_imag_pot_far_swA(EMBRPOS_L);
  }

  // -- Unused -- retain the default value 10001000 (= 136ul)
  //atom_tcon_values[atom_count][6].set();
  //atom_tcon_values[atom_count][7].set();

  ++atom_count;



  // atom_count = 25 contains resistor information regarding the extension
  // system at the bottom side of the slice
  // ---------------------------------------------------------------------------
  atom_res_values[atom_count].clear();
  atom_res_values[atom_count].resize(16, 0);
  atom_tcon_values[atom_count].clear();
  atom_tcon_values[atom_count].resize(8, bitset<8> (136ul) );

  ///////////////////////////////////
  // Fill in resistor values (8-bit)

  am = sl.ana.getAtom(0,1);
    atom_res_values[atom_count][0]  = (uint8_t) am->embr_real_pot_near_tap(EMBRPOS_D);
    atom_res_values[atom_count][1]  = (uint8_t) am->embr_real_pot_far_tap(EMBRPOS_D);
    atom_res_values[atom_count][2]  = (uint8_t) am->embr_imag_pot_near_tap(EMBRPOS_D);
    atom_res_values[atom_count][3]  = (uint8_t) am->embr_imag_pot_far_tap(EMBRPOS_D);

  am = sl.ana.getAtom(0,2);
  atom_res_values[atom_count][4]  = (uint8_t) am->embr_real_pot_near_tap(EMBRPOS_D);
  atom_res_values[atom_count][5]  = (uint8_t) am->embr_real_pot_far_tap(EMBRPOS_D);
  atom_res_values[atom_count][6]  = (uint8_t) am->embr_imag_pot_near_tap(EMBRPOS_D);
  atom_res_values[atom_count][7]  = (uint8_t) am->embr_imag_pot_far_tap(EMBRPOS_D);

  am = sl.ana.getAtom(0,3);
  atom_res_values[atom_count][8]  = (uint8_t) am->embr_real_pot_near_tap(EMBRPOS_D);
  atom_res_values[atom_count][9]  = (uint8_t) am->embr_real_pot_far_tap(EMBRPOS_D);
  atom_res_values[atom_count][10] = (uint8_t) am->embr_imag_pot_near_tap(EMBRPOS_D);
  atom_res_values[atom_count][11] = (uint8_t) am->embr_imag_pot_far_tap(EMBRPOS_D);

  am = sl.ana.getAtom(0,4);
  atom_res_values[atom_count][12] = (uint8_t) am->embr_real_pot_near_tap(EMBRPOS_D);
  atom_res_values[atom_count][13] = (uint8_t) am->embr_real_pot_far_tap(EMBRPOS_D);
  atom_res_values[atom_count][14] = (uint8_t) am->embr_imag_pot_near_tap(EMBRPOS_D);
  atom_res_values[atom_count][15] = (uint8_t) am->embr_imag_pot_far_tap(EMBRPOS_D);

  ////////////////////////////////////////
  // Fill in TCON register values (8-bit)

  // --- IC controlling res1 & res2 & res3 & res4 ---

  // TCON1&2: TCON0 for IC controlling res1 (R0) & res2 (R1)
  // retain the default value 10001000 (= 136ul)
  // TCON3&4: TCON1 for IC controlling res3 (R2) & res4 (R3)
  // retain the default value 10001000 (= 136ul)

  am = sl.ana.getAtom(0,1);
  // TCON1&2: TCON0 for IC controlling res1 (R0) & res2 (R1)
  // Modify R0B (pos 0) according to res1 status
  if ( am->embr_real_pot_near_sw(EMBRPOS_D) ){
    atom_tcon_values[atom_count][0][0] = 1; // R0B
    atom_tcon_values[atom_count][0][1] = 1; // R0W
    // R0A according to res1 swA
    atom_tcon_values[atom_count][0][2] = am->embr_real_pot_near_swA(EMBRPOS_D);
  }
  // Modify R1B (pos 4) according to res2 status
  if ( am->embr_real_pot_far_sw(EMBRPOS_D) ){
    atom_tcon_values[atom_count][0][4] = 1; // R1B
    atom_tcon_values[atom_count][0][5] = 1; // R1W
    // R1A according to res2 swA
    atom_tcon_values[atom_count][0][6] = am->embr_real_pot_far_swA(EMBRPOS_D);
  }

  // TCON3&4: TCON1 for IC controlling res3 (R2) & res4 (R3)
  // Modify R2B (pos 0) according to res3 status
  if ( am->embr_imag_pot_near_sw(EMBRPOS_D) ){
    atom_tcon_values[atom_count][1][0] = 1; // R2B
    atom_tcon_values[atom_count][1][1] = 1; // R2W
    // R2A according to res3 swA
    atom_tcon_values[atom_count][1][2] = am->embr_imag_pot_near_swA(EMBRPOS_D);
  }
  // Modify R3B (pos 4) according to res4 status
  if ( am->embr_imag_pot_far_sw(EMBRPOS_D) ){
    atom_tcon_values[atom_count][1][4] = 1; // R3B
    atom_tcon_values[atom_count][1][5] = 1; // R3W
    // R3A according to res4 swA
    atom_tcon_values[atom_count][1][6] = am->embr_imag_pot_far_swA(EMBRPOS_D);
  }

  // --- IC controlling res5 & res6 & res7 & res8 ---

  // TCON5&6: TCON0 for IC controlling res5 (R0) & res6 (R1)
  // retain the default value 10001000 (= 136ul)
  // TCON7&8: TCON1 for IC controlling res7 (R2) & res8 (R3)
  // retain the default value 10001000 (= 136ul)

  am = sl.ana.getAtom(0,2);
  // TCON5&6: TCON0 for IC controlling res5 (R0) & res6 (R1)
  // Modify R0B (pos 0) according to res5 status
  if ( am->embr_real_pot_near_sw(EMBRPOS_D) ){
    atom_tcon_values[atom_count][2][0] = 1; // R0B
    atom_tcon_values[atom_count][2][1] = 1; // R0W
    // R0A according to res5 swA
    atom_tcon_values[atom_count][2][2] = am->embr_real_pot_near_swA(EMBRPOS_D);
  }
  // Modify R1B (pos 4) according to res6 status
  if ( am->embr_real_pot_far_sw(EMBRPOS_D) ){
    atom_tcon_values[atom_count][2][4] = 1; // R1B
    atom_tcon_values[atom_count][2][5] = 1; // R1W
    // R1A according to res6 swA
    atom_tcon_values[atom_count][2][6] = am->embr_real_pot_far_swA(EMBRPOS_D);
  }

  // TCON7&8: TCON1 for IC controlling res7 (R2) & res8 (R3)
  // Modify R2B (pos 0) according to res7 status
  if ( am->embr_imag_pot_near_sw(EMBRPOS_D) ){
    atom_tcon_values[atom_count][3][0] = 1; // R2B
    atom_tcon_values[atom_count][3][1] = 1; // R2W
    // R2A according to res7 swA
    atom_tcon_values[atom_count][3][2] = am->embr_imag_pot_near_swA(EMBRPOS_D);
  }
  // Modify R3B (pos 4) according to res8 status
  if ( am->embr_imag_pot_far_sw(EMBRPOS_D) ){
    atom_tcon_values[atom_count][3][4] = 1; // R3B
    atom_tcon_values[atom_count][3][5] = 1; // R3W
    // R3A according to res8 swA
    atom_tcon_values[atom_count][3][6] = am->embr_imag_pot_far_swA(EMBRPOS_D);
  }

  // --- IC controlling res9 & res10 & res11 & res12 ---

  // TCON9&10: TCON0 for IC controlling res9 (R0) & res10 (R1)
  // retain the default value 10001000 (= 136ul)
  // TCON11&12: TCON1 for IC controlling res11 (R2) & res12 (R3)
  // retain the default value 10001000 (= 136ul)

  am = sl.ana.getAtom(0,3);
  // TCON9&10: TCON0 for IC controlling res9 (R0) & res10 (R1)
  // Modify R0B (pos 0) according to res9 status
  if ( am->embr_real_pot_near_sw(EMBRPOS_D) ){
    atom_tcon_values[atom_count][4][0] = 1; // R0B
    atom_tcon_values[atom_count][4][1] = 1; // R0W
    // R0A according to res9 swA
    atom_tcon_values[atom_count][4][2] = am->embr_real_pot_near_swA(EMBRPOS_D);
  }
  // Modify R1B (pos 4) according to res10 status
  if ( am->embr_real_pot_far_sw(EMBRPOS_D) ){
    atom_tcon_values[atom_count][4][4] = 1; // R1B
    atom_tcon_values[atom_count][4][5] = 1; // R1W
    // R1A according to res10 swA
    atom_tcon_values[atom_count][4][6] = am->embr_real_pot_far_swA(EMBRPOS_D);
  }

  // TCON11&12: TCON1 for IC controlling res11 (R2) & res12 (R3)
  // Modify R2B (pos 0) according to res11 status
  if ( am->embr_imag_pot_near_sw(EMBRPOS_D) ){
    atom_tcon_values[atom_count][5][0] = 1; // R2B
    atom_tcon_values[atom_count][5][1] = 1; // R2W
    // R2A according to res11 swA
    atom_tcon_values[atom_count][5][2] = am->embr_imag_pot_near_swA(EMBRPOS_D);
  }
  // Modify R3B (pos 4) according to res12 status
  if ( am->embr_imag_pot_far_sw(EMBRPOS_D) ){
    atom_tcon_values[atom_count][5][4] = 1; // R3B
    atom_tcon_values[atom_count][5][5] = 1; // R3W
    // R3A according to res12 swA
    atom_tcon_values[atom_count][5][6] = am->embr_imag_pot_far_swA(EMBRPOS_D);
  }

  // --- IC controlling res13 & res14 & res15 & res16 ---

  // TCON13&14: TCON0 for IC controlling res13 (R0) & res14 (R1)
  // retain the default value 10001000 (= 136ul)
  // TCON11&12: TCON1 for IC controlling res15 (R2) & res16 (R3)
  // retain the default value 10001000 (= 136ul)

  am = sl.ana.getAtom(0,4);
  // TCON13&14: TCON0 for IC controlling res13 (R0) & res14 (R1)
  // Modify R0B (pos 0) according to res13 status
  if ( am->embr_real_pot_near_sw(EMBRPOS_D) ){
    atom_tcon_values[atom_count][6][0] = 1; // R0B
    atom_tcon_values[atom_count][6][1] = 1; // R0W
    // R0A according to res13 swA
    atom_tcon_values[atom_count][6][2] = am->embr_real_pot_near_swA(EMBRPOS_D);
  }
  // Modify R1B (pos 4) according to res14 status
  if ( am->embr_real_pot_far_sw(EMBRPOS_D) ){
    atom_tcon_values[atom_count][6][4] = 1; // R1B
    atom_tcon_values[atom_count][6][5] = 1; // R1W
    // R1A according to res14 swA
    atom_tcon_values[atom_count][6][6] = am->embr_real_pot_far_swA(EMBRPOS_D);
  }

  // TCON11&12: TCON1 for IC controlling res15 (R2) & res16 (R3)
  // Modify R2B (pos 0) according to res15 status
  if ( am->embr_imag_pot_near_sw(EMBRPOS_D) ){
    atom_tcon_values[atom_count][7][0] = 1; // R2B
    atom_tcon_values[atom_count][7][1] = 1; // R2W
    // R2A according to res15 swA
    atom_tcon_values[atom_count][7][2] = am->embr_imag_pot_near_swA(EMBRPOS_D);
  }
  // Modify R3B (pos 4) according to res16 status
  if ( am->embr_imag_pot_far_sw(EMBRPOS_D) ){
    atom_tcon_values[atom_count][7][4] = 1; // R3B
    atom_tcon_values[atom_count][7][5] = 1; // R3W
    // R3A according to res16 swA
    atom_tcon_values[atom_count][7][6] = am->embr_imag_pot_far_swA(EMBRPOS_D);
  }

#ifdef VERBOSE_ENC
  // DEBUG: atom_res_values output
  for ( k = 0 ; k != atom_res_values.size() ; ++k )
    for ( m = 0 ; m != atom_res_values[k].size() ; ++m )
      cout << "atom_res_values["<<k<<"]["<<m<<"] = " <<
               static_cast<unsigned int>( atom_res_values[k][m] ) << endl;
  cout << endl;

  // DEBUG: atom_tcon_values output
  for ( k = 0 ; k != atom_tcon_values.size() ; ++k )
    for ( m = 0 ; m != atom_tcon_values[k].size() ; ++m )
      cout << "atom_tcon_values["<<k<<"]["<<m<<"] = " <<
               atom_tcon_values[k][m] << endl;
  cout << endl;
#endif // VERBOSE_ENC

  // Fill res_conf & res_tcon_conf
  uint32_t temp;
  for ( k = 0 ; k != atom_res_values.size() ; ++k ){
    // Parse atom_res_values for each atom
    for ( m = 0 ; m != static_cast<size_t>(atom_res_values[k].size()/4) ; ++m ){
      // Parse 4 atom_res_values words for each atom:
      // [ res4  res3  res2  res1]  -> word0 for res configuration of atom[k]
      // [ res8  res7  res6  res5]  -> word1
      // [res12 res11 res10  res9]  -> word2
      // [res16 res15 res14 res13]  -> word3
      temp = 0;
      temp |= (uint32_t) atom_res_values[k][4*m];
      temp |= ( (uint32_t) atom_res_values[k][4*m + 1] ) << (1*8);
      temp |= ( (uint32_t) atom_res_values[k][4*m + 2] ) << (2*8);
      temp |= ( (uint32_t) atom_res_values[k][4*m + 3] ) << (3*8);
      res_conf.push_back(temp);
    }

    // Parse atom_tcon_values for each atom
    for (m = 0 ; m != static_cast<size_t>(atom_tcon_values[k].size()/4) ; ++m ){
      // Parse 2 atom_tcon_values words for each atom:
      // [  tcon8&7   tcon6&5   tcon4&3  tcon2&1] -> word0 | for tcon conf of
      // [tcon16&15 tcon14&13 tcon12&11 tcon10&9] -> word1 | atom [k]
      temp = 0;
      temp |= static_cast<uint32_t>( atom_tcon_values[k][4*m + 0].to_ulong() ) << (0*8);
      temp |= static_cast<uint32_t>( atom_tcon_values[k][4*m + 1].to_ulong() ) << (1*8);
      temp |= static_cast<uint32_t>( atom_tcon_values[k][4*m + 2].to_ulong() ) << (2*8);
      temp |= static_cast<uint32_t>( atom_tcon_values[k][4*m + 3].to_ulong() ) << (3*8);
      res_tcon_conf.push_back(temp);
    }
  }

  return 0;
}

int encoder::detail::encode_switches( Slice const& sl, vector<uint32_t>& switches_conf ){

  switches_conf.clear();
  size_t k, m;
  Atom const* am;
  size_t maxver, maxhor; sl.ana.size(maxver, maxhor);

  vector<bitset<16> > atom_sw_status (26, bitset<16> () );
  size_t atom_count = 0;
  for ( k = 0 ; k != maxver ; ++k ){
    for ( m = 0 ; m != maxhor ; ++m ){
      am = sl.ana.getAtom(k,m);

      // Configure all switches at the atom as open (corresponds to 1 because of
      // hardware implementation of the switches using Analog Devices ADG781:
      // IN1 = high -> S1 not connected to D1 (sw open)
      // IN1 = low  -> S1 connected to D1 (sw closed)
      atom_sw_status[atom_count].set();

      // IF
      // branch at position EMBRPOS_R (right) physically exist at the atom (should
      // always be true in current design)
      // THEN
      // modify atom_sw_status according to current PCB implementation
      atom_sw_status[atom_count][0] = !am->embr_real_sw_mid(EMBRPOS_R);
      atom_sw_status[atom_count][1] = !am->embr_real_sw_sc(EMBRPOS_R);

      // IF
      // branch at position EMBRPOS_U (up) physically exist at the atom (should
      // always be true in current design)
      // THEN
      // modify atom_sw_status according to current PCB implementation
      atom_sw_status[atom_count][2] = !am->embr_real_sw_sc(EMBRPOS_U);
      atom_sw_status[atom_count][3] = !am->embr_real_sw_mid(EMBRPOS_U);

      // IF
      // branch at position EMBRPOS_UR (up-right) physically exist at the atom (should
      // always be true in current design)
      // THEN
      // modify atom_sw_status according to current PCB implementation
      atom_sw_status[atom_count][4] = !am->embr_real_sw_sc(EMBRPOS_UR);
      atom_sw_status[atom_count][5] = !am->embr_real_sw_mid(EMBRPOS_UR);

      // Two last bits of first byte are unused - set to 1. ( actually redundant
      // as atom_sw_status[atom_count] has been .set() )
      atom_sw_status[atom_count][6] = 1;
      atom_sw_status[atom_count][7] = 1;

      // modify atom_sw_status according to current PCB implementation
      atom_sw_status[atom_count][ 8] = !am->node_real_sw_current_shunt();
      atom_sw_status[atom_count][ 9] = !am->node_real_sw_current();
      atom_sw_status[atom_count][10] = !am->node_real_sw_voltage();
      atom_sw_status[atom_count][11] = !am->node_real_sw_resistance();
      atom_sw_status[atom_count][12] = !am->node_imag_sw_current_shunt();
      atom_sw_status[atom_count][13] = !am->node_imag_sw_current();
      atom_sw_status[atom_count][14] = !am->node_imag_sw_voltage();
      atom_sw_status[atom_count][15] = !am->node_imag_sw_resistance();

      ++atom_count;
    }
  }
  // atom_count exists the nested loop with value 24

  // atom_sw_status[24] contains switch information regarding the extension
  // system at the left side of the slice
  // ---------------------------------------------------------------------------

  // Configure all switches as open
  atom_sw_status[atom_count].set();

  am = sl.ana.getAtom(0,0);
  // Check physical (should always be ok) existance of EMBRPOS_L branch of
  // atom[0][0]. If so modify atom_sw_status according to current PCB implem.
  atom_sw_status[atom_count][0] = !am->embr_real_sw_sc(EMBRPOS_L);
  atom_sw_status[atom_count][1] = !am->embr_real_sw_mid(EMBRPOS_L);
  atom_sw_status[atom_count][2] = !am->embr_imag_sw_sc(EMBRPOS_L);
  atom_sw_status[atom_count][3] = !am->embr_imag_sw_mid(EMBRPOS_L);

  am = sl.ana.getAtom(1,0);
  // Check physical (should always be ok) existance of EMBRPOS_L branch of
  // atom[1][0]. If so modify atom_sw_status according to current PCB implem.
  atom_sw_status[atom_count][4] = !am->embr_real_sw_sc(EMBRPOS_L);
  atom_sw_status[atom_count][5] = !am->embr_real_sw_mid(EMBRPOS_L);
  atom_sw_status[atom_count][6] = !am->embr_imag_sw_sc(EMBRPOS_L);
  atom_sw_status[atom_count][7] = !am->embr_imag_sw_mid(EMBRPOS_L);

  am = sl.ana.getAtom(2,0);
  // Check physical (should always be ok) existance of EMBRPOS_L branch of
  // atom[2][0]. If so modify atom_sw_status according to current PCB implem.
  atom_sw_status[atom_count][8] = !am->embr_real_sw_sc(EMBRPOS_L);
  atom_sw_status[atom_count][9] = !am->embr_real_sw_mid(EMBRPOS_L);
  atom_sw_status[atom_count][10] = !am->embr_imag_sw_sc(EMBRPOS_L);
  atom_sw_status[atom_count][11] = !am->embr_imag_sw_mid(EMBRPOS_L);

  ++atom_count;



  // atom_sw_status[25] contains switch information regarding the extension
  // system at the bottom side of the slice
  // ---------------------------------------------------------------------------

  // Configure all switches as open
  atom_sw_status[atom_count].set();

  am = sl.ana.getAtom(0,4);
  // Check physical (should always be ok) existance of EMBRPOS_D branch of
  // atom[0][4]. If so modify atom_sw_status according to current PCB implem.
  atom_sw_status[atom_count][0] = !am->embr_real_sw_sc(EMBRPOS_D);
  atom_sw_status[atom_count][1] = !am->embr_real_sw_mid(EMBRPOS_D);
  atom_sw_status[atom_count][2] = !am->embr_imag_sw_sc(EMBRPOS_D);
  atom_sw_status[atom_count][3] = !am->embr_imag_sw_mid(EMBRPOS_D);

  am = sl.ana.getAtom(0,1);
  // Check physical (should always be ok) existance of EMBRPOS_D branch of
  // atom[0][1]. If so modify atom_sw_status according to current PCB implem.
  atom_sw_status[atom_count][4] = !am->embr_real_sw_sc(EMBRPOS_D);
  atom_sw_status[atom_count][5] = !am->embr_real_sw_mid(EMBRPOS_D);
  atom_sw_status[atom_count][6] = !am->embr_imag_sw_sc(EMBRPOS_D);
  atom_sw_status[atom_count][7] = !am->embr_imag_sw_mid(EMBRPOS_D);

  am = sl.ana.getAtom(0,2);
  // Check physical (should always be ok) existance of EMBRPOS_D branch of
  // atom[0][2]. If so modify atom_sw_status according to current PCB implem.
  atom_sw_status[atom_count][ 8] = !am->embr_real_sw_sc(EMBRPOS_D);
  atom_sw_status[atom_count][ 9] = !am->embr_real_sw_mid(EMBRPOS_D);
  atom_sw_status[atom_count][10] = !am->embr_imag_sw_sc(EMBRPOS_D);
  atom_sw_status[atom_count][11] = !am->embr_imag_sw_mid(EMBRPOS_D);

  am = sl.ana.getAtom(0,3);
  // Check physical (should always be ok) existance of EMBRPOS_D branch of
  // atom[0][3]. If so modify atom_sw_status according to current PCB implem.
  atom_sw_status[atom_count][12] = !am->embr_real_sw_sc(EMBRPOS_D);
  atom_sw_status[atom_count][13] = !am->embr_real_sw_mid(EMBRPOS_D);
  atom_sw_status[atom_count][14] = !am->embr_imag_sw_sc(EMBRPOS_D);
  atom_sw_status[atom_count][15] = !am->embr_imag_sw_mid(EMBRPOS_D);

#ifdef VERBOSE_ENC
  for ( k = 0 ; k != atom_sw_status.size() ; ++k )
    cout << "atom_sw_status[" << k << "] = " << atom_sw_status[k] << endl;
#endif // VERBOSE_ENC

  // Fill switches_conf
  uint32_t temp;
  switches_conf.resize( static_cast<size_t>((atom_count+1)/2) );
  for ( k = 0 ; k != switches_conf.size() ; ++k ){
    temp = 0;
    temp |= static_cast<uint32_t>( atom_sw_status[2*k].to_ulong() );
    temp |= static_cast<uint32_t>( atom_sw_status[2*k+1].to_ulong() << 16 ) ;
    switches_conf[k] = temp;
  }

  return 0;
}

int encoder::detail::encode_ggot( Slice const& sl, vector<uint32_t>& ggot_conf ){
  ggot_conf.clear();

  size_t k;
  int32_t tempMSB, tempLSB, temp;

  // ***** ggot_conf *****
  // ----------- GOT gain -----------
  // 31  26 25        13 12         0
  // 000000 [Q3.10 imag] [Q3.10 real]
  // 000000 [ tempMSB  ] [ tempLSB  ]
  // 000000 [         temp          ]
  // --------------------------------
  size_t rows, cols;
  sl.ana.size(rows,cols);
  size_t atomCount = rows*cols;
  vector<pair<int,int> > pos = sl.dig.pipe_gen.position();
  double real_gain, imag_gain;
  for ( k = 0 ; k != atomCount ; ++k ){
    Atom const* am = sl.ana.getAtom(pos[k].first,pos[k].second);

    if ( k < sl.dig.pipe_gen.element_count() ){
      imag_gain = sl.ana.got_gain() * am->node_imag_adc_gain_corr();
      real_gain = sl.ana.got_gain() * am->node_real_adc_gain_corr();
    } else { // k >= sl.dig.pipe_gen.element_count()
      imag_gain = sl.ana.got_gain();
      real_gain = sl.ana.got_gain();
    }

    temp = 0;
    detail::form_word( imag_gain, 13, 10, true, &tempMSB );
    detail::form_word( real_gain, 13, 10, true, &tempLSB);
    temp = (tempMSB << 13) | (tempLSB);
    ggot_conf.push_back( static_cast<uint32_t>(temp) );
  }

  // --------- GOT offset ---------
  // 31    24 23      12 11       0
  // 00000000 [12b imag] [12b real]
  // 00000000 [tempMSB ] [tempLSB ]
  // 00000000 [       temp        ]
  // ------------------------------
  double real_offset, imag_offset;
  int32_t mask12 = (1 << 12) - 1;
  for ( k = 0 ; k != atomCount ; ++k ){
    Atom const* am = sl.ana.getAtom(pos[k].first,pos[k].second);

    if ( k < sl.dig.pipe_gen.element_count() ){
      imag_offset = sl.ana.got_offset() + am->node_imag_adc_offset_corr();
      real_offset = sl.ana.got_offset() + am->node_real_adc_offset_corr();
    } else { // k >= sl.dig.pipe_gen.element_count()
      imag_offset = sl.ana.got_offset();
      real_offset = sl.ana.got_offset();
    }

    temp = 0;
    tempMSB = static_cast<int32_t>( auxiliary::round(imag_offset/DAC_DEF_OUTMAX*pow(2,12)) );
    tempMSB &= mask12;
    tempLSB = static_cast<int32_t>( auxiliary::round(real_offset/DAC_DEF_OUTMAX*pow(2,12)) );
    tempLSB &= mask12;
    temp = (tempMSB << 12) | (tempLSB);
    ggot_conf.push_back( static_cast<uint32_t>(temp) );
  }

  return 0;
}

int encoder::detail::encode_PFpositions( Slice const& sl,
                                         vector<uint32_t>& pos_conf,
                                         vector<uint32_t>& slpos_conf ){
  pos_conf.clear();
  slpos_conf.clear();

  size_t k;
  int32_t temp = 0;
  int32_t temp5bit = 0;
  int32_t mask5bit = (1<<5)-1;  // 0b00000000000000000000000000011111

  // ***** generator positions *****
  for ( k = 0 ; k != 18 ; ++k ){

    // update position for k'th gen (5 bits: [0 unused] 1 - 24 [-31 unused])
    temp5bit = 0;
    if ( k < sl.dig.pipe_PQ.element_count() )
      temp5bit = static_cast<int32_t>(
          sl.dig.pipe_PQ.position()[k].first * sl.dig.pipe_PQ.hor_id_max() +
          sl.dig.pipe_PQ.position()[k].second + 1 );
    temp5bit &= mask5bit;

    // update position word with the position of the k'th gen (if any) - else 0
    temp |= ( temp5bit << (5 *(k%6)) );
    // push position word back into conf and reset temp
    if ( (k+1)%6 == 0 ){
      pos_conf.push_back( static_cast<uint32_t> (temp) );
      temp = 0;
    }
  }

  // ***** nodes number *****
  temp = 0;
  // number of PQ nodes
  temp5bit = static_cast<int32_t>( sl.dig.pipe_PQ.element_count() );
  temp5bit &= mask5bit;
  temp |= ( temp5bit << 0*5 );
  // zero
  temp5bit = 0;
  temp5bit &= mask5bit;
  temp |= ( temp5bit << 1*5 );
  // zero
  temp5bit = 0;
  temp5bit &= mask5bit;
  temp |= ( temp5bit << 2*5 );
  // slack node
  temp5bit = 1;
  temp5bit &= mask5bit;
  temp |= ( temp5bit << 3*5 );

  pos_conf.push_back( static_cast<uint32_t> (temp) );


  // ***** slack (represented as const I load) positions *****
  temp =  sl.dig.pipe_slack.position()[0].first * sl.dig.pipe_slack.hor_id_max()
          + sl.dig.pipe_slack.position()[0].second + 1;
  temp &= mask5bit;
  slpos_conf.push_back(static_cast<uint32_t>(temp));

  return 0;
}

int encoder::detail::encode_PFauxiliary( vector<uint32_t>& conf_conf,
                                         vector<uint32_t>& starter_conf ){
  conf_conf.clear();
  starter_conf.clear();

  uint32_t temp = 0U;
  stamp_NIOS_confirm(temp);
  conf_conf.push_back(temp);

  temp = 1050U;
  stamp_NIOS_confirm(temp);
  starter_conf.push_back(temp);

  return 0;
}

int encoder::detail::encode_iinit( Slice const& sl,
                                   vector<uint32_t>& icar_conf,
                                   vector<uint32_t>& ipol_conf ){
  // TODO
  return 1;
}

int encoder::detail::encode_PQsetpoints( Slice const& sl, vector<uint32_t>& pqset_conf ){
  // TODO
  return 1;
}

int encoder::detail::encode_TDgenerators( Slice const& sl,
                                          vector<uint32_t>& ggot_conf,
                                          vector<uint32_t>& gen_conf1,
                                          vector<uint32_t>& gen_conf2,
                                          vector<uint32_t>& gen_conf3 ){

  ggot_conf.clear();
  gen_conf1.clear();
  gen_conf2.clear();
  gen_conf3.clear();

  size_t k;
  int32_t tempMSB, tempLSB, temp;

  // ***** ggot_conf *****
  // ----------- GOT gain -----------
  // 31  26 25        13 12         0
  // 000000 [Q3.10 imag] [Q3.10 real]
  // 000000 [ tempMSB  ] [ tempLSB  ]
  // 000000 [         temp          ]
  // --------------------------------
  size_t rows, cols; sl.ana.size(rows,cols); size_t atomCount = rows*cols;
  vector<pair<int,int> > pos = sl.dig.pipe_gen.position();
  double real_gain, imag_gain;
  for ( k = 0 ; k != atomCount ; ++k ){

    if ( k < sl.dig.pipe_gen.element_count() ){
      Atom const* am = sl.ana.getAtom(pos[k].first,pos[k].second);
      imag_gain = sl.ana.got_gain() * am->node_imag_adc_gain_corr();
      real_gain = sl.ana.got_gain() * am->node_real_adc_gain_corr();
    } else { // k >= sl.dig.pipe_gen.element_count()
      imag_gain = sl.ana.got_gain();
      real_gain = sl.ana.got_gain();
    }

    temp = 0;
    detail::form_word( imag_gain, 13, 10, true, &tempMSB );
    detail::form_word( real_gain, 13, 10, true, &tempLSB);
    temp = (tempMSB << 13) | (tempLSB);
    ggot_conf.push_back( static_cast<uint32_t>(temp) );
  }

  // --------- GOT offset ---------
  // 31    24 23      12 11       0
  // 00000000 [12b imag] [12b real]
  // 00000000 [tempMSB ] [tempLSB ]
  // 00000000 [       temp        ]
  // ------------------------------
  double real_offset, imag_offset;
  int32_t mask12 = (1 << 12) - 1;
  for ( k = 0 ; k != atomCount ; ++k ){

    if ( k < sl.dig.pipe_gen.element_count() ){
      Atom const* am = sl.ana.getAtom(pos[k].first,pos[k].second);
      imag_offset = sl.ana.got_offset() + am->node_imag_adc_offset_corr();
      real_offset = sl.ana.got_offset() + am->node_real_adc_offset_corr();
    } else { // k >= sl.dig.pipe_gen.element_count()
      imag_offset = sl.ana.got_offset();
      real_offset = sl.ana.got_offset();
    }

    temp = 0;
    tempMSB = static_cast<int32_t>( auxiliary::round(imag_offset/DAC_DEF_OUTMAX*pow(2,12)) );
    tempMSB &= mask12;
    tempLSB = static_cast<int32_t>( auxiliary::round(real_offset/DAC_DEF_OUTMAX*pow(2,12)) );
    tempLSB &= mask12;
    temp = (tempMSB << 12) | (tempLSB);
    ggot_conf.push_back( static_cast<uint32_t>(temp) );
  }


  // ********** gen_conf1 **********
  // Inverse of the impedance Q9.7
  for ( k = 0 ; k != sl.dig.pipe_gen.element_count_max() ; ++k ){
    temp = 0;
    detail::form_word( sl.dig.pipe_gen.xd1inverse[k], 16, 7, false, &temp );
    gen_conf1.push_back( static_cast<uint32_t>(temp) );
  }

  // Initial current
  // 31          16 15           0
  // [Q5.11 imag_I] [Q5.11 real_I]
  // [  tempMBS   ] [  tempLSB   ]
  // [           temp            ]
  for ( k = 0 ; k != sl.dig.pipe_gen.element_count_max() ; ++k ){
    temp = 0;
    detail::form_word( sl.dig.pipe_gen.I0[k].imag(), 16, 11, true, &tempMSB );
    detail::form_word( sl.dig.pipe_gen.I0[k].real(), 16, 11, true, &tempLSB );
    temp = (tempMSB << 16) | (tempLSB);
    gen_conf1.push_back( static_cast<uint32_t>(temp) );
  }

  // Mechanical power Q5.13
  for ( k = 0 ; k != sl.dig.pipe_gen.element_count_max() ; ++k ){
    temp = 0;
    detail::form_word( sl.dig.pipe_gen.pMechanical[k], 18, 13, true, &temp );
    gen_conf1.push_back( static_cast<uint32_t>(temp) );
  }


  // ********** gen_conf2 **********
  // Gain 1 Q8.10
  for ( k = 0 ; k != sl.dig.pipe_gen.element_count_max() ; ++k ){
    temp = 0;
    detail::form_word( sl.dig.pipe_gen.gain1[k], 18, 10, true, &temp );
    gen_conf2.push_back( static_cast<uint32_t>(temp) );
  }

  // Gain 2 Q6.8
  for ( k = 0 ; k != sl.dig.pipe_gen.element_count_max() ; ++k ){
    temp = 0;
    detail::form_word( sl.dig.pipe_gen.gain2[k], 14, 8, true, &temp );
    gen_conf2.push_back( static_cast<uint32_t>(temp) );
  }

  // Gain 3 Q6.8
  for ( k = 0 ; k != sl.dig.pipe_gen.element_count_max() ; ++k ){
    temp = 0;
    detail::form_word( sl.dig.pipe_gen.gain3[k], 14, 8, true, &temp );
    gen_conf2.push_back( static_cast<uint32_t>(temp) );
  }

  // Gain 4 Q5.11
  for ( k = 0 ; k != sl.dig.pipe_gen.element_count_max() ; ++k ){
    temp = 0;
    detail::form_word( sl.dig.pipe_gen.gain4[k], 16, 11, true, &temp );
    gen_conf2.push_back( static_cast<uint32_t>(temp) );
  }

  // Gain 5 Q5.11
  for ( k = 0 ; k != sl.dig.pipe_gen.element_count_max() ; ++k ){
    temp = 0;
    detail::form_word( sl.dig.pipe_gen.gain5[k], 16, 11, true, &temp );
    gen_conf2.push_back( static_cast<uint32_t>(temp) );
  }

  // Gain 6 Q5.13
  for ( k = 0 ; k != sl.dig.pipe_gen.element_count_max() ; ++k ){
    temp = 0;
    detail::form_word( sl.dig.pipe_gen.gain6[k], 18, 13, true, &temp );
    gen_conf2.push_back( static_cast<uint32_t>(temp) );
  }


  // ********** gen_conf3 **********
  int64_t temp64;
  int64_t mask32 = (1LL << 32) - 1;
//  cout << "mask32 = " << mask32 << endl;
  // IG1 Past input values for generators: Accelerating power
  for ( k = 0 ; k != sl.dig.pipe_gen.element_count_max() ; ++k ){
//    cout << "******** Gen no " << k+1 << endl;
    temp64 = 0;
    detail::form_word( sl.dig.pipe_gen.pa0[k], 36, 23, true, &temp64 );
    tempLSB = static_cast<int32_t>( temp64 & mask32 );
    tempMSB = static_cast<int32_t>( (temp64 >> 32) & mask32 );
//    cout << "tempLSB = " << tempLSB << endl;
//    cout << "tempMSB = " << tempMSB << endl;
    gen_conf3.push_back( static_cast<uint32_t>(tempLSB) );
    gen_conf3.push_back( static_cast<uint32_t>(tempMSB) );
//    cout << endl;
  }

  // IG1 Past output values for generators: Omega
  for ( k = 0 ; k != sl.dig.pipe_gen.element_count_max() ; ++k ){
    temp64 = 0LL;
    detail::form_word( sl.dig.pipe_gen.omega0[k], 46, 44, true, &temp64 );
    tempLSB = static_cast<int32_t>( temp64 & mask32 );
    tempMSB = static_cast<int32_t>( (temp64 >> 32) & mask32 );
    gen_conf3.push_back( static_cast<uint32_t>(tempLSB) );
    gen_conf3.push_back( static_cast<uint32_t>(tempMSB) );
  }

  // IG2 Past input values for generators: Omega
  for ( k = 0 ; k != sl.dig.pipe_gen.element_count_max() ; ++k ){
    temp64 = 0LL;
    detail::form_word( sl.dig.pipe_gen.omega0[k], 46, 44, true, &temp64 );
    tempLSB = static_cast<int32_t>( temp64 & mask32 );
    tempMSB = static_cast<int32_t>( (temp64 >> 32) & mask32 );
    gen_conf3.push_back( static_cast<uint32_t>(tempLSB) );
    gen_conf3.push_back( static_cast<uint32_t>(tempMSB) );
  }

  // IG2 Past output values for generators: Delta
  for ( k = 0 ; k != sl.dig.pipe_gen.element_count_max() ; ++k ){
    temp64 = 0LL;
    detail::form_word( sl.dig.pipe_gen.delta0[k]*2/M_PI, 54, 52, true, &temp64 );
    tempLSB = static_cast<int32_t>( temp64 & mask32 );
    tempMSB = static_cast<int32_t>( (temp64 >> 32) & mask32 );
    gen_conf3.push_back( static_cast<uint32_t>(tempLSB) );
    gen_conf3.push_back( static_cast<uint32_t>(tempMSB) );
  }

  return 0;
}

int encoder::detail::encode_TDzloads( Slice const& sl,
                                    vector<uint32_t>& zgot_conf,
                                    vector<uint32_t>& zloads_conf ){

  zgot_conf.clear();
  zloads_conf.clear();

  size_t k;
  int32_t tempMSB, tempLSB, temp;

  // ***** zgot_conf *****
  // ----------- GOT gain -----------
  // 31  26 25        13 12         0
  // 000000 [Q3.10 imag] [Q3.10 real]
  // 000000 [ tempMSB  ] [ tempLSB  ]
  // 000000 [         temp          ]
  // --------------------------------
  size_t rows, cols;
  sl.ana.size(rows,cols);
  size_t atomCount = rows*cols;
  vector<pair<int,int> > pos = sl.dig.pipe_zload.position();
  double real_gain, imag_gain;
  for ( k = 0 ; k != atomCount ; ++k ){

    if ( k < sl.dig.pipe_zload.element_count() ){
      Atom const* am = sl.ana.getAtom(pos[k].first,pos[k].second);
      imag_gain = sl.ana.got_gain() * am->node_imag_adc_gain_corr();
      real_gain = sl.ana.got_gain() * am->node_real_adc_gain_corr();
    } else { // k >= sl.dig.pipe_zload.element_count()
      imag_gain = sl.ana.got_gain();
      real_gain = sl.ana.got_gain();
    }

    temp = 0;
    detail::form_word( imag_gain, 13, 10, true, &tempMSB );
    detail::form_word( real_gain, 13, 10, true, &tempLSB);
    temp = (tempMSB << 13) | (tempLSB);
    zgot_conf.push_back( static_cast<uint32_t>(temp) );
  }

  // ------------ GOT offset ------------
  // 31    24 23         12 11          0
  // 00000000 [12b tempMSB] [12b tempLSB]
  // 00000000 [          temp           ]
  // ------------------------------------
  double real_offset, imag_offset;
  int32_t mask12 = (1 << 12) - 1;
  for ( k = 0 ; k != atomCount ; ++k ){

    if ( k < sl.dig.pipe_zload.element_count() ){
      Atom const* am = sl.ana.getAtom(pos[k].first,pos[k].second);
      imag_offset = sl.ana.got_offset() + am->node_imag_adc_offset_corr();
      real_offset = sl.ana.got_offset() + am->node_real_adc_offset_corr();
    } else { // k >= sl.dig.pipe_zload.element_count()
      imag_offset = sl.ana.got_offset();
      real_offset = sl.ana.got_offset();
    }

    temp = 0;
    tempMSB = static_cast<int32_t>( auxiliary::round(imag_offset/DAC_DEF_OUTMAX*pow(2,12)) );
    tempMSB &= mask12;
    tempLSB = static_cast<int32_t>( auxiliary::round(real_offset/DAC_DEF_OUTMAX*pow(2,12)) );
    tempLSB &= mask12;
    temp = (tempMSB << 12) | (tempLSB);
    zgot_conf.push_back( static_cast<uint32_t>(temp) );
  }

  // ***** zloads_conf *****
  // 31          16 15           0
  // [Q3.13 imag_Y] [Q3.13 real_Y]
  // [  tempMBS   ] [  tempLSB   ]
  // [           temp            ]
  for ( k = 0 ; k != sl.dig.pipe_zload.element_count() ; ++k ){
    detail::form_word( sl.dig.pipe_zload.Yconst[k].real(), 16, 13, true, &tempLSB );
    detail::form_word( sl.dig.pipe_zload.Yconst[k].imag(), 16, 13, true, &tempMSB );
    temp = (tempMSB << 16) | (tempLSB);
    zloads_conf.push_back( static_cast<uint32_t>(temp) );
#ifdef VERBOSE_ENC
    cout << "Real Y = " << sl.dig.pipe_zload.real_Y()[k] << endl;
    cout << "tempLSB = " << tempLSB << endl;
    cout << "Imag Y = " << sl.dig.pipe_zload.imag_Y()[k] << endl;
    cout << "tempMSB = " << tempMSB << endl;
    cout << "temp (MSB+LSB) = " << temp << endl;
#endif // VERBOSE_ENC
  }
  for ( k = sl.dig.pipe_zload.element_count();
        k != sl.dig.pipe_zload.element_count_max() ; ++k )
    zloads_conf.push_back( static_cast<uint32_t>(0) );

  return 0;
}

int encoder::detail::encode_TDiloads( Slice const& sl,
                                    vector<uint32_t>& igot_conf,
                                    vector<uint32_t>& iloads_conf ){

  igot_conf.clear();
  iloads_conf.clear();

  size_t k, pseudo_id;
  int32_t tempMSB, tempLSB, temp;

  // ***** igot_conf *****
  // ----------- GOT gain -----------
  // 31  26 25        13 12         0
  // 000000 [Q3.10 imag] [Q3.10 real]
  // 000000 [ tempMSB  ] [ tempLSB  ]
  // 000000 [         temp          ]
  // --------------------------------
  size_t rows, cols;
  sl.ana.size(rows,cols);
  size_t atomCount = rows*cols;
  vector<pair<int,int> > pos = sl.dig.pipe_iload.position();
  double real_gain, imag_gain;
  for ( k = 0 ; k != atomCount ; ++k ){

    if ( k < sl.dig.pipe_iload.element_count() ){
      Atom const* am = sl.ana.getAtom(pos[k].first,pos[k].second);
      imag_gain = sl.ana.got_gain() * am->node_imag_adc_gain_corr();
      real_gain = sl.ana.got_gain() * am->node_real_adc_gain_corr();
    } else { // k >= sl.dig.pipe_iload.element_count()
      imag_gain = sl.ana.got_gain();
      real_gain = sl.ana.got_gain();
    }

    temp = 0;
    detail::form_word( imag_gain, 13, 10, true, &tempMSB );
    detail::form_word( real_gain, 13, 10, true, &tempLSB);
    temp = (tempMSB << 13) | (tempLSB);
    igot_conf.push_back( static_cast<uint32_t>(temp) );
  }

  // ------------ GOT offset ------------
  // 31    24 23         12 11          0
  // 00000000 [12b tempMSB] [12b tempLSB]
  // 00000000 [          temp           ]
  // ------------------------------------
  double real_offset, imag_offset;
  int32_t mask12 = (1 << 12) - 1;
  for ( k = 0 ; k != atomCount ; ++k ){

    if ( k < sl.dig.pipe_iload.element_count() ){
      Atom const* am = sl.ana.getAtom(pos[k].first,pos[k].second);
      imag_offset = sl.ana.got_offset() + am->node_imag_adc_offset_corr();
      real_offset = sl.ana.got_offset() + am->node_real_adc_offset_corr();
    } else { // k >= sl.dig.pipe_iload.element_count()
      imag_offset = sl.ana.got_offset();
      real_offset = sl.ana.got_offset();
    }

    temp = 0;
    tempMSB = static_cast<int32_t>( auxiliary::round(imag_offset/DAC_DEF_OUTMAX*pow(2,12)) );
    tempMSB &= mask12;
    tempLSB = static_cast<int32_t>( auxiliary::round(real_offset/DAC_DEF_OUTMAX*pow(2,12)) );
    tempLSB &= mask12;
    temp = (tempMSB << 12) | (tempLSB);
    igot_conf.push_back( static_cast<uint32_t>(temp) );
  }

  // ***** iloads_conf *****
  // 31    24 23          12 11           0
  // 00000000 [Q5.7  imag_I] [Q5.7  real_I]
  // 00000000 [  tempMBS   ] [  tempLSB   ]
  // 00000000 [           temp            ]
  // Corresponds to the current that is to be injected to the grid on the first
  // iteration of the emulation. So, ideally this would be good to correspond to
  // the steady state current [pu] in the powersystem. This applies to ANY
  // element, be it an iload, a gen, or whatever
  iloads_conf.resize( sl.dig.pipe_iload.element_count_max(), 0 );

  // Note 1: for xloads the pipe_xload convention is that there is a flow INTO
  // the loads, whereas for the real emulator DAC, the convention is that there
  // is a flow OUT OF the DAC (into the grid). Therefore, currents of loads have
  // to be negated.

  // Note 2: for all injections the real part of the current is negated, because
  // starting from Z*I=V -> (R+jX)*(I_R+jI_I)=(V_R+jV_I), finally the following
  // holds:
  //   (1/X) * V_I =  I_R
  //   (1/X) * V_R = -I_I

  // --- Initial currents of the generators ---
  for ( k = 0 ; k != sl.dig.pipe_gen.element_count() ; ++k ){
    pseudo_id = static_cast<size_t>(sl.dig.pipe_gen.position()[k].first) *
                sl.dig.pipe_gen.hor_id_max() +
                static_cast<size_t>(sl.dig.pipe_gen.position()[k].second);
    detail::form_word(   sl.dig.pipe_gen.I0[k].real()  , 12, 7, true, &tempLSB );
    detail::form_word(  -sl.dig.pipe_gen.I0[k].imag()  , 12, 7, true, &tempMSB );
    temp = (tempMSB << 12) | (tempLSB);
    iloads_conf[pseudo_id] = static_cast<uint32_t>(temp);
  }

  // --- Initial currents of the constant current loads (iloads) ---
  for ( k = 0 ; k != sl.dig.pipe_iload.element_count() ; ++k ){
    pseudo_id = static_cast<size_t>(sl.dig.pipe_iload.position()[k].first) *
                sl.dig.pipe_iload.hor_id_max() +
                static_cast<size_t>(sl.dig.pipe_iload.position()[k].second);
    detail::form_word(    -sl.dig.pipe_iload.Iconst[k].real()   , 12, 7, true, &tempLSB );
    detail::form_word(  -(-sl.dig.pipe_iload.Iconst[k].imag())  , 12, 7, true, &tempMSB );
    temp = (tempMSB << 12) | (tempLSB);
    iloads_conf[pseudo_id] = static_cast<uint32_t>(temp);
  }

  // --- Initial currents of the constant impedance loads (zloads) ---
  for ( k = 0 ; k != sl.dig.pipe_zload.element_count() ; ++k ){
    pseudo_id = static_cast<size_t>(sl.dig.pipe_zload.position()[k].first) *
                sl.dig.pipe_zload.hor_id_max() +
                static_cast<size_t>(sl.dig.pipe_zload.position()[k].second);
    detail::form_word(    -sl.dig.pipe_zload.I0[k].real()   , 12, 7, true, &tempLSB );
    detail::form_word(  -(-sl.dig.pipe_zload.I0[k].imag())  , 12, 7, true, &tempMSB );
    temp = (tempMSB << 12) | (tempLSB);
    iloads_conf[pseudo_id] = static_cast<uint32_t>(temp);
  }
  // --- Initial currents of the constant power loads (ploads) ---
  for ( k = 0 ; k != sl.dig.pipe_pload.element_count() ; ++k ){
    pseudo_id = static_cast<size_t>(sl.dig.pipe_pload.position()[k].first) *
                sl.dig.pipe_pload.hor_id_max() +
                static_cast<size_t>(sl.dig.pipe_pload.position()[k].second);
    detail::form_word(    -sl.dig.pipe_pload.I0[k].real()   , 12, 7, true, &tempLSB );
    detail::form_word(  -(-sl.dig.pipe_pload.I0[k].imag())  , 12, 7, true, &tempMSB );
    temp = (tempMSB << 12) | (tempLSB);
    iloads_conf[pseudo_id] = static_cast<uint32_t>(temp);
  }

  // stamp NIOS confirmation on the last word of iloads_conf
  stamp_NIOS_confirm( iloads_conf.back() );

  return 0;
}

int encoder::detail::encode_TDploads( Slice const& sl, vector<uint32_t>& ploads_conf ){

  ploads_conf.clear();

  size_t k;
  int32_t tempMSB, tempLSB, temp;

  // ***** ploads_conf *****
  // 31          16 15           0
  // [Q2.14     Q ] [Q2.14     P ]
  // [  tempMBS   ] [  tempLSB   ]
  // [           temp            ]
  for ( k = 0 ; k != sl.dig.pipe_pload.element_count() ; ++k ){
    detail::form_word( sl.dig.pipe_pload.Sconst[k].real(), 16, 14, true, &tempLSB );
    detail::form_word( sl.dig.pipe_pload.Sconst[k].imag(), 16, 14, true, &tempMSB );
    temp = (tempMSB << 16) | (tempLSB);
    ploads_conf.push_back( static_cast<uint32_t>(temp) );
#ifdef VERBOSE_ENC
    cout << "P = " << sl.dig.pipe_pload.P()[k] << endl;
    cout << "tempLSB = " << tempLSB << endl;
    cout << "Q = " << sl.dig.pipe_pload.Q()[k] << endl;
    cout << "tempMSB = " << tempMSB << endl;
    cout << "temp (MSB+LSB) = " << temp << endl;
#endif // VERBOSE_ENC
  }
  for ( k = sl.dig.pipe_pload.element_count();
        k != sl.dig.pipe_pload.element_count_max() ; ++k )
    ploads_conf.push_back( static_cast<uint32_t>(0) );

  return 0;
}

int encoder::detail::encode_TDpositions( Slice const& sl, vector<uint32_t>& pos_conf ){

  pos_conf.clear();

  size_t k;
  int32_t temp = 0;
  int32_t temp5bit = 0;
  int32_t mask5bit = (1<<5)-1;  // 0b00000000000000000000000000011111

  // ***** generator positions *****
  for ( k = 0 ; k != 18 ; ++k ){

    // update position for k'th gen (5 bits: [0 unused] 1 - 24 [-31 unused])
    temp5bit = 0;
    if ( k < sl.dig.pipe_gen.element_count() )
      temp5bit = static_cast<int32_t>(
          sl.dig.pipe_gen.position()[k].first * sl.dig.pipe_gen.hor_id_max() +
          sl.dig.pipe_gen.position()[k].second + 1 );
    temp5bit &= mask5bit;

//    cout << "gen no " << k+1 << ": " << temp5bit << endl; // DEBUG
    // update position word with the position of the k'th gen (if any) - else 0
    temp |= ( temp5bit << (5 *(k%6)) );
    // push position word back into conf and reset temp
    if ( (k+1)%6 == 0 ){
      pos_conf.push_back( static_cast<uint32_t> (temp) );
      temp = 0;
    }
  }

  // ***** gens/loads number *****
  temp = 0;
  // number of generators
  temp5bit = static_cast<int32_t>( sl.dig.pipe_gen.element_count() );
  temp5bit &= mask5bit;
  temp |= ( temp5bit << 0*5 );
  // number of const Z loads
  temp5bit = static_cast<int32_t>( sl.dig.pipe_zload.element_count() );
  temp5bit &= mask5bit;
  temp |= ( temp5bit << 1*5 );
  // number of const P loads
  temp5bit = static_cast<int32_t>( sl.dig.pipe_pload.element_count() );
  temp5bit &= mask5bit;
  temp |= ( temp5bit << 2*5 );
  // number of const I loads
  temp5bit = static_cast<int32_t>( sl.dig.pipe_iload.element_count() );
  temp5bit &= mask5bit;
  temp |= ( temp5bit << 3*5 );

  pos_conf.push_back( static_cast<uint32_t> (temp) );

  // ***** const Z load positions *****
  temp = 0;
  for ( k = 0 ; k != sl.dig.pipe_zload.element_count_max() ; ++k ){

    // update position for k'th load (5 bits: [0 unused] 1 - 24 [-31 unused])
    temp5bit = 0;
    if ( k < sl.dig.pipe_zload.element_count() )
      temp5bit = static_cast<int32_t>(
          sl.dig.pipe_zload.position()[k].first * sl.dig.pipe_zload.hor_id_max() +
          sl.dig.pipe_zload.position()[k].second + 1 );
    temp5bit &= mask5bit;

//    cout << "cil no " << k+1 << ": " << temp5bit << endl; // DEBUG
    // update position word with the position of the k'th load (if any) - else 0
    temp |= ( temp5bit << (5 *(k%6)) );
    // push position word back into conf and reset temp
    if ( (k+1)%6 == 0 ){
      pos_conf.push_back( static_cast<uint32_t> (temp) );
      temp = 0;
    }
  }
  --k; // to compensate for the increment in the last for loop
  // flush remaining entries into pos_conf
  if ( (k+1)%6 != 0 ){
    pos_conf.push_back( static_cast<uint32_t> (temp) );
    temp = 0;
  }

  // ***** const P load positions *****
  temp = 0;
  for ( k = 0 ; k != sl.dig.pipe_pload.element_count_max() ; ++k ){

    // update position for k'th load (5 bits: [0 unused] 1 - 24 [-31 unused])
    temp5bit = 0;
    if ( k < sl.dig.pipe_pload.element_count() )
      temp5bit = static_cast<int32_t>(
          sl.dig.pipe_pload.position()[k].first * sl.dig.pipe_pload.hor_id_max() +
          sl.dig.pipe_pload.position()[k].second + 1 );
    temp5bit &= mask5bit;

//    cout << "cpl no " << k+1 << ": " << temp5bit << endl; // DEBUG
    // update position word with the position of the k'th load (if any) - else 0
    temp |= ( temp5bit << (5 *(k%6)) );
    // push position word back into conf and reset temp
    if ( (k+1)%6 == 0 ){
      pos_conf.push_back( static_cast<uint32_t> (temp) );
      temp = 0;
    }
  }
  --k; // to compensate for the increment in the last for loop
  // flush remaining entries into pos_conf
  if ( (k+1)%6 != 0 ){
    pos_conf.push_back( static_cast<uint32_t> (temp) );
    temp = 0;
  }

  // ***** const I load positions *****
  temp = 0;
  for ( k = 0 ; k != sl.dig.pipe_iload.element_count_max() ; ++k ){

    // update position for k'th load (5 bits: [0 unused] 1 - 24 [-31 unused])
    temp5bit = 0;
    if ( k < sl.dig.pipe_iload.element_count() )
      temp5bit = static_cast<int32_t>(
          sl.dig.pipe_iload.position()[k].first * sl.dig.pipe_iload.hor_id_max() +
          sl.dig.pipe_iload.position()[k].second + 1 );
    temp5bit &= mask5bit;

//    cout << "ccl no " << k+1 << ": " << temp5bit << endl; // DEBUG
    // update position word with the position of the k'th load (if any) - else 0
    temp |= ( temp5bit << (5 *(k%6)) );
    // push position word back into conf and reset temp
    if ( (k+1)%6 == 0 ){
      pos_conf.push_back( static_cast<uint32_t> (temp) );
      temp = 0;
    }
  }
  --k; // to compensate for the increment in the last for loop
  // flush remaining entries into pos_conf
  if ( (k+1)%6 != 0 ){
    pos_conf.push_back( static_cast<uint32_t> (temp) );
    temp = 0;
  }

  // stamp NIOS confirmation on the last word of pos_conf
  stamp_NIOS_confirm( pos_conf.back() );

  return 0;
}

int encoder::detail::encode_TDauxiliary( Slice const& sl,
                                       vector<uint32_t>& pert_conf ){

  pert_conf.clear();
  int32_t temp;

  // ***** pert_conf *****
  // Pushing back placeholder words, to be updated
  temp = 0;
  pert_conf.push_back( static_cast<uint32_t>(temp) ); // 552: pert start
  pert_conf.push_back( static_cast<uint32_t>(temp) ); // 553: pert stop
  pert_conf.push_back( static_cast<uint32_t>(temp) ); // 554: pert sw conf
  stamp_NIOS_confirm(temp);
  pert_conf.push_back( static_cast<uint32_t>(temp) ); // 555: not used (NIOS stamped)

  return 0;
}

int encoder::detail::form_word( double val,
                                size_t total_bits,
                                size_t decimal_bits,
                                bool isSigned,
                                int32_t* pword ){

  if ( total_bits > 32)
    // call detail::form_word(..., int64_t* word) instead!
    return 1;
  if ( decimal_bits >= total_bits )
    // invalid number of decimal bits!
    return 2;
  if ( val < 0 && !isSigned )
    // invalid arguments! 'val' is negative and flag 'isSigned' set to false
    return 3;

  int32_t limit_u = (1 << total_bits) - 1;
  int32_t limit_s_hi = (+1 << (total_bits-1) ) - 1;
  int32_t limit_s_lo = (-2 << (total_bits-2) );
  int32_t mask = limit_u;
  int32_t tempword = static_cast<int32_t>( val * (1 << decimal_bits) );
#ifdef VERBOSE_ENC
  cout << "encoder::detail::form_word" << endl;
  cout << "Input arguments" << endl;
  cout << " val = " << val << endl;
  cout << " total bits = " << total_bits << endl;
  cout << " decimal bits = " << decimal_bits << endl;
  cout << " isSigned = " << isSigned << endl;
  cout << "Internal variables" << endl;
  cout << " limit_u = " << limit_u << endl;
  cout << " limit_s_hi = " << limit_s_hi << endl;
  cout << " limit_s_lo = " << limit_s_lo << endl;
  cout << " mask = " << mask << endl;
  cout << " tempword (unlimited) = " << tempword << endl;
#endif // VERBOSE_ENC
  if (isSigned){
    if ( tempword > limit_s_hi )
      tempword = limit_s_hi;
    else if ( tempword < limit_s_lo )
      tempword = limit_s_lo;
  } else{
    if ( tempword > limit_u )
      tempword = limit_u;
  }
#ifdef VERBOSE_ENC
  cout << " tempword (unmasked) = " << tempword << endl;
#endif // VERBOSE_ENC
  tempword &= mask;
#ifdef VERBOSE_ENC
  cout << " tempword (masked) = " << tempword << endl;
#endif // VERBOSE_ENC
  *pword = tempword;

  return 0;
}

int encoder::detail::form_word( double val,
                                size_t total_bits,
                                size_t decimal_bits,
                                bool isSigned,
                                int64_t* pword ){

  if ( total_bits > 64)
    // invalid number of total bits!
    return 1;
  if ( decimal_bits >= total_bits )
    // invalid number of decimal bits!
    return 2;
  if ( val < 0 && !isSigned )
    // invalid arguments! 'val' is negative and flag 'isSigned' set to false
    return 3;

  int64_t limit_u = (1LL << total_bits) - 1LL;
  int64_t limit_s_hi = (+1LL << (total_bits-1) ) - 1LL;
  int64_t limit_s_lo = (-2LL << (total_bits-2) );
  int64_t mask = limit_u;
  int64_t tempword = static_cast<int64_t>( val * ( 1LL << decimal_bits) );
#ifdef VERBOSE_ENC
  cout << "encoder::detail::form_word (64bit)" << endl;
  cout << "Input arguments" << endl;
  cout << " val = " << val << endl;
  cout << " total bits = " << total_bits << endl;
  cout << " decimal bits = " << decimal_bits << endl;
  cout << " isSigned = " << isSigned << endl;
  cout << "Internal variables" << endl;
  cout << " limit_u = " << limit_u << endl;
  cout << " limit_s_hi = " << limit_s_hi << endl;
  cout << " limit_s_lo = " << limit_s_lo << endl;
  cout << " mask = " << mask << endl;
  cout << " tempword (unlimited) = " << tempword << endl;
#endif // VERBOSE_ENC
  if (isSigned){
    if ( tempword > limit_s_hi )
      tempword = limit_s_hi;
    else if ( tempword < limit_s_lo )
      tempword = limit_s_lo;
  } else{
    if ( tempword > limit_u )
      tempword = limit_u;
  }
#ifdef VERBOSE_ENC
  cout << " tempword (unmasked) = " << tempword << endl;
#endif // VERBOSE_ENC
  tempword &= mask;
#ifdef VERBOSE_ENC
  cout << " tempword (masked) = " << tempword << endl;
#endif // VERBOSE_ENC
  *pword = tempword;

  return 0;
}
