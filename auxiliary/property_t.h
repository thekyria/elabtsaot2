
#ifndef PROPERTY_T_H
#define PROPERTY_T_H

#include <boost/any.hpp>

#include <string>

namespace elabtsaot{

enum property_tDataTypes{
  PROPERTYT_DTYPE_INT,
  PROPERTYT_DTYPE_DOUBLE,
  PROPERTYT_DTYPE_BOOL
};

struct property_type {
    property_type(){}
    property_type(int key_,
                  std::string const& name_,
                  std::string const& description_,
                  int dataType_,
                  boost::any const& defaultValue_,
                  boost::any const& minValue_,
                  boost::any const& maxValue_ ){
      key = key_;
      name = name_;
      description = description_;
      dataType = dataType_;
      defaultValue = defaultValue_;
      minValue = minValue_;
      maxValue = maxValue_;
    }

    int key;
    std::string name;
    std::string description;
    int dataType;
    boost::any defaultValue;
    boost::any minValue;
    boost::any maxValue;
};

class compare_property_type { // simple comparison function
   public:
      bool operator()(property_type const x, property_type const y) {
        return (x.key-y.key)>0;
//        return (x-y)>0; // for int it would return x>y
      }
};

} // end of namespace elabtsaot

#endif // PROPERTY_T_H
