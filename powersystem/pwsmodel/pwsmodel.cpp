
#include "pwsmodel.h"
using namespace elabtsaot;

PwsModel::PwsModel( Powersystem const* pws ) :
    _pws(pws){}

int PwsModel::clear(){

  // Clear component elements
  _busElements.clear();
  _branchElements.clear();
  _genElements.clear();
  _loadElements.clear();

  return 0;
}

int PwsModel::addElement( int type, unsigned int extId,
                          bool overwrite, int* mdlId){

  std::auto_ptr<PwsModelElement>
      cd ( new PwsModelElement(type, extId) );

  int tempMdlId = getElement_mdlId( cd->type, cd->extId );

  if ( tempMdlId > -1 ){
    // Component already exists in the model

    if ( overwrite ){
      // If 'overwrite' flag is set replace the component by its new element
      switch(cd->type){
      case PWSMODELELEMENTTYPE_BUS:
        _busElements[tempMdlId] = *cd;
        break;
      case PWSMODELELEMENTTYPE_BR:
        _branchElements[tempMdlId] = *cd;
        break;
      case PWSMODELELEMENTTYPE_GEN:
        _genElements[tempMdlId] = *cd;
        break;
      case PWSMODELELEMENTTYPE_LOAD:
        _loadElements[tempMdlId] = *cd;
        break;
      default:
        if ( mdlId )
          *mdlId = tempMdlId;
        return -1;
        break;
      }

    } else{ // ( !overwrite )
      // If 'overwrite' flag is not set then return 1 to signify unsuccessful
      // operation!
      return 1;
    }

  } else{ // ( tempMdlId == -1 )
    // New component
    // So add its element to the respective xxDescriptors vector
    switch(cd->type){

    case PWSMODELELEMENTTYPE_BUS: // Buses
      _busElements.push_back( cd );
      tempMdlId = _busElements.size()-1;
      break;

    case PWSMODELELEMENTTYPE_BR: // Branches
      _branchElements.push_back( cd );
      tempMdlId = _branchElements.size()-1;
      break;

    case PWSMODELELEMENTTYPE_GEN: // Generators
      _genElements.push_back( cd );
      tempMdlId = _genElements.size()-1;
      break;

    case PWSMODELELEMENTTYPE_LOAD: // Loads
      _loadElements.push_back( cd );
      tempMdlId = _loadElements.size()-1;
      break;

    default: // Any other
      tempMdlId = -1;
      if ( mdlId )
        *mdlId = tempMdlId;
      return -1;
      break;
    }
  }

  if ( mdlId )
    *mdlId = tempMdlId;

  return 0;
}

int PwsModel::remElement( int type, unsigned int extId ){

  // TODO: check argument validity!
  int mdlId = getElement_mdlId( type, extId );

  switch ( type ){
  case PWSMODELELEMENTTYPE_BUS:
    _busElements.erase( _busElements.begin()+mdlId );
    break;
  case PWSMODELELEMENTTYPE_BR:
    _branchElements.erase( _branchElements.begin()+mdlId );
    break;
  case PWSMODELELEMENTTYPE_GEN:
    _genElements.erase( _genElements.begin()+mdlId );
    break;
  case PWSMODELELEMENTTYPE_LOAD:
    _loadElements.erase( _loadElements.begin()+mdlId );
    break;
  }

  return 0;
}

int PwsModel::getElement_mdlId( int type, unsigned int extId) const{
  int ans = -1;        // return value
  size_t k; // counter

  switch ( type ){
  case PWSMODELELEMENTTYPE_BUS:
    for ( k = 0; k != _busElements.size() ; ++k ){
      if ( _busElements.at(k).extId == extId ){
        ans = k;
        break;
      }
    }
    break;
  case PWSMODELELEMENTTYPE_BR:
    for ( k = 0; k != _branchElements.size() ; ++k ){
      if ( _branchElements.at(k).extId == extId ){
        ans = k;
        break;
      }
    }
    break;
  case PWSMODELELEMENTTYPE_GEN:
    for ( k = 0; k != _genElements.size() ; ++k ){
      if ( _genElements.at(k).extId == extId ){
        ans = k;
        break;
      }
    }
    break;
  case PWSMODELELEMENTTYPE_LOAD:
    for ( k = 0; k != _loadElements.size() ; ++k ){
      if ( _loadElements.at(k).extId == extId ){
        ans = k;
        break;
      }
    }
    break;
  default:
    ans = -1;
  }

  return ans;
}

size_t PwsModel::busElements_size() const{
  return _busElements.size();
}
size_t PwsModel::branchElements_size() const{
  return _branchElements.size();
}
size_t PwsModel::genElements_size() const{
  return _genElements.size();
}
size_t PwsModel::loadElements_size() const{
  return _loadElements.size();
}
