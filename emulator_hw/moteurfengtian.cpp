
#include "moteurfengtian.h"
using namespace elabtsaot;

using namespace boost::numeric::ublas;

#include "emulator.h"
#include "precisiontimer.h"

#include <iostream>
using std::cout;
using std::endl;
//#include <string>
using std::string;
#include <ctime> // for time(), ctime()

enum MoteurFengtianProperties{
  SSEMFT_PROPERTY_BETA1,
  SSEMFT_PROPERTY_BETA2,
  SSEMFT_PROPERTY_PTOL,
  SSEMFT_PROPERTY_QTOL,
  SSEMFT_PROPERTY_MAXIT
};

MoteurFengtian::MoteurFengtian(Emulator* emu, Logger* log) :
  SSEngine("MoteurFengtian (based on Emulator) s.s. engine", log),
  _emu(emu) {

  // ----- Prepare properties -----
  property_type tempPt;
  tempPt.key = SSEMFT_PROPERTY_BETA1;
  tempPt.dataType = PROPERTYT_DTYPE_DOUBLE;
  tempPt.name = "Beta 1";
  tempPt.description = "Beta 1 coefficient";
  tempPt.defaultValue = 0.1;
  tempPt.minValue = 1e-3;
  tempPt.maxValue = 1.0;
  _properties[tempPt] = tempPt.defaultValue;

  tempPt.key = SSEMFT_PROPERTY_BETA2;
  tempPt.dataType = PROPERTYT_DTYPE_DOUBLE;
  tempPt.name = "Beta 2";
  tempPt.description = "Beta 2 coefficient";
  tempPt.defaultValue = 0.1;
  tempPt.minValue = 1e-3;
  tempPt.maxValue = 1.0;
  _properties[tempPt] = tempPt.defaultValue;

  tempPt.key = SSEMFT_PROPERTY_PTOL;
  tempPt.dataType = PROPERTYT_DTYPE_DOUBLE;
  tempPt.name = "P convergeance tolerance";
  tempPt.description = "Termination tolerance on per unit P mismatch";
  tempPt.defaultValue = 1e-2;
  tempPt.minValue = 1e-14;
  tempPt.maxValue = 1e-1;
  _properties[tempPt] = tempPt.defaultValue;

  tempPt.key = SSEMFT_PROPERTY_QTOL;
  tempPt.dataType = PROPERTYT_DTYPE_DOUBLE;
  tempPt.name = "Q convergeance tolerance";
  tempPt.description = "Termination tolerance on per unit Q mismatch";
  tempPt.defaultValue = 1e-2;
  tempPt.minValue = 1e-14;
  tempPt.maxValue = 1e-1;
  _properties[tempPt] = tempPt.defaultValue;

  tempPt.key = SSEMFT_PROPERTY_MAXIT;
  tempPt.dataType = PROPERTYT_DTYPE_INT;
  tempPt.name = "Maximum iteration count";
  tempPt.description = "Maximum number of iteration of the algorithm";
  tempPt.defaultValue = 20;
  tempPt.minValue = 1;
  tempPt.maxValue = 127;
  _properties[tempPt] = tempPt.defaultValue;
}

int MoteurFengtian::do_solvePowerFlow(Powersystem const& pws,
                                      matrix<complex>& Y,
                                      vector<complex>& V) const{

  // Before entering into the power flow main routine, check that the power system
  // has been validated
  if ( pws.status() != PWSSTATUS_VALID && pws.status() != PWSSTATUS_PF )
    return 1;
  if ( pws.status() == PWSSTATUS_PF )
    // Nothing to do!
    return 0;

  // ----- Retrieve options -----
  /* beta1       :
     beta2       :
     Ptolerance  :
     Qtolerance  :
     maxIterCount:  */
  double beta1;
  double beta2;
  double Ptolerance;
  double Qtolerance;
  size_t maxIterCount;
  _getOptions(beta1, beta2, Ptolerance, Qtolerance, maxIterCount);

  // ----- Start timer -----
  PrecisionTimer timer;
  timer.Start();
  time_t rawtime;
  time ( &rawtime );
  string time_string = ctime( &rawtime );

  // TODO!!!!

  // ----- End timer -----
  double elapsed_time = timer.Stop();
  bool converged(false);

  if (!converged){
    // Not convergence case
    cout << "Power flow (Guillaume method) did not converge!" << endl;
    return 2;
  }

  else {
    cout << "Power flow (Guillaume method) started at: ";
    cout << time_string << endl;
    cout << "Power flow converged in " << elapsed_time << " seconds " <<endl;
    return 0;
  }

  return 3;
}

void MoteurFengtian::_getOptions( double& beta1,
                                  double& beta2,
                                  double& Ptolerance,
                                  double& Qtolerance,
                                  size_t& maxIterCount ) const{
  // Retrieve boost::any properties
  boost::any anyBeta1 = _getPropertyValueFromKey(SSEMFT_PROPERTY_BETA1);
  boost::any anyBeta2 = _getPropertyValueFromKey(SSEMFT_PROPERTY_BETA2);
  boost::any anyPtolerance = _getPropertyValueFromKey(SSEMFT_PROPERTY_PTOL);
  boost::any anyQtolerance = _getPropertyValueFromKey(SSEMFT_PROPERTY_QTOL);
  boost::any anyMaxIterCount = _getPropertyValueFromKey(SSEMFT_PROPERTY_MAXIT);
  // Store them in output arguments
  beta1 = boost::any_cast<double>( anyBeta1 );
  beta2 = boost::any_cast<double>( anyBeta2 );
  Ptolerance = boost::any_cast<double>( anyPtolerance );
  Qtolerance = boost::any_cast<double>( anyQtolerance );
  maxIterCount = boost::any_cast<int>( anyMaxIterCount );
}
