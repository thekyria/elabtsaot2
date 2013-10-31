
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
    _elementCountMax(element_capacity), _elementCount(0),
    _verIdMax(vertical_dim), _horIdMax(horizontal_dim),
    _position( element_capacity, make_pair(-1,-1) ) {}

void Pipeline::reset(){
  // element_count_max, ver_id_max, hor_id_max remain unchanged
  _elementCount = 0;
  _position.clear();
  _position.resize(_elementCountMax, make_pair(-1,-1) );
}

int Pipeline::search_element(size_t row, size_t col) const{
  int found_index = -1;
  for ( size_t k = 0 ; k != _elementCountMax ; ++k ){
    if ( _position[k] == make_pair((int) row, (int) col) ) {
      // element at atom set pos (row,col) has been found at pipeline
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
size_t Pipeline::element_count_max() const{ return _elementCountMax; }
size_t Pipeline::element_count() const{ return _elementCount; }
size_t Pipeline::ver_id_max() const{ return _verIdMax; }
size_t Pipeline::hor_id_max() const{ return _horIdMax; }
vector<pair<int, int> > Pipeline::position() const{  return _position; }

size_t Pipeline::calculate_pseudo_id(size_t row, size_t col) const{
  return row*_horIdMax + col;
}
