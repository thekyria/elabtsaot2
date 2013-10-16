
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
    xd1inverse(_element_count_max, 0),
    I0(_element_count_max, 0),
    pMechanical(_element_count_max, 0),
    gain1(_element_count_max, 0),
    gain2(_element_count_max, 0),
    gain3(_element_count_max, 0),
    gain4(_element_count_max, 1),
    gain5(_element_count_max, -1),
    gain6(_element_count_max, 0.5),
    pa0(_element_count_max, 0),
    omega0(_element_count_max, 0),
    delta0(_element_count_max, 0) {

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

  xd1inverse.resize(_element_count_max, 0);
  I0.resize(_element_count_max, 0);
  pMechanical.resize(_element_count_max, 0);
  gain1.resize(_element_count_max, 0);
  gain2.resize(_element_count_max, 0);
  gain3.resize(_element_count_max, 0);
  gain4.resize(_element_count_max, 1);
  gain5.resize(_element_count_max, -1);
  gain6.resize(_element_count_max, 0.5);
  pa0.resize(_element_count_max, 0);
  omega0.resize(_element_count_max, 0);
  delta0.resize(_element_count_max, 0);

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
      xd1inverse[m] = xd1inverse[m-1];
      I0[m] = I0[m-1];
      pMechanical[m] = pMechanical[m-1];
      gain1[m] = gain1[m-1];
      gain2[m] = gain2[m-1];
      gain3[m] = gain3[m-1];
      gain4[m] = gain4[m-1];
      gain5[m] = gain5[m-1];
      gain6[m] = gain6[m-1];
      pa0[m] = pa0[m-1];
      omega0[m] = omega0[m-1];
      delta0[m] = delta0[m-1];
    }

  // Insert new element at pipeline position k
  _position[k] = make_pair((int) ver_pos, (int) hor_pos);
  xd1inverse[k] = static_cast<double>(1) / el.xd_1();

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
  I0[k] = I_1;
  pMechanical[k] = el.pgen();
  gain1[k] = 2*el.fss()/el.M();     // f / H
  gain2[k] = el.Ess()/el.xd_1();    // E' / x'_d
  gain3[k] = -gain2[k];           // -E' / x'_d
  gain4[k] = +1;                    // ASK GUILLAUME WHY
  gain5[k] = -gain4[k];           // ASK GUILLAUME WHY
  gain6[k] = 0.5;
  pa0[k] = 0;                       // steady state accelerating power = 0
  omega0[k] = 0;                    // steady state relative omega = 0
  delta0[k] = el.deltass();         // steady state delta [rad]

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
      xd1inverse[k] = 0;
      I0[k] = 0;
      pMechanical[k] = 0;
      gain1[k] = 0;
      gain2[k] = 0;
      gain3[k] = 0;
      gain4[k] = 1;
      gain5[k] = -gain4[k];
      gain6[k] = 0.5;
      pa0[k] = 0;
      omega0[k] = 0;
      delta0[k] = 0;

      continue;
    }

    if ( gen_found ){
      // if generator to delete has been found, then technically delete it by
      // overwriting the contents of k-1 line of the pipeline, with those of
      // the k line, for all k lines after line n where the to-be-deleted
      // generator was found
      _position[k-1] = _position[k];
      xd1inverse[k-1] = xd1inverse[k];
      I0[k-1] = I0[k];
      pMechanical[k-1] = pMechanical[k];
      gain1[k-1] = gain1[k];
      gain2[k-1] = gain2[k];
      gain3[k-1] = gain3[k];
      gain4[k-1] = gain4[k];
      gain5[k-1] = gain5[k];
      gain6[k-1] = gain6[k];
      pa0[k-1] = pa0[k];
      omega0[k-1] = omega0[k];
      delta0[k-1] = delta0[k];
    }

  }

//  if ( !gen_found )
//    // No gen @pos (ver_pos, hor_pos); nothing to delete!
//    return 4;

  return 0;
}
