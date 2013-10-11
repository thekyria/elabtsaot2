
#ifndef SSENGINE_H
#define SSENGINE_H

#include "propertyholder.h"
#include "powersystem.h"

#include <boost/numeric/ublas/matrix.hpp>

#include <map>
#include <list>

namespace elabtsaot{

class Logger;

class SSEngine : public PropertyHolder {

 public:

  SSEngine(std::string description, Logger* log);
  virtual ~SSEngine(){}
  std::string getDescription() const;
  void setLogger(Logger* log);
  int buildY( Powersystem const& pws,
              boost::numeric::ublas::matrix<std::complex<double> >& Y ) const;
  int solveLoadflow( Powersystem& pws ) const;

 protected:

  std::string _SSdescription;
  Logger* _SSlog;

 private:

  virtual void do_getInitialConditions( Powersystem const& pws,
                               boost::numeric::ublas::vector<double>& x,
                               boost::numeric::ublas::vector<double>& F ) const;
  virtual int do_buildY( Powersystem const& pws,
                boost::numeric::ublas::matrix<std::complex<double> >& Y ) const;
  virtual int do_solveLoadflow( Powersystem const& pws,
                           boost::numeric::ublas::vector<double>& x,
                           boost::numeric::ublas::vector<double>& F ) const = 0;
  virtual int do_updatePowersystem( Powersystem& pws,
                         boost::numeric::ublas::vector<double> const& x,
                         boost::numeric::ublas::vector<double> const& F ) const;

};

} // end of namespace elabtsaot

#endif // SSENGINE_H
