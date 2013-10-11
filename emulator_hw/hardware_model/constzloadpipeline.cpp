
#include "constzloadpipeline.h"
using namespace elabtsaot;

#include "load.h"

// #include <complex>
using std::complex;
using std::conj;
// #include <utility> // from parent class
using std::make_pair;
//#include <vector> // from parent class
using std::vector;

ConstZLoadPipeline::ConstZLoadPipeline(size_t element_capacity,
                                       size_t ver_dim, size_t hor_dim) :
    Pipeline(element_capacity, ver_dim, hor_dim),
    _elPtrs( _element_count_max, NULL ),
    _real_Y(_element_count_max, 0),
    _imag_Y(_element_count_max, 0) {}

int ConstZLoadPipeline::reset(){
  // Invoke reset of parent class
  int ans = Pipeline::reset();
  if ( ans ) return 1;
  // element_count_max, ver_id_max, hor_id_max remain unchanged

  _elPtrs.clear();
  _real_Y.clear();
  _imag_Y.clear();

  _elPtrs.resize( _element_count_max, NULL );
  _real_Y.resize( _element_count_max, 0 );
  _imag_Y.resize( _element_count_max, 0 );

  return 0;
}

int ConstZLoadPipeline::insert_element( size_t ver_pos,
                                        size_t hor_pos,
                                        Load const& el,
                                        bool overwrite){

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
      // loop broken when one gen with a greater pseudo id is found
      // at pipeline position k (for extreme case element_count==0: k==0)
      break;
  }

  if ( !pos_already_taken )
    // Make space for new element
    for ( m = _element_count ; m != k ; --m ){
      _position[m] = _position[m-1];
      _elPtrs[m] = _elPtrs[m-1];
      _real_Y[m] = _real_Y[m-1];
      _imag_Y[m] = _imag_Y[m-1];
    }

  // Insert new element at pipeline position k
  _position[k] = make_pair((int) ver_pos, (int) hor_pos);
  complex<double> S(el.pdemand(), el.qdemand());
  complex<double> U(el.Uss());
  complex<double> Y = conj(S) / (U*conj(U));
  _real_Y[k] = real(Y);
  _imag_Y[k] = imag(Y);

  // Store pointer to the element
  _elPtrs[k] = &el;

  if ( !pos_already_taken )
    ++_element_count;

  return 0;
}

int ConstZLoadPipeline::remove_element( size_t ver_pos,
                                        size_t hor_pos){
  if ( _element_count == 0 )
    // No elements to delete!
    return 1;
  if ( ver_pos >= _ver_id_max )
    // Vertical index out of bounds! Nothing to delete!
    return 2;
  if ( hor_pos >= _hor_id_max )
    // Horizontal index out of bounds! Nothing to delete!
    return 3;

  size_t k = 0;
  bool el_found = false;

  for ( k = 0 ; k != _element_count_max ; ++k ){

    if ( (!el_found) &&
         (_position[k] == make_pair( (int) ver_pos, (int) hor_pos)) ){
      // element @ position (ver_pos, hor_pos) found at pipeline line n=k
      el_found = true;

      // reset pipeline line to default values
      _position[k] = make_pair(-1,-1);
      _elPtrs[k] = NULL;
      _real_Y[k] = 0;
      _imag_Y[k] = 0;

      continue;
    }

    if ( el_found ){
      // if element to delete has been found, then technically delete it by
      // overwriting the contents of k-1 line of the pipeline, with those of
      // the k line, for all k lines after line n where the to-be-deleted
      // element was found
      _position[k-1] = _position[k];
      _elPtrs[k-1] = _elPtrs[k];
      _real_Y[k-1] = _real_Y[k];
      _imag_Y[k-1] = _imag_Y[k];
    }

  }

  if ( !el_found )
    // No element @pos (ver_pos, hor_pos); nothing to delete!
    return 4;

  --_element_count;

  return 0;
}

int ConstZLoadPipeline::set_real_Y(size_t pos, double val){
  if ( pos >= _element_count_max )
    // Position out of bounds!
    return 1;
  _real_Y[pos] = val;
  return 0;
}

int ConstZLoadPipeline::set_imag_Y(size_t pos, double val){
  if ( pos >= _element_count_max )
    // Position out of bounds!
    return 1;
  _imag_Y[pos] = val;
  return 0;
}

vector<Load const*> ConstZLoadPipeline::elPtrs() const{ return _elPtrs; }
vector<double> ConstZLoadPipeline::real_Y() const{ return _real_Y; }
vector<double> ConstZLoadPipeline::imag_Y() const{ return _imag_Y; }
