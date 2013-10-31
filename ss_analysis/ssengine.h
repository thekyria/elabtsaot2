
#ifndef SSENGINE_H
#define SSENGINE_H

#include "propertyholder.h"
#include "powersystem.h"

#include <boost/numeric/ublas/vector.hpp>
namespace ublas = boost::numeric::ublas;

#include <complex>
typedef std::complex<double> complex;

namespace elabtsaot{

class Logger;

class SSEngine : public PropertyHolder {
 public:
  SSEngine(std::string description, Logger* log);
  virtual ~SSEngine(){}
  std::string SSDescription() const;
  void setLogger(Logger* log);
  int solvePowerFlow(Powersystem& pws) const;

 protected:
  std::string _SSdescription;
  Logger* _SSlog;

 private:
  virtual int do_solvePowerFlow(Powersystem const& pws, ublas::vector<complex>& V) const = 0;
};

} // end of namespace elabtsaot

#endif // SSENGINE_H
