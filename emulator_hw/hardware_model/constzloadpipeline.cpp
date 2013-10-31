
#include "constzloadpipeline.h"
using namespace elabtsaot;

#include "load.h"

// #include <complex>
using std::conj;
using std::complex;
// #include <utility> // from parent class
using std::make_pair;
//#include <vector> // from parent class
using std::vector;

ConstZLoadPipeline::ConstZLoadPipeline(size_t element_capacity,
                                       size_t ver_dim, size_t hor_dim) :
    Pipeline(element_capacity, ver_dim, hor_dim),
    Yconst(_elementCountMax, 0),
    I0(_elementCountMax, 0) {}

void ConstZLoadPipeline::reset(){
  // Invoke reset of parent class
  Pipeline::reset();
  // element_count_max, ver_id_max, hor_id_max remain unchanged
  Yconst.resize(_elementCountMax, 0);
  I0.resize(_elementCountMax, 0);
}

int ConstZLoadPipeline::insert_element( size_t ver_pos, size_t hor_pos,
                                        Load const& el, bool overwrite){

  if ( _elementCount == _elementCountMax )
    // pipeline is full!
    return 1;
  if ( ver_pos >= _verIdMax )
    // Vertical index out of bounds! Invalid insertion!
    return 2;
  if ( hor_pos >= _horIdMax )
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
  for ( k = 0 ; k != _elementCount ; ++k ){
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
      // loop broken when one gen with a greater pseudo id is found
      // at pipeline position k (for extreme case element_count==0: k==0)
      break;
  }

  if ( !pos_already_taken )
    // Make space for new element
    for ( m = _elementCount ; m != k ; --m ){
      _position[m] = _position[m-1];
      Yconst[m]  = Yconst[m-1];
      I0[m] = I0[m-1];
    }

  // Insert new element at pipeline position k
  _position[k] = make_pair((int) ver_pos, (int) hor_pos);
  complex<double> S(el.Pdemand, el.Qdemand);
  complex<double> U(el.Vss);
  complex<double> Y_ = conj(S) / (U*conj(U));
  complex<double> I = conj(S) / conj(U);
  Yconst[k] = Y_;
  I0[k] = I;

  if ( !pos_already_taken )
    ++_elementCount;

  return 0;
}

int ConstZLoadPipeline::remove_element( size_t ver_pos, size_t hor_pos){
  // Why should the following return non-zero?
//  if ( _element_count == 0 )
//    // No elements to delete!
//    return 1;
//  if ( ver_pos >= _ver_id_max )
//    // Vertical index out of bounds! Nothing to delete!
//    return 2;
//  if ( hor_pos >= _hor_id_max )
//    // Horizontal index out of bounds! Nothing to delete!
//    return 3;

  size_t k = 0;
  bool el_found = false;

  for ( k = 0 ; k != _elementCountMax ; ++k ){

    if ( (!el_found) &&
         (_position[k] == make_pair( (int) ver_pos, (int) hor_pos)) ){
      // element @ position (ver_pos, hor_pos) found at pipeline line n=k
      el_found = true;

      // decrease element count
      --_elementCount;

      // reset pipeline line to default values
      _position[k] = make_pair(-1,-1);
      Yconst[k] = 0;
      I0[k] = 0;

      continue;
    }

    if ( el_found ){
      // if element to delete has been found, then technically delete it by
      // overwriting the contents of k-1 line of the pipeline, with those of
      // the k line, for all k lines after line n where the to-be-deleted
      // element was found
      _position[k-1] = _position[k];
      Yconst[k-1] = Yconst[k];
      I0[k-1] = I0[k];
    }

  }

//  if ( !el_found )
//    // No element @pos (ver_pos, hor_pos); nothing to delete!
//    return 4;

  return 0;
}
