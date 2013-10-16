
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

int SSEngine::solveLoadflow( Powersystem& pws ) const{

  ublas::vector<double> x;
  ublas::vector<double> F;
  do_getInitialConditions(pws, x, F);

  int ans = do_solveLoadflow(pws, x, F);
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
    x(k) = pws.getBus(k)->theta;               // load steady state values -
    x(n+k) = pws.getBus(k)->V;              // estimates from buses

    // Add all gen production
    set<size_t> busGenMap = pws.getBusGenMap().at(k);
    for( set<size_t>::const_iterator i = busGenMap.begin() ;
         i != busGenMap.end() ; ++i ){
      F(  k) += pws.getGenerator(*i)->pgen();
      F(n+k) += pws.getGenerator(*i)->qgen();
    }
    // Subtract all load demand
    set<size_t> busLoadMap = pws.getBusLoadMap().at(k);
    for( set<size_t>::const_iterator i = busLoadMap.begin() ;
         i != busLoadMap.end() ; ++i ){
      F(  k) -= pws.getLoad(*i)->pdemand();
      F(n+k) -= pws.getLoad(*i)->qdemand();
    }
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
  double r, x, xc, g_from, g_to, b_from, b_to, tap, th;
  complex<double> ys, yp_from, yp_to, y11, y12, y21, y22;
  for ( size_t k = 0 ; k != pws.getBrSet_size() ; ++k ){
    Branch const* branch = pws.getBranch(k);

    // Check whether the branch is on-line
    if (!branch->status()) continue;

    // Retrieve data of the branch
    fext = branch->fromBusExtId();
    text = branch->toBusExtId();
    f = pws.getBus_intId(fext);  // from bus internal index
    t = pws.getBus_intId(text);  // to bus internal index

    r = branch->r();                     // resistance
    x = branch->x();                     // reactance
    xc = branch->c_series_x();           // series capacitance
    ys = 1.0/(complex<double>(r,x+xc));  // series admittance

    g_from = branch->g_from();           // from end shunt conductance
    b_from = branch->b_from();           // from end shunt susceptance
    yp_from = complex<double> (g_from, b_from); // from end shunt admittance
    g_to = branch->g_to();               // to end shunt conductance
    b_to = branch->b_to();               // to end shunt susceptance
    yp_to = complex<double> (g_to, b_to); // to end shunt admittance

    tap = branch->Xratio();              // Xformer tap ratio magnitude
    th = branch->Xshift();               // Xformer phase shift angle
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
    Y(k,k) += complex<double>( pws.getBus(k)->gsh, pws.getBus(k)->bsh );

  return 0;
}

int SSEngine::do_updatePowersystem( Powersystem& pws,
                                    ublas::vector<double> const& x,
                                    ublas::vector<double> const& F ) const{

  // Input argument validation
  size_t n = pws.getBusSet_size();          // number of nodes
  if ( pws.status() != PWSSTATUS_VALID && pws.status() != PWSSTATUS_LF )
    return 1;
  if ( x.size() != 2*n )
    return 2;
  if ( F.size() != 2*n )
    return 3;

  pws.set_status( PWSSTATUS_VALID );
  // Store theta-U, P-Q results in the _busSet of the pws
  for ( size_t k = 0 ; k != n ; ++k ){
    Bus* bus = pws.getBus(k);
    bus->theta = x(  k);
    bus->V     = x(n+k);
    bus->P     = F(  k);
    bus->Q     = F(n+k);
  }

  // Calculate branch flows
  size_t fromBusIntId, toBusIntId;
  double r, xbr, xc, g_from, g_to, b_from, b_to, tap, th;
  complex<double> ys, yp_from, yp_to, y11, y12, y21, y22;
  complex<double> u_f, u_t, i_f, i_t, s_f, s_t;

  size_t m = pws.getBrSet_size();
  for ( size_t k = 0 ; k != m ; ++k ){
    Branch* branch = pws.getBranch(k);

    fromBusIntId = pws.getBus_intId( branch->fromBusExtId() );
    u_f = polar( x(n + fromBusIntId) , x(fromBusIntId) );
    toBusIntId = pws.getBus_intId( branch->toBusExtId() );
    u_t = polar( x(n + toBusIntId) , x(toBusIntId) );

    r = branch->r();                 // resistance
    xbr = branch->x();               // reactance
    xc = branch->c_series_x();       // series capacitance
    ys = 1.0/(complex<double>(r,xbr+xc)); // series admittance

    g_from = branch->g_from();    // from end shunt conductance
    b_from = branch->b_from();    // from end shunt susceptance
    yp_from = complex<double> (g_from, b_from); // from end shunt admittance
    g_to = branch->g_to();        // to end shunt conductance
    b_to = branch->b_to();        // to end shunt susceptance
    yp_to = complex<double> (g_to, b_to); // to end shunt admittance

    tap = branch->Xratio();         // Xformer tap ratio magnitude
    th = branch->Xshift();        // Xformer phase shift angle
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

    // Determine current flows i
    i_f = y11 * u_f + y12 * u_t;    // current injected at from end of the br
    i_t = y21 * u_f + y22 * u_t;    // current injected at to end of the br

    // Determine apparent power flows S = P + jQ
    s_f = u_f * conj(i_f);
    s_t = u_t * conj(i_t);

    // Store flow values into branch set
    branch->set_ifrom(i_f);
    branch->set_ito(i_t);
    branch->set_sfrom(s_f);
    branch->set_sto(s_t);
  }

  // Calculate distribution of generation between machines
  double Pgen, Qgen, undistributedPgen, undistributedQgen;
  set<size_t>::iterator i;
  size_t slackGenIntId = pws.getGen_intId( pws.slackGenExtId() );
  size_t slackBusIntId = pws.getBus_intId( pws.slackBusExtId() );
  for ( size_t k = 0 ; k != n ; ++k ){ // iterating through buses
    // Computation for bus k
    set<size_t> buskLoadMap = pws.getBusLoadMap().at(k);
    Pgen = F(k);
    Qgen = F(n+k);

    // Find net P & Q generation by generators at the bus by adding the loads
    for ( i = buskLoadMap.begin() ; i != buskLoadMap.end() ; ++i ){
      Pgen += pws.getLoad(*i)->pdemand();
      Qgen += pws.getLoad(*i)->qdemand();
    }
    undistributedPgen = Pgen;
    undistributedQgen = Qgen;

    // Subtract P&Q generation of non-avr generators
    // P&Q for these gens is fixed, and not determined/changed by loadflow
    set<size_t> buskGenMap = pws.getBusGenMap().at(k);
    for ( i = buskGenMap.begin() ; i != buskGenMap.end() ; ++i ){
      if ( *i == slackGenIntId )
        // If the slack gen is encoutered skip it regardless of avr setting
        continue;

      Generator* gen = pws.getGenerator(*i);
      if ( !gen->avr() ){ // non-avr gens only
        undistributedPgen -= gen->pgen();  // undistributed p at the bus
        undistributedQgen -= gen->qgen();  // undistributed q at the bus
      }
    }

    // Distribute remaining Q among avr generators proportionally to P gen
    for ( i = buskGenMap.begin() ; i != buskGenMap.end() ; ++i ){
      if ( *i == slackGenIntId)
        // If the slack gen is encoutered, it is skipped as it will take over
        // all undistributed generation after subtracting the share of all
        // non slack generators (see end of the for loop)
        continue;

      Generator* gen = pws.getGenerator(*i);
      if ( !gen->avr() )
        // Calculation for non-avr generators has already been performed
        continue;

      // For avr-generators, go on with the computation
      gen->set_qgen( Qgen * gen->pgen()/Pgen );

      undistributedPgen -= gen->pgen();  // undistributed p at the bus
      undistributedQgen -= gen->qgen();  // undistributed q at the bus
    }
    // In case we are at the slack bus the slackGen takes over all the
    // undistributed P & Q generation
    if ( k == slackBusIntId ){
      pws.getGenerator(slackGenIntId)->set_pgen(undistributedPgen);
      pws.getGenerator(slackGenIntId)->set_qgen(undistributedQgen);
    }
  }

  // Calculate machine steady state internal variables
  complex<double> U;  // complex voltage at the machine terminals
  complex<double> I;  // complex current output at the machine terminals
  complex<double> S;  // apparent power output at the machine terminals
  complex<double> E;  // machine complex internal voltage (for model "1.0")
  complex<double> Z;  // machine internal impedance (for model "1.0")
  for ( size_t k = 0 ; k != pws.getGenSet_size() ; ++k ){
    Generator* gen = pws.getGenerator(k);
    if ( gen->model() == GENMODEL_0p0 ){
      S = complex<double> ( gen->pgen(), gen->qgen() );
      int busIntId = pws.getBus_intId(gen->busExtId());
      U = polar( pws.getBus(busIntId)->V , pws.getBus(busIntId)->theta );
      I = conj(S) / conj(U);
      Z = complex<double> ( gen->ra() , gen->xd_1() );
      E = U + I*Z;

      gen->set_Uss(U);
      gen->set_Ess( abs(E) );
      gen->set_deltass( arg(E) );

      //      } else if ( gen->model() == "xxx" ){
    } else {
      // By default, classical model "0.0" considered:
      // voltage source behind transient reactance _xd_1
      S = complex<double> ( gen->pgen(), gen->qgen() );
      int busIntId = pws.getBus_intId( gen->busExtId() );
      U = polar( pws.getBus(busIntId)->V , pws.getBus(busIntId)->theta );
      I = conj(S) / conj(U);
      Z = complex<double> ( gen->ra() , gen->xd_1() );
      E = U + I*Z;

      gen->set_Uss(U);
      gen->set_Ess( abs(E) );
      gen->set_deltass( arg(E) );
    }
  }

  // Calculate load steady state variables
  complex<double> Uload;  // complex voltage at the load terminals
  for ( size_t k = 0 ; k != pws.getLoadSet_size() ; ++k ){
    Load* load = pws.getLoad(k);
    int busIntId = pws.getBus_intId( load->busExtId() );
    Uload = polar( pws.getBus(busIntId)->V , pws.getBus(busIntId)->theta );
    load->set_Uss(Uload);
  }
  pws.set_status( PWSSTATUS_LF );

  return 0;
}
