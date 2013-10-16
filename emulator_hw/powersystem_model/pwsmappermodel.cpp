
#include "pwsmappermodel.h"
using namespace elabtsaot;

#include "powersystem.h"
#include "emulatorhw.h"
#include "graph.h"
#include "auxiliary.h"

//#include <vector>
using std::vector;
#include <stack>
using std::stack;
#include <set>
using std::set;
#include <iostream>
using std::cout;
using std::endl;
#include <utility>
using std::make_pair;
using std::pair;

#ifndef VERBOSE_MMD
#define VERBOSE_MMD 0
#endif // VERBOSE_MMD

PwsMapperModel::PwsMapperModel( Powersystem const* pws, EmulatorHw const* emuhw ) :
    PwsModel(pws),
    _emuhw(emuhw),
    _tabs(0), _rows(0), _cols(0),
    _hints(NULL), _mask(NULL) {
  init();
}

PwsMapperModel::~PwsMapperModel(){ _destroy(); }

int PwsMapperModel::init(){

  // ----- Validate internal pointers -----
  if ( _emuhw == NULL ) return 1;
  if ( _pws == NULL ) return 2;

  // ----- Allocate memory for hint and mask arrays -----
  if ( _tabs ) // if memory has already been allocated for _hints & _mask
    _destroy(); // deallocates already allocated memory for hint and mask arrays
  _tabs = _emuhw->sliceSet.size();
  if ( _tabs > 0 ){
    _emuhw->sliceSet[0].ana.size(_rows, _cols);

    _hints = new int**[2*_tabs - 1];
    _mask  = new int**[2*_tabs - 1];
    for ( size_t k = 0; k != 2*_tabs - 1; ++k ){
      _hints[k] = new int*[2*_rows - 1];
      _mask[k] = new int*[2*_rows - 1];
      for ( size_t m = 0; m != 2*_rows - 1; ++m ){
        _hints[k][m] = new int[2*_cols - 1];
        _mask[k][m] = new int[2*_cols - 1];
        for ( size_t n = 0; n != 2*_cols - 1; ++n ){
          _hints[k][m][n] = MAPHINT_INACTIVE;
          _mask[k][m][n] = MAPHINT_INACTIVE;
        }
      }
    }
  }
//  else {
//    _rows = NULL;
//    _cols = NULL;
//  }

  // ----- Initialize mmd model accoring to pws -----
  clear();
  size_t k, n;
  int ans = 0;
  n = _pws->getBusSet_size();
  for ( k = 0 ; k != n ; ++k ){
    Bus const* bus = _pws->getBus( k );
    ans |= addBusElement( *bus );
  }
  n = _pws->getBrSet_size();
  for ( k = 0 ; k != n ; ++k ){
    Branch const* br = _pws->getBranch( k );
    ans |= addBranchElement( *br );
  }
  n = _pws->getGenSet_size();
  for ( k = 0 ; k != n ; ++k ){
    Generator const* gen = _pws->getGenerator( k );
    ans |= addGenElement( *gen );
  }
  n = _pws->getLoadSet_size();
  for ( k = 0 ; k != n ; ++k ){
    Load const* load = _pws->getLoad( k );
    ans |= addLoadElement( *load );
  }

  return ans;
}

void PwsMapperModel::_destroy(){
  if ( _hints != NULL )
    delete _hints;
  if ( _mask != NULL )
    delete _mask;
}

int PwsMapperModel::copy(PwsMapperModel& other){

  // Check for self assignment
  if ( this == &other )
    return 1;

  _busElements = other._busElements.release();
  _branchElements = other._branchElements.release();
  _genElements = other._genElements.release();
  _loadElements = other._loadElements.release();

  _pws = other._pws;
  _emuhw = other._emuhw;
//  _tabs = other._tabs; // gets calculated hereunder
//  _rows = other._rows; // gets calculated hereunder
//  _cols = other._cols; // gets calculated hereunder

  // ----- Allocate memory for new hint and mask arrays -----
  _destroy(); // deallocates already allocated memory for hint and mask arrays
  _tabs = _emuhw->sliceSet.size();
  if ( _tabs > 0 ){
    _emuhw->sliceSet[0].ana.size(_rows,_cols);

    _hints = new int**[2*_tabs - 1];
    _mask  = new int**[2*_tabs - 1];
    for ( size_t k = 0; k != 2*_tabs - 1; ++k ){
      _hints[k] = new int*[2*_rows - 1];
      _mask[k] = new int*[2*_rows - 1];
      for ( size_t m = 0; m != 2*_rows - 1; ++m ){
        _hints[k][m] = new int[2*_cols - 1];
        _mask[k][m] = new int[2*_cols - 1];
        for ( size_t n = 0; n != 2*_cols - 1; ++n ){
          _hints[k][m][n] = MAPHINT_INACTIVE;
          _mask[k][m][n] = MAPHINT_INACTIVE;
        }
      }
    }
  }

  return 0;
}

int PwsMapperModel::validate(){

  size_t k; // counter

  size_t tabs = _emuhw->sliceSet.size();
  if ( tabs == 0 )
    // The emulator has an invalid structure
    return 1;

  size_t rows, cols;
  _emuhw->sliceSet[0].ana.size(rows,cols); // sliceSet[0] guaranteed to exist
  if ( rows == 0 || cols == 0 )
    // The emulator has invalid structure!
    return 1;

  // ------- Check that all buses are mapped to node valid positions -------
  PwsMapperModelElement const* cdBus;
  for ( k = 0 ; k != _busElements.size() ; ++k ){
    cdBus = elementByIndex( PWSMODELELEMENTTYPE_BUS, k );

    // Assert that bus is mapped
    if ( !cdBus->mapped )
      return 10;

    // Assert that it is mapped onto a valid position
    if (    cdBus->tab < 0 || cdBus->tab > 2*static_cast<int>(tabs)-2
         || cdBus->row < 0 || cdBus->row > 2*static_cast<int>(rows)-2
         || cdBus->col < 0 || cdBus->col > 2*static_cast<int>(cols)-2 )
      return 11;
    if ( !(cdBus->tab%2==0 && cdBus->row%2==0 && cdBus->col%2==0) )
      return 12;
  }

  // ------- Check that all generators are mapped on the same node as -------
  // ------- their buses                                              -------
  PwsMapperModelElement const* cdGen;
  Generator const* pGen;
  for ( k = 0 ; k != _genElements.size() ; ++k ){

    // Retrieve generator component element
    cdGen = elementByIndex( PWSMODELELEMENTTYPE_GEN, k );
    if ( _pws->getGenerator( cdGen->extId, pGen ) ){
      cout << "Generator with mdlId: " << k << " "
           << "and extId: " << cdGen->extId << " "
           << "cannot be properly retrieved!" << endl;
      return 20;
    }

    // Assert that all generators have been mapped
    if ( !cdGen->mapped )
      return 21;

    // Retrieve generator bus component element
    cdBus = element(PWSMODELELEMENTTYPE_BUS, pGen->busExtId() );

    // Compare mapping
    if (    cdGen->tab != cdBus->tab
         || cdGen->row != cdBus->row
         || cdGen->col != cdBus->col )
      return 22;
  }

  // ------- Check that all loads are mapped on the same node as -------
  // ------- their buses                                         -------
  PwsMapperModelElement const* cdLoad;
  Load const* pLoad;
  for ( k = 0 ; k != _loadElements.size() ; ++k ){

    // Retrieve load component element
    cdLoad = elementByIndex( PWSMODELELEMENTTYPE_LOAD, k );
    if ( _pws->getLoad( cdLoad->extId, pLoad ) ){
      cout << "Load with mdlId: " << k << " "
           << "and extId: " << cdLoad->extId << " "
           << "cannot be properly retrieved!" << endl;
      return 30;
    }

    // Assert that all loads have been mapped
    if ( !cdLoad->mapped )
      return 31;

    // Retrieve load bus component element
    cdBus = element(PWSMODELELEMENTTYPE_BUS, pLoad->busExtId );

    // Compare mapping
    if (    cdLoad->tab != cdBus->tab
         || cdLoad->row != cdBus->row
         || cdLoad->col != cdBus->col )
      return 32;
  }

  // ------- Check that all branches are mapped to positions adjacent to -------
  // ------- their end buses                                             -------
  PwsMapperModelElement const* cdBr;
  PwsMapperModelElement const* cdBusF;
  PwsMapperModelElement const* cdBusT;
  Branch const* pBr;
  bool onTab, onRow, onCol;
  for ( k = 0 ; k != _branchElements.size() ; ++k ){

    // Retrieve branch component
    cdBr = elementByIndex( PWSMODELELEMENTTYPE_BR, k );
    if ( _pws->getBranch( cdBr->extId, pBr ) ){
      cout << "Branch with mdlId: " << k << " "
           << "and extId: " << cdBr->extId << " "
           << "cannot be properly retrieved!" << endl;
      return 40;
    }
    // Retrive end bus components
    cdBusF = element( PWSMODELELEMENTTYPE_BUS, pBr->fromBusExtId );
    cdBusT = element( PWSMODELELEMENTTYPE_BUS, pBr->toBusExtId );

    // Assert that the branch is mapped
    if ( !cdBr->mapped )
      return 41;

    // Assert that is is mapped onto a valid position-within limits and non-node
    if (    cdBr->tab < 0 || cdBr->tab > 2*static_cast<int>(tabs)-2
         || cdBr->row < 0 || cdBr->row > 2*static_cast<int>(rows)-2
         || cdBr->col < 0 || cdBr->col > 2*static_cast<int>(cols)-2 )
      return 42;
    onTab = (cdBr->tab%2 == 0);
    onRow = (cdBr->row%2 == 0);
    onCol = (cdBr->col%2 == 0);
    if ( onTab && onRow && onCol )
      return 43;

    if ( onTab & onRow & !onCol ){
      // Intratab - horizontal branch
      if (    // From bus not on any of the ends of the branch
              !( cdBusF->tab == cdBr->tab
              && cdBusF->row == cdBr->row
              && cdBusF->col == cdBr->col-1 )
           && !( cdBusF->tab == cdBr->tab
              && cdBusF->row == cdBr->row
              && cdBusF->col == cdBr->col+1 )
           && // To bus not on any of the ends of the branch
              !( cdBusT->tab == cdBr->tab
              && cdBusT->row == cdBr->row
              && cdBusT->col == cdBr->col-1 )
           && !( cdBusT->tab == cdBr->tab
              && cdBusT->row == cdBr->row
              && cdBusT->col == cdBr->col+1 ) )
        return 44;

    } else if ( onTab & !onRow & onCol ){
      // Intratab - vertical branch
      if (    // From bus not on any of the ends of the branch
              !( cdBusF->tab == cdBr->tab
              && cdBusF->row == cdBr->row-1
              && cdBusF->col == cdBr->col )
           && !( cdBusF->tab == cdBr->tab
              && cdBusF->row == cdBr->row+1
              && cdBusF->col == cdBr->col )
           && // To bus not on any of the ends of the branch
              !( cdBusT->tab == cdBr->tab
              && cdBusT->row == cdBr->row-1
              && cdBusT->col == cdBr->col )
           && !( cdBusT->tab == cdBr->tab
              && cdBusT->row == cdBr->row+1
              && cdBusT->col == cdBr->col ) )
      return 45;

    } else if ( onTab & !onRow & !onCol ){
      // Intratab - diagonal branch
      // Intratab diagonal branches can only be in the following sense:
      // bottom-left -> top-right
      if (    // From bus not on any of the ends of the branch
              !( cdBusF->tab == cdBr->tab
              && cdBusF->row == cdBr->row-1
              && cdBusF->col == cdBr->col-1 )
           && !( cdBusF->tab == cdBr->tab
              && cdBusF->row == cdBr->row+1
              && cdBusF->col == cdBr->col+1 )
           && // To bus not on any of the ends of the branch
              !( cdBusT->tab == cdBr->tab
              && cdBusT->row == cdBr->row-1
              && cdBusT->col == cdBr->col-1 )
           && !( cdBusT->tab == cdBr->tab
              && cdBusT->row == cdBr->row+1
              && cdBusT->col == cdBr->col+1 ) )
      return 46;

    } else if ( !onTab & onRow & onCol ){
      // Intertab branch - exactly vertical
      if (    // From bus not on any of the ends of the branch
              !( cdBusF->tab == cdBr->tab-1
              && cdBusF->row == cdBr->row
              && cdBusF->col == cdBr->col )
           && !( cdBusF->tab == cdBr->tab+1
              && cdBusF->row == cdBr->row
              && cdBusF->col == cdBr->col )
           && // To bus not on any of the ends of the branch
              !( cdBusT->tab == cdBr->tab-1
              && cdBusT->row == cdBr->row
              && cdBusT->col == cdBr->col )
           && !( cdBusT->tab == cdBr->tab+1
              && cdBusT->row == cdBr->row
              && cdBusT->col == cdBr->col ) )
      return 47;

    } else if ( !onTab & onRow & !onCol ){
      // Intertab branch - diagonal in the column sense
      // Corresponds to the diagonal intertab branches on the top of the
      // emulator grid
      if (    // From bus not on any of the ends of the branch
              !( cdBusF->tab == cdBr->tab-1
              && cdBusF->row == cdBr->row
              && cdBusF->col == cdBr->col-1 )
           && !( cdBusF->tab == cdBr->tab+1
              && cdBusF->row == cdBr->row
              && cdBusF->col == cdBr->col+1 )
           && // To bus not on any of the ends of the branch
              !( cdBusT->tab == cdBr->tab-1
              && cdBusT->row == cdBr->row
              && cdBusT->col == cdBr->col-1 )
           && !( cdBusT->tab == cdBr->tab+1
              && cdBusT->row == cdBr->row
              && cdBusT->col == cdBr->col+1 ) )
      return 48;

    } else if ( !onTab & !onRow & onCol ){
      // Intertab branch - diagonal in the row sense
      // Corresponds to the diagonal intertab branches on the right side of
      // the emulator grid
      if (    // From bus not on any of the ends of the branch
              !( cdBusF->tab == cdBr->tab-1
              && cdBusF->row == cdBr->row-1
              && cdBusF->col == cdBr->col )
           && !( cdBusF->tab == cdBr->tab+1
              && cdBusF->row == cdBr->row+1
              && cdBusF->col == cdBr->col )
           && // To bus not on any of the ends of the branch
              !( cdBusT->tab == cdBr->tab-1
              && cdBusT->row == cdBr->row-1
              && cdBusT->col == cdBr->col )
           && !( cdBusT->tab == cdBr->tab+1
              && cdBusT->row == cdBr->row+1
              && cdBusT->col == cdBr->col ) )
      return 49;

    } else if ( !onTab & !onRow & !onCol ){
      // Intertab branch
      // Warning! No such intertab branch is possible on the current version
      // of the emulator hardware! Nov 2011
      cout << "Branch with extId " << cdBr->extId
           << "mapped at invalid position: "
           << cdBr->tab << "," << cdBr->row << "," << cdBr->col << endl;
      return 50;
    }

  }

  // If no rule check faield, mapping is successfully validated!
  return 0;
}

int PwsMapperModel::autoMapping(){

  // Preliminary checks to avoid exhaustively checking a priory non-mapable
  // powersystems

  // Check emulator structure
  if ( _tabs == 0 || _rows == 0 || _cols == 0 )
    return -1;

  // Check number of buses
  if ( _pws->getBusSet_size() > _tabs*_rows*_cols )
    return -2;

  // Check number of branches
  if ( _pws->getBrSet_size() > _emuhw->getEmbrCount() )
    return -3;

  // Check if more than one element is present on a bus
  for ( size_t k = 0 ; k != _pws->getBusSet_size() ; ++k ){
    if ( _pws->getBusGenMap().at(k).size() + _pws->getBusLoadMap().at(k).size() > 1 )
      return -4;
  }

  // Check if graph degree of a pws bus is more than maximum allowed
  for ( size_t k = 0 ; k != _pws->getBusSet_size() ; ++k ){
    if ( _pws->getBusBrMap().at(k).size() > 6 )
      return -5;
  }

  // Check pws topology planarity
  if ( !graph::isPlanar( *_pws ) )
    return -6;

  // --------------------------------------------------------------------------

  size_t k;               // counter
  PwsMapperModelElement const* cd;
  size_t unmapped = 0;    // number of still unplaced bus/branch elements
  size_t available_positions;     // number of available mapping positions
                                        // for the currently selected (hinted)
                                        // component
  size_t min_available_positions; // minimum number of av. map positions

  int curr_type = -1;         // component type of the currently selected comp
  int curr_extId = -1;        // ext id of the currently selected comp
  int curr_mdlId = -1;        // mmd id of the currently selected comp

  int last_type = -1;         // component type of the lastly selected comp
  int last_extId = -1;        // ext id of the lastly selected comp
  int last_choice = -1;       // index of the lastly selected available pos
                              // to place the comp; indexing based on
                              // _countAvailablePositions, _getAvailablePosition

  stack<int> log_choice;          // log for mapping positions indexes
  stack<MappingPosition> log_pos; // log for mapping of positions

  MappingPosition temp_pos;       // temporary mapping position
//  vector<int> temp_pos;       // temporary mapping position:
//                              // .at(0)<-tab ; .at(1)<-row ; .at(2)<-col
  vector<pair<int, unsigned int> > temp_vec_pair;

  for ( k = 0 ; k != busElements_size() ; ++k )
    if ( !elementByIndex( PWSMODELELEMENTTYPE_BUS, k )->mapped )
      ++unmapped;
  for ( k = 0 ; k != branchElements_size() ; ++k )
    if ( !elementByIndex( PWSMODELELEMENTTYPE_BR, k )->mapped )
      ++unmapped;

  // Main mapping loop
  while ( unmapped != 0 ){

    // Among all unmapped buses and branches find the one with the least
    // available possible positions
    min_available_positions = 1000000;
    for ( k = 0 ; k != busElements_size() ; ++k ){
      cd = elementByIndex( PWSMODELELEMENTTYPE_BUS, k );
      if ( !cd->mapped ){
        _hintBus( k );
        available_positions = _countAvailablePositions();
        if ( available_positions < min_available_positions ){
          min_available_positions = available_positions;
          curr_type = PWSMODELELEMENTTYPE_BUS;
          curr_extId = cd->extId;
          curr_mdlId = k;
        }
      }
    }
    for ( k = 0 ; k != branchElements_size() ; ++k ){
      cd = elementByIndex( PWSMODELELEMENTTYPE_BR, k );
      if ( !cd->mapped ){
        _hintBranch( k );
        available_positions = _countAvailablePositions();
        if ( available_positions < min_available_positions ){
          min_available_positions = available_positions;
          curr_type = PWSMODELELEMENTTYPE_BR;
          curr_extId = cd->extId;
          curr_mdlId = k;
        }
      }
    }

    // Component of type 'curr_type' and extId 'curr_extId' will be mapped
    hintComponent( curr_type, curr_mdlId );
    if (    min_available_positions == 0
            || (    last_type == curr_type && last_extId == curr_extId
                 && last_choice==static_cast<int>(min_available_positions)-1) ){
      // IF there is a component with no available placements
      // OR IF all possible mapping positions have been tested
      // THEN trace back until finding a comp with a different mapping

      // Check whether there is a component to be removed
      if ( log_pos.empty() )
        // If current component cannot be mapped and no traceback can be
        // performed then automatic mapping has failed!
        return 1;

      // Keep track of component that will be removed
      temp_vec_pair = elementsMapped( log_pos.top() );
      if ( temp_vec_pair.size() != 1 )
        // Kapoion lako exei i fava!
        return 2;
      last_type = temp_vec_pair.at(0).first;
      last_extId = temp_vec_pair.at(0).second;
      last_choice = log_choice.top();

      // Remove the component
      unmapComponent( log_pos.top() );
      log_pos.pop();
      log_choice.pop();
      ++unmapped;


    } else{
      // EITHER there is a component with at least one available mapping pos
      // OR there are still available mapping positions for current component
      // So place the current component

      int posId;
      if ( curr_type == last_type && curr_extId == last_extId ){
        // The component has previously been removed, so map it to its next
        // available position
        posId = last_choice + 1;
      } else {
        // This is a new component, so map it to its first available position
        posId = 0;
      }
      temp_pos = _getAvailablePosition(posId);
      mapComponent( curr_type, curr_extId, temp_pos );
      // Log mapping
      log_pos.push( temp_pos );
      log_choice.push( posId );
      // Reduce unmapped components counter
      --unmapped;

    }

    auxiliary::stayAlive(); // keep window alive using windows signal
  } // end of main mapping loop

  // Mapping basic elements, buses & branches, is now complete so place the
  // remaining components

  // Map all generators on the same node as their buses
  PwsMapperModelElement const* cdBus;
  PwsMapperModelElement const* cdGen;
  Generator const* pGen;
  for ( k = 0 ; k != genElements_size() ; ++k ){

    // Retrieve generator component element
    cdGen = elementByIndex( PWSMODELELEMENTTYPE_GEN, k );
    if ( _pws->getGenerator( cdGen->extId, pGen ) ){
      cout << "Generator with mdlId: " << k << " "
           << "and extId: " << cdGen->extId << " "
           << "cannot be properly retrieved!" << endl;
      return 20;
    }
    // Retrieve generator bus component element
    cdBus = element(PWSMODELELEMENTTYPE_BUS, pGen->busExtId() );
    // Map generator to the node of the bus
    mapComponent( PWSMODELELEMENTTYPE_GEN, cdGen->extId,
                  cdBus->tab, cdBus->row, cdBus->col );
  }

  // Map all loads on the same node as their buses
  PwsMapperModelElement const* cdLoad;
  Load const* pLoad;
  for ( k = 0 ; k != loadElements_size() ; ++k ){

    // Retrieve generator component element
    cdLoad = elementByIndex( PWSMODELELEMENTTYPE_LOAD, k );
    if ( _pws->getLoad( cdLoad->extId, pLoad ) ){
      cout << "Load with mdlId: " << k << " "
           << "and extId: " << cdLoad->extId << " "
           << "cannot be properly retrieved!" << endl;
      return 20;
    }
    // Retrieve generator bus component element
    cdBus = element(PWSMODELELEMENTTYPE_BUS, pLoad->busExtId );
    // Map generator to the node of the bus
    mapComponent(PWSMODELELEMENTTYPE_LOAD, cdLoad->extId,
                 cdBus->tab, cdBus->row, cdBus->col);
  }

  clearHints();

  return 0;
}

int PwsMapperModel::resetMapping(){

  int ans = 0;

  if ( _tabs > 0 && _rows > 0 && _cols ){
    // Cycle through all mapping positions
    for ( size_t k = 0 ; k != 2*_tabs-1 ; ++k ){
      for ( size_t m = 0 ; m != 2*_rows-1 ; ++m ){
        for ( size_t n = 0 ; n != 2*_cols-1 ; ++n ){
          ans |= unmapComponent( k, m, n );
        }
      }
    }
  }

  // Alternatively: cycle through all compoments
  // ...

  return ans;
}

void PwsMapperModel::clearHints(){

  // Reset all _hints
  if ( _tabs > 0 && _rows > 0 && _cols ){
    for ( size_t i = 0 ; i != 2*_tabs-1; ++i ){
      for ( size_t j = 0 ; j != 2*_rows-1; ++j ){
        for ( size_t k = 0 ; k != 2*_cols-1; ++k ){
          _hints[i][j][k] = MAPHINT_INACTIVE;
        }
      }
    }
  }
}

void PwsMapperModel::hintComponent(int type, int mdlId){

  if ( _tabs == 0 || _rows == 0 || _cols == 0 ){
    // The emulator has invalid structure!
    cout << "PwsMapperModel::hintComponent(): The emulator has invalid structure!";
    cout << endl;
    return;
  }

  clearHints();

  switch(type){
  case PWSMODELELEMENTTYPE_BUS:
    _hintBus( mdlId );
    break;

  case PWSMODELELEMENTTYPE_BR:
    _hintBranch( mdlId );
    break;

  case PWSMODELELEMENTTYPE_GEN:
    _hintGenerator( mdlId );
    break;

  case PWSMODELELEMENTTYPE_LOAD:
    _hintLoad( mdlId );
    break;
  }
}

int PwsMapperModel::mapComponent( int type, unsigned int extId,
                                  MappingPosition const& mapper_pos ){
  return mapComponent( type, extId,
                       mapper_pos.tab, mapper_pos.row, mapper_pos.col );
}

int PwsMapperModel::mapComponent( int type, unsigned int extId,
                                  int mapper_tab,
                                  int mapper_row,
                                  int mapper_col ){

  if ( _tabs == 0 || _rows == 0 || _cols == 0 ){
    // The emulator has invalid structure!
    cout << "PwsMapperModel::mapComponent(): EmulatorHw has invalid structure!";
    cout << endl;
    return -1;
  }

  int mdlId = getElement_mdlId( type, extId );
  // Component selected non existent in component model!
  if ( mdlId == -1 )
    return 1;

  // Argument validation
  if (    mapper_tab < 0 || mapper_tab >= 2*static_cast<int>(_tabs)-1
       || mapper_row < 0 || mapper_row >= 2*static_cast<int>(_rows)-1
       || mapper_col < 0 || mapper_col >= 2*static_cast<int>(_cols)-1 )
    return 2;

  switch ( type ){
  case PWSMODELELEMENTTYPE_BUS:
    if ( mdlId >= static_cast<int>(busElements_size()) )
      // Invalid input argument!
      return 3;
    break;

  case PWSMODELELEMENTTYPE_BR:
    if ( mdlId >= static_cast<int>(branchElements_size()) )
      // Invalid input argument!
      return 3;
    break;

  case PWSMODELELEMENTTYPE_GEN:
    if (mdlId >= static_cast<int>(genElements_size()) )
      // Invalid input argument!
      return 3;
    break;

  case PWSMODELELEMENTTYPE_LOAD:
    if (mdlId >= static_cast<int>(loadElements_size()) )
      // Invalid input argument!
      return 3;
    break;
  }

  // Hint component in question
  hintComponent( type, mdlId );

  // And map it, if the position requested is valid
  if ( _hints[mapper_tab][mapper_row][mapper_col] == MAPHINT_ACTIVEVALID ){

    // Modify element in the powersystem mapper model
    PwsMapperModelElement* cd = elementByIndex( type, mdlId );
    cd->tab = mapper_tab;
    cd->row = mapper_row;
    cd->col = mapper_col;
    cd->mapped = true;

  } else {
    return 3;
  }

  return 0;
}

int PwsMapperModel::unmapComponent(int type, unsigned int extId){

  if ( _tabs == 0 || _rows == 0 || _cols == 0 ){
    // The emulator has invalid structure!
    cout << "PwsMapperModel::unmapComponent(): The emulator has invalid structure!";
    cout << endl;
    return -1;
  }

  int mdlId = getElement_mdlId( type, extId );
  // Component selected non existent in component model!
  if ( mdlId == -1 )
    return 1;

  PwsMapperModelElement* cd = element( type, extId );
  cd->tab = -1;
  cd->row = -1;
  cd->col = -1;
  cd->mapped = false;

  hintComponent( type, mdlId );

  return 0;
}

int PwsMapperModel::unmapComponent( MappingPosition const& mapper_pos ){
  return unmapComponent( mapper_pos.tab, mapper_pos.row, mapper_pos.col );
}

int PwsMapperModel::unmapComponent( int mapper_tab,
                                    int mapper_row,
                                    int mapper_col ){

  if ( _tabs == 0 || _rows == 0 || _cols == 0 ){
    // The emulator has invalid structure!
    cout << "PwsMapperModel::unmapComponent(): The emulator has invalid structure!";
    cout << endl;
    return -1;
  }

  // Argument validation
  if (    mapper_tab < 0 || mapper_tab >= 2*static_cast<int>(_tabs)-1
       || mapper_row < 0 || mapper_row >= 2*static_cast<int>(_rows)-1
       || mapper_col < 0 || mapper_col >= 2*static_cast<int>(_cols)-1 )
    return 1;

  // Unmap all compoments currently mapped at the specified position
  vector< pair<int,unsigned int> > comps;
  PwsMapperModelElement* cd;
  comps = elementsMapped(mapper_tab,mapper_row,mapper_col);
  for ( size_t k = 0 ; k != comps.size() ; ++k ){
    cd = element(comps.at(k).first, comps.at(k).second);
    cd->tab = -1;
    cd->row = -1;
    cd->col = -1;
    cd->mapped = false;
  }

  return 0;
}

int PwsMapperModel::addElement( int type, unsigned int extId,
                                bool overwrite, int* mdlId ){

  std::auto_ptr<PwsMapperModelElement>
      cd ( new PwsMapperModelElement(type, extId) );

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

int PwsMapperModel::addBusElement( Bus bus, bool overwrite, int* mdlId ){

  int ans = addElement(PWSMODELELEMENTTYPE_BUS, bus.extId, overwrite, mdlId);
  if ( ans ){
//    cout << "Error adding bus component (" << bus.extId << ")!" << endl;
    return 1;
  }

  return 0;
}

int PwsMapperModel::addBranchElement( Branch br, bool overwrite, int* mdlId ){

  // Add branch to model
  int tempMdlId;
  int ans = addElement( PWSMODELELEMENTTYPE_BR, br.extId, overwrite, &tempMdlId );
  if ( ans ){
//    cout << "Error adding branch component (" << br.extId << ")!" << endl;
    return 1;
  }
  if (mdlId)
    *mdlId = tempMdlId;

  return 0;
}

int PwsMapperModel::addGenElement( Generator gen, bool overwrite, int* mdlId ){

  // Add gen to model
  int tempMdlId;
  int ans = addElement( PWSMODELELEMENTTYPE_GEN, gen.extId(),
                         overwrite, &tempMdlId );
  if ( ans ){
//    cout << "Error adding gen component (" << gen.extId() << ")!" << endl;
    return 1;
  }
  if (mdlId)
    *mdlId = tempMdlId;

  return 0;
}

int PwsMapperModel::addLoadElement( Load load, bool overwrite, int* mdlId ){

  // Add load to model
  int tempMdlId;
  int ans = addElement( PWSMODELELEMENTTYPE_LOAD, load.extId, overwrite, &tempMdlId );
  if ( ans ){
//    cout << "Error adding load component (" << load.extId() << ")!" << endl;
    return 1;
  }
  if (mdlId)
    *mdlId = tempMdlId;

  return 0;
}

PwsMapperModelElement*
PwsMapperModel::element( int type, unsigned int extId, int* mdlId ){

  switch(type){

  case PWSMODELELEMENTTYPE_BUS: // Buses
    for( size_t k = 0 ; k != _busElements.size() ; ++k ){
      if( _busElements.at(k).extId == extId ){
        if ( mdlId )
          // If output argument mdlId has been initialized (asked for) then
          // return the mmd component model (internal) index of the found comp
          *mdlId = k;
        return dynamic_cast<PwsMapperModelElement*>(&_busElements[k]);
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
        return dynamic_cast<PwsMapperModelElement*>(&_branchElements[k]);
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
        return dynamic_cast<PwsMapperModelElement*>(&_genElements[k]);
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
        return dynamic_cast<PwsMapperModelElement*>(&_loadElements[k]);
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

PwsMapperModelElement const*
PwsMapperModel::element( int type, unsigned int extId, int* mdlId ) const{

  switch(type){

  case PWSMODELELEMENTTYPE_BUS: // Buses
    for( size_t k = 0 ; k != _busElements.size() ; ++k ){
      if( _busElements.at(k).extId == extId ){
        if ( mdlId )
          // If output argument mdlId has been initialized (asked for) then
          // return the mmd component model (internal) index of the found comp
          *mdlId = k;
        return dynamic_cast<PwsMapperModelElement const*>(&_busElements[k]);
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
        return dynamic_cast<PwsMapperModelElement const*>(&_branchElements[k]);
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
        return dynamic_cast<PwsMapperModelElement const*>(&_genElements[k]);
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
        return dynamic_cast<PwsMapperModelElement const*>(&_loadElements[k]);
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

PwsMapperModelElement*
PwsMapperModel::elementByIndex( int type, size_t mdlId){

  switch(type){

  case PWSMODELELEMENTTYPE_BUS: // Buses
    if( mdlId < _busElements.size() )
      return dynamic_cast<PwsMapperModelElement*>(&_busElements[mdlId]);
    else
      return NULL;
    break;

  case PWSMODELELEMENTTYPE_BR: // Branches
    if( mdlId < _branchElements.size() )
      return dynamic_cast<PwsMapperModelElement*>(&_branchElements[mdlId]);
    else
      return NULL;
    break;

  case PWSMODELELEMENTTYPE_GEN: // Generators
    if( mdlId < _genElements.size() )
      return dynamic_cast<PwsMapperModelElement*>(&_genElements[mdlId]);
    else
      return NULL;
    break;

  case PWSMODELELEMENTTYPE_LOAD: // Loads
    if( mdlId < _loadElements.size() )
      return dynamic_cast<PwsMapperModelElement*>(&_loadElements[mdlId]);
    else
      return NULL;
    break;

  default: // Any other
    return NULL;
    break;

  } // end of switch(type)
}

PwsMapperModelElement const*
PwsMapperModel::elementByIndex( int type, size_t mdlId) const{

  switch(type){

  case PWSMODELELEMENTTYPE_BUS: // Buses
    if( mdlId < _busElements.size() )
      return dynamic_cast<PwsMapperModelElement const*>(&_busElements[mdlId]);
    else
      return NULL;
    break;

  case PWSMODELELEMENTTYPE_BR: // Branches
    if( mdlId < _branchElements.size() )
      return dynamic_cast<PwsMapperModelElement const*>(&_branchElements[mdlId]);
    else
      return NULL;
    break;

  case PWSMODELELEMENTTYPE_GEN: // Generators
    if( mdlId < _genElements.size() )
      return dynamic_cast<PwsMapperModelElement const*>(&_genElements[mdlId]);
    else
      return NULL;
    break;

  case PWSMODELELEMENTTYPE_LOAD: // Loads
    if( mdlId < _loadElements.size() )
      return dynamic_cast<PwsMapperModelElement const*>(&_loadElements[mdlId]);
    else
      return NULL;
    break;

  default: // Any other
    return NULL;
    break;

  } // end of switch(type)
}

vector< pair<int,unsigned int> >
PwsMapperModel::elementsMapped( MappingPosition const& mapper_pos ) const{
  return elementsMapped( mapper_pos.tab, mapper_pos.row, mapper_pos.col );
}

vector< pair<int,unsigned int> >
PwsMapperModel::elementsMapped( int mapper_tab,
                                int mapper_row,
                                int mapper_col ) const{
  vector< pair<int,unsigned int> > ans;

  PwsMapperModelElement const* el;
  for( size_t k = 0 ; k != _busElements.size() ; ++k ){
    el = dynamic_cast<PwsMapperModelElement const*>(&_busElements[k]);
    if(    el->mapped
        && el->tab == mapper_tab
        && el->row == mapper_row
        && el->col == mapper_col ){
      ans.push_back( make_pair(PWSMODELELEMENTTYPE_BUS,el->extId) );
    }
  }

  for( size_t k = 0 ; k != _branchElements.size() ; ++k ){
    el = dynamic_cast<PwsMapperModelElement const*>(&_branchElements[k]);
    if(    el->mapped
        && el->tab == mapper_tab
        && el->row == mapper_row
        && el->col == mapper_col ){
      ans.push_back( make_pair(PWSMODELELEMENTTYPE_BR,el->extId) );
    }
  }

  for( size_t k = 0 ; k != _genElements.size() ; ++k ){
    el = dynamic_cast<PwsMapperModelElement const*>(&_genElements[k]);
    if(    el->mapped
        && el->tab == mapper_tab
        && el->row == mapper_row
        && el->col == mapper_col ){
      ans.push_back( make_pair(PWSMODELELEMENTTYPE_GEN,el->extId) );
    }
  }

  for( size_t k = 0 ; k != _loadElements.size() ; ++k ){
    el = dynamic_cast<PwsMapperModelElement const*>(&_loadElements[k]);
    if(    el->mapped
        && el->tab == mapper_tab
        && el->row == mapper_row
        && el->col == mapper_col ){
      ans.push_back( make_pair(PWSMODELELEMENTTYPE_LOAD,el->extId) );
    }
  }

  return ans;
}

int PwsMapperModel::getElementMapping( int type,
                                       unsigned int extId,
                                       MappingPosition* pos,
                                       size_t* embr_pos ) const{
  int tab, row, col;
  int ans = getElementMapping( type, extId, &tab, &row, &col, embr_pos );
  if ( ans ) return ans;

  // Update mapping position in the struct; note: embr_pos has already been
  // handled by the previous getElementMapping call
  pos->tab = tab;
  pos->row = row;
  pos->col = col;
  return 0;
}

int PwsMapperModel::getElementMapping( int type,
                                       unsigned int extId,
                                       int* tab, int* row, int* col,
                                       size_t* embr_pos ) const {

  PwsMapperModelElement const* el = element( type, extId );
  if ( el == NULL )
    return 1;

  int map_tab = el->tab;
  int map_row = el->row;
  int map_col = el->col;
  *tab = floor( static_cast<double>(map_tab)/2.0 );
  *row = floor( static_cast<double>(map_row)/2.0 );
  *col = floor( static_cast<double>(map_col)/2.0 );

  if ( type == PWSMODELELEMENTTYPE_BR ){
    bool onTab = (map_tab%2 == 0);
    bool onRow = (map_row%2 == 0);
    bool onCol = (map_col%2 == 0);
  //  bool isOnLeftExtension = false;
  //  bool isOnBottomExtension = false;
    if ( onTab & onRow & !onCol ){
      // Intratab - horizontal branch
      *embr_pos = EMBRPOS_R;

    } else if ( onTab & !onRow & onCol ){
      // Intratab - vertical branch
      *embr_pos = EMBRPOS_U;

    } else if ( onTab & !onRow & !onCol ){
      // Intratab - diagonal branch
      // Intratab diagonal branches can only be in the following sense:
      // bottom-left -> top-right
      *embr_pos = EMBRPOS_UR;

    } else if ( !onTab & onRow & onCol ){
      // Intertab branch - exactly vertical
      if ( static_cast<size_t>(*row) == (_rows-1) ){
        // Top-row vertical intertab branches correspond to U embr pos
        *embr_pos = EMBRPOS_U;
      } else if ( static_cast<size_t>(*col) == (_cols-1) ){
        // Last-column vertical intertab branches correspond to R embr pos
        *embr_pos = EMBRPOS_R;
      } else if ( col == 0 ){ // && fit_row != _rows
        // First-column vertical intertab branches correspond to L embr pos
        *embr_pos = EMBRPOS_L;
  //      isOnLeftExtension = true;
      } else if ( *row == 0 ){ // && fit_col != 0 && fit_col != _cols
        // Bottom-row vertical intertab branches correspond to D embr pos
        *embr_pos = EMBRPOS_D;
  //      isOnBottomExtension = true;
      } else {
        // Weird mapping!
        return 19;
      }

    } else if ( !onTab & onRow & !onCol ){
      // Intertab branch - diagonal in the column sense
      // Corresponds to the diagonal intertab branches on the top of the
      // emulator grid
      if ( static_cast<size_t>(*row) == (_rows-1) ){
        // Top-row diagonal (col. sense) intertab branches correspond to UR embr
        // pos
        *embr_pos = EMBRPOS_UR;
      } else {
        // Weird mapping!
        return 20;
      }

    } else if ( !onTab & !onRow & onCol ){
      // Intertab branch - diagonal in the row sense
      // Corresponds to the diagonal intertab branches on the right side of
      // the emulator grid
      if ( static_cast<size_t>(*col) == (_cols-1) ){
        // Last-column diagonal (row sense) intertab branches correspond to UR
        // embr pos
        *embr_pos = EMBRPOS_UR;
      } else {
        // Weird mapping!
        return 21;
      }

    } else if ( !onTab & !onRow & !onCol ){
      // Intertab branch
      // Warning! No such intertab branch is possible on the current version
      // of the emulator hardware! Feb 2011
      return 22;
    }
  }

  return 0;
}

vector<vector<vector<int> > > PwsMapperModel::hints() const{

  if ( _tabs == 0 || _rows == 0 || _cols == 0 ){
    // The emulator has invalid structure!
//    cout << "PwsMapperModel::hints(): The emulator has invalid structure!";
    cout << endl;

    return vector<vector<vector<int> > >();
  }

  vector<vector<vector<int> > >
  ans = vector<vector<vector<int> > >( 2*_tabs - 1,
               vector<vector<int> >( 2*_rows - 1,
               vector<int>( 2*_cols - 1, 0 ) ) );

  for ( size_t k = 0 ; k != 2*_tabs - 1 ; ++k )
    for ( size_t m = 0 ; m != 2*_rows - 1 ; ++m )
      for (size_t n = 0 ; n != 2*_cols - 1 ; ++n )
        ans[k][m][n] = _hints[k][m][n];

  return ans;
}

void PwsMapperModel::display_hints() const{

  if ( _tabs == 0 || _rows == 0 || _cols == 0 ){
    // The emulator has invalid structure!
    cout << "PwsMapperModel::display_hints(): The emulator has invalid structure!";
    cout << endl;
    return;
  }

  for ( size_t k = 0 ; k != 2*_tabs - 1 ; ++k ){
    cout << "Slice " << static_cast<double>(k)/2.0 << endl;
    for ( size_t m = 0 ; m != 2*_rows - 1 ; ++m ){
      for (size_t n = 0 ; n != 2*_cols - 1 ; ++n ){
        cout << _hints[k][m][n] << " ";
      }
      cout << endl;
    }
    cout << endl;
  }

  return;
}

void PwsMapperModel::display_mask() const{

  if ( _tabs == 0 || _rows == 0 || _cols == 0 ){
    // The emulator has invalid structure!
    cout << "PwsMapperModel::display_hints(): The emulator has invalid structure!";
    cout << endl;
    return;
  }

  for ( size_t k = 0 ; k != 2*_tabs - 1 ; ++k ){
    cout << "Slice " << static_cast<double>(k)/2.0 << endl;
    for ( size_t m = 0 ; m != 2*_rows - 1 ; ++m ){
      for (size_t n = 0 ; n != 2*_cols - 1 ; ++n ){
        cout << _mask[k][m][n] << " ";
      }
      cout << endl;
    }
    cout << endl;
  }

  return;
}

void PwsMapperModel::_hintBus( int mdlId ){

  // Retrieve bus component element
  PwsMapperModelElement const* cd;
  cd = elementByIndex(PWSMODELELEMENTTYPE_BUS,mdlId);

  // Check if component is already mapped
  if( cd->mapped ){
    _hints[cd->tab][cd->row][cd->col] = MAPHINT_MAPPED;
    return;
  }

  // If it has generators on it and they are all mapped to the same position
  // the bus can only be mapped to that position
  // If the generators are mapped to different positions then there is a
  // problem! and the bus cannot be hinted!
  size_t busIntId = static_cast<size_t>(_pws->getBus_intId(cd->extId));
  set<size_t> busGensIntIds = _pws->getBusGenMap().at(busIntId);
  if ( busGensIntIds.size() > 0 ){
    PwsMapperModelElement const* cdGen;
    unsigned int genExtId;
    vector<int> busPos = vector<int>(3, -1);
    bool busPosIsSet = false;
    for ( set<size_t>::iterator i = busGensIntIds.begin() ;
          i != busGensIntIds.end() ; ++i ){
      // In the folloing normally gen should be present so getGen_extId returns
      // 'unsigned' int
      genExtId = static_cast<unsigned int>(_pws->getGen_extId(*i));
      cdGen = element(PWSMODELELEMENTTYPE_GEN, genExtId);
      if ( !busPosIsSet && cdGen->mapped ){
        busPos[0] = cdGen->tab;
        busPos[1] = cdGen->row;
        busPos[2] = cdGen->col;
        busPosIsSet = true;
        continue;
      }
      if ( busPosIsSet && cdGen->mapped ){
        if (    cdGen->tab != busPos[0]
             || cdGen->row != busPos[1]
             || cdGen->col != busPos[2] ){
          cout << "Bus has generators that have been mapped "
               << "to different mapping positions! Cannot be hinted!" << endl;
          return;
        }
      }
    }
    if ( busPosIsSet){
      _hints[busPos[0]][busPos[1]][busPos[2]] = MAPHINT_ACTIVEVALID;
      return;
    }
  }

  // If it has loads on it and they are all mapped to the same position
  // the bus can only be mapped to that position
  // If the loads are mapped to different positions then there is a
  // problem! and the bus cannot be hinted!
  set<size_t> busLoadsIntIds = _pws->getBusLoadMap().at(busIntId);
  if ( busLoadsIntIds.size() > 0 ){
    PwsMapperModelElement const* cdLoad;
    unsigned int loadExtId;
    vector<int> busPos = vector<int>(3, -1);
    bool busPosIsSet = false;
    for ( set<size_t>::iterator i = busLoadsIntIds.begin() ;
          i != busLoadsIntIds.end() ; ++i ){
      // In the folloing normally load should be present so getLoad_extId returns
      // 'unsigned' int
      loadExtId = static_cast<unsigned int>(_pws->getLoad_extId(*i));
      cdLoad = element(PWSMODELELEMENTTYPE_LOAD, loadExtId);
      if ( !busPosIsSet && cdLoad->mapped ){
        busPos[0] = cdLoad->tab;
        busPos[1] = cdLoad->row;
        busPos[2] = cdLoad->col;
        busPosIsSet = true;
        continue;
      }
      if ( busPosIsSet && cdLoad->mapped ){
        if (    cdLoad->tab != busPos[0]
             || cdLoad->row != busPos[1]
             || cdLoad->col != busPos[2] ){
          cout << "Bus has loads that have been mapped "
               << "to different mapping positions! Cannot be hinted!" << endl;
          return;
        }
      }
    }
    if ( busPosIsSet ){
      _hints[busPos[0]][busPos[1]][busPos[2]] = MAPHINT_ACTIVEVALID;
      return;
    }
  }

  // Set initial _mask
  for ( size_t i = 0; i != 2*_tabs-1; ++i ){
    for ( size_t j = 0; j != 2*_rows-1; ++j ){
      for ( size_t k = 0; k != 2*_cols-1; ++k ){
        if( i%2 || j%2 || k%2 )
          _hints[i][j][k] = MAPHINT_INACTIVE;
        else
          _hints[i][j][k] = MAPHINT_ACTIVEVALID;
      }
    }
  }

  // Places that already have other component registered in are unavailable
  _setMask( MAPHINT_ACTIVEVALID );
  for ( size_t i = 0; i < _tabs; ++i ){
    for ( size_t j = 0; j < _rows; ++j ){
      for ( size_t k = 0; k < _cols; ++k ){
        if ( elementsMapped(2*i,2*j,2*k).size() != 0 )
          _mask[2*i][2*j][2*k] = MAPHINT_INACTIVE;
      }
    }
  }
  _andMask();

  // For every branch ending to that bus apply restrictions in case it is mapped
  set<size_t> busBrIntIds = _pws->getBusBrMap().at(busIntId);
  PwsMapperModelElement const* cdBr;
  unsigned int brExtId;
  for ( set<size_t>::iterator i = busBrIntIds.begin() ;
        i != busBrIntIds.end() ; ++i ){
    // In the folloing normally branch should be present so getBr_extId returns
    // 'unsigned' int
    brExtId = static_cast<unsigned int>(_pws->getBr_extId(*i));
    cdBr = element( PWSMODELELEMENTTYPE_BR, brExtId );
    if ( cdBr->mapped ){
      // All positions are not permitted ...
      _setMask( MAPHINT_INACTIVE );
      // ... except of those where the branch ends
      bool onTab = (cdBr->tab%2 == 0);
      bool onRow = (cdBr->row%2 == 0);
      bool onCol = (cdBr->col%2 == 0);

      if ( onTab & onRow & onCol ){
        // Invalid position for a branch! It would be allowed only to node
        // elements (buses, generators and loads)
        cout << "Branch with extId " << cd->extId
             << "mapped at invalid position: "
             << cd->tab << "," << cd->row << "," << cd->col << endl;
        return;

      } else if ( onTab & onRow & !onCol ){
        // Intratab - horizontal branch
        _mask[cdBr->tab][cdBr->row][cdBr->col-1] = MAPHINT_ACTIVEVALID;
        _mask[cdBr->tab][cdBr->row][cdBr->col+1] = MAPHINT_ACTIVEVALID;

      } else if ( onTab & !onRow & onCol ){
        // Intratab - vertical branch
        _mask[cdBr->tab][cdBr->row-1][cdBr->col] = MAPHINT_ACTIVEVALID;
        _mask[cdBr->tab][cdBr->row+1][cdBr->col] = MAPHINT_ACTIVEVALID;

      } else if ( onTab & !onRow & !onCol ){
        // Intratab - diagonal branch
        // Intratab diagonal branches can only be in the following sense:
        // bottom-left -> top-right
        _mask[cdBr->tab][cdBr->row-1][cdBr->col-1] = MAPHINT_ACTIVEVALID;
        _mask[cdBr->tab][cdBr->row+1][cdBr->col+1] = MAPHINT_ACTIVEVALID;

      } else if ( !onTab & onRow & onCol ){
        // Intertab branch - exactly vertical
        _mask[cdBr->tab-1][cdBr->row][cdBr->col] = MAPHINT_ACTIVEVALID;
        _mask[cdBr->tab+1][cdBr->row][cdBr->col] = MAPHINT_ACTIVEVALID;

      } else if ( !onTab & onRow & !onCol ){
        // Intertab branch - diagonal in the column sense
        // Corresponds to the diagonal intertab branches on the top of the
        // emulator grid
        _mask[cdBr->tab-1][cdBr->row][cdBr->col-1] = MAPHINT_ACTIVEVALID;
        _mask[cdBr->tab+1][cdBr->row][cdBr->col+1] = MAPHINT_ACTIVEVALID;

      } else if ( !onTab & !onRow & onCol ){
        // Intertab branch - diagonal in the row sense
        // Corresponds to the diagonal intertab branches on the right side of
        // the emulator grid
        _mask[cdBr->tab-1][cdBr->row-1][cdBr->col] = MAPHINT_ACTIVEVALID;
        _mask[cdBr->tab+1][cdBr->row+1][cdBr->col] = MAPHINT_ACTIVEVALID;

      } else if ( !onTab & !onRow & !onCol ){
        // Intertab branch
        // Warning! No such intertab branch is possible on the current version
        // of the emulator hardware! Nov 2011
        cout << "Branch with extId " << cd->extId
             << "mapped at invalid position: "
             << cd->tab << "," << cd->row << "," << cd->col << endl;
        return;
      }

      _andMask();
    }
  }

  // For every branch not ending to the bus apply restrictions in case it is
  // mapped
  int brIntId;
  for ( size_t k = 0 ; k != branchElements_size() ; ++k ) {
    cdBr = elementByIndex(PWSMODELELEMENTTYPE_BR,k);
    brIntId = _pws->getBr_intId( cdBr->extId );

    if ( cdBr->mapped && busBrIntIds.find(brIntId)==busBrIntIds.end() ){
      // All positions are permitted ...
      _setMask( MAPHINT_ACTIVEVALID );
      // ... except of those where the branch -not ending to the bus in
      // question- ends
      bool onTab = (cdBr->tab%2 == 0);
      bool onRow = (cdBr->row%2 == 0);
      bool onCol = (cdBr->col%2 == 0);

      if ( onTab & onRow & onCol ){
        // Invalid position for a branch! It would be allowed only to node
        // elements (buses, generators and loads)
        cout << "Branch with extId " << cd->extId
             << "mapped at invalid position: "
             << cd->tab << "," << cd->row << "," << cd->col << endl;
        return;

      } else if ( onTab & onRow & !onCol ){
        // Intratab - horizontal branch
        _mask[cdBr->tab][cdBr->row][cdBr->col-1] = MAPHINT_INACTIVE;
        _mask[cdBr->tab][cdBr->row][cdBr->col+1] = MAPHINT_INACTIVE;

      } else if ( onTab & !onRow & onCol ){
        // Intratab - vertical branch
        _mask[cdBr->tab][cdBr->row-1][cdBr->col] = MAPHINT_INACTIVE;
        _mask[cdBr->tab][cdBr->row+1][cdBr->col] = MAPHINT_INACTIVE;

      } else if ( onTab & !onRow & !onCol ){
        // Intratab - diagonal branch
        // Intratab diagonal branches can only be in the following sense:
        // bottom-left -> top-right
        _mask[cdBr->tab][cdBr->row-1][cdBr->col-1] = MAPHINT_INACTIVE;
        _mask[cdBr->tab][cdBr->row+1][cdBr->col+1] = MAPHINT_INACTIVE;

      } else if ( !onTab & onRow & onCol ){
        // Intertab branch - exactly vertical
        _mask[cdBr->tab-1][cdBr->row][cdBr->col] = MAPHINT_INACTIVE;
        _mask[cdBr->tab+1][cdBr->row][cdBr->col] = MAPHINT_INACTIVE;

      } else if ( !onTab & onRow & !onCol ){
        // Intertab branch - diagonal in the column sense
        // Corresponds to the diagonal intertab branches on the top of the
        // emulator grid
        _mask[cdBr->tab-1][cdBr->row][cdBr->col-1] = MAPHINT_INACTIVE;
        _mask[cdBr->tab+1][cdBr->row][cdBr->col+1] = MAPHINT_INACTIVE;

      } else if ( !onTab & !onRow & onCol ){
        // Intertab branch - diagonal in the row sense
        // Corresponds to the diagonal intertab branches on the right side of
        // the emulator grid
        _mask[cdBr->tab-1][cdBr->row-1][cdBr->col] = MAPHINT_INACTIVE;
        _mask[cdBr->tab+1][cdBr->row+1][cdBr->col] = MAPHINT_INACTIVE;

      } else if ( !onTab & !onRow & !onCol ){
        // Intertab branch
        // Warning! No such intertab branch is possible on the current version
        // of the emulator hardware! Nov 2011
        cout << "Branch with extId " << cd->extId
             << "mapped at invalid position: "
             << cd->tab << "," << cd->row << "," << cd->col << endl;
        return;
      }

      _andMask();
    }
  }

  // Check for max number of generators in case the bus has an attached gen
  if ( busGensIntIds.size() > 0 ){
    for ( size_t k = 0 ; k != _tabs ; ++k ){

      // Count how many generators are already mapped to this slice
      size_t slice_gens = 0;
      for ( size_t m = 0 ; m != _rows ; ++m ) {
        for ( size_t n = 0 ; n != _cols ; ++n ) {
          vector<pair<int,unsigned int> > comps;
          comps = elementsMapped( k, m, n );
          for ( size_t p = 0 ; p != comps.size() ; ++p ) {
            if  ( comps.at(p).first == PWSMODELELEMENTTYPE_GEN )
              ++slice_gens;
          }
        }
      }

      // If maximum number of generators has already been reached invalidate
      // the whole slice
      if ( slice_gens >= _emuhw->sliceSet[k].dig.pipe_gen.element_count_max() ){
        _setMask( MAPHINT_ACTIVEVALID);
        for ( size_t m = 0 ; m != _rows ; ++m )
          for ( size_t n = 0 ; n != _cols ; ++n )
            _mask[2*k][2*m][2*n] = MAPHINT_INACTIVE;
        _andMask();
      }
    }
  }

  return;
}

void PwsMapperModel::_hintBranch( int mdlId ){

  // Retrieve branch component element
  PwsMapperModelElement const* cd;
  cd = elementByIndex(PWSMODELELEMENTTYPE_BR,mdlId);
  Branch const* pBr;
  if ( _pws->getBranch( cd->extId, pBr ) ){
    cout << "Branch with mdlId: " << mdlId << " and extId: " << cd->extId
         << " cannot be properly retrieved! " << endl;
    return;
  }

  // Retrieve from bus and to bus component elements
  PwsMapperModelElement const* cdBusF;
  cdBusF = element(PWSMODELELEMENTTYPE_BUS, pBr->fromBusExtId );
  PwsMapperModelElement const* cdBusT;
  cdBusT = element(PWSMODELELEMENTTYPE_BUS, pBr->toBusExtId );

  // Check whether component is already mapped
  if ( cd->mapped ){
    _hints[cd->tab][cd->row][cd->col] = MAPHINT_MAPPED;
    return;
  }

  // Set initial mask
  for ( size_t i = 0 ; i != 2*_tabs-1 ; ++i ){
    for ( size_t j = 0 ; j != 2*_rows-1 ; ++j ){
      for ( size_t k = 0 ; k != 2*_cols-1 ; ++k ){
        if ( i%2 || j%2 || k%2 ){
          // 'odd' mapping positions are available to branches
          if ( elementsMapped(i,j,k).size() == 0 )
            _hints[i][j][k] = MAPHINT_ACTIVEVALID;
          else
            // unless they are already occupied (by another branch)
            _hints[i][j][k] = MAPHINT_INACTIVE;
        } else {
          // 'even' mapping positions are available only to node elements:
          // buses, generators and loads
          _hints[i][j][k] = MAPHINT_INACTIVE;

        }
      }
    }
  }

  // For the two ends of the branch apply restrictions in case they are mapped
  // For from end
  if ( cdBusF->mapped ){
    // All positions are inactive ...
    _setMask( MAPHINT_INACTIVE );
    // ... except of those adjacent to the mapped bus

    // In the row sense
    if ( cdBusF->row-1 >= 0 )
      _mask[cdBusF->tab][cdBusF->row-1][cdBusF->col] = MAPHINT_ACTIVEVALID;
    if ( cdBusF->row+1 <= 2*static_cast<int>(_rows)-2 )
      _mask[cdBusF->tab][cdBusF->row+1][cdBusF->col] = MAPHINT_ACTIVEVALID;

    // In the column sense
    if ( cdBusF->col-1 >= 0 )
      _mask[cdBusF->tab][cdBusF->row][cdBusF->col-1] = MAPHINT_ACTIVEVALID;
    if ( cdBusF->col+1 <= 2*static_cast<int>(_cols)-2 )
      _mask[cdBusF->tab][cdBusF->row][cdBusF->col+1] = MAPHINT_ACTIVEVALID;

    // In the diagonal sense (row-,col-) & (row+,col+)
    if ( cdBusF->row-1 >= 0 && cdBusF->col-1 >= 0 )
      _mask[cdBusF->tab][cdBusF->row-1][cdBusF->col-1] = MAPHINT_ACTIVEVALID;
    if (    cdBusF->row+1 <= 2*static_cast<int>(_rows)-2
         && cdBusF->col+1 <= 2*static_cast<int>(_cols)-2 )
      _mask[cdBusF->tab][cdBusF->row+1][cdBusF->col+1] = MAPHINT_ACTIVEVALID;

    // In the straight interslice sense
    // Only peripheral nodes have such links
    if ( (    cdBusF->row == 0 || cdBusF->row == 2*static_cast<int>(_rows)-2
           || cdBusF->col == 0 || cdBusF->col == 2*static_cast<int>(_cols)-2 )
         && cdBusF->tab-1 >= 0 )
      _mask[cdBusF->tab-1][cdBusF->row][cdBusF->col] = MAPHINT_ACTIVEVALID;
    if ( (    cdBusF->row == 0 || cdBusF->row == 2*static_cast<int>(_rows)-2
           || cdBusF->col == 0 || cdBusF->col == 2*static_cast<int>(_cols)-2 )
         && cdBusF->tab+1 <= 2*static_cast<int>(_tabs)-2 )
      _mask[cdBusF->tab+1][cdBusF->row][cdBusF->col] = MAPHINT_ACTIVEVALID;

    // In the diagonal (column-wise, col+) interslice sense
    // Only top-row nodes have such links
    if (    cdBusF->row == 2*static_cast<int>(_rows)-2 && cdBusF->col-1 >= 0
         && cdBusF->tab-1 >= 0 )
      _mask[cdBusF->tab-1][cdBusF->row][cdBusF->col-1] = MAPHINT_ACTIVEVALID;
    if (    cdBusF->row == 2*static_cast<int>(_rows)-2
         && cdBusF->col+1 <= 2*static_cast<int>(_cols)-2
         && cdBusF->tab+1 <= 2*static_cast<int>(_tabs)-2 )
      _mask[cdBusF->tab+1][cdBusF->row][cdBusF->col+1] = MAPHINT_ACTIVEVALID;

    // In the diagonal (row-wise, row+) interslice sense
    // Only last-column nodes have such links
    if (     cdBusF->col == 2*static_cast<int>(_cols)-2 && cdBusF->row-1 >= 0
          && cdBusF->tab-1 >= 0 )
      _mask[cdBusF->tab-1][cdBusF->row-1][cdBusF->col] = MAPHINT_ACTIVEVALID;
    if (     cdBusF->col == 2*static_cast<int>(_cols)-2
         && cdBusF->row+1 <= 2*static_cast<int>(_rows)-2
         && cdBusF->tab+1 <= 2*static_cast<int>(_tabs)-2 )
      _mask[cdBusF->tab+1][cdBusF->row+1][cdBusF->col] = MAPHINT_ACTIVEVALID;
    // TODO Check what is going on on atom [2,5] with the diag interslice link
    // e.g. if (     cdBusF->col == 2*_cols-2 && cdBusF->row+1 <= 2*_rows-4

    _andMask();
  }

  // For to end
  if ( cdBusT->mapped ){
    // All positions are inactive ...
    _setMask( MAPHINT_INACTIVE );
    // ... except of those adjacent to the mapped bus

    // In the row sense
    if ( cdBusT->row-1 >= 0 )
      _mask[cdBusT->tab][cdBusT->row-1][cdBusT->col] = MAPHINT_ACTIVEVALID;
    if ( cdBusT->row+1 <= 2*static_cast<int>(_rows)-2 )
      _mask[cdBusT->tab][cdBusT->row+1][cdBusT->col] = MAPHINT_ACTIVEVALID;

    // In the column sense
    if ( cdBusT->col-1 >= 0 )
      _mask[cdBusT->tab][cdBusT->row][cdBusT->col-1] = MAPHINT_ACTIVEVALID;
    if ( cdBusT->col+1 <= 2*static_cast<int>(_cols)-2 )
      _mask[cdBusT->tab][cdBusT->row][cdBusT->col+1] = MAPHINT_ACTIVEVALID;

    // In the diagonal sense (row-,col-) & (row+,col+)
    if ( cdBusT->row-1 >= 0 && cdBusT->col-1 >= 0 )
      _mask[cdBusT->tab][cdBusT->row-1][cdBusT->col-1] = MAPHINT_ACTIVEVALID;
    if (    cdBusT->row+1 <= 2*static_cast<int>(_rows)-2
         && cdBusT->col+1 <= 2*static_cast<int>(_cols)-2 )
      _mask[cdBusT->tab][cdBusT->row+1][cdBusT->col+1] = MAPHINT_ACTIVEVALID;

    // In the straight interslice sense
    // Only peripheral nodes have such links
    if ( (    cdBusT->row == 0 || cdBusT->row == 2*static_cast<int>(_rows)-2
           || cdBusT->col == 0 || cdBusT->col == 2*static_cast<int>(_cols)-2 )
         && cdBusT->tab-1 >= 0 )
      _mask[cdBusT->tab-1][cdBusT->row][cdBusT->col] = MAPHINT_ACTIVEVALID;
    if ( (    cdBusT->row == 0 || cdBusT->row == 2*static_cast<int>(_rows)-2
           || cdBusT->col == 0 || cdBusT->col == 2*static_cast<int>(_cols)-2 )
         && cdBusT->tab+1 <= 2*static_cast<int>(_tabs)-2 )
      _mask[cdBusT->tab+1][cdBusT->row][cdBusT->col] = MAPHINT_ACTIVEVALID;

    // In the diagonal (column-wise, col+) interslice sense
    // Only top-row nodes have such links
    if (    cdBusT->row == 2*static_cast<int>(_rows)-2 && cdBusT->col-1 >= 0
         && cdBusT->tab-1 >= 0 )
      _mask[cdBusT->tab-1][cdBusT->row][cdBusT->col-1] = MAPHINT_ACTIVEVALID;
    if (    cdBusT->row == 2*static_cast<int>(_rows)-2
         && cdBusT->col+1 <= 2*static_cast<int>(_cols)-2
         && cdBusT->tab+1 <= 2*static_cast<int>(_tabs)-2 )
      _mask[cdBusT->tab+1][cdBusT->row][cdBusT->col+1] = MAPHINT_ACTIVEVALID;

    // In the diagonal (row-wise, row+) interslice sense
    // Only last-column nodes have such links
    if (     cdBusT->col == 2*static_cast<int>(_cols)-2 && cdBusT->row-1 >= 0
          && cdBusT->tab-1 >= 0 )
      _mask[cdBusT->tab-1][cdBusT->row-1][cdBusT->col] = MAPHINT_ACTIVEVALID;
    if (     cdBusT->col == 2*static_cast<int>(_cols)-2
         && cdBusT->row+1 <= 2*static_cast<int>(_rows)-2
         && cdBusT->tab+1 <= 2*static_cast<int>(_tabs)-2 )
      _mask[cdBusT->tab+1][cdBusT->row+1][cdBusT->col] = MAPHINT_ACTIVEVALID;
    // TODO Check what is going on on atom [2,5] with the diag interslice link
    // e.g. if (     cdBusT->col == 2*_cols-2 && cdBusT->row+1 <= 2*_rows-4

    _andMask();
  }

  // For all buses that are mapped, invalidate incident positions unless the bus
  // is an end of the branch in question
  PwsMapperModelElement const* cdBus;
  for ( size_t k = 0 ; k != busElements_size() ; ++k ){
    cdBus = elementByIndex(PWSMODELELEMENTTYPE_BUS,k);
    if (    cdBus->mapped
         && cdBus->extId != cdBusF->extId && cdBus->extId != cdBusT->extId ) {
      _setMask( MAPHINT_ACTIVEVALID );

      // In the row sense
      if ( cdBus->row-1 >= 0 )
        _mask[cdBus->tab][cdBus->row-1][cdBus->col] = MAPHINT_INACTIVE;
      if ( cdBus->row+1 <= 2*static_cast<int>(_rows)-2 )
        _mask[cdBus->tab][cdBus->row+1][cdBus->col] = MAPHINT_INACTIVE;

      // In the column sense
      if ( cdBus->col-1 >= 0 )
        _mask[cdBus->tab][cdBus->row][cdBus->col-1] = MAPHINT_INACTIVE;
      if ( cdBus->col+1 <= 2*static_cast<int>(_cols)-2 )
        _mask[cdBus->tab][cdBus->row][cdBus->col+1] = MAPHINT_INACTIVE;

      // In the diagonal sense (row-,col-) & (row+,col+)
      if ( cdBus->row-1 >= 0 && cdBus->col-1 >= 0 )
        _mask[cdBus->tab][cdBus->row-1][cdBus->col-1] = MAPHINT_INACTIVE;
      if (    cdBus->row+1 <= 2*static_cast<int>(_rows)-2
           && cdBus->col+1 <= 2*static_cast<int>(_cols)-2 )
        _mask[cdBus->tab][cdBus->row+1][cdBus->col+1] = MAPHINT_INACTIVE;

      // In the straight interslice sense
      // Only peripheral nodes have such links
      if ( (    cdBus->row == 0 || cdBus->row == 2*static_cast<int>(_rows)-2
             || cdBus->col == 0 || cdBus->col == 2*static_cast<int>(_cols)-2 )
           && cdBus->tab-1 >= 0 )
        _mask[cdBus->tab-1][cdBus->row][cdBus->col] = MAPHINT_INACTIVE;
      if ( (    cdBus->row == 0 || cdBus->row == 2*static_cast<int>(_rows)-2
             || cdBus->col == 0 || cdBus->col == 2*static_cast<int>(_cols)-2 )
           && cdBus->tab+1 <= 2*static_cast<int>(_tabs)-2 )
        _mask[cdBus->tab+1][cdBus->row][cdBus->col] = MAPHINT_INACTIVE;

      // In the diagonal (column-wise, col+) interslice sense
      // Only top-row nodes have such links
      if (    cdBus->row == 2*static_cast<int>(_rows)-2 && cdBus->col-1 >= 0
           && cdBus->tab-1 >= 0 )
        _mask[cdBus->tab-1][cdBus->row][cdBus->col-1] = MAPHINT_INACTIVE;
      if (    cdBus->row == 2*static_cast<int>(_rows)-2
           && cdBus->col+1 <= 2*static_cast<int>(_cols)-2
           && cdBus->tab+1 <= 2*static_cast<int>(_tabs)-2 )
        _mask[cdBus->tab+1][cdBus->row][cdBus->col+1] = MAPHINT_INACTIVE;

      // In the diagonal (row-wise, row+) interslice sense
      // Only last-column nodes have such links
      if (     cdBus->col == 2*static_cast<int>(_cols)-2
            && cdBus->row-1 >= 0
            && cdBus->tab-1 >= 0 )
        _mask[cdBus->tab-1][cdBus->row-1][cdBus->col] = MAPHINT_INACTIVE;
      if (    cdBus->col == 2*static_cast<int>(_cols)-2
           && cdBus->row+1 <= 2*static_cast<int>(_rows)-2
           && cdBus->tab+1 <= 2*static_cast<int>(_tabs)-2 )
        _mask[cdBus->tab+1][cdBus->row+1][cdBus->col] = MAPHINT_INACTIVE;
      // TODO Check what is going on on atom [2,5] with the diag interslice link
      // e.g. if (     cdBus->col == 2*_cols-2 && cdBus->row+1 <= 2*_rows-4

      _andMask();
    }
  }

  return;
}

void PwsMapperModel::_hintGenerator( int mdlId ){

  // Retrieve generator component element
  PwsMapperModelElement const* cd;
  cd = elementByIndex(PWSMODELELEMENTTYPE_GEN,mdlId);
  Generator const* pGen;
  if ( _pws->getGenerator( cd->extId, pGen ) ){
    cout << "Generator with mdlId: " << mdlId << " and extId: " << cd->extId
         << " cannot be properly retrieved! " << endl;
    return;
  }

  // Retrieve generator bus component element
  PwsMapperModelElement const* cdBus;
  cdBus = element(PWSMODELELEMENTTYPE_BUS, pGen->busExtId() );

  // Check if component is already mapped
  if( cd->mapped ){
    _hints[cd->tab][cd->row][cd->col] = MAPHINT_MAPPED;
    return;
  }
  // If its node is already mapped then there is only one place available
  if( cdBus->mapped ){
    _hints[cdBus->tab][cdBus->row][cdBus->col] = MAPHINT_ACTIVEVALID;
    return;
  }
  // Available places are all free ones
  for ( size_t i = 0 ; i != _tabs ; ++i ){
    for ( size_t j = 0 ; j != _rows ; ++j ){
      for ( size_t k = 0 ; k != _cols ; ++k ){
        if( elementsMapped(2*i,2*j,2*k).size()!=0 )
          _hints[2*i][2*j][2*k] = MAPHINT_INACTIVE;
        else
          _hints[2*i][2*j][2*k] = MAPHINT_ACTIVEVALID;
      }
    }
  }

  return;
}

void PwsMapperModel::_hintLoad( int mdlId ){

  // Retrieve load component element
  PwsMapperModelElement const* cd;
  cd = elementByIndex(PWSMODELELEMENTTYPE_LOAD,mdlId);
  Load const* pLoad;
  if ( _pws->getLoad( cd->extId, pLoad ) ){
    cout << "Load with mdlId: " << mdlId << " and extId: " << cd->extId
         << " cannot be properly retrieved! " << endl;
    return;
  }

  // Retrieve generator bus component element
  PwsMapperModelElement const* cdBus;
  cdBus = element(PWSMODELELEMENTTYPE_BUS, pLoad->busExtId );

  // Check if component is already mapped
  if( cd->mapped ){
    _hints[cd->tab][cd->row][cd->col] = MAPHINT_MAPPED;
    return;
  }
  // If its node is already mapped then there is only one available place
  if( cdBus->mapped ){
    _hints[cdBus->tab][cdBus->row][cdBus->col] = MAPHINT_ACTIVEVALID;
    return;
  }
  // Available places are all free ones
  for(unsigned i=0; i<_tabs; i++){
    for(unsigned j=0; j<_rows; j++){
      for(unsigned k=0; k<_cols; k++)
        if( elementsMapped(2*i,2*j,2*k).size()!=0 )
          _hints[2*i][2*j][2*k] = MAPHINT_INACTIVE;
        else
          _hints[2*i][2*j][2*k] = MAPHINT_ACTIVEVALID;
    }
  }
}

// TODO: rewrite with &&
void PwsMapperModel::_andMask(){

  for( size_t i = 0 ; i != 2*_tabs-1 ; ++i ){
    for ( size_t j = 0 ; j != 2*_rows-1 ; ++j ){
      for (size_t k = 0 ; k != 2*_cols-1 ; k++ ){
        if(     _mask[i][j][k] == MAPHINT_INACTIVE
            || _hints[i][j][k] == MAPHINT_INACTIVE )
          _hints[i][j][k] = MAPHINT_INACTIVE;
      }
    }
  }

  return;
}

void PwsMapperModel::_orMask(){

  for ( size_t i = 0 ; i != 2*_tabs-1 ; ++i ){
    for ( size_t j = 0 ; j != 2*_rows-1 ; ++j ){
      for ( size_t k = 0 ; k != 2*_cols-1 ; ++k ){
        if(     _mask[i][j][k] == MAPHINT_ACTIVEVALID
            || _hints[i][j][k] == MAPHINT_ACTIVEVALID )
          _hints[i][j][k] = MAPHINT_ACTIVEVALID;
        else
          _hints[i][j][k] = MAPHINT_INACTIVE;
      }
    }
  }
}

void PwsMapperModel::_setMask( int state ){

  for ( size_t i = 0 ; i != 2*_tabs-1 ; ++i )
    for ( size_t j = 0 ; j != 2*_rows-1 ; ++j )
      for ( size_t k = 0 ; k != 2*_cols-1 ; ++k )
        _mask[i][j][k] = state;
}

size_t PwsMapperModel::_countAvailablePositions(){

  size_t ans = 0;

  for ( size_t i = 0 ; i != 2*_tabs-1 ; ++i )
    for ( size_t j = 0 ; j != 2*_rows-1 ; ++j )
      for ( size_t k = 0 ; k != 2*_cols-1 ; ++k )
        if ( _hints[i][j][k] == MAPHINT_ACTIVEVALID )
          ++ans;

  return ans;
}

MappingPosition PwsMapperModel::_getAvailablePosition( int nth_pos ){

  MappingPosition ans;
//  MappingPosition ans = {.tab=-1, .row=-1, .col=-1}; // should work in C99!
  ans.tab = -1;
  ans.row = -1;
  ans.col = -1;

  for ( size_t i = 0 ; i != 2*_tabs-1 ; ++i ){
    for ( size_t j = 0 ; j != 2*_rows-1 ; ++j ){
      for ( size_t k = 0 ; k != 2*_cols-1 ; ++k ){
        if ( _hints[i][j][k] == MAPHINT_ACTIVEVALID ){
          --nth_pos;
          if ( nth_pos < 0 ){
            ans.tab = i;
            ans.row = j;
            ans.col = k;
            goto label_return;
          }
        }
      }
    }
  }

  label_return:
  return ans;
}

// DEPRECATED: old _getAvailablePosition() returning vector<int>
//vector<int> PwsMapperModel::_getAvailablePosition( int nth_pos ){
//  vector<int> ans = vector<int>( 3, -1);
//  for ( size_t i = 0 ; i != 2*_tabs-1 ; ++i ){
//    for ( size_t j = 0 ; j != 2*_rows-1 ; ++j ){
//      for ( size_t k = 0 ; k != 2*_cols-1 ; ++k ){
//        if ( _hints[i][j][k] == MAPHINT_ACTIVEVALID ){
//          --nth_pos;
//          if ( nth_pos < 0 ){
//            ans[0] = i;
//            ans[1] = j;
//            ans[2] = k;
//            goto label_return;
//          }
//        }
//      }
//    }
//  }
//  label_return:
//  return ans;
//}
