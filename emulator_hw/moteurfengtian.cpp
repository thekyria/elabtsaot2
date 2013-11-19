
#include "moteurfengtian.h"
using namespace elabtsaot;

//using namespace ublas;

#include "emulator.h"
#include "encoder.h"
#include "precisiontimer.h"
#include "auxiliary.h"

#include <boost/timer/timer.hpp>

#include <iostream>
using std::cout;
using std::endl;
using std::vector;
using std::polar;

enum MoteurFengtianProperties{
  MFT_PROPERTY_BETA1,
  MFT_PROPERTY_BETA2,
  MFT_PROPERTY_PTOL,
  MFT_PROPERTY_QTOL,
  MFT_PROPERTY_MAXIT,
  MFT_PROPERTY_METHOD
};

enum MoteurFengtianMethod{
  MFT_METHOD_GPF = 0, // Guillaume Power Flow
  MFT_METHOD_DC = 1
};

#define GPF_ADDR_BETA 604
#define GPF_ADDR_START 605
#define GPF_ADDR_STOP 606
#define GPF_ADDR_MAXITER 607
#define GPF_ADDR_NIOSSAMPLE 608
#define GPF_ADDR_TOL 609
#define GPF_ADDR_ITERCOUNT 610
#define GPF_ADDR_RESULTS 611

#define GPF_WRCODE_START 1111
#define GPF_WRCODE_RESET 2222
#define GPF_WRCODE_HIZ   6666

#define GPF_RDCODE_CONVERGED 9999
#define GPF_RDCODE_NOTCONVERGED 6666
#define GPF_RDCODE_ISRESET 3333

#define DEFAULT_TIMEOUT 5.0

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

  tempPt.key = MFT_PROPERTY_METHOD;
  tempPt.dataType = PROPERTYT_DTYPE_INT;
  tempPt.name = "Method";
  tempPt.description = "0: Guillaume's Power Flow; 1: DC Power Flow";
  tempPt.defaultValue = static_cast<int>(MFT_METHOD_GPF);
  tempPt.minValue = static_cast<int>(MFT_METHOD_GPF);
  tempPt.maxValue = static_cast<int>(MFT_METHOD_DC);
  _properties[tempPt] = tempPt.defaultValue;
}

int MoteurFengtian::do_solvePowerFlow(Powersystem const& pws, ublas::vector<complex>& V) const{

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
     maxIterCount:
     method      : */
  double beta1;
  double beta2;
  double Ptolerance;
  double Qtolerance;
  size_t maxIterCount;
  int method;
  _getOptions(beta1, beta2, Ptolerance, Qtolerance, maxIterCount, method);

  // Initialize output voltages
  bool converged(false);
  size_t busCount = pws.getBusCount();
  V.resize(busCount);
  for (size_t k(0); k!=busCount; ++k){
    Bus const* bus = pws.getBus(k);
    V(k) = polar(bus->V,bus->theta);
  }

  switch (method){
  /*****************************************************************************
   * GUILLAUME (LANZ) POWER FLOW METHOD
   ****************************************************************************/
  case MFT_METHOD_GPF:{
    cout << "MoteurFengtian using GPF" << endl;
    // Map & fit & encode pws & end calib. mode
    int ans = _emu->preconditionEmulator(EMU_OPTYPE_GPF);
    if (ans) return ans;

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
      _emu->encoding[k][GPF_ADDR_BETA-1]    = tempBeta;
      _emu->encoding[k][GPF_ADDR_MAXITER-1] = tempMaxIter;
      _emu->encoding[k][GPF_ADDR_TOL-1]     = tempTol;
    }

    // Finally write the modified _emu->encoding vector to the devices
    cout << "Writing encoding ..." << endl;
    ans = _emu->writeEncoding(false);
    if (ans) return 42;

    size_t sliceCount = _emu->getHwSliceCount();
    vector<int> devId(sliceCount);
    for(size_t k(0); k!=_emu->getHwSliceCount();++k)
      devId[k] = _emu->sliceDeviceMap(k);
    // Determine the start codes
    vector<unsigned int> _NIOSStartCodes(sliceCount,0); // start code for each emulator slice
    for (size_t sliceId_(0); sliceId_!=sliceCount; ++sliceId_){
      Slice* sl = &_emu->emuhw()->sliceSet[sliceId_];
      if (   sl->dig.pipe_PFPQ.element_count()   ==0
          && sl->dig.pipe_PFslack.element_count()==0)  // if all pipelines empty
        _NIOSStartCodes[sliceId_] = GPF_WRCODE_HIZ; // then start code=6666 (==slice in HiZ)
      else
        _NIOSStartCodes[sliceId_] = GPF_WRCODE_START; // if not empty normal start
    }

    // Write the start codes
    cout << "Writing start codes ..." << endl;
    size_t address(GPF_ADDR_START);
    vector<uint32_t> encoding605(1);
    for (size_t sliceId_(0); sliceId_!=sliceCount; ++sliceId_){
      cout << "slice: " << sliceId_ << " start_code: " << _NIOSStartCodes[sliceId_] << endl;
      encoding605[0] = _NIOSStartCodes[sliceId_];
      ans = _emu->usbWrite(devId[sliceId_], address, encoding605);
    }
    if (ans) return 43;

    // Check convergeance (read stop code)
    cout << "Polling convergence flag ... ";
    ans = _waitForGPFConvergence(10.0, converged);
    if (ans) return 44;
    if (converged)
      cout << "converged! (" << GPF_RDCODE_CONVERGED << ")" << endl;
    else
      cout << "not converged! (" << GPF_RDCODE_NOTCONVERGED << ")" << endl;

    vector<uint32_t> readBuffer;
    // Read number of iterations by the algorithm
    address = GPF_ADDR_ITERCOUNT;
    ans = _emu->usbRead(devId[0], address, 1, readBuffer);
    if (ans) return 45;
    size_t iterCount = static_cast<size_t>(readBuffer[0]);
    cout << "Iteration count (address " << GPF_ADDR_ITERCOUNT << "): " << iterCount << endl;

    // Retrieve the mapping of the buses
    vector<MappingPosition> busMap(busCount);
    for (size_t k(0); k!=busCount; ++k){
      Bus const* bus = pws.getBus(k);
      MappingPosition tempMapPos; size_t tempEmbrPos; // tempEmbrPos redundant (pointer not used)
      _emu->mmd()->getElementMapping(PWSMODELELEMENTTYPE_BUS, bus->extId, &tempMapPos, &tempEmbrPos);
      busMap[k] = tempMapPos;
    }

    // Read and actual voltage results & update results
    cout << "Reading results ... " << endl;
    address = GPF_ADDR_RESULTS;
    for (size_t sliceId_(0); sliceId_!=sliceCount; ++sliceId_){
      using auxiliary::operator <<;

      // Read results for sliceId_
      ans = _emu->usbRead(devId[sliceId_], address, 24, readBuffer);
      cout << "readBuffer: " << readBuffer << endl;
      // Parse them for sliceId_
      vector<complex> out;
      _parseVoltage(readBuffer,out);
      cout << "readBuffer (parsed): " << out << endl;

      // Check which buses will have results on this slice
      PQPipeline* pipe = &_emu->emuhw()->sliceSet[sliceId_].dig.pipe_PFPQ;
      for (size_t k(0); k!=busMap.size(); ++k){
        MappingPosition tempMapPos = busMap[k];
        if (tempMapPos.tab == static_cast<int>(sliceId_)){
          // For these results, get index in the PQPipeline of the slice
          int tempRow = tempMapPos.row;
          int tempCol = tempMapPos.col;
          int tempIndex = pipe->search_element(tempRow,tempCol);
          // If found in the pipeline the respective V result is updated
          if (tempIndex >= 0)
            V(k) = out[tempIndex];
        }
      }
    }

    // Reset the board after power flow is complete
    cout << "Reseting slices ... ";
    address = GPF_ADDR_START;
    encoding605[0] = GPF_WRCODE_RESET;
    for (size_t sliceId_(0); sliceId_!=sliceCount; ++sliceId_){
      cout << " " << sliceId_;
      ans = _emu->usbWrite(sliceId_, address, encoding605);
    }
    if (ans) return 46;
    cout << " done!" << endl;

  break;}

  /*****************************************************************************
   * DC POWER FLOW METHOD
   ****************************************************************************/
  case MFT_METHOD_DC:{
    cout << "MoteurFengtian using DC PF" << endl;

    // TODO!

  break;}
  }

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
  boost::any anyMethod = _getPropertyValueFromKey(MFT_PROPERTY_METHOD);
  // Store them in output arguments
  beta1 = boost::any_cast<double>( anyBeta1 );
  beta2 = boost::any_cast<double>( anyBeta2 );
  Ptolerance = boost::any_cast<double>( anyPtolerance );
  Qtolerance = boost::any_cast<double>( anyQtolerance );
  maxIterCount = boost::any_cast<int>( anyMaxIterCount );
  method = boost::any_cast<int>( anyMethod );
}

int MoteurFengtian::_waitForGPFConvergence(double timeout_, bool& converged) const{
  // If timeout negative then default to 5 seconds
  double timeout=(timeout_>0)?timeout_:DEFAULT_TIMEOUT;

  // Retrieve device ids for slices
  size_t sliceCount = _emu->getHwSliceCount();
  vector<int> devId(sliceCount);
  for(size_t k(0); k!=_emu->getHwSliceCount();++k)
    devId[k] = _emu->sliceDeviceMap(k);

  // Set a time threshold of 'timeout' seconds for the operation
  double elapsedTime = 0;
  PrecisionTimer timer; // counts in seconds
  timer.Start();

  vector<uint32_t> readBuffer;
  while (elapsedTime<timeout){
    int ans = _emu->usbRead(devId[0], GPF_ADDR_STOP, 1, readBuffer);
    if (ans) return 2;
    if (readBuffer[0]==GPF_RDCODE_CONVERGED){
      converged = true;
      return 0;
    }
    if (readBuffer[0]==GPF_RDCODE_NOTCONVERGED){
      converged = false;
      return 0;
    }
    // Update timer
    elapsedTime += timer.Stop();
    timer.Start();
  }
//  converged = false;
  return 1;
}

void MoteurFengtian::_parseVoltage(vector<uint32_t> const& val, vector<complex>& out) const{
  out.resize(val.size(),complex(0.0,0.0));
  // Vector val contains signed 0x0000Q2.14 format numbers
  unsigned int bit16 = (1<<15);
  unsigned int signExpansionL = ~( (1<<16) - 1 );
  for ( size_t k = 0 ; k != val.size() ; ++k ){
    int32_t LSB = static_cast<int32_t>( val[k]&0x0000FFFF );
    int32_t MSB = static_cast<int32_t>( (val[k]&0xFFFF0000)>>16 );
    if (LSB&bit16) // get sign
      LSB |= signExpansionL;
    if (MSB&bit16) // get sign
      MSB |= signExpansionL;

    double temp_real = static_cast<double>(LSB) / static_cast<double>(1<<14);
    double temp_imag = static_cast<double>(MSB) / static_cast<double>(1<<14);
    out[k] = complex(temp_real,temp_imag);
  }
}
