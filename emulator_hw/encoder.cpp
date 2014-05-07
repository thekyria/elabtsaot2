
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
#include <cmath>            // M_PI, double pow(double,double)
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

int encoder::encodeSliceGPF(Slice const& sl, vector<uint32_t>& sliceConf){

  sliceConf.clear();

  int ans(0);

  // Vector        | vec.add | cyp.add [num.of.words] corresponing func.
  //---------------|---------|-------------------------------------------------
  // adcGain_conf  |   0: 23 |   1: 24 [ 24] detail::encode_GPFADCDACgot
  // adcOffset_conf|  24: 47 |  25: 48 [ 24] detail::encode_GPFADCDACgot
  // dacGain_conf  |  48: 71 |  49: 72 [ 24] detail::encode_GPFADCDACgot
  // dacOffset_conf|  72: 95 |  73: 96 [ 24] detail::encode_GPFADCDACgot
  //  none         |  96:341 |  97:342 [246]  none
  // nodeCount_conf|     342 |     343 [  1] detail::encode_GPFpositions
  // PQpos_conf    | 343:346 | 344:347 [  4] detail::encode_GPFpositions
  //  none         | 347:350 | 348:351 [  4]  none
  // slpos_conf    | 351:354 | 352:355 [  4] detail::encode_GPFpositions
  // vref_conf     | 355:356 | 356:357 [  2] detail::encode_vref
  // icar_conf     | 357:380 | 358:381 [ 24] detail::encode_GPFIinit
  // starter_conf  |     381 |     382 [  1]  none (placeholder words)
  // res_conf      | 382:485 | 383:486 [104] detail::encode_resistors
  // res_tcon_conf | 486:537 | 487:538 [ 52] detail::encode_resistors
  // switches_conf | 538:550 | 539:551 [ 13] detail::encode_switches
  //  none         | 551:553 | 552:554 [  3]  none
  // conf_conf     |     554 |     555 [  1] detail::encode_GPFauxiliary
  // pqset_conf    | 555:578 | 556:579 [ 24] detail::encode_GPFPQsetpoints
  // ipol_conf     | 579:602 | 580:603 [ 24] detail::encode_GPFIinit
  //  none         | 603:606 | 604:607 [  4]  none
  // nios_conf     |     607 |     608 [  1] detail::encode_GPFauxiliary
  //  none         |     608 |     609 [  1]  none

  vector<uint32_t> adcGain_conf;
  vector<uint32_t> adcOffset_conf;
  vector<uint32_t> dacGain_conf;
  vector<uint32_t> dacOffset_conf;
  vector<uint32_t> none;
  vector<uint32_t> nodeCount_conf;
  vector<uint32_t> PQpos_conf;
  vector<uint32_t> slpos_conf;
  vector<uint32_t> conf_conf;
  vector<uint32_t> vref_conf;
  vector<uint32_t> icar_conf;
  vector<uint32_t> res_conf;
  vector<uint32_t> res_tcon_conf;
  vector<uint32_t> switches_conf;
  vector<uint32_t> pqset_conf;
  vector<uint32_t> ipol_conf;
  vector<uint32_t> nios_conf;

  ans |= detail::encode_GPFADCDACgot(sl, adcGain_conf, adcOffset_conf, dacGain_conf, dacOffset_conf);
  ans |= detail::encode_GPFpositions(sl, nodeCount_conf, PQpos_conf, slpos_conf);
  detail::encode_GPFauxiliary(conf_conf, nios_conf);
  ans |= detail::encode_vref(sl, vref_conf);
  detail::encode_GPFIinit(sl, icar_conf, ipol_conf);
  ans |= detail::encode_resistors(sl, res_conf, res_tcon_conf );
  ans |= detail::encode_switches(sl, switches_conf);
  detail::encode_GPFPQsetpoints(sl, pqset_conf);
  if (ans) return ans;

  sliceConf.insert(sliceConf.end(), adcGain_conf.begin(), adcGain_conf.end() );
  sliceConf.insert(sliceConf.end(), adcOffset_conf.begin(), adcOffset_conf.end() );
  sliceConf.insert(sliceConf.end(), dacGain_conf.begin(), dacGain_conf.end() );
  sliceConf.insert(sliceConf.end(), dacOffset_conf.begin(), dacOffset_conf.end() );
  none.resize(246,0); sliceConf.insert(sliceConf.end(), none.begin(), none.end() );
  sliceConf.insert(sliceConf.end(), nodeCount_conf.begin(), nodeCount_conf.end() );
  sliceConf.insert(sliceConf.end(), PQpos_conf.begin(), PQpos_conf.end() );
  none.resize(4,0); sliceConf.insert(sliceConf.end(), none.begin(), none.end() );
  sliceConf.insert(sliceConf.end(), slpos_conf.begin(), slpos_conf.end() );
  sliceConf.insert(sliceConf.end(), vref_conf.begin(), vref_conf.end() );
  sliceConf.insert(sliceConf.end(), icar_conf.begin(), icar_conf.end() );
  none.resize(1,0); sliceConf.insert(sliceConf.end(), none.begin(), none.end() );
  sliceConf.insert(sliceConf.end(), res_conf.begin(), res_conf.end() );
  sliceConf.insert(sliceConf.end(), res_tcon_conf.begin(), res_tcon_conf.end() );
  sliceConf.insert(sliceConf.end(), switches_conf.begin(), switches_conf.end() );
  none.resize(3,0); sliceConf.insert(sliceConf.end(), none.begin(), none.end() );
  sliceConf.insert(sliceConf.end(), conf_conf.begin(), conf_conf.end() );
  sliceConf.insert(sliceConf.end(), pqset_conf.begin(), pqset_conf.end() );
  sliceConf.insert(sliceConf.end(), ipol_conf.begin(), ipol_conf.end() );
  none.resize(4,0); sliceConf.insert(sliceConf.end(), none.begin(), none.end() );
  sliceConf.insert(sliceConf.end(), nios_conf.begin(), nios_conf.end() );
  none.resize(1,0); sliceConf.insert(sliceConf.end(), none.begin(), none.end() );

  return 0;
}

int encoder::encodeSliceDCPF(Slice const& sl, vector<uint32_t>& sliceConf){
  sliceConf.clear();
  int ans(0);
  // Vector        | vec.add | cyp.add [num.of.words] corresponing func.
  //---------------|---------|-------------------------------------------------
  // got_conf      |   0: 47 |   1: 48 [ 48] detail::encode_DCPFgot
  //  none         |  48:338 |  49:339 [291]
  // ppos_conf     | 339:342 | 340:355 [  4] detail::encode_DCPFpositions
  //  none         | 343:350 | 344:351 [  8]
  // thpos_conf    | 351:354 | 352:355 [  4] detail::encode_DCPFpositions
  // vref_conf     | 355:356 | 356:357 [  2] detail::encode_vref
  // i_conf        | 357:380 | 358:381 [ 24] detail::encode_DCPFI
  // starter_conf  |     381 |     382 [  1] detail::encode_DCPFauxiliary
  // res_conf      | 382:485 | 383:486 [104] detail::encode_resistors
  // res_tcon_conf | 486:537 | 487:538 [ 52] detail::encode_resistors
  // switches_conf | 538:550 | 539:551 [ 13] detail::encode_switches
  //  none         | 551:553 | 552:554 [  3]
  // conf_conf     |     554 |     555 [  1] detail::encode_DCPFauxiliary
  //  none         | 555:605 | 556:606 [ 51]
  // nios_conf     | 606:607 | 607:608 [  2] detail::encode_DCPFauxiliary
  vector<uint32_t> got_conf;
  vector<uint32_t> none;
  vector<uint32_t> ppos_conf;
  vector<uint32_t> thpos_conf;
  vector<uint32_t> vref_conf;
  vector<uint32_t> i_conf;
  vector<uint32_t> starter_conf;
  vector<uint32_t> res_conf;
  vector<uint32_t> res_tcon_conf;
  vector<uint32_t> switches_conf;
  vector<uint32_t> conf_conf;
  vector<uint32_t> nios_conf;

  ans |= detail::encode_DCPFgot(sl, got_conf);
  ans |= detail::encode_DCPFpositions(sl, ppos_conf, thpos_conf);
  ans |= detail::encode_vref(sl, vref_conf);
  detail::encode_DCPFI(sl, i_conf);
  detail::encode_DCPFauxiliary(sl, starter_conf, conf_conf, nios_conf);
  ans |= detail::encode_resistors(sl, res_conf, res_tcon_conf );
  ans |= detail::encode_switches(sl, switches_conf);


  sliceConf.insert(sliceConf.end(), got_conf.begin(), got_conf.end() );
  none.resize(291,0); sliceConf.insert(sliceConf.end(), none.begin(), none.end() );
  sliceConf.insert(sliceConf.end(), ppos_conf.begin(), ppos_conf.end() );
  none.resize(  8,0); sliceConf.insert(sliceConf.end(), none.begin(), none.end() );
  sliceConf.insert(sliceConf.end(), thpos_conf.begin(), thpos_conf.end() );
  sliceConf.insert(sliceConf.end(), vref_conf.begin(), vref_conf.end() );
  sliceConf.insert(sliceConf.end(), i_conf.begin(), i_conf.end() );
  sliceConf.insert(sliceConf.end(), starter_conf.begin(), starter_conf.end() );
  sliceConf.insert(sliceConf.end(), res_conf.begin(), res_conf.end() );
  sliceConf.insert(sliceConf.end(), res_tcon_conf.begin(), res_tcon_conf.end() );
  sliceConf.insert(sliceConf.end(), switches_conf.begin(), switches_conf.end() );
  none.resize(  3,0); sliceConf.insert(sliceConf.end(), none.begin(), none.end() );
  sliceConf.insert(sliceConf.end(), conf_conf.begin(), conf_conf.end() );
  none.resize( 51,0); sliceConf.insert(sliceConf.end(), none.begin(), none.end() );
  sliceConf.insert(sliceConf.end(), nios_conf.begin(), nios_conf.end() );

  return 0;
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
  ans |= detail::encode_TDauxiliary( pert_conf );
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

int encoder::detail::encode_vref( Slice const& sl, vector<uint32_t>& vref_conf ){
  vref_conf.clear();

  // Retrieve ref DAC tap setting
  int32_t tempVrefReal = sl.ana.real_voltage_ref.tap();
  int32_t tempVrefImag = sl.ana.imag_voltage_ref.tap();

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

      atom_res_values[atom_count][0]  = (uint8_t) am->embr_real[EMBRPOS_R].pot_far_tap();
      atom_res_values[atom_count][1]  = (uint8_t) am->embr_real[EMBRPOS_R].pot_near_tap();

      atom_res_values[atom_count][2]  = (uint8_t) am->embr_real[EMBRPOS_U].pot_near_tap();
      atom_res_values[atom_count][3]  = (uint8_t) am->embr_real[EMBRPOS_U].pot_far_tap();

      atom_res_values[atom_count][4]  = (uint8_t) am->embr_imag[EMBRPOS_R].pot_far_tap();
      atom_res_values[atom_count][5]  = (uint8_t) am->embr_imag[EMBRPOS_R].pot_near_tap();

      atom_res_values[atom_count][6]  = (uint8_t) am->embr_imag[EMBRPOS_U].pot_near_tap();
      atom_res_values[atom_count][7]  = (uint8_t) am->embr_imag[EMBRPOS_U].pot_far_tap();

      atom_res_values[atom_count][8]  = (uint8_t) am->embr_real[EMBRPOS_UR].pot_near_tap();
      atom_res_values[atom_count][9]  = (uint8_t) am->embr_real[EMBRPOS_UR].pot_far_tap();
      atom_res_values[atom_count][10] = (uint8_t) am->embr_imag[EMBRPOS_UR].pot_near_tap();
      atom_res_values[atom_count][11] = (uint8_t) am->embr_imag[EMBRPOS_UR].pot_far_tap();

      atom_res_values[atom_count][12] = (uint8_t) am->node.imag_pot_resistance_tap();
      atom_res_values[atom_count][13] = (uint8_t) am->node.imag_pot_current_tap();
      atom_res_values[atom_count][14] = (uint8_t) am->node.real_pot_resistance_tap();
      atom_res_values[atom_count][15] = (uint8_t) am->node.real_pot_current_tap();

      ////////////////////////////////////////
      // Fill in TCON register values (8-bit)

      // --- IC controlling res1 & res2 & res3 & res4 ---

      // TCON1&2: TCON0 for IC controlling res1 (R0) & res2 (R1)
      // Modify R0B (pos 0) according to res1 status
      if( am->embr_real[EMBRPOS_R].pot_far_sw() ){
        atom_tcon_values[atom_count][0][0] = 1; // R0B
        atom_tcon_values[atom_count][0][1] = 1; // R0W
        // R0A according to res1 swA
        atom_tcon_values[atom_count][0][2] = am->embr_real[EMBRPOS_R].pot_far_swA();
      }
      // Modify R1B (pos 4) according to res2 status
      if( am->embr_real[EMBRPOS_R].pot_near_sw() ){
        atom_tcon_values[atom_count][0][4] = 1; // R1B
        atom_tcon_values[atom_count][0][5] = 1; // R1W
        // R1A according to res2 swA
        atom_tcon_values[atom_count][0][6] = am->embr_real[EMBRPOS_R].pot_near_swA();
      }

      // TCON3&4: TCON1 for IC controlling res3 (R2) & res4 (R3)
      // Modify R2B (pos 0) according to res3 status
      if( am->embr_real[EMBRPOS_U].pot_near_sw() ){
        atom_tcon_values[atom_count][1][0] = 1; // R2B
        atom_tcon_values[atom_count][1][1] = 1; // R2W
        // R2A according to res3 swA
        atom_tcon_values[atom_count][1][2] = am->embr_real[EMBRPOS_U].pot_near_swA();
      }
      // Modify R3B (pos 4) according to res4 status
      if( am->embr_real[EMBRPOS_U].pot_far_sw() ){
        atom_tcon_values[atom_count][1][4] = 1; // R3B
        atom_tcon_values[atom_count][1][5] = 1; // R3W
        // R3A according to res4 swA
        atom_tcon_values[atom_count][1][6] = am->embr_real[EMBRPOS_U].pot_far_swA();
      }

      // --- IC controlling res5 & res6 & res7 & res8 ---

      // TCON5&6: TCON0 for IC controlling res5 (R0) & res6 (R1)
      // Modify R0B (pos 0) according to res5 status
      if( am->embr_imag[EMBRPOS_R].pot_far_sw() ){
        atom_tcon_values[atom_count][2][0] = 1; // R0B
        atom_tcon_values[atom_count][2][1] = 1; // R0W
        // R0A according to res5 swA
        atom_tcon_values[atom_count][2][2] = am->embr_imag[EMBRPOS_R].pot_far_swA();
      }
      // Modify R1B (pos 4) according to res6 status
      if( am->embr_imag[EMBRPOS_R].pot_near_sw() ){
        atom_tcon_values[atom_count][2][4] = 1; // R1B
        atom_tcon_values[atom_count][2][5] = 1; // R1W
        // R1A according to res6 swA
        atom_tcon_values[atom_count][2][6] = am->embr_imag[EMBRPOS_R].pot_near_swA();
      }

      // TCON7&8: TCON1 for IC controlling res7 (R2) & res8 (R3)
      // Modify R2B (pos 0) according to res7 status
      if( am->embr_imag[EMBRPOS_U].pot_near_sw() ){
        atom_tcon_values[atom_count][3][0] = 1; // R2B
        atom_tcon_values[atom_count][3][1] = 1; // R2W
        // R2A according to res7 swA
        atom_tcon_values[atom_count][3][2] = am->embr_imag[EMBRPOS_U].pot_near_swA();
      }
      // Modify R3B (pos 4) according to res8 status
      if ( am->embr_imag[EMBRPOS_U].pot_far_sw() ){
        atom_tcon_values[atom_count][3][4] = 1; // R3B
        atom_tcon_values[atom_count][3][5] = 1; // R3W
        // R3A according to res8 swA
        atom_tcon_values[atom_count][3][6] = am->embr_imag[EMBRPOS_U].pot_far_swA();
      }

      // --- IC controlling res9 & res10 & res11 & res12 ---

      // TCON9&10: TCON0 for IC controlling res9 (R0) & res10 (R1)
      // retain the default value 10001000 (= 136ul)
      // TCON11&12: TCON1 for IC controlling res11 (R2) & res12 (R3)
      // retain the default value 10001000 (= 136ul)

      // Modify R0B (pos 0) according to res9 status
      if( am->embr_real[EMBRPOS_UR].pot_near_sw() ){
        atom_tcon_values[atom_count][4][0] = 1; // R0B
        atom_tcon_values[atom_count][4][1] = 1; // R0W
        // R0A according to res9 swA
        atom_tcon_values[atom_count][4][2] = am->embr_real[EMBRPOS_UR].pot_near_swA();
      }
      // Modify R1B (pos 4) according to res10 status
      if( am->embr_real[EMBRPOS_UR].pot_far_sw() ){
        atom_tcon_values[atom_count][4][4] = 1; // R1B
        atom_tcon_values[atom_count][4][5] = 1; // R1W
        // R1A according to res10 swA
        atom_tcon_values[atom_count][4][6] = am->embr_real[EMBRPOS_UR].pot_far_swA();
      }

      // Modify R2B (pos 0) according to res11 status
      if( am->embr_imag[EMBRPOS_UR].pot_near_sw() ){
        atom_tcon_values[atom_count][5][0] = 1; // R2B
        atom_tcon_values[atom_count][5][1] = 1; // R2W
        // R2A according to res11 swA
        atom_tcon_values[atom_count][5][2] = am->embr_imag[EMBRPOS_UR].pot_near_swA();
      }
      // Modify R3B (pos 4) according to res12 status
      if( am->embr_imag[EMBRPOS_UR].pot_far_sw() ){
        atom_tcon_values[atom_count][5][4] = 1; // R3B
        atom_tcon_values[atom_count][5][5] = 1; // R3W
        // R3A according to res12 swA
        atom_tcon_values[atom_count][5][6] = am->embr_imag[EMBRPOS_UR].pot_far_swA();
      }

      // --- IC controlling res13 & res14 & res15 & res16 ---

      // TCON13&14: TCON0 for IC controlling res13 (R0) & res14 (R1)
      // retain the default value 10001000 (= 136ul)
      // TCON15&16: TCON1 for IC controlling res15 (R2) & res16 (R3)
      // retain the default value 10001000 (= 136ul)

      // FOR THE NODE
      // TCON13&14: TCON0 for IC controlling res13 (R0) & res14 (R1)
      // Modify R0B (pos 0) according to res13 status
      if( am->node.imag_pot_resistance_sw() ){
        atom_tcon_values[atom_count][6][0] = 1; // R0B
        atom_tcon_values[atom_count][6][1] = 1; // R0W
        // R0A according to res13 swA
        atom_tcon_values[atom_count][6][2] = am->node.imag_pot_resistance_swA();
      }
      // Modify R1B (pos 4) according to res14 status
      if( am->node.imag_pot_current_sw() ){
        atom_tcon_values[atom_count][6][4] = 1; // R1B
        atom_tcon_values[atom_count][6][5] = 1; // R1W
        // R1A according to res14 swA
        atom_tcon_values[atom_count][6][6] = am->node.imag_pot_current_swA();
      }

      // TCON15&16: TCON1 for IC controlling res15 (R2) & res16 (R3)
      // Modify R2B (pos 0) according to res15 status
      if( am->node.real_pot_resistance_sw() ){
        atom_tcon_values[atom_count][7][0] = 1; // R2B
        atom_tcon_values[atom_count][7][1] = 1; // R2W
        // R2A according to res15 swA
        atom_tcon_values[atom_count][7][2] = am->node.real_pot_resistance_swA();
      }
      // Modify R3B (pos 4) according to res16 status
      if( am->node.real_pot_current_sw() ){
        atom_tcon_values[atom_count][7][4] = 1; // R3B
        atom_tcon_values[atom_count][7][5] = 1; // R3W
        // R3A according to res16 swA
        atom_tcon_values[atom_count][7][6] = am->node.real_pot_current_swA();
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
  atom_res_values[atom_count][0]  = (uint8_t) am->embr_real[EMBRPOS_L].pot_far_tap();
  atom_res_values[atom_count][1]  = (uint8_t) am->embr_real[EMBRPOS_L].pot_near_tap();
  atom_res_values[atom_count][2]  = (uint8_t) am->embr_imag[EMBRPOS_L].pot_far_tap();
  atom_res_values[atom_count][3]  = (uint8_t) am->embr_imag[EMBRPOS_L].pot_near_tap();

  am = sl.ana.getAtom(1,0);
  atom_res_values[atom_count][4]  = (uint8_t) am->embr_real[EMBRPOS_L].pot_near_tap();
  atom_res_values[atom_count][5]  = (uint8_t) am->embr_real[EMBRPOS_L].pot_far_tap();
  atom_res_values[atom_count][6]  = (uint8_t) am->embr_imag[EMBRPOS_L].pot_near_tap();
  atom_res_values[atom_count][7]  = (uint8_t) am->embr_imag[EMBRPOS_L].pot_far_tap();

  am = sl.ana.getAtom(2,0);
  atom_res_values[atom_count][8]  = (uint8_t) am->embr_real[EMBRPOS_L].pot_near_tap();
  atom_res_values[atom_count][9]  = (uint8_t) am->embr_real[EMBRPOS_L].pot_far_tap();
  atom_res_values[atom_count][10] = (uint8_t) am->embr_imag[EMBRPOS_L].pot_near_tap();
  atom_res_values[atom_count][11] = (uint8_t) am->embr_imag[EMBRPOS_L].pot_far_tap();

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
  if ( am->embr_real[EMBRPOS_L].pot_far_sw() ){
    atom_tcon_values[atom_count][0][0] = 1; // R0B
    atom_tcon_values[atom_count][0][1] = 1; // R0W
    // R0A according to res1 swA
    atom_tcon_values[atom_count][0][2] = am->embr_real[EMBRPOS_L].pot_far_swA();
  }
  // Modify R1B (pos 4) according to res2 status
  if ( am->embr_real[EMBRPOS_L].pot_near_sw() ){
    atom_tcon_values[atom_count][0][4] = 1; // R1B
    atom_tcon_values[atom_count][0][5] = 1; // R1W
    // R1A according to res2 swA
    atom_tcon_values[atom_count][0][6] = am->embr_real[EMBRPOS_L].pot_near_swA();
  }

  // TCON3&4: TCON1 for IC controlling res3 (R2) & res4 (R3)
  // Modify R2B (pos 0) according to res3 status
  if ( am->embr_imag[EMBRPOS_L].pot_far_sw() ){
    atom_tcon_values[atom_count][1][0] = 1; // R2B
    atom_tcon_values[atom_count][1][1] = 1; // R2W
    // R2A according to res3 swA
    atom_tcon_values[atom_count][1][2] = am->embr_imag[EMBRPOS_L].pot_far_swA();
  }
  // Modify R3B (pos 4) according to res4 status
  if ( am->embr_imag[EMBRPOS_L].pot_near_sw() ){
    atom_tcon_values[atom_count][1][4] = 1; // R3B
    atom_tcon_values[atom_count][1][5] = 1; // R3W
    // R3A according to res4 swA
    atom_tcon_values[atom_count][1][6] = am->embr_imag[EMBRPOS_L].pot_near_swA();
  }

  // --- IC controlling res5 & res6 & res7 & res8 ---

  // TCON5&6: TCON0 for IC controlling res5 (R0) & res6 (R1)
  // retain the default value 10001000 (= 136ul)
  // TCON7&8: TCON1 for IC controlling res7 (R2) & res8 (R3)
  // retain the default value 10001000 (= 136ul)

  am = sl.ana.getAtom(1,0);
  // TCON5&6: TCON0 for IC controlling res5 (R0) & res6 (R1)
  // Modify R0B (pos 0) according to res5 status
  if ( am->embr_real[EMBRPOS_L].pot_near_sw() ){
    atom_tcon_values[atom_count][2][0] = 1; // R0B
    atom_tcon_values[atom_count][2][1] = 1; // R0W
    // R0A according to res5 swA
    atom_tcon_values[atom_count][2][2] = am->embr_real[EMBRPOS_L].pot_near_swA();
  }
  // Modify R1B (pos 4) according to res6 status
  if ( am->embr_real[EMBRPOS_L].pot_far_sw() ){
    atom_tcon_values[atom_count][2][4] = 1; // R1B
    atom_tcon_values[atom_count][2][5] = 1; // R1W
    // R1A according to res6 swA
    atom_tcon_values[atom_count][2][6] = am->embr_real[EMBRPOS_L].pot_far_swA();
  }

  // TCON7&8: TCON1 for IC controlling res7 (R2) & res8 (R3)
  // Modify R2B (pos 0) according to res7 status
  if ( am->embr_imag[EMBRPOS_L].pot_near_sw() ){
    atom_tcon_values[atom_count][3][0] = 1; // R2B
    atom_tcon_values[atom_count][3][1] = 1; // R2W
    // R2A according to res7 swA
    atom_tcon_values[atom_count][3][2] = am->embr_imag[EMBRPOS_L].pot_near_swA();
  }
  // Modify R3B (pos 4) according to res8 status
  if ( am->embr_imag[EMBRPOS_L].pot_far_sw() ){
    atom_tcon_values[atom_count][3][4] = 1; // R3B
    atom_tcon_values[atom_count][3][5] = 1; // R3W
    // R3A according to res8 swA
    atom_tcon_values[atom_count][3][6] = am->embr_imag[EMBRPOS_L].pot_far_swA();
  }

  // --- IC controlling res9 & res10 & res11 & res12 ---

  // TCON9&10: TCON0 for IC controlling res9 (R0) & res10 (R1)
  // retain the default value 10001000 (= 136ul)
  // TCON11&12: TCON1 for IC controlling res11 (R2) & res12 (R3)
  // retain the default value 10001000 (= 136ul)

  am = sl.ana.getAtom(2,0);
  // TCON9&10: TCON0 for IC controlling res9 (R0) & res10 (R1)
  // Modify R0B (pos 0) according to res9 status
  if ( am->embr_real[EMBRPOS_L].pot_near_sw() ){
    atom_tcon_values[atom_count][4][0] = 1; // R0B
    atom_tcon_values[atom_count][4][1] = 1; // R0W
    // R0A according to res9 swA
    atom_tcon_values[atom_count][4][2] = am->embr_real[EMBRPOS_L].pot_near_swA();
  }
  // Modify R1B (pos 4) according to res10 status
  if ( am->embr_real[EMBRPOS_L].pot_far_sw() ){
    atom_tcon_values[atom_count][4][4] = 1; // R1B
    atom_tcon_values[atom_count][4][5] = 1; // R1W
    // R1A according to res10 swA
    atom_tcon_values[atom_count][4][6] = am->embr_real[EMBRPOS_L].pot_far_swA();
  }

  // TCON11&12: TCON1 for IC controlling res11 (R2) & res12 (R3)
  // Modify R2B (pos 0) according to res11 status
  if ( am->embr_imag[EMBRPOS_L].pot_near_sw() ){
    atom_tcon_values[atom_count][5][0] = 1; // R2B
    atom_tcon_values[atom_count][5][1] = 1; // R2W
    // R2A according to res11 swA
    atom_tcon_values[atom_count][5][2] = am->embr_imag[EMBRPOS_L].pot_near_swA();
  }
  // Modify R3B (pos 4) according to res12 status
  if ( am->embr_imag[EMBRPOS_L].pot_far_sw() ){
    atom_tcon_values[atom_count][5][4] = 1; // R3B
    atom_tcon_values[atom_count][5][5] = 1; // R3W
    // R3A according to res12 swA
    atom_tcon_values[atom_count][5][6] = am->embr_imag[EMBRPOS_L].pot_far_swA();
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
  atom_res_values[atom_count][0]  = (uint8_t) am->embr_real[EMBRPOS_D].pot_near_tap();
  atom_res_values[atom_count][1]  = (uint8_t) am->embr_real[EMBRPOS_D].pot_far_tap();
  atom_res_values[atom_count][2]  = (uint8_t) am->embr_imag[EMBRPOS_D].pot_near_tap();
  atom_res_values[atom_count][3]  = (uint8_t) am->embr_imag[EMBRPOS_D].pot_far_tap();

  am = sl.ana.getAtom(0,2);
  atom_res_values[atom_count][4]  = (uint8_t) am->embr_real[EMBRPOS_D].pot_near_tap();
  atom_res_values[atom_count][5]  = (uint8_t) am->embr_real[EMBRPOS_D].pot_far_tap();
  atom_res_values[atom_count][6]  = (uint8_t) am->embr_imag[EMBRPOS_D].pot_near_tap();
  atom_res_values[atom_count][7]  = (uint8_t) am->embr_imag[EMBRPOS_D].pot_far_tap();

  am = sl.ana.getAtom(0,3);
  atom_res_values[atom_count][8]  = (uint8_t) am->embr_real[EMBRPOS_D].pot_near_tap();
  atom_res_values[atom_count][9]  = (uint8_t) am->embr_real[EMBRPOS_D].pot_far_tap();
  atom_res_values[atom_count][10] = (uint8_t) am->embr_imag[EMBRPOS_D].pot_near_tap();
  atom_res_values[atom_count][11] = (uint8_t) am->embr_imag[EMBRPOS_D].pot_far_tap();

  am = sl.ana.getAtom(0,4);
  atom_res_values[atom_count][12] = (uint8_t) am->embr_real[EMBRPOS_D].pot_near_tap();
  atom_res_values[atom_count][13] = (uint8_t) am->embr_real[EMBRPOS_D].pot_far_tap();
  atom_res_values[atom_count][14] = (uint8_t) am->embr_imag[EMBRPOS_D].pot_near_tap();
  atom_res_values[atom_count][15] = (uint8_t) am->embr_imag[EMBRPOS_D].pot_far_tap();

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
  if ( am->embr_real[EMBRPOS_D].pot_near_sw() ){
    atom_tcon_values[atom_count][0][0] = 1; // R0B
    atom_tcon_values[atom_count][0][1] = 1; // R0W
    // R0A according to res1 swA
    atom_tcon_values[atom_count][0][2] = am->embr_real[EMBRPOS_D].pot_near_swA();
  }
  // Modify R1B (pos 4) according to res2 status
  if ( am->embr_real[EMBRPOS_D].pot_far_sw() ){
    atom_tcon_values[atom_count][0][4] = 1; // R1B
    atom_tcon_values[atom_count][0][5] = 1; // R1W
    // R1A according to res2 swA
    atom_tcon_values[atom_count][0][6] = am->embr_real[EMBRPOS_D].pot_far_swA();
  }

  // TCON3&4: TCON1 for IC controlling res3 (R2) & res4 (R3)
  // Modify R2B (pos 0) according to res3 status
  if ( am->embr_imag[EMBRPOS_D].pot_near_sw() ){
    atom_tcon_values[atom_count][1][0] = 1; // R2B
    atom_tcon_values[atom_count][1][1] = 1; // R2W
    // R2A according to res3 swA
    atom_tcon_values[atom_count][1][2] = am->embr_imag[EMBRPOS_D].pot_near_swA();
  }
  // Modify R3B (pos 4) according to res4 status
  if ( am->embr_imag[EMBRPOS_D].pot_far_sw() ){
    atom_tcon_values[atom_count][1][4] = 1; // R3B
    atom_tcon_values[atom_count][1][5] = 1; // R3W
    // R3A according to res4 swA
    atom_tcon_values[atom_count][1][6] = am->embr_imag[EMBRPOS_D].pot_far_swA();
  }

  // --- IC controlling res5 & res6 & res7 & res8 ---

  // TCON5&6: TCON0 for IC controlling res5 (R0) & res6 (R1)
  // retain the default value 10001000 (= 136ul)
  // TCON7&8: TCON1 for IC controlling res7 (R2) & res8 (R3)
  // retain the default value 10001000 (= 136ul)

  am = sl.ana.getAtom(0,2);
  // TCON5&6: TCON0 for IC controlling res5 (R0) & res6 (R1)
  // Modify R0B (pos 0) according to res5 status
  if ( am->embr_real[EMBRPOS_D].pot_near_sw() ){
    atom_tcon_values[atom_count][2][0] = 1; // R0B
    atom_tcon_values[atom_count][2][1] = 1; // R0W
    // R0A according to res5 swA
    atom_tcon_values[atom_count][2][2] = am->embr_real[EMBRPOS_D].pot_near_swA();
  }
  // Modify R1B (pos 4) according to res6 status
  if ( am->embr_real[EMBRPOS_D].pot_far_sw() ){
    atom_tcon_values[atom_count][2][4] = 1; // R1B
    atom_tcon_values[atom_count][2][5] = 1; // R1W
    // R1A according to res6 swA
    atom_tcon_values[atom_count][2][6] = am->embr_real[EMBRPOS_D].pot_far_swA();
  }

  // TCON7&8: TCON1 for IC controlling res7 (R2) & res8 (R3)
  // Modify R2B (pos 0) according to res7 status
  if ( am->embr_imag[EMBRPOS_D].pot_near_sw() ){
    atom_tcon_values[atom_count][3][0] = 1; // R2B
    atom_tcon_values[atom_count][3][1] = 1; // R2W
    // R2A according to res7 swA
    atom_tcon_values[atom_count][3][2] = am->embr_imag[EMBRPOS_D].pot_near_swA();
  }
  // Modify R3B (pos 4) according to res8 status
  if ( am->embr_imag[EMBRPOS_D].pot_far_sw() ){
    atom_tcon_values[atom_count][3][4] = 1; // R3B
    atom_tcon_values[atom_count][3][5] = 1; // R3W
    // R3A according to res8 swA
    atom_tcon_values[atom_count][3][6] = am->embr_imag[EMBRPOS_D].pot_far_swA();
  }

  // --- IC controlling res9 & res10 & res11 & res12 ---

  // TCON9&10: TCON0 for IC controlling res9 (R0) & res10 (R1)
  // retain the default value 10001000 (= 136ul)
  // TCON11&12: TCON1 for IC controlling res11 (R2) & res12 (R3)
  // retain the default value 10001000 (= 136ul)

  am = sl.ana.getAtom(0,3);
  // TCON9&10: TCON0 for IC controlling res9 (R0) & res10 (R1)
  // Modify R0B (pos 0) according to res9 status
  if ( am->embr_real[EMBRPOS_D].pot_near_sw() ){
    atom_tcon_values[atom_count][4][0] = 1; // R0B
    atom_tcon_values[atom_count][4][1] = 1; // R0W
    // R0A according to res9 swA
    atom_tcon_values[atom_count][4][2] = am->embr_real[EMBRPOS_D].pot_near_swA();
  }
  // Modify R1B (pos 4) according to res10 status
  if ( am->embr_real[EMBRPOS_D].pot_far_sw() ){
    atom_tcon_values[atom_count][4][4] = 1; // R1B
    atom_tcon_values[atom_count][4][5] = 1; // R1W
    // R1A according to res10 swA
    atom_tcon_values[atom_count][4][6] = am->embr_real[EMBRPOS_D].pot_far_swA();
  }

  // TCON11&12: TCON1 for IC controlling res11 (R2) & res12 (R3)
  // Modify R2B (pos 0) according to res11 status
  if ( am->embr_imag[EMBRPOS_D].pot_near_sw() ){
    atom_tcon_values[atom_count][5][0] = 1; // R2B
    atom_tcon_values[atom_count][5][1] = 1; // R2W
    // R2A according to res11 swA
    atom_tcon_values[atom_count][5][2] = am->embr_imag[EMBRPOS_D].pot_near_swA();
  }
  // Modify R3B (pos 4) according to res12 status
  if ( am->embr_imag[EMBRPOS_D].pot_far_sw() ){
    atom_tcon_values[atom_count][5][4] = 1; // R3B
    atom_tcon_values[atom_count][5][5] = 1; // R3W
    // R3A according to res12 swA
    atom_tcon_values[atom_count][5][6] = am->embr_imag[EMBRPOS_D].pot_far_swA();
  }

  // --- IC controlling res13 & res14 & res15 & res16 ---

  // TCON13&14: TCON0 for IC controlling res13 (R0) & res14 (R1)
  // retain the default value 10001000 (= 136ul)
  // TCON11&12: TCON1 for IC controlling res15 (R2) & res16 (R3)
  // retain the default value 10001000 (= 136ul)

  am = sl.ana.getAtom(0,4);
  // TCON13&14: TCON0 for IC controlling res13 (R0) & res14 (R1)
  // Modify R0B (pos 0) according to res13 status
  if ( am->embr_real[EMBRPOS_D].pot_near_sw() ){
    atom_tcon_values[atom_count][6][0] = 1; // R0B
    atom_tcon_values[atom_count][6][1] = 1; // R0W
    // R0A according to res13 swA
    atom_tcon_values[atom_count][6][2] = am->embr_real[EMBRPOS_D].pot_near_swA();
  }
  // Modify R1B (pos 4) according to res14 status
  if ( am->embr_real[EMBRPOS_D].pot_far_sw() ){
    atom_tcon_values[atom_count][6][4] = 1; // R1B
    atom_tcon_values[atom_count][6][5] = 1; // R1W
    // R1A according to res14 swA
    atom_tcon_values[atom_count][6][6] = am->embr_real[EMBRPOS_D].pot_far_swA();
  }

  // TCON11&12: TCON1 for IC controlling res15 (R2) & res16 (R3)
  // Modify R2B (pos 0) according to res15 status
  if ( am->embr_imag[EMBRPOS_D].pot_near_sw() ){
    atom_tcon_values[atom_count][7][0] = 1; // R2B
    atom_tcon_values[atom_count][7][1] = 1; // R2W
    // R2A according to res15 swA
    atom_tcon_values[atom_count][7][2] = am->embr_imag[EMBRPOS_D].pot_near_swA();
  }
  // Modify R3B (pos 4) according to res16 status
  if ( am->embr_imag[EMBRPOS_D].pot_far_sw() ){
    atom_tcon_values[atom_count][7][4] = 1; // R3B
    atom_tcon_values[atom_count][7][5] = 1; // R3W
    // R3A according to res16 swA
    atom_tcon_values[atom_count][7][6] = am->embr_imag[EMBRPOS_D].pot_far_swA();
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
      atom_sw_status[atom_count][0] = !am->embr_real[EMBRPOS_R].sw_mid();
      atom_sw_status[atom_count][1] = !am->embr_real[EMBRPOS_R].sw_sc();

      // IF
      // branch at position EMBRPOS_U (up) physically exist at the atom (should
      // always be true in current design)
      // THEN
      // modify atom_sw_status according to current PCB implementation
      atom_sw_status[atom_count][2] = !am->embr_real[EMBRPOS_U].sw_sc();
      atom_sw_status[atom_count][3] = !am->embr_real[EMBRPOS_U].sw_mid();

      // IF
      // branch at position EMBRPOS_UR (up-right) physically exist at the atom (should
      // always be true in current design)
      // THEN
      // modify atom_sw_status according to current PCB implementation
      atom_sw_status[atom_count][4] = !am->embr_real[EMBRPOS_UR].sw_sc();
      atom_sw_status[atom_count][5] = !am->embr_real[EMBRPOS_UR].sw_mid();

      // Two last bits of first byte are unused - set to 1. ( actually redundant
      // as atom_sw_status[atom_count] has been .set() )
      atom_sw_status[atom_count][6] = 1;
      atom_sw_status[atom_count][7] = 1;

      // modify atom_sw_status according to current PCB implementation
      atom_sw_status[atom_count][ 8] = !am->node.real_sw_current_shunt;
      atom_sw_status[atom_count][ 9] = !am->node.real_sw_current;
      atom_sw_status[atom_count][10] = !am->node.real_sw_voltage;
      atom_sw_status[atom_count][11] = !am->node.real_sw_resistance;
      atom_sw_status[atom_count][12] = !am->node.imag_sw_current_shunt;
      atom_sw_status[atom_count][13] = !am->node.imag_sw_current;
      atom_sw_status[atom_count][14] = !am->node.imag_sw_voltage;
      atom_sw_status[atom_count][15] = !am->node.imag_sw_resistance;

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
  atom_sw_status[atom_count][0] = !am->embr_real[EMBRPOS_L].sw_sc();
  atom_sw_status[atom_count][1] = !am->embr_real[EMBRPOS_L].sw_mid();
  atom_sw_status[atom_count][2] = !am->embr_imag[EMBRPOS_L].sw_sc();
  atom_sw_status[atom_count][3] = !am->embr_imag[EMBRPOS_L].sw_mid();

  am = sl.ana.getAtom(1,0);
  // Check physical (should always be ok) existance of EMBRPOS_L branch of
  // atom[1][0]. If so modify atom_sw_status according to current PCB implem.
  atom_sw_status[atom_count][4] = !am->embr_real[EMBRPOS_L].sw_sc();
  atom_sw_status[atom_count][5] = !am->embr_real[EMBRPOS_L].sw_mid();
  atom_sw_status[atom_count][6] = !am->embr_imag[EMBRPOS_L].sw_sc();
  atom_sw_status[atom_count][7] = !am->embr_imag[EMBRPOS_L].sw_mid();

  am = sl.ana.getAtom(2,0);
  // Check physical (should always be ok) existance of EMBRPOS_L branch of
  // atom[2][0]. If so modify atom_sw_status according to current PCB implem.
  atom_sw_status[atom_count][8] = !am->embr_real[EMBRPOS_L].sw_sc();
  atom_sw_status[atom_count][9] = !am->embr_real[EMBRPOS_L].sw_mid();
  atom_sw_status[atom_count][10] = !am->embr_imag[EMBRPOS_L].sw_sc();
  atom_sw_status[atom_count][11] = !am->embr_imag[EMBRPOS_L].sw_mid();

  ++atom_count;



  // atom_sw_status[25] contains switch information regarding the extension
  // system at the bottom side of the slice
  // ---------------------------------------------------------------------------

  // Configure all switches as open
  atom_sw_status[atom_count].set();

  am = sl.ana.getAtom(0,4);
  // Check physical (should always be ok) existance of EMBRPOS_D branch of
  // atom[0][4]. If so modify atom_sw_status according to current PCB implem.
  atom_sw_status[atom_count][0] = !am->embr_real[EMBRPOS_D].sw_sc();
  atom_sw_status[atom_count][1] = !am->embr_real[EMBRPOS_D].sw_mid();
  atom_sw_status[atom_count][2] = !am->embr_imag[EMBRPOS_D].sw_sc();
  atom_sw_status[atom_count][3] = !am->embr_imag[EMBRPOS_D].sw_mid();

  am = sl.ana.getAtom(0,1);
  // Check physical (should always be ok) existance of EMBRPOS_D branch of
  // atom[0][1]. If so modify atom_sw_status according to current PCB implem.
  atom_sw_status[atom_count][4] = !am->embr_real[EMBRPOS_D].sw_sc();
  atom_sw_status[atom_count][5] = !am->embr_real[EMBRPOS_D].sw_mid();
  atom_sw_status[atom_count][6] = !am->embr_imag[EMBRPOS_D].sw_sc();
  atom_sw_status[atom_count][7] = !am->embr_imag[EMBRPOS_D].sw_mid();

  am = sl.ana.getAtom(0,2);
  // Check physical (should always be ok) existance of EMBRPOS_D branch of
  // atom[0][2]. If so modify atom_sw_status according to current PCB implem.
  atom_sw_status[atom_count][ 8] = !am->embr_real[EMBRPOS_D].sw_sc();
  atom_sw_status[atom_count][ 9] = !am->embr_real[EMBRPOS_D].sw_mid();
  atom_sw_status[atom_count][10] = !am->embr_imag[EMBRPOS_D].sw_sc();
  atom_sw_status[atom_count][11] = !am->embr_imag[EMBRPOS_D].sw_mid();

  am = sl.ana.getAtom(0,3);
  // Check physical (should always be ok) existance of EMBRPOS_D branch of
  // atom[0][3]. If so modify atom_sw_status according to current PCB implem.
  atom_sw_status[atom_count][12] = !am->embr_real[EMBRPOS_D].sw_sc();
  atom_sw_status[atom_count][13] = !am->embr_real[EMBRPOS_D].sw_mid();
  atom_sw_status[atom_count][14] = !am->embr_imag[EMBRPOS_D].sw_sc();
  atom_sw_status[atom_count][15] = !am->embr_imag[EMBRPOS_D].sw_mid();

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

int encoder::detail::encode_GPFADCDACgot(Slice const& sl,
                                         vector<uint32_t>& adcGain_conf,
                                         vector<uint32_t>& adcOffset_conf,
                                         vector<uint32_t>& dacGain_conf,
                                         vector<uint32_t>& dacOffset_conf){

  int32_t tempMSB, tempLSB, temp;
  size_t rows, cols;
  sl.ana.size(rows,cols);
  size_t atomCount = rows*cols;
  vector<pair<int,int> > pos = sl.dig.pipe_GPFPQ.position();

  adcGain_conf.clear();
  // ----------- ADC gain -----------
  // 31  26 25        13 12         0
  // 000000 [Q3.10 imag] [Q3.10 real]
  // 000000 [ tempMSB  ] [ tempLSB  ]
  // 000000 [         temp          ]
  // --------------------------------
  double real_gain, imag_gain;
  for (size_t k=0 ; k!=atomCount; ++k){

    if (k < sl.dig.pipe_GPFPQ.element_count()){
      Atom const* am = sl.ana.getAtom(pos[k].first,pos[k].second);
      imag_gain = sl.ana.ADCGain * am->node.imag_adc_gain_corr;
      real_gain = sl.ana.ADCGain * am->node.real_adc_gain_corr;
    } else { // k >= sl.dig.pipe_GPFPQ.element_count()
      imag_gain = sl.ana.ADCGain;
      real_gain = sl.ana.ADCGain;
    }

    temp = 0;
    detail::form_word(imag_gain, 13, 10, true, &tempMSB);
    detail::form_word(real_gain, 13, 10, true, &tempLSB);
    temp = (tempMSB << 13) | (tempLSB);
    adcGain_conf.push_back(static_cast<uint32_t>(temp));
  }

  // --------- ADC offset ---------
  // 31    24 23      12 11       0
  // 00000000 [12b imag] [12b real]
  // 00000000 [tempMSB ] [tempLSB ]
  // 00000000 [       temp        ]
  // ------------------------------
  adcOffset_conf.clear();
  double real_offset, imag_offset;
  int32_t mask12 = (1<<12) - 1;
  for (size_t k=0; k!=atomCount; ++k){
    if ( k < sl.dig.pipe_GPFPQ.element_count() ){
      Atom const* am = sl.ana.getAtom(pos[k].first,pos[k].second);
      imag_offset = sl.ana.ADCOffset + am->node.imag_adc_offset_corr;
      real_offset = sl.ana.ADCOffset + am->node.real_adc_offset_corr;
    } else { // k >= sl.dig.pipe_GPFPQ.element_count()
      imag_offset = sl.ana.ADCOffset;
      real_offset = sl.ana.ADCOffset;
    }

    temp = 0;
    tempMSB = static_cast<int32_t>(auxiliary::round(imag_offset/NODE_DAC_MAXOUT*pow(2,12)));
    tempMSB &= mask12;
    tempLSB = static_cast<int32_t>(auxiliary::round(real_offset/NODE_DAC_MAXOUT*pow(2,12)));
    tempLSB &= mask12;
    temp = (tempMSB << 12) | (tempLSB);
    adcOffset_conf.push_back( static_cast<uint32_t>(temp) );
  }

  // ----------- DAC gain -----------
  // 31  24 23        12 11         0
  // 000000 [Q2.10 imag] [Q2.10 real]
  // 000000 [ tempMSB  ] [ tempLSB  ]
  // 000000 [         temp          ]
  // --------------------------------
  dacGain_conf.clear();
  for (size_t v(0); v!=cols; ++v){
    for (size_t h(0); h!=rows; ++h){
      Atom const* atom = sl.ana.getAtom(v,h);
      if (sl.dig.pipe_GPFslack.search_element(v,h) < 0){
        detail::form_word(atom->node.imag_dac_gain_corr, 12, 10, false, &tempMSB);
        detail::form_word(atom->node.real_adc_gain_corr, 12, 10, false, &tempLSB);
      } else  {
        // If the node contains a slack element, the nominal values are entered!
        detail::form_word(NODE_DAC_GAIN_CORR_NOMINAL, 12, 10, false, &tempMSB);
        detail::form_word(NODE_DAC_GAIN_CORR_NOMINAL, 12, 10, false, &tempLSB);
      }
      tempMSB &= mask12;
      tempLSB &= mask12;
      temp = (tempMSB << 12) | (tempLSB);
      dacGain_conf.push_back(static_cast<uint32_t>(temp));
    }
  }

  // ---------- DAC offset ----------
  // 31  24 23        12 11         0
  // 000000 [Q2.10 imag] [Q2.10 real]
  // 000000 [ tempMSB  ] [ tempLSB  ]
  // 000000 [         temp          ]
  // --------------------------------
  dacOffset_conf.clear();
  for (size_t v(0); v!=cols; ++v){
    for (size_t h(0); h!=rows; ++h){
      Atom const* atom = sl.ana.getAtom(v,h);
      if (sl.dig.pipe_GPFslack.search_element(v,h) < 0){
        tempMSB = static_cast<int32_t>(auxiliary::round(atom->node.imag_dac_offset_corr/NODE_DAC_MAXOUT*pow(2,12)));
        tempLSB = static_cast<int32_t>(auxiliary::round(atom->node.real_dac_offset_corr/NODE_DAC_MAXOUT*pow(2,12)));
      } else {
        // If the node contains a slack element, the nominal values are entered!
        tempMSB = static_cast<int32_t>(auxiliary::round(NODE_DAC_OFFSET_CORR_NOMINAL/NODE_DAC_MAXOUT*pow(2,12)));
        tempLSB = static_cast<int32_t>(auxiliary::round(NODE_DAC_OFFSET_CORR_NOMINAL/NODE_DAC_MAXOUT*pow(2,12)));
      }
      tempMSB &= mask12;
      tempLSB &= mask12;
      temp = (tempMSB << 12) | (tempLSB);
      dacOffset_conf.push_back(static_cast<uint32_t>(temp));
    }
  }

  return 0;
}

int encoder::detail::encode_GPFpositions(Slice const& sl,
                                         vector<uint32_t>& nodeCount_conf,
                                         vector<uint32_t>& PQpos_conf,
                                         vector<uint32_t>& slpos_conf){
  nodeCount_conf.clear();
  PQpos_conf.clear();
  slpos_conf.clear();

  int32_t temp(0);
  int32_t temp5bit(0);
  int32_t mask5bit = (1<<5)-1;  // 0b00000000000000000000000000011111

  // ***** node count (nodeCount_conf) *****
  temp = 0;
  // number of PQ nodes
  temp5bit = 0;
  temp5bit &= mask5bit;
  temp |= ( temp5bit << 0*5 );
  // zero
  temp5bit = static_cast<int32_t>( sl.dig.pipe_GPFPQ.element_count() );
  temp5bit &= mask5bit;
  temp |= ( temp5bit << 1*5 );
  // zero
  temp5bit = 0;
  temp5bit &= mask5bit;
  temp |= ( temp5bit << 2*5 );
  // slack node
  temp5bit = static_cast<int32_t>( sl.dig.pipe_GPFslack.element_count() ); // should be == 1
  temp5bit &= mask5bit;
  temp |= ( temp5bit << 3*5 );

  nodeCount_conf.push_back( static_cast<uint32_t> (temp) );

  // ***** PQ nodes positions (PQpos_conf) *****
  temp = 0;
//  for (size_t k=0; k!=MAXPQNODES; ++k){ // for some weird reason 18 positions are needed
  for (size_t k=0; k!=sl.dig.pipe_GPFPQ.element_count_max(); ++k){
    // update position for k'th gen (5 bits: [0 unused] 1 - 24 [-31 unused])
    temp5bit = 0;
    if ( k < sl.dig.pipe_GPFPQ.element_count() )
      temp5bit = static_cast<int32_t>(
          sl.dig.pipe_GPFPQ.position()[k].first * sl.dig.pipe_GPFPQ.hor_id_max() +
          sl.dig.pipe_GPFPQ.position()[k].second + 1 );
    temp5bit &= mask5bit;

    // update position word with the position of the k'th gen (if any) - else 0
    temp |= ( temp5bit << (5 *(k%6)) );
    // push position word back into conf and reset temp
    if ((k+1)%6==0){
      PQpos_conf.push_back(static_cast<uint32_t>(temp));
      temp = 0;
    }
  }

  // ***** Slack (represented as const I load) position(s) (slpos_conf) *****
  temp = 0;
  for (size_t k=0; k!=sl.dig.pipe_GPFslack.element_count_max(); ++k){
    // update position for k'th gen (5 bits: [0 unused] 1 - 24 [-31 unused])
    temp5bit = 0;
    if ( k < sl.dig.pipe_GPFslack.element_count() )
      temp5bit = static_cast<int32_t>(
                   sl.dig.pipe_GPFslack.position()[k].first * sl.dig.pipe_GPFslack.hor_id_max() +
                   sl.dig.pipe_GPFslack.position()[k].second + 1 );
    temp5bit &= mask5bit;

    // update position word with the position of the k'th gen (if any) - else 0
    temp |= ( temp5bit << (5 *(k%6)) );
    // push position word back into conf and reset temp
    if ((k+1)%6==0){
      slpos_conf.push_back(static_cast<uint32_t>(temp));
      temp = 0;
    }
  }
  stamp_NIOS_confirm(slpos_conf.back());

  return 0;
}

void encoder::detail::encode_GPFauxiliary(vector<uint32_t>& conf_conf, vector<uint32_t>& nios_conf){
  conf_conf.clear();
  uint32_t temp = 0U;
  stamp_NIOS_confirm(temp);
  conf_conf.push_back(temp);

  nios_conf.clear();
  temp = 1U;
  nios_conf.push_back(temp);
}

void encoder::detail::encode_GPFIinit(Slice const& sl,
                                      vector<uint32_t>& icar_conf,
                                      vector<uint32_t>& ipol_conf){
  size_t rows, cols;
  sl.ana.size(rows, cols);
  size_t atomCount = rows*cols;
  int32_t tempMSB, tempLSB, temp;
  // --------- Icartesian ---------
  // 31    24 23      12 11       0
  // 00000000 [12b imag] [12b real]
  // 00000000 [tempMSB ] [tempLSB ]
  // 00000000 [       temp        ]
  // ------------------------------
  icar_conf.resize(atomCount,0);
  GPFPQPipeline const& pipePQ(sl.dig.pipe_GPFPQ);
  vector<pair<int,int> > pos = pipePQ.position();
  for (size_t k=0; k!=pipePQ.element_count(); ++k){
    // Get the atom which corresponds to the current node
    int ver = pos[k].first;
    int hor = pos[k].second;
    Atom const* atom = sl.ana.getAtom(ver,hor);

    // Get the (original, perfect, uncalibrated) current that is stored in the pipelines
    complex<double> I0 = pipePQ.I0[k];
    double Ireal =  I0.real();
    double Iimag = -I0.imag();

    // Calibrate the current
    // NOTICE: Ireal is going to flow in the imaginary (in a voltage sense,
    // Vimag) network so it is to be calibrated with imag_ corrections.
    // Analogously for Iimag, to be calcibrated with real_ corrections.
    Ireal += atom->node.imag_dac_offset_corr; // TODO!
    Ireal *= atom->node.imag_dac_gain_corr;   // imag_dac_gain_corr is a multiplicative factor, so no translation is required
    Iimag += atom->node.real_dac_offset_corr; // TODO!
    Iimag *= atom->node.real_dac_gain_corr;   // real_dac_gain_corr is a multiplicative factor, so no translation is required

    // Form the word
    detail::form_word(Ireal, 12, 7, true, &tempLSB);
    detail::form_word(Iimag, 12, 7, true, &tempMSB);
    temp = (tempMSB<<12)|(tempLSB);

    // Write it to the _conf (using nodeId indexing)
    size_t nodeId = pipePQ.calculate_pseudo_id(ver, hor);
    icar_conf[nodeId] = static_cast<uint32_t>(temp);
  }
  // Include the slack in the cartesian configuration
  // Comment: both real and imag currents for the slack are negated because
  // on the emulator the slack is implemented in a "load" pipeline, that has a
  // different convention for the directions of the currents:
  // (copy-pasted comment from encodeTDiloads)
  // Note 1: for xloads the pipe_xload convention is that there is a flow INTO
  // the loads, whereas for the real emulator DAC, the convention is that there
  // is a flow OUT OF the DAC (into the grid). Therefore, currents of loads have
  // to be negated.
  GPFSlackPipeline const& pipeSlack(sl.dig.pipe_GPFslack);
  pos = pipeSlack.position();
  for (size_t k(0); k!=pipeSlack.element_count(); ++k){
    // Get the atom which corresponds to the current node
    int ver = pos[k].first;
    int hor = pos[k].second;
    Atom const* atom = sl.ana.getAtom(ver,hor);

    // Get the (original, perfect, uncalibrated) current that is stored in the pipelines
    complex<double> I0 = pipeSlack.I0[k];
    double Ireal = -  I0.real() ;
    double Iimag = -(-I0.imag());

    // Calibrate the current
    // NOTICE: Ireal is going to flow in the imaginary (in a voltage sense,
    // Vimag) network so it is to be calibrated with imag_ corrections.
    // Analogously for Iimag, to be calcibrated with real_ corrections.
    Ireal += atom->node.imag_dac_offset_corr; // TODO!
    Ireal *= atom->node.imag_dac_gain_corr;   // imag_dac_gain_corr is a multiplicative factor, so no translation is required
    Iimag += atom->node.real_dac_offset_corr; // TODO!
    Iimag *= atom->node.real_dac_gain_corr;   // real_dac_gain_corr is a multiplicative factor, so no translation is required

    // Form the word
    detail::form_word(Ireal, 12, 7, true, &tempLSB);
    detail::form_word(Iimag, 12, 7, true, &tempMSB);
    temp = (tempMSB<<12)|(tempLSB);

    // Write it to the _conf (using nodeId indexing)
    size_t nodeId = pipeSlack.calculate_pseudo_id(ver, hor);
    icar_conf[nodeId] = static_cast<uint32_t>(temp);
  }

  // --------- Ipolar --------------
  // 31           16 15            0
  // [Q5.11  arg(I)] [Q5.11  abs(I)]
  // [  tempMBS    ] [  tempLSB    ]
  // [            temp             ]
  // -------------------------------
  ipol_conf.resize(atomCount,0);
  for (size_t k=0; k!=pipePQ.element_count(); ++k){
    complex<double> I0 = pipePQ.I0[k];
    detail::form_word(std::abs(I0) , 16, 11, true, &tempLSB);
    detail::form_word(std::arg(I0) , 16, 10, true, &tempMSB);
    temp = (tempMSB<<16)|(tempLSB);
    ipol_conf[k] = static_cast<uint32_t>(temp);
  }

  // Stamp confirm
  stamp_NIOS_confirm(icar_conf.back());
}

void encoder::detail::encode_GPFPQsetpoints(Slice const& sl, vector<uint32_t>& pqset_conf){
  size_t rows, cols;
  sl.ana.size(rows, cols);
  size_t atomCount = rows*cols;

  pqset_conf.resize(atomCount,0);
  int32_t tempMSB, tempLSB, temp;
  complex<double> S;
  // --------- S setpoint ----------
  // 31           16 15            0
  // [Q5.11      Q ] [Q5.11      P ]
  // [  tempMBS    ] [  tempLSB    ]
  // [            temp             ]
  // -------------------------------
  GPFPQPipeline const& pipePQ(sl.dig.pipe_GPFPQ);
  for (size_t k=0; k!=pipePQ.element_count(); ++k){
    S = pipePQ.Sset[k];
    detail::form_word(S.real(), 16, 11, true, &tempLSB);
    detail::form_word(S.imag(), 16, 11, true, &tempMSB);
    temp = (tempMSB<<16)|(tempLSB);
    pqset_conf[k] = static_cast<uint32_t>(temp);
  }
}

int encoder::detail::encode_DCPFgot(Slice const& sl, vector<uint32_t>& got_conf){
  got_conf.clear();

  // ***** got_conf *****
  vector<uint32_t> gain_conf;
  // ----------- GOT gain -----------
  // 31  26 25        13 12         0
  // 000000 [Q3.10 imag] [Q3.10 real]
  // 000000 [ tempMSB  ] [ tempLSB  ]
  // 000000 [         temp          ]
  // --------------------------------
  vector<uint32_t> offset_conf;
  // --------- GOT offset ---------
  // 31    24 23      12 11       0
  // 00000000 [12b imag] [12b real]
  // 00000000 [tempMSB ] [tempLSB ]
  // 00000000 [       temp        ]
  // ------------------------------
  int32_t tempMSB, tempLSB, temp;
  int32_t mask12 = (1 << 12) - 1;
  for (size_t k(0);k!=MAX_VERATOMCOUNT;++k){
    for (size_t m(0);m!=MAX_HORATOMCOUNT;++m){
      // An element is added into the P-node pipeline only if there is a
      // non-zero IInjection at the position [k][m]
      if (sl.dig.injectionTypes[k][m]==NODE_IINJECTION){
        Atom const* am = sl.ana.getAtom(k,m);

        // gain
        double imag_gain = sl.ana.ADCGain * am->node.imag_adc_gain_corr;
        double real_gain = sl.ana.ADCGain * am->node.real_adc_gain_corr;
        temp = 0;
        detail::form_word(imag_gain, 13, 10, true, &tempMSB);
        detail::form_word(real_gain, 13, 10, true, &tempLSB);
        temp = (tempMSB << 13) | (tempLSB);
        gain_conf.push_back(static_cast<uint32_t>(temp));

        // offset
        double imag_offset = sl.ana.ADCOffset + am->node.imag_adc_offset_corr;
        double real_offset = sl.ana.ADCOffset + am->node.real_adc_offset_corr;
        temp = 0;
        tempMSB = static_cast<int32_t>(auxiliary::round(imag_offset/DAC_DEF_OUTMAX*pow(2,12)));
        tempMSB &= mask12;
        tempLSB = static_cast<int32_t>(auxiliary::round(real_offset/DAC_DEF_OUTMAX*pow(2,12)));
        tempLSB &= mask12;
        temp = (tempMSB << 12) | (tempLSB);
        offset_conf.push_back(static_cast<uint32_t>(temp));
      }
      // th-node elements are not considered; for them got correction is not
      // required as we do not make any measurements with the ADC for those nodes
      // (got corrections are needed for ADC reads)
    }
  }
  // Resize gain_conf to the correct size
  detail::form_word(sl.ana.ADCGain, 13, 10, true, &tempMSB);
  detail::form_word(sl.ana.ADCGain, 13, 10, true, &tempLSB);
  temp = (tempMSB << 13) | (tempLSB);
  gain_conf.resize(MAX_VERATOMCOUNT*MAX_HORATOMCOUNT, temp);
  // Resize offset_conf to the correct size
  tempMSB = static_cast<int32_t>(auxiliary::round(sl.ana.ADCOffset/DAC_DEF_OUTMAX*pow(2,12)));
  tempMSB &= mask12;
  tempLSB = static_cast<int32_t>(auxiliary::round(sl.ana.ADCOffset/DAC_DEF_OUTMAX*pow(2,12)));
  tempLSB &= mask12;
  temp = (tempMSB << 12) | (tempLSB);
  offset_conf.resize(MAX_VERATOMCOUNT*MAX_HORATOMCOUNT, temp);
  // Note: in the hereinabove resizing with value 0; would be the same; extra
  // values are actually never read

  // append gain_conf and offset_conf to (the output argument) got_conf
  got_conf.insert(got_conf.end(), gain_conf.begin(), gain_conf.end() );
  got_conf.insert(got_conf.end(), offset_conf.begin(), offset_conf.end() );
  return 0;
}

int encoder::detail::encode_DCPFpositions(Slice const& sl,
                                          vector<uint32_t>& ppos_conf,
                                          vector<uint32_t>& thpos_conf){
  ppos_conf.clear();
  thpos_conf.clear();

  int32_t temp     = 0;         // (32bit) word to be appended to p/th-pos_conf
  int32_t temp5bit = 0;         // 5bit-pos of the element currently under examination
  int32_t mask5bit = (1<<5)-1;  // 0b00000000000000000000000000011111

  // ----- P-nodes as IInjections -----
  size_t Ielements(0);
  for (size_t k(0);k!=MAX_VERATOMCOUNT;++k){
    for (size_t m(0);m!=MAX_HORATOMCOUNT;++m){
      // An element is added into the P-node pipeline only if there is a
      // non-zero IInjection at the position [k][m]
      if (sl.dig.injectionTypes[k][m]==NODE_IINJECTION){
        // update position for Ielements'th P-node (5 bits: [0 unused] 1-24 [-31 unused])
        temp5bit = static_cast<int32_t>(k*MAX_HORATOMCOUNT+m+1);
        temp5bit &= mask5bit;
        temp |= (temp5bit << (5*(Ielements%6)));
        // push position word back into conf and reset temp
        if ((Ielements+1)%6==0){
          ppos_conf.push_back(static_cast<uint32_t>(temp));
          temp = 0;
        }
        Ielements++; // increase the number of Ielements found
      }
    }
  }
  // Flush remaining entries into ppos_conf
  if (temp!=0){
    ppos_conf.push_back(static_cast<uint32_t>(temp));
    temp=0;
  }
  ppos_conf.resize(3,0); // arrange size of ppos_conf

  // ----- th-nodes as VInjection -----
  size_t Velements(0);
  for (size_t k(0);k!=MAX_VERATOMCOUNT;++k){
    for (size_t m(0);m!=MAX_HORATOMCOUNT;++m){
      // An element is added into the th-node pipeline only if there is a
      // non-zero VInjection at position [k][m]
      if (sl.dig.injectionTypes[k][m]==NODE_VINJECTION){
        // update position for Velements'th th-node (5 bits: [0 unused] 1-24 [-31 unused])
        temp5bit = static_cast<int32_t>(k*MAX_HORATOMCOUNT+m+1);
        temp5bit &= mask5bit;
        temp |= (temp5bit << (5*(Velements%6)));
        // push position word back into conf and reset temp
        if ((Velements+1)%6==0){
          thpos_conf.push_back(static_cast<uint32_t>(temp));
          temp = 0;
        }
        Velements++; // increase the number of Velements found
      }
    }
  }
  // Flush remaining entries into thpos_conf
  if (temp!=0){
    thpos_conf.push_back(static_cast<uint32_t>(temp));
    temp=0;
  }
  thpos_conf.resize(4,0);
  stamp_NIOS_confirm(thpos_conf.back()); // stamp NIOS confirmation on the last word of thpos_conf

  // ----- nodes count -----
  temp = 0;
  // number of P-nodes
  temp5bit = static_cast<int32_t>(Ielements);
  temp5bit &= mask5bit;
  temp |= (temp5bit<<0*5 );
  // zero
  temp5bit = 0;
  temp5bit &= mask5bit;
  temp |= (temp5bit<<1*5 );
  // zero
  temp5bit = 0;
  temp5bit &= mask5bit;
  temp |= (temp5bit<<2*5 );
  // number of th-nodes
  temp5bit = static_cast<int32_t>(Velements);
  temp5bit &= mask5bit;
  temp |= (temp5bit<<3*5);
  ppos_conf.push_back(static_cast<uint32_t>(temp));

  return 0;
}

void encoder::detail::encode_DCPFI(Slice const& sl, vector<uint32_t>& i_conf){

  i_conf.resize(MAX_VERATOMCOUNT*MAX_HORATOMCOUNT,0);

  // --------- Icartesian -------
  // 31    24 23    12 11       0
  // 00000000 00000000 [12b real]
  // 00000000 00000000 [  temp  ]
  // ----------------------------
  int32_t temp;
  int32_t mask12bit = (1<<12)-1;  // 0b00000000000000000000111111111111
  for (size_t k(0); k!=MAX_VERATOMCOUNT; ++k){
    for (size_t m(0); m!=MAX_HORATOMCOUNT; ++m){
      size_t nodeId = static_cast<size_t>(k*MAX_HORATOMCOUNT+m+1);
      if (sl.dig.injectionTypes[k][m]==NODE_IINJECTION){
        // see Emulator::nodeSetDCPF for what is written in IInjections
        detail::form_word(sl.dig.IInjections[k][m], 12, 7, true, &temp);
        i_conf[nodeId] = temp;
      } else if (sl.dig.injectionTypes[k][m]==NODE_VINJECTION){
        // The corrected DAC code has to be written in i_conf, if the node is a V injection
        double realV = sl.dig.VInjections[k][m]+sl.ana.real_voltage_ref.out();
        // Node DAC value 4095 is max tap setting, giving 5Volts * 4095/4096 as max V out
        double Vmax = NODE_DAC_MAXOUT*(pow(2,NODE_DAC_RES)-1)/pow(2,NODE_DAC_RES);
        double normalizedRealV = realV/Vmax; // [0,1]
        double VtapD = normalizedRealV*(pow(2,NODE_DAC_RES)-1); // convert to tap (double)
        VtapD = auxiliary::round(VtapD); // round double
        temp = static_cast<int32_t>(VtapD); // cast into uint
        temp -= - pow(2,NODE_DAC_RES)/2; // subtract midrange (2048) Guillaume requires that!
        if (temp<0) temp+=pow(2,NODE_DAC_RES); // bring into [0,4095] range
        temp &= mask12bit;
        i_conf[nodeId] = static_cast<uint32_t>(temp);
      }
    }
  }
}

void encoder::detail::encode_DCPFauxiliary(Slice const& sl,
                                           vector<uint32_t>& starter_conf,
                                           vector<uint32_t>& conf_conf,
                                           vector<uint32_t>& nios_conf){
  uint32_t temp(0U);

  // ----- starter_conf -----
  starter_conf.clear();
  // Check if slice is empty (DCPF-wise)
  bool empty(true);
  for (size_t k(0); k<MAX_VERATOMCOUNT; k++){
    for (size_t m(0); m!=MAX_HORATOMCOUNT; ++m){
      if (sl.dig.injectionTypes[k][m]==NODE_IINJECTION){
        empty=false;
        k=MAX_VERATOMCOUNT; // to break the outer loop
        break;              // to break the inner loop
      }
    }
  }
  if (empty) temp = 0U;
  else       temp = 1U;
  stamp_NIOS_confirm(temp);
  starter_conf.push_back(temp);

  // ----- conf_conf -----
  conf_conf.clear();
  temp = 0U;
  stamp_NIOS_confirm(temp);
  conf_conf.push_back(temp);

  // ----- nios_conf -----
  nios_conf.resize(2,1U);
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
  vector<pair<int,int> > pos = sl.dig.pipe_TDgen.position();
  double real_gain, imag_gain;
  for ( k = 0 ; k != atomCount ; ++k ){

    if ( k < sl.dig.pipe_TDgen.element_count() ){
      Atom const* am = sl.ana.getAtom(pos[k].first,pos[k].second);
      imag_gain = sl.ana.ADCGain * am->node.imag_adc_gain_corr;
      real_gain = sl.ana.ADCGain * am->node.real_adc_gain_corr;
    } else { // k >= sl.dig.pipe_TDgen.element_count()
      imag_gain = sl.ana.ADCGain;
      real_gain = sl.ana.ADCGain;
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

    if ( k < sl.dig.pipe_TDgen.element_count() ){
      Atom const* am = sl.ana.getAtom(pos[k].first,pos[k].second);
      imag_offset = sl.ana.ADCOffset + am->node.imag_adc_offset_corr;
      real_offset = sl.ana.ADCOffset + am->node.real_adc_offset_corr;
    } else { // k >= sl.dig.pipe_TDgen.element_count()
      imag_offset = sl.ana.ADCOffset;
      real_offset = sl.ana.ADCOffset;
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
  for ( k = 0 ; k != sl.dig.pipe_TDgen.element_count_max() ; ++k ){
    temp = 0;
    detail::form_word( sl.dig.pipe_TDgen.xd1inverse[k], 16, 7, false, &temp );
    gen_conf1.push_back( static_cast<uint32_t>(temp) );
  }

  // Initial current
  // 31          16 15           0
  // [Q5.11 imag_I] [Q5.11 real_I]
  // [  tempMBS   ] [  tempLSB   ]
  // [           temp            ]
  for ( k = 0 ; k != sl.dig.pipe_TDgen.element_count_max() ; ++k ){
    temp = 0;
    detail::form_word( sl.dig.pipe_TDgen.I0[k].imag(), 16, 11, true, &tempMSB );
    detail::form_word( sl.dig.pipe_TDgen.I0[k].real(), 16, 11, true, &tempLSB );
    temp = (tempMSB << 16) | (tempLSB);
    gen_conf1.push_back( static_cast<uint32_t>(temp) );
  }

  // Mechanical power Q5.13
  for ( k = 0 ; k != sl.dig.pipe_TDgen.element_count_max() ; ++k ){
    temp = 0;
    detail::form_word( sl.dig.pipe_TDgen.pMechanical[k], 18, 13, true, &temp );
    gen_conf1.push_back( static_cast<uint32_t>(temp) );
  }


  // ********** gen_conf2 **********
  // Gain 1 Q8.10
  for ( k = 0 ; k != sl.dig.pipe_TDgen.element_count_max() ; ++k ){
    temp = 0;
    detail::form_word( sl.dig.pipe_TDgen.gain1[k], 18, 10, true, &temp );
    gen_conf2.push_back( static_cast<uint32_t>(temp) );
  }

  // Gain 2 Q6.8
  for ( k = 0 ; k != sl.dig.pipe_TDgen.element_count_max() ; ++k ){
    temp = 0;
    detail::form_word( sl.dig.pipe_TDgen.gain2[k], 14, 8, true, &temp );
    gen_conf2.push_back( static_cast<uint32_t>(temp) );
  }

  // Gain 3 Q6.8
  for ( k = 0 ; k != sl.dig.pipe_TDgen.element_count_max() ; ++k ){
    temp = 0;
    detail::form_word( sl.dig.pipe_TDgen.gain3[k], 14, 8, true, &temp );
    gen_conf2.push_back( static_cast<uint32_t>(temp) );
  }

  // Gain 4 Q5.11
  for ( k = 0 ; k != sl.dig.pipe_TDgen.element_count_max() ; ++k ){
    temp = 0;
    detail::form_word( sl.dig.pipe_TDgen.gain4[k], 16, 11, true, &temp );
    gen_conf2.push_back( static_cast<uint32_t>(temp) );
  }

  // Gain 5 Q5.11
  for ( k = 0 ; k != sl.dig.pipe_TDgen.element_count_max() ; ++k ){
    temp = 0;
    detail::form_word( sl.dig.pipe_TDgen.gain5[k], 16, 11, true, &temp );
    gen_conf2.push_back( static_cast<uint32_t>(temp) );
  }

  // Gain 6 Q5.13
  for ( k = 0 ; k != sl.dig.pipe_TDgen.element_count_max() ; ++k ){
    temp = 0;
    detail::form_word( sl.dig.pipe_TDgen.gain6[k], 18, 13, true, &temp );
    gen_conf2.push_back( static_cast<uint32_t>(temp) );
  }


  // ********** gen_conf3 **********
  int64_t temp64;
  int64_t mask32 = (1LL << 32) - 1;
//  cout << "mask32 = " << mask32 << endl;
  // IG1 Past input values for generators: Accelerating power
  for ( k = 0 ; k != sl.dig.pipe_TDgen.element_count_max() ; ++k ){
//    cout << "******** Gen no " << k+1 << endl;
    temp64 = 0;
    detail::form_word( sl.dig.pipe_TDgen.pa0[k], 36, 23, true, &temp64 );
    tempLSB = static_cast<int32_t>( temp64 & mask32 );
    tempMSB = static_cast<int32_t>( (temp64 >> 32) & mask32 );
//    cout << "tempLSB = " << tempLSB << endl;
//    cout << "tempMSB = " << tempMSB << endl;
    gen_conf3.push_back( static_cast<uint32_t>(tempLSB) );
    gen_conf3.push_back( static_cast<uint32_t>(tempMSB) );
//    cout << endl;
  }

  // IG1 Past output values for generators: Omega
  for ( k = 0 ; k != sl.dig.pipe_TDgen.element_count_max() ; ++k ){
    temp64 = 0LL;
    detail::form_word( sl.dig.pipe_TDgen.omega0[k], 46, 44, true, &temp64 );
    tempLSB = static_cast<int32_t>( temp64 & mask32 );
    tempMSB = static_cast<int32_t>( (temp64 >> 32) & mask32 );
    gen_conf3.push_back( static_cast<uint32_t>(tempLSB) );
    gen_conf3.push_back( static_cast<uint32_t>(tempMSB) );
  }

  // IG2 Past input values for generators: Omega
  for ( k = 0 ; k != sl.dig.pipe_TDgen.element_count_max() ; ++k ){
    temp64 = 0LL;
    detail::form_word( sl.dig.pipe_TDgen.omega0[k], 46, 44, true, &temp64 );
    tempLSB = static_cast<int32_t>( temp64 & mask32 );
    tempMSB = static_cast<int32_t>( (temp64 >> 32) & mask32 );
    gen_conf3.push_back( static_cast<uint32_t>(tempLSB) );
    gen_conf3.push_back( static_cast<uint32_t>(tempMSB) );
  }

  // IG2 Past output values for generators: Delta
  for ( k = 0 ; k != sl.dig.pipe_TDgen.element_count_max() ; ++k ){
    temp64 = 0LL;
    detail::form_word( sl.dig.pipe_TDgen.delta0[k]*2/M_PI, 54, 52, true, &temp64 );
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
  vector<pair<int,int> > pos = sl.dig.pipe_TDzload.position();
  double real_gain, imag_gain;
  for ( k = 0 ; k != atomCount ; ++k ){

    if ( k < sl.dig.pipe_TDzload.element_count() ){
      Atom const* am = sl.ana.getAtom(pos[k].first,pos[k].second);
      imag_gain = sl.ana.ADCGain * am->node.imag_adc_gain_corr;
      real_gain = sl.ana.ADCGain * am->node.real_adc_gain_corr;
    } else { // k >= sl.dig.pipe_zload.element_count()
      imag_gain = sl.ana.ADCGain;
      real_gain = sl.ana.ADCGain;
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

    if ( k < sl.dig.pipe_TDzload.element_count() ){
      Atom const* am = sl.ana.getAtom(pos[k].first,pos[k].second);
      imag_offset = sl.ana.ADCOffset + am->node.imag_adc_offset_corr;
      real_offset = sl.ana.ADCOffset + am->node.real_adc_offset_corr;
    } else { // k >= sl.dig.pipe_zload.element_count()
      imag_offset = sl.ana.ADCOffset;
      real_offset = sl.ana.ADCOffset;
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
  for ( k = 0 ; k != sl.dig.pipe_TDzload.element_count() ; ++k ){
    detail::form_word( sl.dig.pipe_TDzload.Yconst[k].real(), 16, 13, true, &tempLSB );
    detail::form_word( sl.dig.pipe_TDzload.Yconst[k].imag(), 16, 13, true, &tempMSB );
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
  for ( k = sl.dig.pipe_TDzload.element_count();
        k != sl.dig.pipe_TDzload.element_count_max() ; ++k )
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
  vector<pair<int,int> > pos = sl.dig.pipe_TDiload.position();
  double real_gain, imag_gain;
  for ( k = 0 ; k != atomCount ; ++k ){

    if ( k < sl.dig.pipe_TDiload.element_count() ){
      Atom const* am = sl.ana.getAtom(pos[k].first,pos[k].second);
      imag_gain = sl.ana.ADCGain * am->node.imag_adc_gain_corr;
      real_gain = sl.ana.ADCGain * am->node.real_adc_gain_corr;
    } else { // k >= sl.dig.pipe_iload.element_count()
      imag_gain = sl.ana.ADCGain;
      real_gain = sl.ana.ADCGain;
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

    if ( k < sl.dig.pipe_TDiload.element_count() ){
      Atom const* am = sl.ana.getAtom(pos[k].first,pos[k].second);
      imag_offset = sl.ana.ADCOffset + am->node.imag_adc_offset_corr;
      real_offset = sl.ana.ADCOffset + am->node.real_adc_offset_corr;
    } else { // k >= sl.dig.pipe_iload.element_count()
      imag_offset = sl.ana.ADCOffset;
      real_offset = sl.ana.ADCOffset;
    }

    temp = 0;
    tempMSB = static_cast<int32_t>(auxiliary::round(imag_offset/DAC_DEF_OUTMAX*pow(2,12)));
    tempMSB &= mask12;
    tempLSB = static_cast<int32_t>(auxiliary::round(real_offset/DAC_DEF_OUTMAX*pow(2,12)));
    tempLSB &= mask12;
    temp = (tempMSB << 12) | (tempLSB);
    igot_conf.push_back(static_cast<uint32_t>(temp));
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
  iloads_conf.resize(sl.dig.pipe_TDiload.element_count_max(), 0);

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
  for ( k = 0 ; k != sl.dig.pipe_TDgen.element_count() ; ++k ){
    pseudo_id = static_cast<size_t>(sl.dig.pipe_TDgen.position()[k].first) *
                sl.dig.pipe_TDgen.hor_id_max() +
                static_cast<size_t>(sl.dig.pipe_TDgen.position()[k].second);
    detail::form_word(   sl.dig.pipe_TDgen.I0[k].real()  , 12, 7, true, &tempLSB );
    detail::form_word(  -sl.dig.pipe_TDgen.I0[k].imag()  , 12, 7, true, &tempMSB );
    temp = (tempMSB << 12) | (tempLSB);
    iloads_conf[pseudo_id] = static_cast<uint32_t>(temp);
  }

  // --- Initial currents of the constant current loads (iloads) ---
  for ( k = 0 ; k != sl.dig.pipe_TDiload.element_count() ; ++k ){
    pseudo_id = static_cast<size_t>(sl.dig.pipe_TDiload.position()[k].first) *
                sl.dig.pipe_TDiload.hor_id_max() +
                static_cast<size_t>(sl.dig.pipe_TDiload.position()[k].second);
    detail::form_word(    -sl.dig.pipe_TDiload.Iconst[k].real()   , 12, 7, true, &tempLSB );
    detail::form_word(  -(-sl.dig.pipe_TDiload.Iconst[k].imag())  , 12, 7, true, &tempMSB );
    temp = (tempMSB << 12) | (tempLSB);
    iloads_conf[pseudo_id] = static_cast<uint32_t>(temp);
  }

  // --- Initial currents of the constant impedance loads (zloads) ---
  for ( k = 0 ; k != sl.dig.pipe_TDzload.element_count() ; ++k ){
    pseudo_id = static_cast<size_t>(sl.dig.pipe_TDzload.position()[k].first) *
                sl.dig.pipe_TDzload.hor_id_max() +
                static_cast<size_t>(sl.dig.pipe_TDzload.position()[k].second);
    detail::form_word(    -sl.dig.pipe_TDzload.I0[k].real()   , 12, 7, true, &tempLSB );
    detail::form_word(  -(-sl.dig.pipe_TDzload.I0[k].imag())  , 12, 7, true, &tempMSB );
    temp = (tempMSB << 12) | (tempLSB);
    iloads_conf[pseudo_id] = static_cast<uint32_t>(temp);
  }

  // --- Initial currents of the constant power loads (ploads) ---
  for ( k = 0 ; k != sl.dig.pipe_TDpload.element_count() ; ++k ){
    pseudo_id = static_cast<size_t>(sl.dig.pipe_TDpload.position()[k].first) *
                sl.dig.pipe_TDpload.hor_id_max() +
                static_cast<size_t>(sl.dig.pipe_TDpload.position()[k].second);
    detail::form_word(    -sl.dig.pipe_TDpload.I0[k].real()   , 12, 7, true, &tempLSB );
    detail::form_word(  -(-sl.dig.pipe_TDpload.I0[k].imag())  , 12, 7, true, &tempMSB );
    temp = (tempMSB << 12) | (tempLSB);
    iloads_conf[pseudo_id] = static_cast<uint32_t>(temp);
  }

  // stamp NIOS confirmation on the last word of iloads_conf
  stamp_NIOS_confirm(iloads_conf.back());

  return 0;
}

int encoder::detail::encode_TDploads(Slice const& sl, vector<uint32_t>& ploads_conf){

  ploads_conf.clear();

  size_t k;
  int32_t tempMSB, tempLSB, temp;

  // ***** ploads_conf *****
  // 31          16 15           0
  // [Q2.14     Q ] [Q2.14     P ]
  // [  tempMBS   ] [  tempLSB   ]
  // [           temp            ]
  for ( k = 0 ; k != sl.dig.pipe_TDpload.element_count() ; ++k ){
    detail::form_word( sl.dig.pipe_TDpload.Sconst[k].real(), 16, 14, true, &tempLSB );
    detail::form_word( sl.dig.pipe_TDpload.Sconst[k].imag(), 16, 14, true, &tempMSB );
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
  for ( k = sl.dig.pipe_TDpload.element_count();
        k != sl.dig.pipe_TDpload.element_count_max() ; ++k )
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
//  for (k=0; k!=sl.dig.pipe_TDgen.element_count_max(); ++k){
  for (k=0; k!=18; ++k){ // for some weird reason 18 positions are needed

    // update position for k'th gen (5 bits: [0 unused] 1 - 24 [-31 unused])
    temp5bit = 0;
    if ( k < sl.dig.pipe_TDgen.element_count() )
      temp5bit = static_cast<int32_t>(
          sl.dig.pipe_TDgen.position()[k].first * sl.dig.pipe_TDgen.hor_id_max() +
          sl.dig.pipe_TDgen.position()[k].second + 1 );
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
  temp5bit = static_cast<int32_t>( sl.dig.pipe_TDgen.element_count() );
  temp5bit &= mask5bit;
  temp |= ( temp5bit << 0*5 );
  // number of const Z loads
  temp5bit = static_cast<int32_t>( sl.dig.pipe_TDzload.element_count() );
  temp5bit &= mask5bit;
  temp |= ( temp5bit << 1*5 );
  // number of const P loads
  temp5bit = static_cast<int32_t>( sl.dig.pipe_TDpload.element_count() );
  temp5bit &= mask5bit;
  temp |= ( temp5bit << 2*5 );
  // number of const I loads
  temp5bit = static_cast<int32_t>( sl.dig.pipe_TDiload.element_count() );
  temp5bit &= mask5bit;
  temp |= ( temp5bit << 3*5 );

  pos_conf.push_back( static_cast<uint32_t> (temp) );

  // ***** const Z load positions *****
  temp = 0;
  for ( k = 0 ; k != sl.dig.pipe_TDzload.element_count_max() ; ++k ){

    // update position for k'th load (5 bits: [0 unused] 1 - 24 [-31 unused])
    temp5bit = 0;
    if ( k < sl.dig.pipe_TDzload.element_count() )
      temp5bit = static_cast<int32_t>(
          sl.dig.pipe_TDzload.position()[k].first * sl.dig.pipe_TDzload.hor_id_max() +
          sl.dig.pipe_TDzload.position()[k].second + 1 );
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
  for ( k = 0 ; k != sl.dig.pipe_TDpload.element_count_max() ; ++k ){

    // update position for k'th load (5 bits: [0 unused] 1 - 24 [-31 unused])
    temp5bit = 0;
    if ( k < sl.dig.pipe_TDpload.element_count() )
      temp5bit = static_cast<int32_t>(
          sl.dig.pipe_TDpload.position()[k].first * sl.dig.pipe_TDpload.hor_id_max() +
          sl.dig.pipe_TDpload.position()[k].second + 1 );
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
  for ( k = 0 ; k != sl.dig.pipe_TDiload.element_count_max() ; ++k ){

    // update position for k'th load (5 bits: [0 unused] 1 - 24 [-31 unused])
    temp5bit = 0;
    if ( k < sl.dig.pipe_TDiload.element_count() )
      temp5bit = static_cast<int32_t>(
          sl.dig.pipe_TDiload.position()[k].first * sl.dig.pipe_TDiload.hor_id_max() +
          sl.dig.pipe_TDiload.position()[k].second + 1 );
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

int encoder::detail::encode_TDauxiliary(vector<uint32_t>& pert_conf){

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
