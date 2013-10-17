
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
  tempPt.description = "Start power flow computation from a flat point, ie. |V|=1 ang(V)=0 for all buses";
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

int MoteurRenard::do_solvePowerFlow( Powersystem const& pws,
                                     ublas::vector<double>& x,
                                     ublas::vector<double>& F ) const{

  // Before entering into the power flow main routine, check that the power system
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

  // TODO
  return 721;
  // ----- Start power flow computation proper -----

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


  // ************** Calculate full Jacobian J **************
  /*
    J = [dP/dth dP/dU]   and   [DP] = [J] * [dth] => [DP] = [JPth JPU] * [dth]
        [dQ/dth dQ/dU]         [DQ]         [dU ]    [DQ]   [JQth JQU]   [dU]
    JPth = dP/dth
    JPU  = dP/dU
    JQth = dQ/dth
    JQU  = dQ/dU
  */

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
