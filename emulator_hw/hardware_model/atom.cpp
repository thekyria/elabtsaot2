
#include "atom.h"
using namespace elabtsaot;

//#include <vector>
using std::vector;
#include <limits>
using std::numeric_limits;

Atom::Atom() :
    embr_real(EMBRPOS_UL+1), embr_imag(EMBRPOS_UL+1), _embr_exist(EMBRPOS_UL+1, false){

  // In initialization list and hereunder: emulator branch physical existance
  // vector is initialized to false for all positions but EMBRPOS_U, EMBRPOS_UR,
  // EMBRPOS_R
  _embr_exist[EMBRPOS_U ] = true;
  _embr_exist[EMBRPOS_UR] = true;
  _embr_exist[EMBRPOS_R ] = true;

  // In initialization list: emulator branch virtual existance vector is
  // initialized to false for all positions
}

int Atom::reset( bool complete ){
  // Reset node
  node.reset( complete );
  // Reset branches
  int ans = 0;
  for ( size_t k = 0 ; k != embr_real.size() ; ++k )
    ans |= embr_real[k].reset( complete );
  for ( size_t k = 0 ; k != embr_imag.size() ; ++k )
    ans |= embr_imag[k].reset( complete );
  return ans;
}

size_t Atom::getEmbrCount() const{
  size_t ans = 0;
  for ( size_t k = 0 ; k != _embr_exist.size() ; ++k )
    if ( _embr_exist[k] )
      ++ans;
  return ans;
}

void Atom::calibrate(Atom const& cal_am){
  // Real ADC & DAC
  node.real_adc_offset_corr = cal_am.node.real_adc_offset_corr;
  node.real_adc_gain_corr = cal_am.node.real_adc_gain_corr;
  node.real_dac_offset_corr = cal_am.node.real_dac_offset_corr;
  node.real_dac_gain_corr = cal_am.node.real_dac_gain_corr;

  // Imag ADC & DAC
  node.imag_adc_offset_corr = cal_am.node.imag_adc_offset_corr;
  node.imag_adc_gain_corr = cal_am.node.imag_adc_gain_corr;
  node.imag_dac_offset_corr = cal_am.node.imag_dac_offset_corr;
  node.imag_dac_gain_corr = cal_am.node.imag_dac_gain_corr;

  node.set_real_pot_current_rab( cal_am.node.real_pot_current_rab(), true);
  node.set_imag_pot_current_rab( cal_am.node.imag_pot_current_rab(), true);
  node.set_real_pot_current_rw( cal_am.node.real_pot_current_rw(), true);
  node.set_imag_pot_current_rw( cal_am.node.imag_pot_current_rw(), true);
  node.set_real_pot_resistance_rab( cal_am.node.real_pot_resistance_rab(), true);
  node.set_imag_pot_resistance_rab( cal_am.node.imag_pot_resistance_rab(), true);
  node.set_real_pot_resistance_rw( cal_am.node.real_pot_resistance_rw(), true);
  node.set_imag_pot_resistance_rw( cal_am.node.imag_pot_resistance_rw(), true);

  if ( _embr_exist[EMBRPOS_U] ){
    embr_real[EMBRPOS_U].set_pot_near_rab(cal_am.embr_real[EMBRPOS_U].pot_near_rab(), true);
    embr_imag[EMBRPOS_U].set_pot_near_rab(cal_am.embr_imag[EMBRPOS_U].pot_near_rab(), true);
    embr_real[EMBRPOS_U].set_pot_far_rab(cal_am.embr_real[EMBRPOS_U].pot_far_rab(), true);
    embr_imag[EMBRPOS_U].set_pot_far_rab(cal_am.embr_imag[EMBRPOS_U].pot_far_rab(), true);

    embr_real[EMBRPOS_U].set_pot_near_rw(cal_am.embr_real[EMBRPOS_U].pot_near_rw(), true);
    embr_imag[EMBRPOS_U].set_pot_near_rw(cal_am.embr_imag[EMBRPOS_U].pot_near_rw(), true);
    embr_real[EMBRPOS_U].set_pot_far_rw(cal_am.embr_real[EMBRPOS_U].pot_far_rw(), true);
    embr_imag[EMBRPOS_U].set_pot_far_rw(cal_am.embr_imag[EMBRPOS_U].pot_far_rw(), true);
  }

  if ( _embr_exist[EMBRPOS_UR] ){
    embr_real[EMBRPOS_UR].set_pot_near_rab(cal_am.embr_real[EMBRPOS_UR].pot_near_rab(), true);
    embr_imag[EMBRPOS_UR].set_pot_near_rab(cal_am.embr_imag[EMBRPOS_UR].pot_near_rab(), true);
    embr_real[EMBRPOS_UR].set_pot_far_rab(cal_am.embr_real[EMBRPOS_UR].pot_far_rab(), true);
    embr_imag[EMBRPOS_UR].set_pot_far_rab(cal_am.embr_imag[EMBRPOS_UR].pot_far_rab(), true);

    embr_real[EMBRPOS_UR].set_pot_near_rw(cal_am.embr_real[EMBRPOS_UR].pot_near_rw(), true);
    embr_imag[EMBRPOS_UR].set_pot_near_rw(cal_am.embr_imag[EMBRPOS_UR].pot_near_rw(), true);
    embr_real[EMBRPOS_UR].set_pot_far_rw(cal_am.embr_real[EMBRPOS_UR].pot_far_rw(), true);
    embr_imag[EMBRPOS_UR].set_pot_far_rw(cal_am.embr_imag[EMBRPOS_UR].pot_far_rw(), true);
  }

  if ( _embr_exist[EMBRPOS_R] ){
    embr_real[EMBRPOS_R].set_pot_near_rab(cal_am.embr_real[EMBRPOS_R].pot_near_rab(), true);
    embr_imag[EMBRPOS_R].set_pot_near_rab(cal_am.embr_imag[EMBRPOS_R].pot_near_rab(), true);
    embr_real[EMBRPOS_R].set_pot_far_rab(cal_am.embr_real[EMBRPOS_R].pot_far_rab(), true);
    embr_imag[EMBRPOS_R].set_pot_far_rab(cal_am.embr_imag[EMBRPOS_R].pot_far_rab(), true);

    embr_real[EMBRPOS_R].set_pot_near_rw(cal_am.embr_real[EMBRPOS_R].pot_near_rw(), true);
    embr_imag[EMBRPOS_R].set_pot_near_rw(cal_am.embr_imag[EMBRPOS_R].pot_near_rw(), true);
    embr_real[EMBRPOS_R].set_pot_far_rw(cal_am.embr_real[EMBRPOS_R].pot_far_rw(), true);
    embr_imag[EMBRPOS_R].set_pot_far_rw(cal_am.embr_imag[EMBRPOS_R].pot_far_rw(), true);
  }

  // Should never happen with current hardware topology (as of Feb 2012)
  if ( _embr_exist[EMBRPOS_DR] ){
    embr_real[EMBRPOS_DR].set_pot_near_rab(cal_am.embr_real[EMBRPOS_DR].pot_near_rab(), true);
    embr_imag[EMBRPOS_DR].set_pot_near_rab(cal_am.embr_imag[EMBRPOS_DR].pot_near_rab(), true);
    embr_real[EMBRPOS_DR].set_pot_far_rab(cal_am.embr_real[EMBRPOS_DR].pot_far_rab(), true);
    embr_imag[EMBRPOS_DR].set_pot_far_rab(cal_am.embr_imag[EMBRPOS_DR].pot_far_rab(), true);

    embr_real[EMBRPOS_DR].set_pot_near_rw(cal_am.embr_real[EMBRPOS_DR].pot_near_rw(), true);
    embr_imag[EMBRPOS_DR].set_pot_near_rw(cal_am.embr_imag[EMBRPOS_DR].pot_near_rw(), true);
    embr_real[EMBRPOS_DR].set_pot_far_rw(cal_am.embr_real[EMBRPOS_DR].pot_far_rw(), true);
    embr_imag[EMBRPOS_DR].set_pot_far_rw(cal_am.embr_imag[EMBRPOS_DR].pot_far_rw(), true);
  }

  if ( _embr_exist[EMBRPOS_D] ){
    embr_real[EMBRPOS_D].set_pot_near_rab(cal_am.embr_real[EMBRPOS_D].pot_near_rab(), true);
    embr_imag[EMBRPOS_D].set_pot_near_rab(cal_am.embr_imag[EMBRPOS_D].pot_near_rab(), true);
    embr_real[EMBRPOS_D].set_pot_far_rab(cal_am.embr_real[EMBRPOS_D].pot_far_rab(), true);
    embr_imag[EMBRPOS_D].set_pot_far_rab(cal_am.embr_imag[EMBRPOS_D].pot_far_rab(), true);

    embr_real[EMBRPOS_D].set_pot_near_rw(cal_am.embr_real[EMBRPOS_D].pot_near_rw(), true);
    embr_imag[EMBRPOS_D].set_pot_near_rw(cal_am.embr_imag[EMBRPOS_D].pot_near_rw(), true);
    embr_real[EMBRPOS_D].set_pot_far_rw(cal_am.embr_real[EMBRPOS_D].pot_far_rw(), true);
    embr_imag[EMBRPOS_D].set_pot_far_rw(cal_am.embr_imag[EMBRPOS_D].pot_far_rw(), true);
  }

  // Should never happen with current hardware topology (as of Feb 2012)
  if ( _embr_exist[EMBRPOS_DL] ){
    embr_real[EMBRPOS_DL].set_pot_near_rab(cal_am.embr_real[EMBRPOS_DL].pot_near_rab(), true);
    embr_imag[EMBRPOS_DL].set_pot_near_rab(cal_am.embr_imag[EMBRPOS_DL].pot_near_rab(), true);
    embr_real[EMBRPOS_DL].set_pot_far_rab(cal_am.embr_real[EMBRPOS_DL].pot_far_rab(), true);
    embr_imag[EMBRPOS_DL].set_pot_far_rab(cal_am.embr_imag[EMBRPOS_DL].pot_far_rab(), true);

    embr_real[EMBRPOS_DL].set_pot_near_rw(cal_am.embr_real[EMBRPOS_DL].pot_near_rw(), true);
    embr_imag[EMBRPOS_DL].set_pot_near_rw(cal_am.embr_imag[EMBRPOS_DL].pot_near_rw(), true);
    embr_real[EMBRPOS_DL].set_pot_far_rw(cal_am.embr_real[EMBRPOS_DL].pot_far_rw(), true);
    embr_imag[EMBRPOS_DL].set_pot_far_rw(cal_am.embr_imag[EMBRPOS_DL].pot_far_rw(), true);
  }

  if ( _embr_exist[EMBRPOS_L] ){
    embr_real[EMBRPOS_L].set_pot_near_rab(cal_am.embr_real[EMBRPOS_L].pot_near_rab(), true);
    embr_imag[EMBRPOS_L].set_pot_near_rab(cal_am.embr_imag[EMBRPOS_L].pot_near_rab(), true);
    embr_real[EMBRPOS_L].set_pot_far_rab(cal_am.embr_real[EMBRPOS_L].pot_far_rab(), true);
    embr_imag[EMBRPOS_L].set_pot_far_rab(cal_am.embr_imag[EMBRPOS_L].pot_far_rab(), true);

    embr_real[EMBRPOS_L].set_pot_near_rw(cal_am.embr_real[EMBRPOS_L].pot_near_rw(), true);
    embr_imag[EMBRPOS_L].set_pot_near_rw(cal_am.embr_imag[EMBRPOS_L].pot_near_rw(), true);
    embr_real[EMBRPOS_L].set_pot_far_rw(cal_am.embr_real[EMBRPOS_L].pot_far_rw(), true);
    embr_imag[EMBRPOS_L].set_pot_far_rw(cal_am.embr_imag[EMBRPOS_L].pot_far_rw(), true);
  }

  // Should never happen with current hardware topology (as of Feb 2012)
  if ( _embr_exist[EMBRPOS_UL] ){
    embr_real[EMBRPOS_UL].set_pot_near_rab(cal_am.embr_real[EMBRPOS_UL].pot_near_rab(), true);
    embr_imag[EMBRPOS_UL].set_pot_near_rab(cal_am.embr_imag[EMBRPOS_UL].pot_near_rab(), true);
    embr_real[EMBRPOS_UL].set_pot_far_rab(cal_am.embr_real[EMBRPOS_UL].pot_far_rab(), true);
    embr_imag[EMBRPOS_UL].set_pot_far_rab(cal_am.embr_imag[EMBRPOS_UL].pot_far_rab(), true);

    embr_real[EMBRPOS_UL].set_pot_near_rw(cal_am.embr_real[EMBRPOS_UL].pot_near_rw(), true);
    embr_imag[EMBRPOS_UL].set_pot_near_rw(cal_am.embr_imag[EMBRPOS_UL].pot_near_rw(), true);
    embr_real[EMBRPOS_UL].set_pot_far_rw(cal_am.embr_real[EMBRPOS_UL].pot_far_rw(), true);
    embr_imag[EMBRPOS_UL].set_pot_far_rw(cal_am.embr_imag[EMBRPOS_UL].pot_far_rw(), true);
  }
}

double Atom::getMinMaxAchievableR() const{

  double minMaxR = numeric_limits<double>::max();

  // Take into account branches
  for ( size_t k = 0 ; k != _embr_exist.size() ; ++k ){
    if ( _embr_exist[k] ){
      // real embrs
      if ( embr_real[k].pot_near_getRMax() < minMaxR )
        minMaxR = embr_real[k].pot_near_getRMax();
      if ( embr_real[k].pot_far_getRMax() < minMaxR )
        minMaxR = embr_real[k].pot_far_getRMax();
      // imag embrs
      if ( embr_imag[k].pot_near_getRMax() < minMaxR )
        minMaxR = embr_imag[k].pot_near_getRMax();
      if ( embr_imag[k].pot_far_getRMax() < minMaxR )
        minMaxR = embr_imag[k].pot_far_getRMax();
    }
  }

  // --- Take into account node ---
  // real part for the node
  if ( node.real_pot_current_getRMax() < minMaxR )
    minMaxR = node.real_pot_current_getRMax();
  if ( node.real_pot_resistance_getRMax() < minMaxR )
    minMaxR = node.real_pot_resistance_getRMax();
  // imag part for the node
  if ( node.imag_pot_current_getRMax() < minMaxR )
    minMaxR = node.imag_pot_current_getRMax();
  if ( node.imag_pot_resistance_getRMax() < minMaxR )
    minMaxR = node.imag_pot_resistance_getRMax();

  return minMaxR;
}

void Atom::set_embr_exist(size_t pos, bool val){ _embr_exist[pos] = val; }
vector<bool> Atom::embr_exist() const{ return _embr_exist; }
bool Atom::embr_exist(size_t pos) const{ return _embr_exist[pos]; }
