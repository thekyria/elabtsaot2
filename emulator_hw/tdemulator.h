/*!
\file tdemulator.h

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef TDEMULATOR_H
#define TDEMULATOR_H

#include "tdengine.h"

#include <stdint.h> // for uintX_t
#include <stdlib.h> // for size_t
#include <vector>
#include <map>
#include <set>

namespace elabtsaot{

class Logger;
class Emulator;
class TDResultIdentifier;

class TDEmulator : public TDEngine {

 public:

  TDEmulator(Powersystem const* pws, Emulator* emu, Logger* log);

  // Run simulations
  int resetEmulation( bool waitForAck = false );

 private:

  // Implementation of virtual parent functions
  bool do_isEngineCompatible(Scenario const& sce) const;
  int do_setTimeStep(double seconds, double* pMismatch = NULL );
  int do_simulate( Scenario const& sce, TDResults& res );
  int do_checkStability( std::vector<Scenario> const& scenarios,
       std::vector<std::map<unsigned int,bool> >& genStable,
       std::vector<std::map<unsigned int,double> >* genInstabilityTime = 0);
  int do_checkCCT( std::vector<Scenario> const& scenarios, double precision,
                   std::vector<double>& cctMin, std::vector<double>& cctMax );

  // Property functions
  void updateProperties(std::list<property> const& properties);
  int updateProperty(property const& property_);
  int _getIntTStepOption() const;
  int _getNIOSSampleRate() const;
  int _getDampIt() const;
  int _getTrigDelay() const;
  int _setIntTStepOption(int val);
  int _setNIOSSampleRate(int val);
  int _setDampIt(int val);
  int _setTrigDelay(int val);
  void _updateTimeStep();

  // Encoding functions
  void _encodeTimeOptions();
  int _encodeScenarioBase( Scenario const& sce,
                           size_t& sliceId,
                           unsigned int& stopStartIter,
                           unsigned int& dampIter,
                           unsigned int& switchConf,
                           unsigned int& stopSample,
                           bool& refittingPerformed );
  int _encodeScenario(Scenario sce);
  void _resetDampingEncoding();
  void _resetSpiesEncoding();
  int _encodeScenarioCCT( Scenario sce,
                          double maxFaultTime,
                          double precision,
                          size_t& sceSliceId );
  /*! \param[out] sceRunCount scenarios that were actually encoded (the other
                  were deemed as incompatible with this kind of analysis)
      \param[out] map containing the index mapping between the actual scenario
                  encoding and the scenarios input argument */
  int _encodeMultiScenarios( std::vector<Scenario> scenarios,
                             size_t& sceRunCount,
                             std::map<size_t,size_t>& newSceIndex );
  int _encodeMultiCCT( std::vector<Scenario> const& scenarios,
                       double precision,
                       std::map<size_t,size_t>& newSceIndex,
                       size_t& sceRunCount,
                       std::vector<size_t>& scePerSlice,
                       double& maxFault, double& stopTime );

  int _getStartCode( TDResultIdentifier const& tdri,
                     size_t& sliceId, unsigned int& startCode ) const;
  int _getSpyCode( TDResultIdentifier const& tdri,
                   unsigned int& spyCode ) const;

  // Waiting functions
  int _waitForStopCodes( std::vector<int> const& stopCodes, double timeout );
  int _waitForResults( std::vector<uint32_t> const& runCodes, double timeout );
  int _waitForSpy( size_t sliceId, uint32_t spyCode, double timeout );

  // Parsing functions
  void _parseGenAngles( std::vector<uint32_t> const& resultsRead,
                        std::vector<double>& ys, bool unfold = false );
  void _parseGenSpeeds( std::vector<uint32_t> const& resultsRead,
                        std::vector<double>& ys );
  void _parseGenPmec( std::vector<uint32_t> const& resultsRead,
                      std::vector<double>& ys );
  void _parseVoltage( std::vector<uint32_t> const& resultsRead,
                      std::vector<double>& ys1, std::vector<double>& ys2 );
  void _parseVoltage( std::vector<uint32_t> const& resultsRead,
                      bool realPart, std::vector<double>& ys);
  void _parseCurrent( std::vector<uint32_t> const& resultsRead,
                      std::vector<double>& ys1, std::vector<double>& ys2 );
  void _parseCurrent( std::vector<uint32_t> const& resultsRead,
                      bool realPart, std::vector<double>& ys );

  // ---------- Variables ----------
  Emulator* _emu;
  std::set<std::pair<int,int> > _supportedResults; // std::set<std::pair<TDResultsElement,TDResultsVariable> >
  std::vector<double> _int_tstep_values; //!< Time step values for the FPGA integrators

};

struct TDResultsRetrievalInfo{
  TDResultsRetrievalInfo(){}
  TDResultsRetrievalInfo( size_t sliceId_, size_t runId_ )
    { sliceId = sliceId_; runId = runId_; }
  size_t sliceId;
  size_t runId;
};

} // end of namespace elabtsaot

#endif // TDEMULATOR_H
