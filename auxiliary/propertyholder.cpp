
#include "propertyholder.h"
using namespace elabtsaot;

//#include <vector>
using std::vector;
//#include <list>
using std::list;
//#include <map>
using std::map;

vector<property_type> PropertyHolder::getPropertyTypes() const{
  vector<property_type> options;
  for ( property_tuples::const_iterator it = _properties.begin() ;
        it != _properties.end() ; ++it )
    options.push_back( it->first );
  return options;
}

list<property> PropertyHolder::getPropertyList() const{
  list<property> propertyList;
  for ( property_tuples::const_iterator it = _properties.begin() ;
        it != _properties.end() ; ++it ){
    property tempProp;
    tempProp.key = (it->first).key;
    tempProp.value = it->second;
    propertyList.push_back( tempProp );
  }
  return propertyList;
}

void PropertyHolder::updateProperties( list<property> const& properties ){
  property_tuples::iterator itOpt;
  for ( list<property>::const_iterator it = properties.begin() ;
        it != properties.end() ; ++it ){
    itOpt = _getPropertyTupleFromKey(it->key);
    if ( itOpt != _properties.end() ){
      itOpt->second = it->value;
    }
  }
}

int PropertyHolder::updateProperty( property const& property_ ){
  // Check if property exists
  property_tuples::iterator it;
  it = _getPropertyTupleFromKey(property_.key);
  if ( it == _properties.end() )
    return 1;

//  // TODO: validate input argument
//  property_type prop_type = it->first;
//  if (    property_.value < prop_type.minValue
//       || property_.value > prop_type.minValue  )
//    return 2; // val not respecting limits

  // register new value and return
  it->second = property_.value;
  return 0;
}

boost::any PropertyHolder::_getPropertyValueFromKey( int key ) const{
  property_tuples::const_iterator it;
  it = _getPropertyTupleFromKey( key );
  return it->second;
}

property_tuples::const_iterator PropertyHolder::_getPropertyTupleFromKey( int key ) const{
  property_tuples::const_iterator it;
  for ( it = _properties.begin() ; it != _properties.end() ; ++it ){
    if ( it->first.key == key )
      break;
  }
  // If property has not been found this would return an interator to .end()
  return it;
}

property_tuples::iterator PropertyHolder::_getPropertyTupleFromKey( int key ){
  property_tuples::iterator it;
  for ( it = _properties.begin() ; it != _properties.end() ; ++it ){
    if ( it->first.key == key )
      break;
  }
  // If property has not been found this would return an interator to .end()
  return it;
}
