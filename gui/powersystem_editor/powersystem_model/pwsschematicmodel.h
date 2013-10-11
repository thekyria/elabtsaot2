/*!
\file pwsschematicmodel.h
\brief Definition file for class PwsSchematicModel

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
\author Fragkiskos Koufogiannis, fkoufis at gmail dot com
*/

#ifndef PWSSCHEMATICMODEL_H
#define PWSSCHEMATICMODEL_H

#include "pwsmodel.h"

#include "pwsschematicmodelelement.h"

namespace elabtsaot{

class Powersystem;

class PwsSchematicModel : public PwsModel{

 public:

  PwsSchematicModel( Powersystem const* pws );
  PwsSchematicModel();
  virtual ~PwsSchematicModel(){}
  int init();
  int validate();
  int copy( PwsSchematicModel& other );

  //! Planarizes the schematic representation of the given powersystem
  int planarizeSchematic( Powersystem const* pws );

  //! Element addition
  //@{
  int addBusElement( Bus bus, int x, int y, bool horizontal = true,
                     bool overwrite = false, int* mdlId = 0 );
  int addElement( int type, unsigned int extId, bool overwrite, int* mdlId=0 );
  int addBusElement( Bus bus, bool overwrite = false, int* mdlId = 0 );
  int addBranchElement( Branch br, bool overwrite = false, int* mdlId = 0 );
  int addGenElement( Generator gen, bool overwrite = false, int* mdlId = 0 );
  int addLoadElement( Load load, bool overwrite = false, int* mdlId = 0 );
  //@}

  //! Element access
  //@{
  //! Returns component of type 'type' and external index 'extId'; 'mdlId' is
  //! the mmd componenent model (internal) index of the asked for component
  PwsSchematicModelElement*
  element( int type, unsigned int extId, int* mdlId = 0);
  PwsSchematicModelElement const*
  element( int type, unsigned int extId, int* mdlId = 0) const;
  //! Returns element of type 'type' and internal model id 'mdlId'
  PwsSchematicModelElement*
  elementByIndex( int type, size_t mdlId );
  PwsSchematicModelElement const*
  elementByIndex( int type, size_t mdlId ) const;
  //@}

};

} // end of namespace elabtsaot

#endif // PWSMAPPERMODEL_H
