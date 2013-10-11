
#include "pipeline.h"
using namespace elabtsaot;

//#include <string>
using std::string;
//#include <vector>
using std::vector;
// #include <utility>
using std::pair;
using std::make_pair;

Pipeline::Pipeline(size_t element_capacity,
                   size_t vertical_dim,
                   size_t horizontal_dim) :
    _element_count_max(element_capacity), _element_count(0),
    _ver_id_max(vertical_dim), _hor_id_max(horizontal_dim),
    _position( element_capacity, make_pair(-1,-1) ) {}

int Pipeline::reset(){
  // element_count_max, ver_id_max, hor_id_max remain unchanged
  _element_count = 0;
  _position.clear();
  _position.resize(_element_count_max, make_pair(-1,-1) );
  return 0;
}

int Pipeline::search_element(size_t ver_pos, size_t hor_pos) const{
  int found_index = -1;
  for ( size_t k = 0 ; k != _element_count_max ; ++k ){
    if ( _position[k] == make_pair((int) ver_pos, (int) hor_pos) ) {
      // element at atom set pos (ver_pos,hor_pos) has been found at pipeline
      // pos k
      found_index = k;
      break;
    }
    if ( _position[k] == make_pair( -1,-1 ) )
      // element has not been found in pipeline; loop broken and -1 returned
      break;
  }
  return found_index;
}

// Low level getters
vector<string> Pipeline::stages() const{ return _stages; }
size_t Pipeline::element_count_max() const{ return _element_count_max; }
size_t Pipeline::element_count() const{ return _element_count; }
size_t Pipeline::ver_id_max() const{ return _ver_id_max; }
size_t Pipeline::hor_id_max() const{ return _hor_id_max; }
vector<pair<int, int> > Pipeline::position() const{  return _position; }

size_t Pipeline::calculate_pseudo_id(size_t ver_pos, size_t hor_pos) const{
  return ver_pos*_hor_id_max + hor_pos;
}
