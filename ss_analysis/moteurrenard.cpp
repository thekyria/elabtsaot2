
#include "moteurrenard.h"
using namespace elabtsaot;

#include "logger.h"
#include "precisiontimer.h"

#include <boost/numeric/ublas/lu.hpp> // for matrix operations
#include <boost/numeric/ublas/io.hpp> // for ublas::matrix '<<'
//#include <boost/numeric/ublas/matrix.hpp> // Required for matrix operations
namespace ublas = boost::numeric::ublas;

#include <set>
using std::set;
//#include <string>
using std::string;
#include <vector>
using std::vector;
#include <iostream>
using std::cout;
using std::endl;
#include <complex>
using std::complex;
#include <limits>
using std::numeric_limits;
#include <ctime>                      // for time(), ctime()
#include <cmath>                      // for M_PI constant
#define _USE_MATH_DEFINES

enum MoteurRenardProperties{
  SSEMRN_PROPERTY_FLATSTART,
  SSEMRN_PROPERTY_QLIM,
  SSEMRN_PROPERTY_TOL,
  SSEMRN_PROPERTY_MAXIT
};

//! Bus type enumeration
enum BusType {
  BUSTYPE_UNDEF,         //!< Undefined bus type (possibly only yet)
  BUSTYPE_PQ,            //!< PQ bus type: P and Q injection defined at the bus
  BUSTYPE_PV,            //!< PV bus type: P injection and voltage magnitude
                         //!< defined at the bus
  BUSTYPE_SLACK          //!< Slack bus: Voltage magnitude and angle defined
                         //!< defined at the bus
};

MoteurRenard::MoteurRenard(Logger* log):
    SSEngine("MoteurRenard s.s. engine", log){

  // ----- Prepare properties -----
  property_type tempPt;
  tempPt.key = SSEMRN_PROPERTY_FLATSTART;
  tempPt.dataType = PROPERTYT_DTYPE_BOOL;
  tempPt.name = "Flat start";
  tempPt.description = "Start loadflow computation from a flat point, ie. |V|=1 ang(V)=0 for all buses";
  tempPt.defaultValue = true;
  _properties[tempPt] = tempPt.defaultValue;

  tempPt.key = SSEMRN_PROPERTY_QLIM;
  tempPt.dataType = PROPERTYT_DTYPE_BOOL;
  tempPt.name = "Enforce Q limits";
  tempPt.description = "Enforce gen reactive power limits at expense of |V|";
  tempPt.defaultValue = true;
  _properties[tempPt] = tempPt.defaultValue;

  tempPt.key = SSEMRN_PROPERTY_TOL;
  tempPt.dataType = PROPERTYT_DTYPE_DOUBLE;
  tempPt.name = "Convergeance tolerance";
  tempPt.description = "Termination tolerance on per unit P & Q mismatch";
  tempPt.defaultValue = 1e-8;
  tempPt.minValue = 1e-14;
  tempPt.maxValue = 1e-3;
  _properties[tempPt] = tempPt.defaultValue;

  tempPt.key = SSEMRN_PROPERTY_MAXIT;
  tempPt.dataType = PROPERTYT_DTYPE_INT;
  tempPt.name = "Maximum iteration count";
  tempPt.description = "Maximum number of iteration of the algorithm";
  tempPt.defaultValue = 20;
  tempPt.minValue = 5;
  tempPt.maxValue = 100;
  _properties[tempPt] = tempPt.defaultValue;
}

int MoteurRenard::do_solveLoadflow( Powersystem const& pws,
                                    ublas::vector<double>& x,
                                    ublas::vector<double>& F ) const{

  // Before entering into the loadflow main routine, check that the power system
  // has been validated
  if ( pws.status() != PWSSTATUS_VALID && pws.status() != PWSSTATUS_LF )
    return 1;
  if ( pws.status() == PWSSTATUS_LF )
    // Nothing to do!
    return 0;

  // ----- Retrieve options -----
  /* flatStart: start iteration from a flat voltage profile (Vmag=1, Vang=0)
   tolerance: convergence criteria tolerance
   enforceQLimits : if true, generator Q limits are enforced, by converting pv
                    buses to pq buses when qgen limits are violated
   maxIterCount: maximum number of allowable iterations of main loop converge */
  bool flatStart;
  bool enforceQLimits;
  double tolerance;
  size_t maxIterCount;
  _getOptions( flatStart, enforceQLimits, tolerance, maxIterCount );

  // ----- Start timer -----
  PrecisionTimer timer;
  timer.Start();
  time_t rawtime;
  time ( &rawtime );
  string time_string = ctime( &rawtime );

  // ----- Start loadflow computation proper -----

  /* Let be:
  x   = [th]                   the vector of unknown values
        [ U]
  Fsp = [Psp]                  the vector of setpoint power values
        [Qsp]
  F   = [P=f(th,U)]            the vector of calculated power values
        [Q=g(th,U)]
  DF  = Fsp - F                the calculated power mismatch
  J   = dF/dx = [dP/dth dP/dU] the Jacobian of the non linear system DF = J*Dx
                [dQ/dth dQ/dU]

  __Algorithm to solve case__
  Init x  : known th for slack bus, know U's for slack and BUSTYPE_PV buses.
  Init F  : according to x.
  Init Fsp: known P's for BUSTYPE_PV and BUSTYPE_PQ buses,
            known Q for BUSTYPE_PQ buses, value from F for the rest.
  Loop:
    Calculate DF
    Check convergence criteria: if norm(DF)<tolerance exit and x_solution = x.
    Calculate J
    Build pv, pq index vectors
    Build reduced DFred vector: include P for BUSTYPE_PV & BUSTYPE_PQ buses and
                                        Q for BUSTYPE_PQ buses
    Build reduced Jred matrix: no dF/dx elements for slack x's
                               no dQ/dx and dF/dU elements for BUSTYPE_PV buses
    Solve DFred = Jred*Dxred: solve system for Dxred
    Update x from Dxred: according to pv,pq index vectors
    Update F = F(x)
    Update Fsp from F: P & Q for slack bus, Q for BUSTYPE_PV buses
  End_loop
  */

  // ***************************** Initialization *****************************

  size_t k, l, m;                           // counters
  size_t n = pws.getBusSet_size();          // number of nodes
  size_t p = 0;                             // number of BUSTYPE_PV nodes
  size_t q = 0;                             // number of BUSTYPE_PQ nodes

  // ************** Init x, F, Fmin, Fmax & Fsp **************
  x.resize(2*n); x.clear();                       // (0:n-1) th , (n:2n-1) U
  for (k = 0 ; k != n ; ++k){
    if ( flatStart ){                             // flat start
      x(k) = 0;                                   // flat start angle = 0 rad
      x(n+k) = 1;                                 // flat start voltage = 1 pu
    } else {                                      // not flat start
      x(k) = pws.getBus(k)->thss();               // load steady state values -
      x(n+k) = pws.getBus(k)->Vss();              // estimates from buses
    }
  }
  ublas::vector<double> Fsp(2*n); Fsp.clear();    // (0:n-1) Psp, (n:2n-1) Qsp
  F.resize(2*n); F.clear();                       // (0:n-1) P  , (n:2n-1) Q
  ublas::vector<double> Fmin(2*n); Fmin.clear();  // generation min capabilities
  ublas::vector<double> Fmax(2*n); Fmax.clear();  // generation max capabilities
  ublas::vector<double> DF(2*n);                  // power mismatch Fsp-F
                                                  // (0:n-1) DP  , (n:2n-1) DQ
  ublas::vector<size_t> busType(n);         // 0:BUSTYPE_UNDEF
  busType.clear();                                // 1:BUSTYPE_PQ
                                                  // 2:BUSTYPE_PV
                                                  // 3:BUSTYPE_SLACK

  // Init th, U and limits at slack bus
  double const kangleRef = 0;                   // angle reference [rad]
  size_t sl_gen_intId = pws.getGen_intId( pws.slackGenExtId() );
  size_t sl_bus_intId = pws.getBus_intId( pws.slackBusExtId() );
  busType(sl_bus_intId) = BUSTYPE_SLACK;
  // Slack bus reference angle = 0 [rad] & voltage = v_setpoint of the slack gen
  x(    sl_bus_intId) = kangleRef;
  x(n + sl_bus_intId) = pws.getGenerator(sl_gen_intId)->voltageSetpoint();
  // Infinite P & Q generation capability
  Fmin(sl_bus_intId) = -numeric_limits<float>::infinity();
  Fmax(sl_bus_intId) = +numeric_limits<float>::infinity();
  Fmin(n + sl_bus_intId) = -numeric_limits<float>::infinity();
  Fmax(n + sl_bus_intId) = +numeric_limits<float>::infinity();

  // Initialize Psp and U at buses with gens
  size_t busIntId;
  for(k = 0; k != pws.getGenSet_size(); ++k){
    const Generator* gen = pws.getGenerator(k);
    // Check whether gen(k) is online
    if ( !gen->status() )
      continue;

    busIntId = pws.getBus_intId( gen->busExtId() );

    // Check whether the bus is the slack bus
    if ( busType(busIntId) == BUSTYPE_SLACK )
      // gen at slack makes no sense as it gets absorbed by slack genereration
      continue;

    Fsp(busIntId) += gen->pgen();       // P(busIntId) += P(k)

    Fmin(busIntId) += gen->pmin();      // P capability at bus(busIntId)
    Fmax(busIntId) += gen->pmax();      // augmented by gen k'th limits
    Fmin(n + busIntId) += gen->qmin();  // Q capability at bus(busIntId)
    Fmax(n + busIntId) += gen->qmax();  // augmented by gen k'th limits

    if ( gen->avr() ){

      // if the gen has avr, that is, it can enforce a V to its bus then the bus
      // is a PV bus with U(busIntId)=gen(k).Vsetpoint
      x(n + busIntId) = gen->voltageSetpoint();
      // !Validation makes sure no diff Vsetpoints on the same bus!

      if ( busType(busIntId) != BUSTYPE_PV ){   // if it's not flagged as
        busType(busIntId) = BUSTYPE_PV;         // BUSTYPE_PV yet, flag it
        ++p;
      }

    } else { // ( !gen->avr() )

      // if the gen does not have avr, that is, it cannot enforce a V to its bus
      // the bus is essentially a PQ bus (unless it's already flagged as PV by
      // another gen)
      Fsp(n + busIntId) += gen->qgen();       // Q(busIntId) += Q(k)

      if ( (busType(busIntId) != BUSTYPE_PV) ){     // if bus is not BUSTYPE_PV
        if ( busType(busIntId) != BUSTYPE_PQ ){     // if it's not flagged as
          busType(busIntId) = BUSTYPE_PQ;           // BUSTYPE_PQ yet, flag it
          ++q;
        }
      }

    }
  }

  // Initialize Psp and Qsp at buses with loads
  for(k = 0; k != pws.getLoadSet_size(); ++k){
    const Load* load = pws.getLoad(k);
    // Check whether load(k) is online
    if ( !load->status() )
      continue;

    busIntId = pws.getBus_intId(load->busExtId());
    if ( (busType(busIntId) != BUSTYPE_SLACK) ){          // if bus is not Slack
      // load at slack makes no sense as it gets absorbed by slack genereration
      Fsp(busIntId) -= load->pdemand();
      Fsp(n + busIntId) -= load->qdemand();

      Fmin(busIntId) -= load->pdemand();      // P capability at busIntId
      Fmax(busIntId) -= load->pdemand();      // shifted by load k demand
      Fmin(n + busIntId) -= load->qdemand();  // Q capability at busIntId
      Fmax(n + busIntId) -= load->qdemand();  // shifted by load k demand

      if ( (busType(busIntId) != BUSTYPE_PV) ){     // if bus is not BUSTYPE_PV
        if ( busType(busIntId) != BUSTYPE_PQ ){     // if it's not flagged as
          busType(busIntId) = BUSTYPE_PQ;           // BUSTYPE_PQ yet, flag it
          ++q;
        }
      }
    }
  }

  // Build admittance matrix Y
  ublas::matrix<complex<double> > Y;
  buildY(pws, Y);
  // Build conductance matrix G (real of Y) & susceptance matrix B (imag of Y)
  ublas::matrix<double > G(Y.size1(),Y.size2());
  ublas::matrix<double > B(Y.size1(),Y.size2());
  for (size_t k = 0; k < Y.size1() ; ++k){
    for (size_t l = 0; l < Y.size2() ; ++l){
      G(k,l) = real(Y(k,l));
      B(k,l) = imag(Y(k,l));
    }
  }

  // Calculate F
  double costh, sinth;  // temporary variables to speed-up cos, sin computations
  for(k = 0; k != n; ++k){
    for(m = 0; m != n; ++m){
      costh = cos( x(k)- x(m));
      sinth = sin( x(k)- x(m));

      //P(k) += G(k,m)*  U(m)*cos(th(k)-th(m)) + B(k,m)*  U(m)*sin(th(k)-th(m));
      F(k)   += G(k,m)*x(n+m)*costh + B(k,m)*x(n+m)*sinth;

      //Q(k) += G(k,m)*  U(m)*sin(th(k)-th(m)) - B(k,m)*  U(m)*sin(th(k)-th(m));
      F(n+k) += G(k,m)*x(n+m)*sinth - B(k,m)*x(n+m)*costh;
    }
    F(k)   *= x(n+k); // P(k) *= U(k)
    F(n+k) *= x(n+k); // Q(k) *= U(k)
  }

  // Init Fsp according to F
  for (k = 0; k!= n ; ++k){
    switch (busType(k)) {
     case BUSTYPE_PQ:
      break;
     case BUSTYPE_PV:
      Fsp(n+k) = F(n+k);
      break;
     case BUSTYPE_SLACK:
      Fsp(k) = F(k);
      Fsp(n+k) = F(n+k);
      break;
     case BUSTYPE_UNDEF:
     default:
      busType(k) = BUSTYPE_PQ;    // unclassified buses are flagged BUSTYPE_PQ
                                  // with Pdem = Qdem = 0
      q++;
      Fsp(k) = 0;         // P(k) = 0;
      Fsp(n+k) = 0;       // Q(k) = 0;
      break;
    }
  }

  // ************** Init DF **************
  DF = Fsp - F;

  // ************** Variables decleration **************
  size_t iteration_count = 0;

  ublas::vector<double> Dx(2*n); Dx.clear();      // (0:n-1) Dth , (n:2n-1) DU

  ublas::matrix<double> JPth(n,n); JPth.clear();  // Jacobian dP/dth
  ublas::matrix<double>  JPU(n,n);  JPU.clear();  // Jacobian dP/dU
  ublas::matrix<double> JQth(n,n); JQth.clear();  // Jacobian dQ/dth
  ublas::matrix<double>  JQU(n,n);  JQU.clear();  // Jacobian dQ/dU

  ublas::vector<size_t> pv;                 // pv buses index vector
  ublas::vector<size_t> pq;                 // pq buses index vector

  ublas::vector<double> DFred;                    // reduced DF vector

  ublas::matrix<double> Jred;                     // reduced Jacobian matrix

  // Variables used in convergence criterion
  double norm_1_DF = norm_1(DF);                  // 1-norm of power mismatch DF
  double norm_2_DF = norm_2(DF);                  // 2-norm of power mismatch DF
  double norm_inf_DF = norm_inf(DF);              // inf-norm of pwr mismatch DF
  double norm_1_Dx = norm_1(Dx);                  // 1-norm of x movement
  double norm_2_Dx = norm_2(Dx);                  // 2-norm of x movement
  double norm_inf_Dx = norm_inf(Dx);              // inf-norm of x movement

  // ************** Main Loop  **************
  while(iteration_count != maxIterCount){
    ++iteration_count;

    // ************** Convergence criterion based on DF ************************
    // *************************************************************************
    norm_1_DF = norm_1(DF);
    norm_2_DF = norm_2(DF);
    norm_inf_DF = norm_inf(DF);

    if ( norm_2_DF < tolerance ) break;
    // *************************************************************************
    // *************************************************************************

    // ************** Calculate full Jacobian J **************
    /*
      J = [dP/dth dP/dU]   and   [DP] = [J] * [dth] => [DP] = [JPth JPU] * [dth]
          [dQ/dth dQ/dU]         [DQ]         [dU ]    [DQ]   [JQth JQU]   [dU]
      JPth = dP/dth
      JPU  = dP/dU
      JQth = dQ/dth
      JQU  = dQ/dU
    */

    JPth.clear();
    JPU.clear();
    JQth.clear();
    JQU.clear();
    for(k = 0 ; k != n; ++k){
      for(m = 0 ; m != n; ++m){
        if(m!=k){
          costh = cos( x(k)- x(m));
          sinth = sin( x(k)- x(m));

          // Diagonal elements
          JPth(k,k) += x(n+m)*(-G(k,m)*sinth + B(k,m)*costh );
          JPU(k,k)  += x(n+m)*( G(k,m)*costh + B(k,m)*sinth );

          JQth(k,k) += x(n+m)*( G(k,m)*costh + B(k,m)*sinth );
          JQU(k,k)  += x(n+m)*( G(k,m)*sinth - B(k,m)*costh );

          // Non-diagonal elements
          JPth(k,m) =  x(n+k) * x(n+m) * ( G(k,m)*sinth - B(k,m)*costh );
          JPU(k,m)  =  x(n+k) * ( G(k,m)*costh + B(k,m)*sinth );

          JQth(k,m) = -x(n+k) * x(n+m) * ( G(k,m)*costh + B(k,m)*sinth );
          JQU(k,m)  =  x(n+k) * ( G(k,m)*sinth - B(k,m)*costh );
        }
      }
      // Diagonal elements
      JPth(k,k) *= x(n+k);
      JPU(k,k)  +=  2 * x(n+k) * G(k,k);

      JQth(k,k) *= x(n+k);
      JQU(k,k)  += -2 * x(n+k) * B(k,k);
    }

    // ************** Build pv, pq index vectors **************
    pq.resize(q); pv.resize(p);
    pq.clear();   pv.clear();
    l = 0;        m = 0;
    for (k = 0 ; k != n ; ++k){
      if (busType(k) == BUSTYPE_PQ)
        pq(l++) = k;
      else if (busType(k) == BUSTYPE_PV)
        pv(m++) = k;
    }

    // ************** Build reduced DFred vector **************
    DFred.resize(p+2*q);
    for (k = 0 ; k != p ; ++k)        // DF for BUSTYPE_PV buses (only DP)
      DFred(      k) = DF(    pv(k)); // DPred(k)=DP(pv(k))-for BUSTYPE_PV buses
    for (k = 0 ; k != q ; ++k){       // DF for BUSTYPE_PQ buses (DP & DQ)
      DFred(p   + k) = DF(    pq(k)); // DPred(k)=DP(pq(k))-for BUSTYPE_PQ buses
      DFred(p+q + k) = DF(n + pq(k)); // DQred(k)=DQ(pq(k))-for BUSTYPE_PQ buses
    }


    // ************** Build reduced Jred matrix **************
/*
     DFred   =     Jred        Dxred
  (p+2q x 1)   (p+2q x p+2q) (p+2q x 1)

In the following:
PVz: z'th BUSTYPE_PV bus, PQz: z'th BUSTYPE_PQ bus

DFred = [ DP(PV1) .. DP(PVp)  DP(PQ1)  .. DP(PQq)  DQ(PQ1)   .. DQ(PQq)  ]' =
      = [ DF(PV1) .. DF(PVp)  DF(PQq)  .. DF(PQq)  DF(n+PQ1) .. DQ(n+PQq)]'

Dxred = [Dth(PV1) .. Dth(PVp) Dth(PQ1) .. Dth(PQq) DU(PQ1)   .. DU(PQq)  ]' =
      = [ Dx(PV1) .. Dx(PVp)  Dx(PQ1)  .. Dx(PQq)  Dx(n+PQ1) .. Dx(n+PQq)]'

Jred = [
JPth(PV1,PV1)..JPth(PV1,PVp) JPth(PV1,PQ1)..JPth(PV1,PQq) JPU(PV1,PQ1)..JPU(PV1,PQq)
     ..              ..             ..              ..            ..             ..
JPth(PVp,PV1)..JPth(PVp,PVp) JPth(PVp,PQ1)..JPth(PVp,PQq) JPU(PVp,PQ1)..JPU(PVp,PQq)
JPth(PQ1,PV1)..JPth(PQ1,PVp) JPth(PQ1,PQ1)..JPth(PQ1,PQq) JPU(PQ1,PQ1)..JPU(PQ1,PQq)
     ..              ..             ..              ..            ..             ..
JPth(PQq,PV1)..JPth(PQq,PVp) JPth(PQq,PQ1)..JPth(PQq,PQq) JPU(PQq,PQ1)..JPU(PQq,PQq)
JQth(PQ1,PV1)..JQth(PQ1,PVp) JQth(PQ1,PQ1)..JQth(PQ1,PQq) JQU(PQ1,PQ1)..JQU(PQ1,PQq)
     ..              ..             ..              ..            ..             ..
JQth(PQq,PV1)..JQth(PQq,PVp) JQth(PQq,PQ1)..JQth(PQq,PQq) JQU(PQq,PQ1)..JQU(PQq,PQq)
] =
                      [ subJred(1,1) subJred(1,2) subJred(1,3) ]
                      [ subJred(2,1) subJred(2,2) subJred(2,3) ]
                      [ subJred(3,1) subJred(3,2) subJred(3,3) ]

*/
    Jred.resize(p+2*q , p+2*q);
    for (k = 0 ; k != p ; ++k){
      for (m = 0 ; m != p ; ++m)
        Jred(k       , m      ) = JPth( pv(k) , pv(m) );  // subJred(1,1)

      for (m = 0 ; m != q ; ++m){
        Jred(k       , p + m  ) = JPth( pv(k) , pq(m) );  // subJred(1,2)
        Jred(k       , p+q + m) =  JPU( pv(k) , pq(m) );  // subJred(1,3)
      }
    }
    for (k = 0 ; k != q ; ++k){
      for (m = 0 ; m != p ; ++m){
        Jred(p + k   , m      ) = JPth( pq(k) , pv(m) );  // subJred(2,1)

        Jred(p+q + k , m      ) = JQth( pq(k) , pv(m) );  // subJred(3,1)
      }

      for (m = 0 ; m != q ; ++m){
        Jred(p + k   , p + m  ) = JPth( pq(k) , pq(m) );  // subJred(2,2)
        Jred(p + k   , p+q + m) =  JPU( pq(k) , pq(m) );  // subJred(2,3)

        Jred(p+q + k , p + m  ) = JQth( pq(k) , pq(m) );  // subJred(3,2)
        Jred(p+q + k , p+q + m) =  JQU( pq(k) , pq(m) );  // subJred(3,3)
      }
    }

    // ************** Solve DFred = Jred*Dxred for Dxred **************
    ublas::permutation_matrix<size_t>* p_pm =
     new ublas::permutation_matrix<size_t> ( Jred.size1() );
    ublas::matrix<double>* p_M = new ublas::matrix<double> ( Jred );
    ublas::vector<double>* p_Dxred = new ublas::vector<double> ( DFred );

    BOOST_TRY {
      ublas::lu_factorize( *p_M, *p_pm );
      ublas::lu_substitute( *p_M, *p_pm, *p_Dxred );
    } BOOST_CATCH(ublas::singular const& ex) {
      cout << "Singularity likely!" << endl;
      cout << "Exception message: " << ex.what() << endl;
    } BOOST_CATCH(std::exception const& ex) {
      cout << "Other exception caught!" << endl;
      cout << "Exception message: " << ex.what() << endl;
    } BOOST_CATCH(...){
      cout << "Operation failed!" << endl;
    }
    BOOST_CATCH_END
    // p_Dxred points to a ublas::vector<double> that holds the solution Dxred
    delete p_pm;
    delete p_M;

    // ************** Update x from Dxred **************
    // Get Dx
    Dx.clear();
    for (k = 0; k != p; ++k)
      Dx( pv(k) ) = (*p_Dxred)(k);
    for (k = 0; k != q; ++k){
      Dx( pq(k) ) = (*p_Dxred)(p + k);
      Dx( n + pq(k) ) = (*p_Dxred)(p + q + k);
    }
    delete p_Dxred;

    // ************** Convergence criterion based on Dx ************************
    // *************************************************************************
    norm_1_Dx = norm_1(Dx);
    norm_2_Dx = norm_2(Dx);
    norm_inf_Dx = norm_inf(Dx);
    // if ( norm_2_Dx < tolerance ) break;
    // *************************************************************************
    // *************************************************************************

    // Update x by adding Dx
    // Attention: Do not modify th & U for slack bus and U for BUSTYPE_PV buses!
    for (k = 0; k != n; ++k){
      switch (busType(k)) {
       case BUSTYPE_SLACK:
        break;
       case BUSTYPE_PQ:
        x(n + k) += Dx(n + k); //  U(k) =  U(k) +  DU(k)
        // BUSTYPE_PQ nodes (load nodes) are converted to BUSTYPE_PV nodes only when at the bus
        // there is equipment that can regulate the voltage (like gens, C's etc)
        // TODO
       case BUSTYPE_PV:
        x(    k) += Dx(    k); // th(k) = th(k) + Dth(k)
       default:
        // Keep thetas in the interval [-pi,pi)
        if (x(k) >= M_PI) x(k) -= 2*M_PI;
        else if (x(k) < -M_PI) x(k) += 2*M_PI;
        break;
      }
    }

    // ************** Update F = F(x) **************
    F.clear();
    for(k = 0; k != n; ++k){
      for(m = 0; m != n; ++m){
        costh = cos( x(k)- x(m));
        sinth = sin( x(k)- x(m));

        //P(k)+=G(k,m)*  U(m)*cos(th(k)-th(m)) + B(k,m)*  U(m)*sin(th(k)-th(m));
        F(k)  +=G(k,m)*x(n+m)*costh + B(k,m)*x(n+m)*sinth;

        //Q(k)+=G(k,m)*  U(m)*sin(th(k)-th(m)) - B(k,m)*  U(m)*sin(th(k)-th(m));
        F(n+k)+=G(k,m)*x(n+m)*sinth - B(k,m)*x(n+m)*costh;
      }
      F(k)   *= x(n+k); // P(k) *= U(k)
      F(n+k) *= x(n+k); // Q(k) *= U(k)

      if ( enforceQLimits ){
        // Check for Q generation capabilities violations at BUSTYPE_PV buses [Fmin, Fmax]
        if (busType(k) == BUSTYPE_PV){
          if (F(n+k) > Fmax(n+k)){
            F(n+k) = Fmax(n+k);
            Fsp(n+k) = F(n+k);
            busType(k) = BUSTYPE_PQ;
            --p; ++q;
          } else if (F(n+k) < Fmin(n+k)){
            F(n+k) = Fmin(n+k);
            Fsp(n+k) = F(n+k);
            busType(k) = BUSTYPE_PQ;
            --p; ++q;
          }
        }
      }
    }

    // ************** Update Fsp from F **************
    // Slack bus has no P&Q setpoint, so for each iteration F is considered sp
    // BUSTYPE_PV buses have no Q setpoint so for each iteration P(PVz) is considered sp
    for (k = 0; k != n; ++k){
      switch (busType(k)) {
       case BUSTYPE_SLACK:
        Fsp(    k) = F(    k);  // Psp(k) = P(k)
       case BUSTYPE_PV:
        Fsp(n + k) = F(n + k);  // Qsp(k) = Q(k)
       case BUSTYPE_PQ:
       case BUSTYPE_UNDEF:
       default:
        break;
      }
    }

    // ************** Calculate DF **************
    DF = Fsp - F;

  } // END OF MAIN LOOP

  double elapsed_time = timer.Stop();

  // ************** Loadflow results **************
  bool converged = false;
  if (iteration_count != maxIterCount){
    converged = true;
  }

  if (!converged){
    // Not convergence case
    // Do not save any results in the Powersystem class
    // Display output
    cout << "Load flow did not converge!" << endl;
    cout << "Final theta - U values:" << endl;
    for ( k = 0 ; k != n ; ++k ){
      cout << "theta[" << k << "] = " << x(k);
      cout << "\tU[" << k << "] = " << x(n+k);
      cout << endl;
    }
    cout << endl;

    cout << "Final calculated P - Q values: " << endl;
    for ( k = 0 ; k != n ; ++k ){
      cout << "P[" << k << "] = " << F(k);
      cout << "\tQ[" << k << "] = " << F(n+k);
      cout << endl;
    }
    cout << endl;

    cout << "Final calculated minus setpoint power mismatch: " << endl;
    for ( k = 0 ; k != n ; ++k ){
      cout << "DP[" << k << "] = " << DF(k);
      cout << "\tDQ[" << k << "] = " << DF(n+k);
      cout << endl;
    }
    cout << endl;
    cout << "Power mismatch vector norm : " << norm_2(DF) << endl;
    cout << endl;

    return 2;
  }

  // Convergence case
  else {
    cout << "AC Power Flow (Newton-Raphson method) started at: ";
    cout << time_string;
    cout << "Load flow converged in " <<iteration_count<< " iterations ";
    cout << "and " << elapsed_time << " seconds " <<endl;
    return 0;
  }

  return 3;
}

void MoteurRenard::_getOptions( bool& flatStart,
                                bool& enforceQLimits,
                                double& tolerance,
                                size_t& maxIterCount ) const{
  // Retrieve boost::any properties
  boost::any anyFlatStart = _getPropertyValueFromKey(SSEMRN_PROPERTY_FLATSTART);
  boost::any anyEnforceQLimits = _getPropertyValueFromKey(SSEMRN_PROPERTY_QLIM);
  boost::any anyTolerance = _getPropertyValueFromKey(SSEMRN_PROPERTY_TOL);
  boost::any anyMaxIterCount = _getPropertyValueFromKey(SSEMRN_PROPERTY_MAXIT);
  // Store them in output arguments
  flatStart = boost::any_cast<bool>( anyFlatStart );
  enforceQLimits = boost::any_cast<bool>( anyEnforceQLimits );
  tolerance = boost::any_cast<double>( anyTolerance );
  maxIterCount = boost::any_cast<int>( anyMaxIterCount );
}
