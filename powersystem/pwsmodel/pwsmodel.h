/*!
\file pwsmodel.h
\brief Definition file for class PwsModel

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef PWSMODEL_H
#define PWSMODEL_H

#include <boost/ptr_container/ptr_vector.hpp> //using boost::ptr_vector;

#include "pwsmodelelement.h"

namespace elabtsaot{

class Powersystem;
class Bus;
class Branch;
class Generator;
class Load;

class PwsModel{

 public:

  PwsModel( Powersystem const* pws = NULL );
  virtual ~PwsModel(){}
  virtual int init() = 0;
  virtual int validate() = 0;

  //! Clear the powersystem model - reset it to its initial state
  virtual int clear();

  //! Element add and remove
  //@{
  //! If component with (cd.type,cd.extId) already exists in the model then its
  //! entry in one of the 'xxElements' vectors is updated; otherwise the
  //! element of the component is added in the respective vector.
  virtual int addElement( int type, unsigned int extId,
                          bool overwrite, int* mdlId = 0);
  virtual int addBusElement( Bus bus, bool overwrite, int* mdlId = 0) = 0;
  virtual int addBranchElement( Branch br, bool overwrite, int* mdlId = 0) = 0;
  virtual int addGenElement( Generator gen, bool overwrite, int* mdlId = 0) = 0;
  virtual int addLoadElement( Load load, bool overwrite, int* mdlId = 0 ) = 0;
  //! Removes the component of given type and external id.
  virtual int remElement( int type, unsigned int extId );
  //@}

  //! Returns the mdlId (internal index of the component model) for a component
  //! of a given type and external id; if not found, returns -1
  int getElement_mdlId(int type, unsigned int extId) const;
  size_t busElements_size() const;    //!< get the size of _busElements
  size_t branchElements_size() const; //!< get the size of _branchElements
  size_t genElements_size() const;    //!< get the size of _genElements
  size_t loadElements_size() const;   //!< get the size of _loadElements

protected:

  Powersystem const* _pws;
  boost::ptr_vector<PwsModelElement> _busElements;
  boost::ptr_vector<PwsModelElement> _branchElements;
  boost::ptr_vector<PwsModelElement> _genElements;
  boost::ptr_vector<PwsModelElement> _loadElements;

};

} // end of namespace elabtsaot

#endif // PWSMODEL_H
