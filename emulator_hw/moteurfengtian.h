
#ifndef MOTEURFENGTIAN_H
#define MOTEURFENGTIAN_H

#include "ssengine.h"

namespace elabtsaot{

class Emulator;

class MoteurFengtian : public SSEngine {

 public:

  MoteurFengtian(Emulator* emu, Logger* log = NULL);
  virtual ~MoteurFengtian(){}

  int resetGPF() const;

private:

  int do_solvePowerFlow(Powersystem const& pws, ublas::vector<complex>& V) const;
  void _getOptions(bool& flatStart, double& beta1, double& beta2,
                   double& Ptolerance, double& Qtolerance,
                   size_t& maxIterCount, int& method ) const;
  int _waitForGPFConvergence(double timeout_, bool& converged) const;
  void _parseVoltage(std::vector<uint32_t> const& val, std::vector<complex>& out) const;

  Emulator* _emu;

};

} // end of namespace elabtsaot

#endif // MOTEURFENGTIAN_H
