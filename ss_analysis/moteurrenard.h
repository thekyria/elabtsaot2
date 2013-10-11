
#ifndef MOTEURRENARD_H
#define MOTEURRENARD_H

#include "ssengine.h"

namespace elabtsaot{

class MoteurRenard : public SSEngine {

 public:

  MoteurRenard(Logger* log = NULL);
  virtual ~MoteurRenard(){}

private:

  int do_solveLoadflow( Powersystem const& pws,
                        boost::numeric::ublas::vector<double>& x,
                        boost::numeric::ublas::vector<double>& F ) const;
  void _getOptions( bool& flatStart,
                    bool& enforceQLimits,
                    double& tolerance,
                    size_t& maxIterCount ) const;

};

} // end of namespace elabtsaot

#endif // MOTEURRENARD_H
