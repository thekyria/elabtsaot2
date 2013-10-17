
#include "ssengine.h"
using namespace elabtsaot;

#include <boost/numeric/ublas/lu.hpp> // for matrix operations
#include <boost/numeric/ublas/io.hpp> // for ublas::matrix '<<'
//#include <boost/numeric/ublas/matrix.hpp> // Required for matrix operations
namespace ublas = boost::numeric::ublas;

#include <complex>
using std::complex;
using std::polar;
#include <set>
using std::set;
//#include <map>
using std::map;
#include <vector>
using std::vector;
//#include <list>
using std::list;

SSEngine::SSEngine( std::string description, Logger* log ) :
  _SSdescription(description), _SSlog(log){}

std::string SSEngine::getDescription() const{ return _SSdescription; }

void SSEngine::setLogger(Logger* log){ _SSlog = log; }

int SSEngine::buildY( Powersystem const& pws,
                      ublas::matrix<std::complex<double> >& Y) const{
  return do_buildY(pws, Y);
}

int SSEngine::solvePowerFlow( Powersystem& pws ) const{

  ublas::vector<double> x;
  ublas::vector<double> F;
  do_getInitialConditions(pws, x, F);

  int ans = do_solvePowerFlow(pws, x, F);
  if ( ans ) return 1000 + ans ;

  ans = do_updatePowersystem(pws, x, F);
  if ( ans ) return 2000 + ans;

  return 0;
}

void SSEngine::do_getInitialConditions( Powersystem const& pws,
                                        ublas::vector<double>& x,
                                        ublas::vector<double>& F ) const{

  // Initialize output vectors
  size_t n = pws.getBusSet_size();          // number of nodes
  x.resize(2*n); x.clear();
  F.resize(2*n); F.clear();

  for ( size_t k = 0 ; k != n ; ++k ){
    Bus const* bus = pws.getBus(k);
    x(  k) = bus->theta;
    x(n+k) = bus->V;
    F(  k) = bus->P;
    F(n+k) = bus->Q;
  }
}

int SSEngine::do_buildY( Powersystem const& pws,
                         ublas::matrix<std::complex<double> >& Y) const{
  // Resize Y to (nxn) and initialize to 0(+j0) for all values
  size_t n = pws.getBusSet_size();
  Y.resize(n,n);
  Y.clear();

  unsigned int fext, text;
  size_t f, t;
  double r, x, g_from, g_to, b_from, b_to, tap, th;
  complex<double> ys, yp_from, yp_to, y11, y12, y21, y22;
  for ( size_t k = 0 ; k != pws.getBrSet_size() ; ++k ){
    Branch const* branch = pws.getBranch(k);

    // Check whether the branch is on-line
    if (!branch->status) continue;

    // Retrieve data of the branch
    fext = branch->fromBusExtId;
    text = branch->toBusExtId;
    f = pws.getBus_intId(fext);  // from bus internal index
    t = pws.getBus_intId(text);  // to bus internal index

    r = branch->R;                     // resistance
    x = branch->X;                     // reactance
    ys = 1.0/(complex<double>(r,x));  // series admittance

    g_from = branch->Gfrom;           // from end shunt conductance
    b_from = branch->Bfrom;           // from end shunt susceptance
    yp_from = complex<double>(g_from, b_from); // from end shunt admittance
    g_to = branch->Gto;               // to end shunt conductance
    b_to = branch->Bto;               // to end shunt susceptance
    yp_to = complex<double>(g_to, b_to); // to end shunt admittance

    tap = branch->Xratio;              // Xformer tap ratio magnitude
    th = branch->Xshift;               // Xformer phase shift angle
    if(tap == 0){
      tap = 1;                      // assuming that (tap==0) => denotes line
      th = 0;
    }

    // Determine y-parameters
    // two-port y11-parameter
    y11 = (ys + yp_from) / complex<double>(tap*tap , 0.0);
    // two-port y12-parameter
    y12 = -ys / (complex<double>(tap , 0.0) * exp(complex<double>(0.0 , -th)));
    // two-port y21-parameter
    y21 = -ys / (complex<double>(tap , 0.0) * exp(complex<double>(0.0 , th)));
    // two-port y22-parameter
    y22 = ys + yp_to;

    // Update admittance matrix Y
    Y(f,f) += y11;
    Y(f,t) += y12;
    Y(t,f) += y21;
    Y(t,t) += y22;
  }

  // Include bus shunt admittances
  for ( size_t k = 0 ; k != pws.getBusSet_size() ; ++k )
    Y(k,k) += complex<double>( pws.getBus(k)->Gsh, pws.getBus(k)->Bsh );

  return 0;
}

int SSEngine::do_updatePowersystem( Powersystem& pws,
                                    ublas::vector<double> const& x,
                                    ublas::vector<double> const& F ) const{

  // Input argument validation
  size_t n = pws.getBusSet_size();          // number of nodes

  pws.set_status( PWSSTATUS_VALID );
  // Store theta-U, P-Q results in the _busSet of the pws
  for ( size_t k = 0 ; k != n ; ++k ){
    Bus* bus = pws.getBus(k);
    bus->theta = x(  k);
    bus->V     = x(n+k);
    bus->P     = F(  k);
    bus->Q     = F(n+k);
  }

  // Calculate machine steady state variables
  for (size_t k = 0 ; k != pws.getGenSet_size() ; ++k){
    Generator* gen = pws.getGenerator(k);
    int busIntId = pws.getBus_intId(gen->busExtId);
    Bus* bus = pws.getBus(busIntId);
    gen->Pgen = bus->P;
    gen->Qgen = bus->Q;
    gen->Vss = polar(bus->V,bus->theta);
    gen->updateVariables();
  }

  // Calculate load steady state variables
  for ( size_t k = 0 ; k != pws.getLoadSet_size() ; ++k ){
    Load* load = pws.getLoad(k);
    int busIntId = pws.getBus_intId( load->busExtId );
    Bus* bus = pws.getBus(busIntId);
    load->Pdemand = -bus->P;
    load->Qdemand = -bus->Q;
    load->Vss = polar(bus->V,bus->theta);
  }
  pws.set_status( PWSSTATUS_LF );

  return 0;
}
