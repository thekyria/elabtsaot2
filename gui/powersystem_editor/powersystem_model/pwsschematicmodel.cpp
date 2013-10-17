
#include "pwsschematicmodel.h"
using namespace elabtsaot;

#include "powersystem.h"
#include "graph.h"

#include <vector>
using std::vector;
using std::pair;
#include <iostream>
using std::cout;
using std::endl;

PwsSchematicModel::PwsSchematicModel( Powersystem const* pws ) :
    PwsModel(pws){
  init();
}

PwsSchematicModel::PwsSchematicModel() : PwsModel(NULL) {}

int PwsSchematicModel::init(){

  // ----- Validate internal pointers -----
  if ( _pws == NULL )
    return -1;

  // ----- Initialize model according to pws -----
  this->clear();
  size_t k, n;
  int ans = 0;
  n = _pws->getBusSet_size();
  for( k = 0 ; k != n ; ++k ){
    Bus const* bus = _pws->getBus( k );
    ans |= this->addBusElement( *bus, true );
  }
  n = _pws->getBrSet_size();
  for( k = 0 ; k != n ; ++k ){
    Branch const* br = _pws->getBranch(k);
    ans |= this->addBranchElement( *br, true );
  }
  n = _pws->getGenSet_size();
  for( k = 0 ; k != n ; ++k ){
    Generator const* gen = _pws->getGenerator(k);
    ans |= this->addGenElement( *gen, true );
  }
  n = _pws->getLoadSet_size();
  for( k = 0 ; k != n ; ++k ){
    Load const *load = _pws->getLoad(k);
    ans |= this->addLoadElement( *load, true );
  }

  return ans;
}

int PwsSchematicModel::validate(){

  // ----- Validate internal pointers -----
  if ( _pws == NULL )
    return -1;

  size_t k, m; // counter

  // ----- Check that there are no duplicate elements in the model -----
  unsigned int cur_extId;
  for ( k = 0 ; k != _busElements.size() ; ++k ){
    cur_extId = _busElements[k].extId;
    for ( m = k+1 ; m != _busElements.size() ; ++m ){
      if ( _busElements[m].extId == cur_extId )
        // Duplicate bus entry found!
        return 1;
    }
  }
  for ( k = 0 ; k != _branchElements.size() ; ++k ){
    cur_extId = _branchElements[k].extId;
    for ( m = k+1 ; m != _branchElements.size() ; ++m ){
      if ( _branchElements[m].extId == cur_extId )
        // Duplicate branch entry found!
        return 2;
    }
  }
  for ( k = 0 ; k != _genElements.size() ; ++k ){
    cur_extId = _genElements[k].extId;
    for ( m = k+1 ; m != _genElements.size() ; ++m ){
      if ( _genElements[m].extId == cur_extId )
        // Duplicate gen entry found!
        return 3;
    }
  }
  for ( k = 0 ; k != _loadElements.size() ; ++k ){
    cur_extId = _loadElements[k].extId;
    for ( m = k+1 ; m != _loadElements.size() ; ++m ){
      if ( _loadElements[m].extId == cur_extId )
        // Duplicate gen entry found!
        return 4;
    }
  }

  bool found;
  // ----- Ensure that each powersystem element has a respective element -----
  for ( k = 0; k != _pws->getBusSet_size() ; ++k ){
    cur_extId = _pws->getBus_extId(k);
    found = false;
    for ( m = 0 ; m != _busElements.size() ; ++m ){
      if ( _busElements[k].extId == cur_extId ){
        found = true;
        break;
      }
    }
    if ( !found )
      // No corresponding bus model element was found for bus k
      return 11;
  }
  for ( k = 0; k != _pws->getBrSet_size() ; ++k ){
    cur_extId = _pws->getBr_extId(k);
    found = false;
    for ( m = 0 ; m != _branchElements.size() ; ++m ){
      if ( _branchElements[k].extId == cur_extId ){
        found = true;
        break;
      }
    }
    if ( !found )
      // No corresponding branch model element was found for branch k
      return 11;
  }
  for ( k = 0; k != _pws->getGenSet_size() ; ++k ){
    cur_extId = _pws->getGen_extId(k);
    found = false;
    for ( m = 0 ; m != _genElements.size() ; ++m ){
      if ( _genElements[k].extId == cur_extId ){
        found = true;
        break;
      }
    }
    if ( !found )
      // No corresponding gen model element was found for gen k
      return 11;
  }
  for ( k = 0; k != _pws->getLoadSet_size() ; ++k ){
    cur_extId = _pws->getLoad_extId(k);
    found = false;
    for ( m = 0 ; m != _loadElements.size() ; ++m ){
      if ( _loadElements[k].extId == cur_extId ){
        found = true;
        break;
      }
    }
    if ( !found )
      // No corresponding load model element was found for load k
      return 11;
  }

  return 0;
}

int PwsSchematicModel::copy(PwsSchematicModel& other){

  // Check for self assignment
  if ( this == &other )
    return 1;

  _busElements = other._busElements.release();
  _branchElements = other._branchElements.release();
  _genElements = other._genElements.release();
  _loadElements = other._loadElements.release();

  return 0;
}

int PwsSchematicModel::planarizeSchematic( Powersystem const* pws ){

  // --- Draw planarized schematic ---
  // Initialize topology graph
  vector< pair<int,int> > edges;
  size_t k, n;
  n = pws->getBrSet_size();
  for( k = 0 ; k != n ; ++k ){
    // Get next branch
    Branch const* br = pws->getBranch(k);
    if( !br ) break;
    int fr = pws->getBus_intId( br->fromBusExtId );
    int to = pws->getBus_intId( br->toBusExtId );
    if( fr<0 || to<0 )
      return 1;
    // Add edge
    pair<int,int> e = pair<int,int>(fr,to);
    edges.push_back( e );
  }
  // Count how many buses exist in the network
  n = pws->getBusSet_size();
  if ( n <= 0 ) return 2; // Planarization of an empty powersystem is trivial!
  // Calculate planarized graph
  vector< pair<int,int> > pos;
  if (n > 0)
    pos = graph::planarizeGraph(n, edges);

  // ---- Update PwsSchematicModelElements with their respective positions ----
  int ans = 0;
  n = pws->getBusSet_size();
  for( k = 0 ; k != n ; ++k ){
    Bus const* bus = pws->getBus( k );
    ans |= addBusElement( *bus, 60*pos[k].first, 30*pos[k].second, true, true );
  }
  // ... with branches
  n = pws->getBrSet_size();
  for( k = 0 ; k != n ; ++k ){
    Branch const* br = pws->getBranch(k);
    ans |= this->addBranchElement( *br, true );
  }

  // ... with generators
  n = pws->getGenSet_size();
  for( k = 0 ; k != n ; ++k ){
    Generator const* gen = pws->getGenerator(k);
    ans |= addGenElement( *gen, true );
  }

  // ... with loads
  n = pws->getLoadSet_size();
  for( k = 0 ; k != n ; ++k ){
    Load const *load = pws->getLoad(k);
    ans |= addLoadElement( *load, true );
  }

  return ans;
}

int PwsSchematicModel::addBusElement( Bus bus, int x, int y, bool horizontal,
                                      bool overwrite, int* mdlId ){

  // Add bus to the model
  int tempMdlId;
  int ans = addBusElement(bus, overwrite, &tempMdlId );
  if ( ans ){
    return 1;
  }
  PwsSchematicModelElement* cd;
  cd = elementByIndex( PWSMODELELEMENTTYPE_BUS,
                       static_cast<size_t>(tempMdlId));
  if ( mdlId )
    *mdlId = tempMdlId;

  // Set exact place for the bus
  cd->x1 = x;
  cd->y1 = y;
  if( horizontal ){
    cd->x2 = cd->x1 + 30;
    cd->y2 = cd->y1 + 5;
  }else{
    cd->x2 = cd->x1 + 5;
    cd->y2 = cd->y1 + 30;
  }

  return 0;
}

int PwsSchematicModel::addElement( int type, unsigned int extId,
                                   bool overwrite, int* mdlId ){

  std::auto_ptr<PwsSchematicModelElement>
      cd ( new PwsSchematicModelElement(type, extId) );

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

int PwsSchematicModel::addBusElement( Bus bus, bool overwrite, int* mdlId ){

  int ans = addElement(PWSMODELELEMENTTYPE_BUS, bus.extId, overwrite, mdlId);
  if ( ans ){
//    cout << "Error adding bus component (" << bus.extId() << ")!" << endl;
    return 1;
  }

  return 0;
}

int PwsSchematicModel::addBranchElement( Branch br, bool overwrite, int* mdlId ){

  // Add branch to model
  int tempMdlId;
  int ans = addElement( PWSMODELELEMENTTYPE_BR, br.extId, overwrite, &tempMdlId );
  if ( ans ){
//    cout << "Error adding branch component (" << br.extId() << ")!" << endl;
    return 1;
  }
  PwsSchematicModelElement* cd = elementByIndex(PWSMODELELEMENTTYPE_BR,
                                          static_cast<size_t>(tempMdlId));
  if (mdlId)
    *mdlId = tempMdlId;

  // Retrieve buses' elements
  PwsSchematicModelElement const* cd1;
  cd1 = element(PWSMODELELEMENTTYPE_BUS, br.fromBusExtId );
  if( cd1 == NULL )
    return 2;
  PwsSchematicModelElement const* cd2;
  cd2 = element(PWSMODELELEMENTTYPE_BUS, br.toBusExtId );
  if( cd2 == NULL )
    return 3;

  // Resolve exact place for the branch
  cd->x1 = ( cd1->x1 + cd1->x2 )/2; // the middle of the first bus
  cd->y1 = ( cd1->y1 + cd1->y2 )/2;
  cd->x2 = ( cd2->x1 + cd2->x2 )/2; // the middle of the second bus
  cd->y2 = ( cd2->y1 + cd2->y2 )/2;

  return 0;
}

int PwsSchematicModel::addGenElement( Generator gen, bool overwrite, int* mdlId ){

  // Add gen to model
  int tempMdlId;
  int ans = addElement( PWSMODELELEMENTTYPE_GEN, gen.extId, overwrite, &tempMdlId );
  if ( ans ){
//    cout << "Error adding gen component (" << gen.extId() << ")!" << endl;
    return 1;
  }
  PwsSchematicModelElement* cd;
  cd = elementByIndex( PWSMODELELEMENTTYPE_GEN, static_cast<size_t>(tempMdlId));
  if (mdlId)
    *mdlId = tempMdlId;

  // Retrive the model element of the bus of the generator
  PwsSchematicModelElement const* cdBus;
  cdBus = element(PWSMODELELEMENTTYPE_BUS, gen.busExtId );
  if( cdBus == NULL )
    return 2;

  // Resolve exact place for the generator
  cd->y1 = (cdBus->y1 + cdBus->y2)/2;
  cd->x1 = (cdBus->x1 + cdBus->x2)/2;
  if( abs( cdBus->y2-cdBus->y1 ) == 5 ){ // Bus is horizontal
    cd->x1 -= 10;
    cd->x2 = cd->x1 + 20;
    cd->y2 = cd->y1 - 25;
  } else {
    cd->y1 -= 10;
    cd->x2 = cd->x1 - 25;
    cd->y2 = cd->y1 + 20;
  }

  return 0;
}

int PwsSchematicModel::addLoadElement( Load load,
                                       bool overwrite, int* mdlId ){

  // Add load to model
  int tempMdlId;
  int ans = addElement( PWSMODELELEMENTTYPE_LOAD, load.extId, overwrite, &tempMdlId );
  if ( ans ){
//    cout << "Error adding load component (" << load.extId() << ")!" << endl;
    return 1;
  }
  PwsSchematicModelElement* cd;
  cd = elementByIndex( PWSMODELELEMENTTYPE_LOAD,
                       static_cast<size_t>(tempMdlId) );
  if (mdlId)
    *mdlId = tempMdlId;

  // Retrive the model element of the bus of the load
  PwsSchematicModelElement const* cdBus;
  cdBus = element(PWSMODELELEMENTTYPE_BUS, load.busExtId );
  if( cdBus == NULL )
    return 1;

  // Resolve exact place for the load
  cd->y1 = (cdBus->y1 + cdBus->y2)/2;
  cd->x1 = (cdBus->x1 + cdBus->x2)/2;
  if( abs( cdBus->y2-cdBus->y1 ) == 5 ){ // Bus is horizontal
    cd->x1 -= 10;
    cd->x2 = cd->x1 + 20;
    cd->y2 = cd->y1 - 25;
  }else{
    cd->y1 -= 10;
    cd->x2 = cd->x1 - 25;
    cd->y2 = cd->y1 + 20;
  }

  return 0;
}

PwsSchematicModelElement*
PwsSchematicModel::element( int type, unsigned int extId, int* mdlId){

  switch(type){

  case PWSMODELELEMENTTYPE_BUS: // Buses
    for( size_t k = 0 ; k != _busElements.size() ; ++k ){
      if( _busElements.at(k).extId == extId ){
        if ( mdlId )
          // If output argument mdlId has been initialized (asked for) then
          // return the mmd component model (internal) index of the found comp
          *mdlId = k;
        return dynamic_cast<PwsSchematicModelElement*>(&_busElements[k]);
      }
    }
    break;

  case PWSMODELELEMENTTYPE_BR: // Branches
    for( size_t k = 0 ; k != _branchElements.size() ; ++k ){
      if( _branchElements.at(k).extId == extId ){
        if ( mdlId )
          // If output argument mdlId has been initialized (asked for) then
          // return the mmd component model (internal) index of the found comp
          *mdlId = k;
        return dynamic_cast<PwsSchematicModelElement*>(&_branchElements[k]);
      }
    }
    break;

  case PWSMODELELEMENTTYPE_GEN: // Generators
    for( size_t k = 0 ; k != _genElements.size() ; ++k ){
      if( _genElements.at(k).extId == extId ){
        if ( mdlId )
          // If output argument mdlId has been initialized (asked for) then
          // return the mmd component model (internal) index of the found comp
          *mdlId = k;
        return dynamic_cast<PwsSchematicModelElement*>(&_genElements[k]);
      }
    }
    break;

  case PWSMODELELEMENTTYPE_LOAD: // Loads
    for( size_t k = 0 ; k != _loadElements.size() ; ++k ){
      if( _loadElements.at(k).extId == extId ){
        if ( mdlId )
          // If output argument mdlId has been initialized (asked for) then
          // return the mmd component model (internal) index of the found comp
          *mdlId = k;
        return dynamic_cast<PwsSchematicModelElement*>(&_loadElements[k]);
      }
    }
    break;

  default: // Any other
    if ( mdlId )
      // If output argument mdlId has been initialized (asked for) then
      // return -1, signifying component not found
      *mdlId = -1;
    return NULL;
    break;

  } // end of switch(type)

  if ( mdlId )
    // If output argument mdlId has been initialized (asked for) then
    // return -1, signifying component not found
    *mdlId = -1;
  return NULL;
}

PwsSchematicModelElement const*
PwsSchematicModel::element( int type, unsigned int extId, int* mdlId) const{

  switch(type){

  case PWSMODELELEMENTTYPE_BUS: // Buses
    for( size_t k = 0 ; k != _busElements.size() ; ++k ){
      if( _busElements.at(k).extId == extId ){
        if ( mdlId )
          // If output argument mdlId has been initialized (asked for) then
          // return the mmd component model (internal) index of the found comp
          *mdlId = k;
        return dynamic_cast<PwsSchematicModelElement const*>(&_busElements[k]);
      }
    }
    break;

  case PWSMODELELEMENTTYPE_BR: // Branches
    for( size_t k = 0 ; k != _branchElements.size() ; ++k ){
      if( _branchElements.at(k).extId == extId ){
        if ( mdlId )
          // If output argument mdlId has been initialized (asked for) then
          // return the mmd component model (internal) index of the found comp
          *mdlId = k;
        return dynamic_cast<PwsSchematicModelElement const*>(&_branchElements[k]);
      }
    }
    break;

  case PWSMODELELEMENTTYPE_GEN: // Generators
    for( size_t k = 0 ; k != _genElements.size() ; ++k ){
      if( _genElements.at(k).extId == extId ){
        if ( mdlId )
          // If output argument mdlId has been initialized (asked for) then
          // return the mmd component model (internal) index of the found comp
          *mdlId = k;
        return dynamic_cast<PwsSchematicModelElement const*>(&_genElements[k]);
      }
    }
    break;

  case PWSMODELELEMENTTYPE_LOAD: // Loads
    for( size_t k = 0 ; k != _loadElements.size() ; ++k ){
      if( _loadElements.at(k).extId == extId ){
        if ( mdlId )
          // If output argument mdlId has been initialized (asked for) then
          // return the mmd component model (internal) index of the found comp
          *mdlId = k;
        return dynamic_cast<PwsSchematicModelElement const*>(&_loadElements[k]);
      }
    }
    break;

  default: // Any other
    if ( mdlId )
      // If output argument mdlId has been initialized (asked for) then
      // return -1, signifying component not found
      *mdlId = -1;
    return NULL;
    break;

  } // end of switch(type)

  if ( mdlId )
    // If output argument mdlId has been initialized (asked for) then
    // return -1, signifying component not found
    *mdlId = -1;
  return NULL;
}

PwsSchematicModelElement*
PwsSchematicModel::elementByIndex( int type, size_t mdlId ){

  switch(type){

  case PWSMODELELEMENTTYPE_BUS: // Buses
    if( mdlId < _busElements.size() )
      return dynamic_cast<PwsSchematicModelElement*>(&_busElements[mdlId]);
    else
      return NULL;
    break;

  case PWSMODELELEMENTTYPE_BR: // Branches
    if( mdlId < _branchElements.size() )
      return dynamic_cast<PwsSchematicModelElement*>(&_branchElements[mdlId]);
    else
      return NULL;
    break;

  case PWSMODELELEMENTTYPE_GEN: // Generators
    if( mdlId < _genElements.size() )
      return dynamic_cast<PwsSchematicModelElement*>(&_genElements[mdlId]);
    else
      return NULL;
    break;

  case PWSMODELELEMENTTYPE_LOAD: // Loads
    if( mdlId < _loadElements.size() )
      return dynamic_cast<PwsSchematicModelElement*>(&_loadElements[mdlId]);
    else
      return NULL;
    break;

  default: // Any other
    return NULL;
    break;

  } // end of switch(type)
}

PwsSchematicModelElement const*
PwsSchematicModel::elementByIndex( int type, size_t mdlId ) const{

  switch(type){

  case PWSMODELELEMENTTYPE_BUS: // Buses
    if( mdlId < _busElements.size() )
      return dynamic_cast<PwsSchematicModelElement const*>
          (&_busElements[mdlId]);
    else
      return NULL;
    break;

  case PWSMODELELEMENTTYPE_BR: // Branches
    if( mdlId < _branchElements.size() )
      return dynamic_cast<PwsSchematicModelElement const*>
          (&_branchElements[mdlId]);
    else
      return NULL;
    break;

  case PWSMODELELEMENTTYPE_GEN: // Generators
    if( mdlId < _genElements.size() )
      return dynamic_cast<PwsSchematicModelElement const*>
          (&_genElements[mdlId]);
    else
      return NULL;
    break;

  case PWSMODELELEMENTTYPE_LOAD: // Loads
    if( mdlId < _loadElements.size() )
      return dynamic_cast<PwsSchematicModelElement const*>
          (&_loadElements[mdlId]);
    else
      return NULL;

    break;

  default: // Any other
    return NULL;
    break;

  } // end of switch(type)
}

