
#include "ssengine.h"
using namespace elabtsaot;

using namespace ublas;
#include "ssutils.h"

SSEngine::SSEngine( std::string description, Logger* log ) :
  _SSdescription(description), _SSlog(log){}

std::string SSEngine::SSDescription() const{ return _SSdescription; }
void SSEngine::setLogger(Logger* log){ _SSlog = log; }

int SSEngine::solvePowerFlow(Powersystem& pws) const{

  // Solve power flow problem proper
  vector<complex> V;
  int ans = do_solvePowerFlow(pws,V);
  if (ans) return ans ;

  // Update power system according to the solution
  matrix<complex,column_major> Y;
  ssutils::buildY(pws,Y);
  ssutils::updatePowersystem(pws,Y,V);
  return 0;
}
