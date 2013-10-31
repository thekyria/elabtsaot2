#ifndef MOTEURLAPACK_H
#define MOTEURLAPACK_H

#include "ssengine.h"

namespace elabtsaot{

class MoteurLapack : public SSEngine {

 public:

  MoteurLapack(Logger* log = NULL);
  virtual ~MoteurLapack(){}

private:

  int do_solvePowerFlow(Powersystem const& pws, ublas::vector<complex>& V) const;
  void _getOptions(bool& flatStart, double& tolerance,
                   int& maxIterCount, int& method) const;
};

} // end of namespace elabtsaot

#endif // MOTEURLAPACK_H
