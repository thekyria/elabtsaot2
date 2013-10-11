/*!
\file pwsmodelelement.h
\brief Definition file for class PwsModelElement

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef PWSMODELELEMENT_H
#define PWSMODELELEMENT_H

namespace elabtsaot{

enum PwsModelElementType{
  PWSMODELELEMENTTYPE_BUS  = 0,
  PWSMODELELEMENTTYPE_BR   = 1,
  PWSMODELELEMENTTYPE_GEN  = 2,
  PWSMODELELEMENTTYPE_LOAD = 3
};

class PwsModelElement{

 public:

  PwsModelElement(int type = -1, int extId = 0):
      type(type), extId(extId){}
  virtual ~PwsModelElement(){}

  int type;           // Type of component that element refers to
                      // -1: Invalid, 0: Bus, 1: Branch, 2: Generator, 3: Load
  unsigned int extId; // extId of the power system element the element refers to

};

} // end of namespace elabtsaot

#endif // PWSMODELELEMENT_H
