
#ifndef PROPERTY_H
#define PROPERTY_H

#include <boost/any.hpp>

namespace elabtsaot{

struct property {
    property(){}
    property(int key_, boost::any const& value_)
      { key = key_; value = value_; }

    int key;
    boost::any value;
};

} // end of namespace elabtsaot

#endif // PROPERTY_H
