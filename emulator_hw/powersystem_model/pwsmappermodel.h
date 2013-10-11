/*!
\file pwsmappermodel.h
\brief Definition file for class PwsMapperModel

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
\author Fragkiskos Koufogiannis, fkoufis at gmail dot com
*/

#ifndef PWSMAPPERMODEL_H
#define PWSMAPPERMODEL_H

#include "pwsmodel.h"
#include "pwsmappermodelelement.h"

#include <vector>

namespace elabtsaot{

struct MappingPosition;

class EmulatorHw;
class Powersystem;

enum MapHint{
  MAPHINT_INACTIVE  = 0,
  MAPHINT_ACTIVEVALID = 2,
  MAPHINT_MAPPED = 3
};

class PwsMapperModel : public PwsModel{

 public:

  PwsMapperModel( Powersystem const* pws, EmulatorHw const* emuhw );
  virtual ~PwsMapperModel();
  int init();
  int copy(PwsMapperModel& other);
  int validate();

  int autoMapping();
  int resetMapping();
  void clearHints();
  void hintComponent(int type, int mdlId);
  int mapComponent( int type, unsigned int extId,
                    MappingPosition const& mapper_pos );
  int mapComponent( int type, unsigned int extId,
                    int mapper_tab,
                    int mapper_row,
                    int mapper_col );
  int unmapComponent( int type, unsigned int extId );
  int unmapComponent( MappingPosition const& mapper_pos );
  int unmapComponent( int mapper_tab,
                      int mapper_row,
                      int mapper_col );

  //! Element addition
  //@{
  int addElement(int type, unsigned int extId, bool overwrite, int* mdlId = 0);
  int addBusElement( Bus bus, bool overwrite = false, int* mdlId = 0 );
  int addBranchElement( Branch br, bool overwrite = false, int* mdlId = 0 );
  int addGenElement( Generator gen, bool overwrite = false, int* mdlId = 0 );
  int addLoadElement( Load load, bool overwrite = false, int* mdlId = 0 );
  //@}

  //! Returns component of type 'type' and external index 'extId'; 'mdlId' is
  //! the mmd componenent model (internal) index of the asked for component
  PwsMapperModelElement*
  element( int type, unsigned int extId, int* mdlId = 0);
  PwsMapperModelElement const*
  element( int type, unsigned int extId, int* mdlId = 0) const;
  //! Returns element of type 'type' and internal model id 'mdlId'
  PwsMapperModelElement*
  elementByIndex( int type, size_t mdlId );
  PwsMapperModelElement const*
  elementByIndex( int type, size_t mdlId ) const;
  std::vector< std::pair<int,unsigned int> >
  elementsMapped( MappingPosition const& mapper_pos ) const;
  /*! \warning mapper_tab, mapper_row and mapper_col coordinates refer to mapper
               numbering. That is nodes are in positions [2*k, 2*m, 2*n]. In
               between positions are branches. */
  std::vector<std::pair<int,unsigned int> > elementsMapped(int mapper_tab,
                                                   int mapper_row,
                                                   int mapper_col) const;
  int getElementMapping( int type, unsigned int extId,
                         MappingPosition* pos, size_t* embr_pos ) const;
  int getElementMapping( int type, unsigned int extId,
                   int* tab, int* row, int* col, size_t* embr_pos ) const;

  // --- getters ---
  std::vector<std::vector<std::vector<int> > > hints() const;
  void display_hints() const;
  void display_mask() const;

 private:

  void _destroy();
  void _hintBus( int mdlId );
  void _hintBranch(int mdlId );
  void _hintGenerator( int mdlId );
  void _hintLoad( int mdlId );
  void _andMask();
  void _orMask();
  void _setMask( int state );
  size_t _countAvailablePositions();
  MappingPosition _getAvailablePosition( int nth_pos );
//  std::vector<int> _getAvailablePosition( int nth_pos );

  // ------------ variables ------------
  EmulatorHw const* _emuhw;

  size_t _tabs, _rows, _cols; //! Tab, row and column count of the _emuhw
  //! Matrix that contains mapping information
  /*!
    Size of the matrix is [ 2x_tabs X 2x_rows X 2x_cols ]. Each cell takes
    values according to the MapHint enumeration.
    - Cells with coordinates [ 2*k , 2*m , 2*n ] are reserved for nodes - node at
    tab (slice) k, row m and columns n.
    - Cells with coordinates in between nodes are reserved for branches connecting
    those nodes. e.g. for a given node [2*k,2*m,2*n]: [2*k+1,2*m,2*n] is the
    interslice branch to the next (above) slice; [2*k,2*m+1,2*n] is the
    (EMBRPOS_) branch to the vertically next node; [2*k,2*m,2*n+1] is the
    (EMBRPOS_R) branch to the horizontally next node; [2*k,2*m+1,2*n+1] is the
    (EMBRPOS_UR) branch to the diagonally next node.
  */
  int*** _hints;
  //! Matrix that contains mapping information stemming from a single rule
  //! The _mask is then merged with _hints
  int*** _mask;

};

//! MappingPosition.tab, .row, .col refer to mapping (and not normal emulator)
//! indexing
struct MappingPosition{
  MappingPosition(){}
  MappingPosition(int tab_, int row_, int col_):tab(tab_),row(row_),col(col_){}
  int tab;
  int row;
  int col;
};

} // end of namespace elabtsaot

#endif // PWSMAPPERMODEL_H
