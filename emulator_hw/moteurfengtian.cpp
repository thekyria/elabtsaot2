
#include "moteurfengtian.h"
using namespace elabtsaot;

using namespace boost::numeric::ublas;

#include "emulator.h"
#include "encoder.h"

#include <boost/timer/timer.hpp>

#include <iostream>
using std::cout;
using std::endl;
//#include <string>
using std::string;
#include <ctime> // for time(), ctime()

enum MoteurFengtianProperties{
  MFT_PROPERTY_BETA1,
  MFT_PROPERTY_BETA2,
  MFT_PROPERTY_PTOL,
  MFT_PROPERTY_QTOL,
  MFT_PROPERTY_MAXIT,
  MFT_PROPERTY_METHOD
};

enum MoteurFengtianMethod{
  MFT_METHOD_GUILLAUME = 0,
  MFT_METHOD_DC = 1
};

MoteurFengtian::MoteurFengtian(Emulator* emu, Logger* log) :
  SSEngine("MoteurFengtian (based on Emulator) s.s. engine", log),
  _emu(emu) {

  // ----- Prepare properties -----
  property_type tempPt;
  tempPt.key = MFT_PROPERTY_BETA1;
  tempPt.dataType = PROPERTYT_DTYPE_DOUBLE;
  tempPt.name = "Beta 1";
  tempPt.description = "Beta 1 coefficient";
  tempPt.defaultValue = 0.1;
  tempPt.minValue = 1e-3;
  tempPt.maxValue = 1.0;
  _properties[tempPt] = tempPt.defaultValue;

  tempPt.key = MFT_PROPERTY_BETA2;
  tempPt.dataType = PROPERTYT_DTYPE_DOUBLE;
  tempPt.name = "Beta 2";
  tempPt.description = "Beta 2 coefficient";
  tempPt.defaultValue = 0.1;
  tempPt.minValue = 1e-3;
  tempPt.maxValue = 1.0;
  _properties[tempPt] = tempPt.defaultValue;

  tempPt.key = MFT_PROPERTY_PTOL;
  tempPt.dataType = PROPERTYT_DTYPE_DOUBLE;
  tempPt.name = "P convergeance tolerance";
  tempPt.description = "Termination tolerance on per unit P mismatch";
  tempPt.defaultValue = 1e-2;
  tempPt.minValue = 1e-14;
  tempPt.maxValue = 1e-1;
  _properties[tempPt] = tempPt.defaultValue;

  tempPt.key = MFT_PROPERTY_QTOL;
  tempPt.dataType = PROPERTYT_DTYPE_DOUBLE;
  tempPt.name = "Q convergeance tolerance";
  tempPt.description = "Termination tolerance on per unit Q mismatch";
  tempPt.defaultValue = 1e-2;
  tempPt.minValue = 1e-14;
  tempPt.maxValue = 1e-1;
  _properties[tempPt] = tempPt.defaultValue;

  tempPt.key = MFT_PROPERTY_MAXIT;
  tempPt.dataType = PROPERTYT_DTYPE_INT;
  tempPt.name = "Maximum iteration count";
  tempPt.description = "Maximum number of iteration of the algorithm";
  tempPt.defaultValue = 20;
  tempPt.minValue = 1;
  tempPt.maxValue = 127;
  _properties[tempPt] = tempPt.defaultValue;
}

int MoteurFengtian::do_solvePowerFlow(Powersystem const& pws, vector<complex>& V) const{

  boost::timer::auto_cpu_timer t; // when t goes out of scope it prints timing info

  // Before entering into the power flow main routine, check that the power system
  // has been validated
  if ( pws.status() != PWSSTATUS_VALID && pws.status() != PWSSTATUS_PF )
    return 1;
//  if ( pws.status() == PWSSTATUS_PF )
//    // Nothing to do!
//    return 0;

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
  int method;
  _getOptions(beta1, beta2, Ptolerance, Qtolerance, maxIterCount, method);

  bool converged(false);

  // Map & fit & encode pws & end calib. mode
  int ans = _emu->preconditionEmulator(EMU_OPTYPE_PF);
  if ( ans ) return ans;

  // Finalize encoding
  // beta
  int32_t tempMSB, tempLSB;
  encoder::detail::form_word(beta2, 11, 10, false, &tempMSB);
  encoder::detail::form_word(beta1, 11, 10, false, &tempLSB);
  int32_t tempBeta = (tempMSB<<16) | (tempLSB);
  // tolerance
  encoder::detail::form_word(Qtolerance, 16, 11, false, &tempMSB);
  encoder::detail::form_word(Ptolerance, 16, 11, false, &tempLSB);
  int32_t tempTol = (tempMSB<<16) | (tempLSB);
  // max iteration
  int32_t mask7 = (1<<7)-1;
  int32_t tempMaxIter = maxIterCount;
  tempMaxIter &= mask7;
  for (size_t k(0);k!=_emu->encoding.size();++k){
    _emu->encoding[k][603] = tempBeta;
    _emu->encoding[k][606] = tempMaxIter;
    _emu->encoding[k][608] = tempTol;
  }

  // Finally write the modified _emu->encoding vector to the devices
  ans = _emu->writeEncoding(false);
  if (ans) return 42;

  // Write the start code
  size_t address605(605);
  std::vector<uint32_t> encoding605(1, 1111);
  for (size_t sliceId_(0); sliceId_!=_emu->getHwSliceCount(); ++sliceId_)
    _emu->usbWrite(sliceId_, address605, encoding605);

  // TODO

  if (!converged) return 2;
  return 0;
}

void MoteurFengtian::_getOptions(double& beta1,
                                 double& beta2,
                                 double& Ptolerance,
                                 double& Qtolerance,
                                 size_t& maxIterCount,
                                 int& method) const{
  // Retrieve boost::any properties
  boost::any anyBeta1 = _getPropertyValueFromKey(MFT_PROPERTY_BETA1);
  boost::any anyBeta2 = _getPropertyValueFromKey(MFT_PROPERTY_BETA2);
  boost::any anyPtolerance = _getPropertyValueFromKey(MFT_PROPERTY_PTOL);
  boost::any anyQtolerance = _getPropertyValueFromKey(MFT_PROPERTY_QTOL);
  boost::any anyMaxIterCount = _getPropertyValueFromKey(MFT_PROPERTY_MAXIT);
  // Store them in output arguments
  beta1 = boost::any_cast<double>( anyBeta1 );
  beta2 = boost::any_cast<double>( anyBeta2 );
  Ptolerance = boost::any_cast<double>( anyPtolerance );
  Qtolerance = boost::any_cast<double>( anyQtolerance );
  maxIterCount = boost::any_cast<int>( anyMaxIterCount );
}
