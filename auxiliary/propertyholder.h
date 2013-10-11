
#ifndef PROPERTYHOLDER_H
#define PROPERTYHOLDER_H

#include "property.h"
#include "property_t.h"

#include <vector>
#include <list>
#include <map>

namespace elabtsaot{

typedef std::map<property_type,boost::any,compare_property_type> property_tuples;

/* Nomenclature
  'property'       : struct property { int key; boost::any value; }
  'property value' : boost::any
  'property key'   : int
  'property type'  : struct property_t { key, name, description, dataType, ...}
  'property tuple' : map<property_t,boost::any,compare_property_t>::(const_)iterator
*/
class PropertyHolder{

 public:

  std::vector<property_type> getPropertyTypes() const;
  std::list<property> getPropertyList() const;
  virtual void updateProperties( std::list<property> const& properties );
  virtual int updateProperty( property const& property_ );

 protected:

  property_tuples _properties;

  boost::any _getPropertyValueFromKey( int key ) const;
  property_tuples::const_iterator _getPropertyTupleFromKey( int key ) const;
  property_tuples::iterator _getPropertyTupleFromKey( int key );

};

} // end of namespace elabtsaot

#endif // PROPERTYHOLDER_H
