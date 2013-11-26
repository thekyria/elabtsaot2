/*!
\file logger.h

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef LOGGER_H
#define LOGGER_H

namespace elabtsaot{

class Logger {

public:

  virtual void notifyProgress(double val)=0;

};

} // end of namespace elabtsaot

#endif // LOGGER_H
