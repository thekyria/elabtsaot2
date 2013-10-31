
#ifndef MOTEURRENARD_H
#define MOTEURRENARD_H

#include "ssengine.h"

namespace elabtsaot{

class MoteurRenard : public SSEngine {

 public:

  MoteurRenard(Logger* log = NULL);
  virtual ~MoteurRenard(){}

private:

  int do_solvePowerFlow(Powersystem const& pws, ublas::vector<complex>& V) const;
  void _getOptions(bool& flatStart, double& tolerance, size_t& maxIterCount) const;
};

} // end of namespace elabtsaot

#endif // MOTEURRENARD_H
