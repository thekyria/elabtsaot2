
#include "generatorpipeline.h"
using namespace elabtsaot;

#include "generator.h"

//#include <complex>
using std::complex;
using std::conj;
// #include <utility> // from parent class
using std::make_pair;
//#include <vector>
using std::vector;

GeneratorPipeline::GeneratorPipeline(size_t element_capacity,
                                     size_t ver_dim, size_t hor_dim) :
    Pipeline(element_capacity, ver_dim, hor_dim),
    _xd_1_inverse(_element_count_max, 0),
    _i_ss(_element_count_max, 0),
    _p_mechanical(_element_count_max, 0),
    _gain_1(_element_count_max, 0),
    _gain_2(_element_count_max, 0),
    _gain_3(_element_count_max, 0),
    _gain_4(_element_count_max, 1),
    _gain_5(_element_count_max, -1),
    _gain_6(_element_count_max, 0.5),
    _pa_0(_element_count_max, 0),
    _omega_0(_element_count_max, 0),
    _delta_0(_element_count_max, 0) {

  // Initialize stages
  _stages.push_back("pa");
  _stages.push_back("omega");
  _stages.push_back("delta");
}

int GeneratorPipeline::reset(){

  // Invoke reset of parent class
  int ans = Pipeline::reset();
  if ( ans ) return 1;
  // element_count_max, ver_id_max, hor_id_max remain unchanged

  _xd_1_inverse.clear();
  _i_ss.clear();
  _p_mechanical.clear();
  _gain_1.clear();
  _gain_2.clear();
  _gain_3.clear();
  _gain_4.clear();
  _gain_5.clear();
  _gain_6.clear();
  _pa_0.clear();
  _omega_0.clear();
  _delta_0.clear();

  _xd_1_inverse.resize(_element_count_max, 0);
  _i_ss.resize(_element_count_max, 0);
  _p_mechanical.resize(_element_count_max, 0);
  _gain_1.resize(_element_count_max, 0);
  _gain_2.resize(_element_count_max, 0);
  _gain_3.resize(_element_count_max, 0);
  _gain_4.resize(_element_count_max, 1);
  _gain_5.resize(_element_count_max, -1);
  _gain_6.resize(_element_count_max, 0.5);
  _pa_0.resize(_element_count_max, 0);
  _omega_0.resize(_element_count_max, 0);
  _delta_0.resize(_element_count_max, 0);

  return 0;
}

// High level functions
int GeneratorPipeline::insert_element(size_t ver_pos, size_t hor_pos,
                                      Generator const& el, bool overwrite){

  if ( _element_count == _element_count_max )
    // pipeline is full!
    return 1;
  if ( ver_pos >= _ver_id_max )
    // Vertical index out of bounds! Invalid insertion!
    return 2;
  if ( hor_pos >= _hor_id_max )
    // Horizontal index out of bounds! Invalid insertion!
    return 3;

  size_t k = 0, m;

  // Pseudo_id is a priority index calculated as:
  // pseudo_id(ver_id,hor_id) = ver_id*hor_id_max + hor_id (see related func.)
  // Elements in the pipeline are sorted with incresing pseudo_id, that is
  // first in the pipeline would be stored an el. at pos (0,0), then (0,1) up
  // to (0,hor_id_max-1) then (1,0) [yielding a pseudo_id = hor_id_max] (1,1)
  // and so on
  size_t new_pseudo_id = calculate_pseudo_id(ver_pos, hor_pos);
  size_t old_pseudo_id;
  // The new element should be stored at a position in the pipeline that
  // retains the sorting in increasing pseudo_ids.
  // This means that the new element should be stored at pipeline position k
  // at which the first element with greater pseudo_id than the new el is
  // encountered. As such, all previously stored els in the pipeline with
  // greater pseudo_ids are moved one position further into the pipeline (i.e.
  // @(m) -> @(m+1) for all m >= k) to make space for the new el.

  // Find position k for new element
  bool pos_already_taken = false;
  for ( k = 0 ; k != _element_count ; ++k ){
    old_pseudo_id =
        calculate_pseudo_id( static_cast<size_t>( _position[k].first),
                             static_cast<size_t>(_position[k].second) );
    if (new_pseudo_id == old_pseudo_id){
      // equality means that for position (ver_pos,hor_pos) there is already a
      // element registered at pipeline position k. pipeline entry k should
      // be updated with data for the new element, if arg. 'overwrite' is true
      if ( !overwrite )
        // Position already taken! Cannot insert element; overwrite disabled!
        return 4;
      pos_already_taken = true;
      break;
    }
    if (new_pseudo_id < old_pseudo_id)
      // loop broken when one element with a greater pseudo id is found
      // at pipeline position k (for extreme case element_count==0: k==0)
      break;
  }

  if ( !pos_already_taken )
    // Make space for new element
    for ( m = _element_count ; m != k ; --m ){
      _position[m] = _position[m-1];
      _xd_1_inverse[m] = _xd_1_inverse[m-1];
      _i_ss[m] = _i_ss[m-1];
      _p_mechanical[m] = _p_mechanical[m-1];
      _gain_1[m] = _gain_1[m-1];
      _gain_2[m] = _gain_2[m-1];
      _gain_3[m] = _gain_3[m-1];
      _gain_4[m] = _gain_4[m-1];
      _gain_5[m] = _gain_5[m-1];
      _gain_6[m] = _gain_6[m-1];
      _pa_0[m] = _pa_0[m-1];
      _omega_0[m] = _omega_0[m-1];
      _delta_0[m] = _delta_0[m-1];
    }

  // Insert new element at pipeline position k
  _position[k] = make_pair((int) ver_pos, (int) hor_pos);
  _xd_1_inverse[k] = static_cast<double>(1) / el.xd_1();

  /* Generator Norton equivalent
      ________>I  to the grid
      |   |
   I' |   | \/ I''
   /\ O   []
      |   [] xd_1
      |   |
     --- ---
      -   -
    */
  // Current injection to the grid
  complex<double> I  = conj ( complex<double>(el.pgen(),el.qgen()) / el.Uss() );
  // Current I'' flowing through the xd_1 Norton equivalent impedance to ground
  complex<double> I_2 = el.Uss() / complex<double>(el.ra(), el.xd_1());
  // Internal Norton equivalent current I'
  complex<double> I_1 = I + I_2;
  _i_ss[k] = I_1;
  _p_mechanical[k] = el.pgen();
  _gain_1[k] = 2*el.fss()/el.M();     // f / H
  _gain_2[k] = el.Ess()/el.xd_1();    // E' / x'_d
  _gain_3[k] = -_gain_2[k];           // -E' / x'_d
  _gain_4[k] = +1;                    // ASK GUILLAUME WHY
  _gain_5[k] = -_gain_4[k];           // ASK GUILLAUME WHY
  _gain_6[k] = 0.5;
  _pa_0[k] = 0;                       // steady state accelerating power = 0
  _omega_0[k] = 0;                    // steady state relative omega = 0
  _delta_0[k] = el.deltass();         // steady state delta [rad]

  if ( !pos_already_taken )
    ++_element_count;

  return 0;
}

int GeneratorPipeline::remove_element(size_t ver_pos, size_t hor_pos){

  // Why should the following return non-zero?
//  if ( _element_count == 0 )
//    // No gens to delete!
//    return 1;
//  if ( ver_pos >= _ver_id_max )
//    // Vertical index out of bounds! Nothing to delete!
//    return 2;
//  if ( hor_pos >= _hor_id_max )
//    // Horizontal index out of bounds! Nothing to delete!
//    return 3;

  size_t k = 0;
  bool gen_found = false;

  for ( k = 0 ; k != _element_count_max ; ++k ){

    if ( (!gen_found) &&
         (_position[k] == make_pair( (int) ver_pos, (int) hor_pos)) ){
      // generator @ position (ver_pos, hor_pos) found at pipeline line n=k
      gen_found = true;

      // decrease element count
      --_element_count;

      // reset pipeline line to default values
      _position[k] = make_pair(-1,-1);
      _xd_1_inverse[k] = 0;
      _i_ss[k] = 0;
      _p_mechanical[k] = 0;
      _gain_1[k] = 0;
      _gain_2[k] = 0;
      _gain_3[k] = 0;
      _gain_4[k] = 1;
      _gain_5[k] = -_gain_4[k];
      _gain_6[k] = 0.5;
      _pa_0[k] = 0;
      _omega_0[k] = 0;
      _delta_0[k] = 0;

      continue;
    }

    if ( gen_found ){
      // if generator to delete has been found, then technically delete it by
      // overwriting the contents of k-1 line of the pipeline, with those of
      // the k line, for all k lines after line n where the to-be-deleted
      // generator was found
      _position[k-1] = _position[k];
      _xd_1_inverse[k-1] = _xd_1_inverse[k];
      _i_ss[k-1] = _i_ss[k];
      _p_mechanical[k-1] = _p_mechanical[k];
      _gain_1[k-1] = _gain_1[k];
      _gain_2[k-1] = _gain_2[k];
      _gain_3[k-1] = _gain_3[k];
      _gain_4[k-1] = _gain_4[k];
      _gain_5[k-1] = _gain_5[k];
      _gain_6[k-1] = _gain_6[k];
      _pa_0[k-1] = _pa_0[k];
      _omega_0[k-1] = _omega_0[k];
      _delta_0[k-1] = _delta_0[k];
    }

  }

//  if ( !gen_found )
//    // No gen @pos (ver_pos, hor_pos); nothing to delete!
//    return 4;

  return 0;
}

// Low level setters
int GeneratorPipeline::set_xd_1_inverse(size_t pos, double val){
  if ( pos >= _element_count_max )
    // Position out of bounds!
    return 1;
  if ( val <= 0 )
    // Invalid 1/x_d_1 value!
    return 2;

  _xd_1_inverse[pos] = val;

  return 0;
}

int GeneratorPipeline::set_i_0(size_t pos, complex<double> val){
  if ( pos >= _element_count_max )
    // Position out of bounds!
    return 1;

  _i_ss[pos] = val;

  return 0;
}

int GeneratorPipeline::set_p_mechanical(size_t pos, double val){
  if ( pos >= _element_count_max )
    // Position out of bounds!
    return 1;
  if ( val <= 0 )
    // Invalid mechanical power!
    return 2;

  _p_mechanical[pos] = val;

  return 0;
}

int GeneratorPipeline::set_gain_1(size_t pos, double val){
  if ( pos >= _element_count_max )
    // Position out of bounds!
    return 1;

  _gain_1[pos] = val;

  return 0;
}

int GeneratorPipeline::set_gain_2(size_t pos, double val){
  if ( pos >= _element_count_max )
    // Position out of bounds!
    return 1;

  _gain_2[pos] = val;

  return 0;
}

int GeneratorPipeline::set_gain_3(size_t pos, double val){
  if ( pos >= _element_count_max )
    // Position out of bounds!
    return 1;

  _gain_3[pos] = val;

  return 0;
}

int GeneratorPipeline::set_gain_4(size_t pos, double val){
  if ( pos >= _element_count_max )
    // Position out of bounds!
    return 1;

  _gain_4[pos] = val;

  return 0;
}

int GeneratorPipeline::set_gain_5(size_t pos, double val){
  if ( pos >= _element_count_max )
    // Position out of bounds!
    return 1;

  _gain_5[pos] = val;

  return 0;
}

int GeneratorPipeline::set_gain_6(size_t pos, double val){
  if ( pos >= _element_count_max )
    // Position out of bounds!
    return 1;

  _gain_6[pos] = val;

  return 0;
}

int GeneratorPipeline::set_pa_0(size_t pos, double val){
  if ( pos >= _element_count_max )
    // Position out of bounds!
    return 1;

  _pa_0[pos] = val;

  return 0;
}

int GeneratorPipeline::set_omega_0(size_t pos, double val){
  if ( pos >= _element_count_max )
    // Position out of bounds!
    return 1;

  _omega_0[pos] = val;

  return 0;
}

int GeneratorPipeline::set_delta_0(size_t pos, double val){
  if ( pos >= _element_count_max )
    // Position out of bounds!
    return 1;

  _delta_0[pos] = val;

  return 0;
}

// Low level getters
vector<double> GeneratorPipeline::xd_1_inverse() const{ return _xd_1_inverse; }
vector<complex<double> > GeneratorPipeline::i_ss() const{ return _i_ss; }
vector<double> GeneratorPipeline::p_mechanical() const{ return _p_mechanical; }
vector<double> GeneratorPipeline::gain_1() const{ return _gain_1; }
vector<double> GeneratorPipeline::gain_2() const{ return _gain_2; }
vector<double> GeneratorPipeline::gain_3() const{ return _gain_3; }
vector<double> GeneratorPipeline::gain_4() const{ return _gain_4; }
vector<double> GeneratorPipeline::gain_5() const{ return _gain_5; }
vector<double> GeneratorPipeline::gain_6() const{ return _gain_6; }
vector<double> GeneratorPipeline::pa_0() const{ return _pa_0; }
vector<double> GeneratorPipeline::omega_0() const{ return _omega_0; }
vector<double> GeneratorPipeline::delta_0() const{ return _delta_0; }

