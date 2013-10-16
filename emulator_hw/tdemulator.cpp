
#include "tdemulator.h"
using namespace elabtsaot;

#include "powersystem.h"
#include "emulator.h"
#include "scenario.h"
#include "auxiliary.h"
#include "precisiontimer.h"
#include "encoder.h"
#include "tdresults.h"

//#include <vector>
using std::vector;
//#include <map>
using std::map;
using std::pair;
//#include <set>
using std::set;
#include <iostream>
using std::cout;
using std::endl;
#include <limits>
using std::numeric_limits;
#include <utility>
using std::make_pair;
#include <list>
using std::list;
#include <complex>
using std::complex;
#include <bitset>
using std::bitset;

#define DBL_EPSILON 0.0000001
#define DOUBLE_EQ(x,v) (((v - DBL_EPSILON) < x) && (x <( v + DBL_EPSILON)))

static const double INT_TSTEP_VALUES[] = { 0.000061035,
                                           0.000122070,
                                           0.000244140,
                                           0.000488281,
                                           0.000976562,
                                           0.001953124,
                                           0.003906248,
                                           0.007812496 };

//! See TDEmulator constructor for details on properties
enum TDEmulatorProperties{
  TDEHWE_PROPERTY_INTOPT,
  TDEHWE_PROPERTY_NIOSS,
  TDEHWE_PROPERTY_DAMPIT,
  TDEHWE_PROPERTY_TRIGD
};

TDEmulator::TDEmulator(Powersystem const* pws, Emulator* emu, Logger* log) :
  TDEngine(pws, "Hardware simulator", 0.0, log),
  _emu(emu),
  _int_tstep_values( INT_TSTEP_VALUES,
                     INT_TSTEP_VALUES
                     + sizeof(INT_TSTEP_VALUES)/sizeof(INT_TSTEP_VALUES[0]) ){

  // Initialize set of supported results
//  _supportedResults.clear();
  _supportedResults.insert( make_pair(TDRESULTS_OTHER, TDRESULTS_TIME) );
  _supportedResults.insert( make_pair(TDRESULTS_GENERATOR, TDRESULTS_ANGLE) );
  _supportedResults.insert( make_pair(TDRESULTS_GENERATOR, TDRESULTS_SPEED) );
  _supportedResults.insert( make_pair(TDRESULTS_GENERATOR, TDRESULTS_ACCELERATING_POWER) );
  _supportedResults.insert( make_pair(TDRESULTS_GENERATOR, TDRESULTS_VOLTAGE_REAL) );
  _supportedResults.insert( make_pair(TDRESULTS_GENERATOR, TDRESULTS_VOLTAGE_IMAG) );
  _supportedResults.insert( make_pair(TDRESULTS_GENERATOR, TDRESULTS_CURRENT_REAL) );
  _supportedResults.insert( make_pair(TDRESULTS_GENERATOR, TDRESULTS_CURRENT_IMAG) );
  _supportedResults.insert( make_pair(TDRESULTS_BUS, TDRESULTS_VOLTAGE_REAL) );
  _supportedResults.insert( make_pair(TDRESULTS_BUS, TDRESULTS_VOLTAGE_IMAG) );
//  _supportedResults.insert( make_pair(TDRESULTS_BUS, TDRESULTS_CURRENT_REAL) );
//  _supportedResults.insert( make_pair(TDRESULTS_BUS, TDRESULTS_CURRENT_IMAG) );
  _supportedResults.insert( make_pair(TDRESULTS_LOAD, TDRESULTS_VOLTAGE_REAL) );
  _supportedResults.insert( make_pair(TDRESULTS_LOAD, TDRESULTS_VOLTAGE_IMAG) );
//  _supportedResults.insert( make_pair(TDRESULTS_LOAD, TDRESULTS_CURRENT_REAL) );
//  _supportedResults.insert( make_pair(TDRESULTS_LOAD, TDRESULTS_CURRENT_IMAG) );

  // ----- Prepare properties -----
  property_type tempPt;
  tempPt.key = TDEHWE_PROPERTY_INTOPT;
  tempPt.dataType = PROPERTYT_DTYPE_INT;
  tempPt.name = "Integrator time step option";
  tempPt.description = "Time step that the internal FPGA integrator considers";
  tempPt.defaultValue = 0;
  tempPt.minValue = 0;
  tempPt.maxValue = static_cast<int>(_int_tstep_values.size()-1);
  _properties[tempPt] = tempPt.defaultValue;

  tempPt.key = TDEHWE_PROPERTY_NIOSS;
  tempPt.dataType = PROPERTYT_DTYPE_INT;
  tempPt.name = "NIOS sample rate";
  tempPt.description = "NIOS samples results generated in the pipelines with this rate";
  tempPt.defaultValue = 50;
  tempPt.minValue = 1;
  tempPt.maxValue = 100;
  _properties[tempPt] = tempPt.defaultValue;

  tempPt.key = TDEHWE_PROPERTY_DAMPIT;
  tempPt.dataType = PROPERTYT_DTYPE_INT;
  tempPt.name = "Damping iteration option";
  tempPt.description = "-1: damped till begging of scenario; 0: no damping; else: manual setting";
  tempPt.defaultValue = -1;
  tempPt.minValue = -1;
  tempPt.maxValue = 100000;
  _properties[tempPt] = tempPt.defaultValue;
  /* If -1 the system is damped until the beginning of the first event in the
  scenario; this was found to perform better, because it allows for the analog
  hw grid to settle down to its steady state point; before the beginning of the
  dynamic phenomena */

  tempPt.key = TDEHWE_PROPERTY_TRIGD;
  tempPt.dataType = PROPERTYT_DTYPE_INT;
  tempPt.name = "Cycle trigger delay";
  tempPt.description = "Number of FPGA clock cycles between two computation iterations; -1: auto; 0+: manual";
  tempPt.defaultValue = -1;
  tempPt.minValue = -1;
  tempPt.maxValue = numeric_limits<int>::max();
  _properties[tempPt] = tempPt.defaultValue;

  // and update the time step
  _updateTimeStep();
}

int TDEmulator::resetEmulation( bool waitForAck ){

  PrecisionTimer timer; // counts in seconds
  timer.Start();

  // Write reset code (2222) to all devices
  for ( size_t k = 0 ; k != _emu->getUSBDevicesCount() ; ++k ){
    vector<uint32_t> resetData;
    resetData.clear();
    resetData.push_back( static_cast<uint32_t>(2222) );
//    resetData.push_back( static_cast<uint32_t>(0) );
//    resetData.push_back( static_cast<uint32_t>(0) );
//    resetData.push_back( static_cast<uint32_t>(0) );
    int ans = _emu->usbWrite( k, 556, resetData );
    if ( ans ) return k;

    cout << "TDEmulator::resetEmulation(): ";
    cout << "write reset command to dev" << k << " " << timer.Stop() << " s" << endl;
    timer.Start();
  }

  // Check for stop code 3333 denoting that the reset was successfully performed
  if ( waitForAck ){
    vector<int> stopCodes( _emu->getUSBDevicesCount(), 3333 );
    int ans =_waitForStopCodes( stopCodes, 5.0 );
    cout << "TDEmulator::resetEmulation(): ";
    cout << "wait for reset ack " << timer.Stop() << " s" << endl;
    timer.Start();
    return ans;
  }

  return 0;
}

bool TDEmulator::do_isEngineCompatible(Scenario const& sce_) const{

  Scenario sce(sce_); // copy-construct local Scenario object for non-const
                      // operations
  size_t k; // counter

  // First the scenario has to be compatible with the powersystem
  if ( !sce.checkPwsCompatibility( *_pws ) )
    return false;

  // For the checks hereunder a reduced (minimal) version of each scenario is
  // supposed (no duplicate events, no invalid events, no events with no
  // effect, etc).
  sce.reduce(); // Removes duplicates etc.

  // Also the events in the scenario are required to be in increasing time order
  sce.sort_t(); // Sort events in increasing time

  vector<Event> events = sce.getEventSet();
  // For the moment the only scenarios that are compatible with the hardware
  // emulator are scenarios that can be translated to turning on-off a single
  // switch of the emulator.
  for ( k = 0 ; k != events.size() ; ++k ){
    if ( !events[k].status() ) continue;
    if (    events[k].element_type() == EVENT_ELEMENT_TYPE_GEN    // corresponding to gen events
         || events[k].element_type() == EVENT_ELEMENT_TYPE_LOAD ) // corresponding to load events
      // Gen & load events are not supported
      return false;
    if ( events[k].element_type() == EVENT_ELEMENT_TYPE_BUS )    // corresponding to bus events
      if ( events[k].event_type() == EVENT_EVENTTYPE_BUSFAULT    // 3ph fault
           && events[k].bool_arg()                 //           ON
           && ( events[k].double_arg_1()!=0        // 3ph fault rsh to gnd
                || events[k].double_arg_2()!=0) )  // 3ph fault xsh to gnd
        // Only perfect (rsh=0 & xsh=0) 3ph faults are supported
        // Note: double_arg_1 & _2 are disregarded for 3ph fault OFF events
        return false;
    if ( events[k].element_type() == EVENT_ELEMENT_TYPE_BRANCH ) // corresponding to branch events
      if ( events[k].event_type() == EVENT_EVENTTYPE_BRFAULT     // 3ph fault
           && events[k].bool_arg()                 //           ON
           && ( events[k].double_arg_1()!=0        // 3ph fault rsh to gnd
                || events[k].double_arg_2()!=0) )  // 3ph fault xsh to gnd
        // Only perfect (rsh=0 & xsh=0) 3ph faults are supported
        // Note: double_arg_1 & _2 are disregarded for 3ph fault OFF events
        return false;
  }
  // So far it has been made sure that only bus and branch events are involved
  // in the scenario, and that all 3ph faults are perfect

  // Bus 3ph faults can be translated to branch 3ph faults at an outbound branch
  // of the faulted bus at a location very close to that bus ( fault_location ~
  // 0% == fault almost at from bus )
  // However events are supported only at 1 branch per scenario time
  // (respectively translates to 1 bus per scenario).
  int element_type = -1;
  int element_extId = -1;
  for ( k = 0 ; k != events.size() ; ++k ){
    if ( !events[k].status() ) continue;
    // Determine the type (bus=0 or branch=1) and the extId of the faulted elem.
    if ( element_type == -1 && element_extId == -1 ){
      element_type = events[k].element_type();
      element_extId = events[k].element_extId();
      continue;
    }
    // Check if an event refers to an element not identical to the original
    // faulted element
    if(    events[k].element_type() != static_cast<unsigned int>(element_type)
        || events[k].element_extId()!= static_cast<unsigned int>(element_extId))
      return false;
  }
  // So far it hase been made sure that all events in the scenario refer to the
  // same powersystem element

  // There is a hardware constraint that all the events should be translated to
  // changes on the same switch (turning on or off). The only exception is that
  // branch events can always be accompanied by a branch trip.
  int event_type = -1;
  for ( k = 0 ; k != events.size() ; ++k ){
    if ( !events[k].status() ) continue;
    // Determine the event type
    if ( event_type == -1 ){
      event_type = events[k].event_type();
      continue;
    }
    // Check if all the events are of the same type (or extra trip events)
    if (    events[k].event_type() != static_cast<unsigned int>(event_type)  // event type is not the same
         && !(   element_type == EVENT_ELEMENT_TYPE_BRANCH
              && events[k].event_type() == EVENT_EVENTTYPE_BRTRIP) ) // and not a branch trip
      return false;
  }
  // So far it have been made sure that all events are either
  // bus    - 3ph faults ,  or
  // branch - 3ph faults (with/without trips)
  //        - trips
  //        - short-circuits (with/without trips)

  // In any case there can be only one ON event of a type
  size_t bus_3phFault_count = 0;
  size_t branch_3phFault_count = 0;
  size_t branch_trip_count = 0;
  size_t branch_short_count = 0;
  for ( k = 0 ; k != events.size() ; ++k ){
    if ( !events[k].status() ) continue;

    // Count the bus 3ph faults
    if ( element_type == EVENT_ELEMENT_TYPE_BUS ) // faulted element is a bus
      if (    events[k].event_type() == EVENT_EVENTTYPE_BUSFAULT
           && events[k].bool_arg() )
        if ( ++bus_3phFault_count > 1 )
          return false;

    // Count branch faults -
    if ( element_type == EVENT_ELEMENT_TYPE_BRANCH ){
      // - 3ph faults (on)
      if (    events[k].event_type() == EVENT_EVENTTYPE_BRFAULT
           && events[k].bool_arg() ){
        if ( ++branch_3phFault_count > 1 )
          return false;
      }
      // - trips (on)
      else if (    events[k].event_type() == EVENT_EVENTTYPE_BRTRIP
           && events[k].bool_arg() ){
        if ( ++branch_trip_count > 1 )
          return false;
      }
      // - short-circuits (on)
      else if (    events[k].event_type() == EVENT_EVENTTYPE_BRSHORT
           && events[k].bool_arg() ){
        if ( ++branch_short_count > 1 )
          return false;
      }
    }
  }
  // So far it have been made sure that there is a maximum of 1 ON event of the
  // accepted event types

  // A more detailed look on actual allowed events is as follows:
  // i.e. there can be:
  // * at most one "bus-3ph_fault-ON"
  // * at most one "branch-3ph_fault-ON" with/without a "branch-trip-ON"
  //   concurrent to the respective "branch-3ph_fault-OFF" event
  // * at most one "branch-trip-ON"
  // * at most one "branch-short-ON" with/without "branch-trip-ON"
  //   concurrent to the respective "branch-short-ON" event
  if ( branch_3phFault_count > 0 && branch_short_count > 0 )
    return false; // a 3ph fault can only coexist with a branch-trip event
  // Note: there is no need to check for the coexistance of bus with branch
  // events as this has been ensured by checks above (element_type & -_extId)
  // So far it has been ensured that a qualitatively acceptable combination of
  // ON events exist.

  // If there is a "branch-3ph_fault-ON" with a "branch-trip-ON" or a
  // "branch-short-ON" with "branch-trip-ON", the latter in both cases has to be
  // concurrent to the respective fault ON event.
  // In any case the fault OFF event has to be only one
  size_t branch_3phFault_off_count = 0;
  size_t branch_short_off_count = 0;
  double fault_off_time = -1;
  double trip_on_time = -1;
  for ( k = 0 ; k != events.size() ; ++k ){
    if ( !events[k].status() ) continue;

    // Count branch faults -
    if ( element_type == EVENT_ELEMENT_TYPE_BRANCH ){
      // - 3ph faults (off)
      if (    events[k].event_type() == EVENT_EVENTTYPE_BRFAULT
           && !events[k].bool_arg() ){
        if ( fault_off_time == -1 )
          fault_off_time = events[k].time();
        if ( ++branch_3phFault_off_count > 1 )
          return false;
      }
      // - short-circuits (off)
      else if (    events[k].event_type() == EVENT_EVENTTYPE_BRSHORT
                && !events[k].bool_arg() ){
        if ( fault_off_time == -1 )
          fault_off_time = events[k].time();
        if ( ++branch_short_off_count > 1 )
          return false;
      }
      // - trips (on)
      else if (    events[k].event_type() == EVENT_EVENTTYPE_BRTRIP
                && events[k].bool_arg() )
        if ( trip_on_time == -1 )
          trip_on_time = events[k].time();
    }
  }
  if ( branch_3phFault_count == 1 && branch_trip_count == 1 ){
    if ( branch_3phFault_off_count != 1 )
      return false;
    if ( !DOUBLE_EQ(fault_off_time,trip_on_time) )
      return false;
  }
  if ( branch_short_count == 1 && branch_trip_count == 1 ){
    if ( branch_short_off_count != 1 )
      return false;
    if ( !DOUBLE_EQ(fault_off_time,trip_on_time) )
      return false;
  }

  // The scenario has passed all checks so it should be valid with hardware
  // emulator
  return true;
}

int TDEmulator::do_setTimeStep( double seconds, double* pMismatch ){

  // Determine the greatest intTStepOption that is less than seconds
  int k;
  for (k=0; k!=static_cast<int>(_int_tstep_values.size()); ++k)
    if (_int_tstep_values[k]>seconds) break;
  if (--k<0) return 1; // invalid time step requested

  int intTStepOption = k;
  int ans = _setIntTStepOption(intTStepOption); // actually register intTStepOption
  if ( ans )
    return 2;

  // Calculate NIOS sample rate to minimize mismatch between seconds and _timeStep
  double intTimeStepValue = _int_tstep_values[intTStepOption];
  int NIOSSampleRate = static_cast<int>( auxiliary::round(seconds/intTimeStepValue) );
  ans = _setNIOSSampleRate(NIOSSampleRate); // actually register NIOSSampleRate

  // Update _timeStep
  _timeStep = intTimeStepValue * static_cast<double>(NIOSSampleRate);

  // Update mismatch output argument and return
  if ( pMismatch != NULL )
    *pMismatch = seconds-_timeStep;
  return 0;
}

#ifdef BUILD_NOSPIES

// OLD (no spies ver)
int TDEmulator::do_simulate( Scenario const& sce, TDResults& res ){

  PrecisionTimer timer; // counts in seconds
  timer.Start();
  // --------------- Map & fit & encode pws & end calib. mode ---------------
  int ans = _emu->preconditionEmulator();
  if ( ans ) return ans;
  // _emu->encoding has been initialized to a size: [slicecount]x[560]
  cout << "TDEmulator::do_simulate(): ";
  cout << "preconditioning proper " << timer.Stop() << " s" << endl;
  timer.Start();

  // --------------- Write encoding ---------------
  // ---- Set the time options : Determine the start and stop reading samples
  // according to the timeStep and the start/stop time requested by the scenario
  /* Modifies:
  _emu->encoding[*][381] (wr @cyp.add 382): _NIOSSampleRate + interval betw. 2 starts
  _emu->encoding[*][558] (wr @cyp.add 559): _NIOSSampleRate
  So it must be run after encodePowersystem() */
  _encodeTimeOptions();
  // Determine start & stop sample (for later, results retrieval use)
  unsigned int startSample, stopSample;
  startSample = static_cast<unsigned int>( ceil(sce.startTime()/_timeStep) );
  stopSample  = static_cast<unsigned int>( floor(sce.stopTime()/_timeStep) );
  cout << "TDEmulator::do_simulate(): ";
  cout << "encode time options " << timer.Stop() << " s" << endl;
  timer.Start();

  // Encode the scenario

  /* Modifies:
  For slice k containing the perturbation:
    _emu->encoding[k][551] (cyp.add 552) = pertStopStart
    _emu->encoding[k][552] (cyp.add 553) = not used
    _emu->encoding[k][553] (cyp.add 554) = switchConf
  For all slices:
    _emu->encoding[*][554] (cyp.add 555) = NIOSconfirm + stopSample
    _emu->encoding[*][559] (cyp.add 560) = pertStopStart
  So it must be run after encodePowersystem() */
  ans = _encodeScenario( sce );
  if ( ans ) return 41;
  cout << "TDEmulator::do_simulate(): ";
  cout << "encode scenario " << timer.Stop() << " s" << endl;
  timer.Start();

  // Finally write the modified _emu->encoding vector to the devices
  ans = _writeEncoding( false );
  if ( ans ) return 42;
  cout << "TDEmulator::do_simulate(): ";
  cout << "write encoding " << timer.Stop() << " s" << endl;
  timer.Start();

  // --------------- Verify that results requested can be provided -------------
  vector<double> resultsData; // temp results data vector
  vector<TDResultIdentifier> resultsIdentifiers = res.identifiers();
  vector<bool> resultsRequested = res.dataExists();
  vector<bool> resultsReady( resultsRequested.size(), false );
  for ( size_t k = 0 ; k != res.dataExists().size() ; ++k ){
    // Check whether the results are requested
    if( !resultsRequested[k] )
      continue;

    // Check whether the requested results are supported by the emulator
    if( _supportedResults.count(make_pair(resultsIdentifiers[k].elementType,
                                          resultsIdentifiers[k].variable))==0 ){
      // Mark result as non available and continue
      resultsRequested[k] = false;
      continue;
    }

    // For bus voltages see whether there is a generator or a load mapped onto
    // them, otherwise the bus voltage cannot be retrieved (ie. of fork buses)
    if (    resultsIdentifiers[k].elementType == TDRESULTS_BUS
         && (    resultsIdentifiers[k].variable == TDRESULTS_VOLTAGE_REAL
              || resultsIdentifiers[k].variable == TDRESULTS_VOLTAGE_IMAG ) ){
      unsigned int busExtId = resultsIdentifiers[k].extId;
      int busIntId = _pws->getBus_intId( busExtId );
      if ( busIntId < 0){
        // Bus does not exist in the bus set! Should never happen
        // Mark results as non available and continue
        resultsRequested[k] = false;
        continue;
      }
      bool genMapped = false;
      bool loadMapped = false;
      if ( _pws->busGenMap().at( busIntId ).size() == 1 ){
        genMapped = true;
      } else if ( _pws->busLoadMap().at( busIntId ).size() == 1 ){
        loadMapped = true;
      }
      if ( !genMapped && !loadMapped ){
        // No observable (pipeline) element is mapped onto the node
        // Mark results as non available and continue
        resultsRequested[k] = false;
        continue;
      }
    }

    // ----- Provide results that do not need emulation runs -----
    // Results requested == time: No emulation required for time
    if (    resultsIdentifiers[k].elementType == TDRESULTS_OTHER
         && resultsIdentifiers[k].variable == TDRESULTS_TIME ){
      resultsData.clear();
      for ( unsigned int m = startSample ; m != stopSample ; ++m )
        resultsData.push_back( m*_timeStep);
      res.set_data( k, resultsData );
      resultsReady[k] = true;
      continue;
    }

    // Check for generator particularities: if the mechanical starting time of
    // a generator is greater than GEN_MECHSTARTTIME_THRESHOLD, then instead of
    // the generator pipeline of the correspoding slice, it is placed in the
    // const I load pipeline; for those gens a special treatment for results is
    // required
    if ( resultsIdentifiers[k].elementType == TDRESULTS_GENERATOR ){

      Generator const* pGen;
      ans = _pws->getGenerator( resultsIdentifiers[k].extId, pGen );
      if ( pGen->M() < GEN_MECHSTARTTIME_THRESHOLD )
        // Gen is a 'normal' gen, treated in the gen pipeline
        continue;

      // else { // The gen is treated in the const I pipeline
      resultsData.clear();

      // Compute the current
      complex<double> S( pGen->pgen(), pGen->qgen() );
      complex<double> I = conj(S) / conj(pGen->Uss());
      PwsMapperModelElement const*
        el = _emu->mmd()->element( PWSMODELELEMENTTYPE_GEN, pGen->extId() );
      int tab = floor( static_cast<double>(el->tab)/2.0 );
      int row = floor( static_cast<double>(el->row)/2.0 );
      int col = floor( static_cast<double>(el->col)/2.0 );
      int pipelineId = _emu->emuhw()->sliceSet[tab].dig.pipe_iload.search_element(row,col);
      if ( pipelineId < 0 ) return 11;

      switch ( resultsIdentifiers[k].variable ){

      case TDRESULTS_ANGLE:
        resultsData.resize( stopSample-startSample, pGen->deltass() );
        break;

      case TDRESULTS_SPEED:
        resultsData.resize( stopSample-startSample, 0 ); // 0 == omegass
        break;

      case TDRESULTS_CURRENT_REAL:
        resultsData.resize( stopSample-startSample, I.real() );
        break;

      case TDRESULTS_CURRENT_IMAG:
        resultsData.resize( stopSample-startSample, I.imag() );
        break;

      case TDRESULTS_ACCELERATING_POWER:
        resultsRequested[k] = false; // Pacc data cannot be provided for gens in
                                     // the const I load pipeline
      case TDRESULTS_VOLTAGE_REAL: // handled by special logic in _getStartCode()
      case TDRESULTS_VOLTAGE_IMAG: // handled by special logic in _getStartCode()
      default:
        continue;                  // ! before setting resultsReady[k] = true
      }
      res.set_data( k, resultsData );
      resultsReady[k] = true;
      continue;
    }
  }

  // --------------- Determine emulation runs needed ----------------
  // --------------- for results requested           ----------------
  vector<vector<unsigned int> > _NIOSStartCodes; // start code for each emulator slice
  // ( number_of_slices x number_of_emulation_runs )
  // contains all the start codes corresponding to each slice for all the
  // emulation runs required to retrieve all the requested results. e.g.
  // In the following example 3 runs in total are required on a full four slices
  // system, in order to retrieve 8 different results (time excluded)

  //          run1 run2 run3
  //  slice0  1111 1112 XXXX(1)
  //  slice1  1112 1114 1115
  //  slice2  1130 1131 XXXX
  //  slice3  1124 XXXX XXXX
  // In the previous example XXXX are indifferent values, in the sense that no
  // any more results are required from the slice in question.
  // According to Guillaume Lanz and as of Mar 2012, the best way to go is input
  // the last start code at that slice. So XXXX(1) would be 1112
  _NIOSStartCodes.clear();
  _NIOSStartCodes.resize( _emu->getHwSliceCount() );
  // The retrievalInfo map contains information on how which results correspond
  // to which request, in the retrieval phase (follows shortly after).
  map<size_t,TDResultsRetrievalInfo> retrievalInfo;
  map<size_t,TDResultsRetrievalInfo>::const_iterator riit;
  for ( size_t k = 0; k != resultsRequested.size() ; ++k ){

    // Skip for non-requested for non-requested or already provided results
    if ( !resultsRequested[k] || resultsReady[k] )
      continue;

    size_t sliceId;
    unsigned int startCode;
    ans = _getStartCode( resultsIdentifiers[k], sliceId, startCode );
    if ( ans ){
      // There was a problem retrieving the correct start code for the results
      // request k
      // Mark results as non available and continue
      continue;
    }
    // Check whether start code has already been registered for the slice in qst
    bool startCodeFound = false;
    unsigned int startCodeFoundRun;
    for ( size_t m = 0; m != _NIOSStartCodes[sliceId].size() ; ++m ){
      if ( _NIOSStartCodes[sliceId][m] == startCode ){
        startCodeFound = true;
        startCodeFoundRun = m;
        break; // so respective request was found at position [m]
      }
    }
    TDResultsRetrievalInfo temp;
    if ( !startCodeFound ){
      _NIOSStartCodes[sliceId].push_back(startCode);
      temp.sliceId = sliceId;
      temp.runId = _NIOSStartCodes[sliceId].size()-1;
    } else { // startCodeFound
      temp.sliceId = sliceId;
      temp.runId = startCodeFoundRun;
    }
    retrievalInfo.insert(pair<size_t,TDResultsRetrievalInfo>(k,temp));
  }
  // Precondition _NIOSStartCodes as explained
  size_t numberOfRuns = 0;
  for ( size_t k = 0 ; k != _NIOSStartCodes.size() ; ++k ){
    if ( _NIOSStartCodes[k].size() > numberOfRuns )
      numberOfRuns = _NIOSStartCodes[k].size();
    if ( _NIOSStartCodes[k].size() == 0 ){
      Slice* sl = &_emu->emuhw()->sliceSet[k];
      if (    sl->dig.pipe_gen.element_count()   == 0   // if all sl pipelines are
           && sl->dig.pipe_zload.element_count() == 0   // empty then start code is
           && sl->dig.pipe_iload.element_count() == 0   // 6666 == slice in HiZ
           && sl->dig.pipe_pload.element_count() == 0 )
        _NIOSStartCodes[k].push_back( 6666 );
      else                                          // if sl is not empty then
        _NIOSStartCodes[k].push_back( 1111 );       // put a placeholder start
                                                    // code to have the sl start
    }
  }
  for ( size_t k = 0 ; k != _NIOSStartCodes.size() ; ++k ){
    unsigned int lastStartCode = _NIOSStartCodes[k].at(_NIOSStartCodes[k].size()-1);
    for ( size_t m = _NIOSStartCodes[k].size() ; m < numberOfRuns ; ++m ){
      _NIOSStartCodes[k].push_back(lastStartCode);
    }
  }

  // ---- DEBUG COUT
//  for ( size_t k = 0 ; k != _NIOSStartCodes.size() ; ++k ){
//    cout << "_NIOSStartCode[" << k << "]: ";
//    for ( size_t m = 0 ; m != _NIOSStartCodes[k].size() ; ++m ){
//      cout << _NIOSStartCodes[k][m] << " ";
//    }
//    cout << endl;
//  }
//  cout << endl;
  // ---- END OF DEBUG COUT

  cout << "TDEmulator::do_simulate(): ";
  cout << "result requests validation " << timer.Stop() << " s" << endl;
  timer.Start();

  // --------------- Write start commands and Retrieve results ---------------
  vector<uint32_t> encoding556, readBuffer, runCodes;
  size_t runId_, sliceId_; // counters
  for ( runId_ = 0 ; runId_ != numberOfRuns ; ++runId_ ){

    // Reset emulator
    ans = resetEmulation(false); // DEBUG: change to resetEmulation(true);
    if ( ans ) return 50;
    cout << "TDEmulator::do_simulate(): ";
    cout << "(run " << runId_ << ") ";
    cout << "reset " << timer.Stop() << " s" << endl;
    timer.Start();

    // Write start commands to all slices
    runCodes.clear();
    for ( sliceId_ = 0 ; sliceId_ != _emu->getHwSliceCount() ; ++sliceId_ ){
      encoding556.clear();
      encoding556.push_back( _NIOSStartCodes[sliceId_][runId_] );
      runCodes.push_back( _NIOSStartCodes[sliceId_][runId_] );
      int devId = _emu->sliceDeviceMap(sliceId_);
      ans = _emu->usbWrite( devId, 556, encoding556 );
//      if ( ans ) // TODO
    }
    cout << "TDEmulator::do_simulate(): ";
    cout << "(run " << runId_ << ") ";
    cout << "issue start commands " << timer.Stop() << " s" << endl;
    timer.Start();

    // Wait for results
    ans = _waitForResults( runCodes, 30.0 ); // Blocking!
    if ( ans ){
      cout << "Emulator hardware CRASHED IN A FATAL WAY! ";
      cout << "Please switch off the lights in your office,";
      cout << "take a deep breath and hard-reset it ..." << endl;
      return 9;
    }
//    auxiliary::sleep(500000);
    cout << "TDEmulator::do_simulate(): ";
    cout << "(run " << runId_ << ") ";
    cout << "wait for stop codes " << timer.Stop() << " s" << endl;
    timer.Start();

    // Read and parse results according to retrievalInfo
    vector<double> resultsBuffer;
    for ( sliceId_ = 0 ; sliceId_ != _emu->getHwSliceCount() ; ++sliceId_ ){

      // Check to see whether results read from the emulator correspond to any
      // requested results
      for (riit = retrievalInfo.begin() ; riit != retrievalInfo.end() ; ++riit){
        TDResultsRetrievalInfo tdrri = riit->second;
        if ( tdrri.runId == runId_ && tdrri.sliceId == sliceId_ ){
          // Results for resId correspond to this runId_ and sliceId_
          size_t resId = riit->first;

          // So perform a read operation on the slice
          int devId = _emu->sliceDeviceMap(sliceId_);
          // 561 is the address that the FPGA starts writing the requested results to
          ans = _emu->usbRead(devId, 561+startSample, stopSample, readBuffer);
          cout << "TDEmulator::do_simulate(): ";
          cout << "(run " << runId_ << ") ";
          cout << "read dev" <<  devId << " " << timer.Stop() << " s" << endl;
          timer.Start();

          switch ( resultsIdentifiers[resId].variable ){
          case TDRESULTS_ANGLE:
            _parseGenAngles( readBuffer, resultsBuffer, true );
            break;
          case TDRESULTS_SPEED:
            _parseGenSpeeds( readBuffer, resultsBuffer );
            break;
          case TDRESULTS_ACCELERATING_POWER:
            _parseGenPmec( readBuffer, resultsBuffer );
            break;
          case TDRESULTS_VOLTAGE_REAL:
            _parseVoltage( readBuffer, true, resultsBuffer );
            break;
          case TDRESULTS_VOLTAGE_IMAG:
            _parseVoltage( readBuffer, false, resultsBuffer );
            break;
          /* Current in the resultsBuffer represents the current that comes out
          of the amplifier. In the case of generators, this current is not the
          one injected at the generator bus. In the following schema is is I',
          whereas I is really needed.

          Generator Norton equivalent:

                     I to the grid
              -------->-----
              |   |
           I' ^   | \/ I''
              |   |
              |  [ ]
              O  [ ] xd_1
              |  [ ]
              |   |
              |   |
             --- ---
              -   -

          So I'' (flowing through the xd_1) has to be subtracted */
          case TDRESULTS_CURRENT_REAL:
            _parseCurrent( readBuffer, true, resultsBuffer );
            break;
          case TDRESULTS_CURRENT_IMAG:
            _parseCurrent( readBuffer, false, resultsBuffer );
            break;
          default:
            break;
          } // end switch ( resultsIdentifiers[resId].variable )

          // Store results to res struct
          res.set_data( resId, resultsBuffer );
          resultsReady[resId] = true;
          cout << "TDEmulator::do_simulate(): ";
          cout << "(run " << runId_ << ") ";
          cout << "parse results from dev" <<  devId << " " << timer.Stop() << " s" << endl;
          timer.Start();

        } // end if ( tdrri.runId == runId_ && tdrri.sliceId == sliceId_ )
      } // retrievalInfo loop
      if ( _log != NULL )
     _log->notifyProgress((sliceId_+1)/_emu->getHwSliceCount()*100);
      auxiliary::stayAlive();
    } // number of slices read & parse results loop
  } // number of runs loop

  // Update available results vector
  for ( size_t k = 0; k != resultsRequested.size() ; ++k )
    res.set_dataExists( k, resultsRequested[k] & resultsReady[k] );

  cout << "TDEmulator::do_simulate(): ";
  cout << "simulation (" << numberOfRuns << " runs) completed!" << endl;
  return 0;
}

#else

#define NEUTRAL_STARTCODE 1110

int TDEmulator::do_simulate( Scenario const& sce, TDResults& res ){

  PrecisionTimer timer; // counts in seconds
  timer.Start();
  // --------------- Map & fit & encode pws & end calib. mode ---------------
  int ans = _emu->preconditionEmulator();
  if ( ans ) return ans;
  // _emu->encoding has been initialized to a size: [slicecount]x[560]
  cout << "TDEmulator::do_simulate(): ";
  cout << "preconditioning proper " << timer.Stop() << " s" << endl;
  timer.Start();

  // --------------- Encoding ---------------
  // Set the time options : Determine the start and stop reading samples
  // according to the timeStep and the start/stop time requested by the scenario
  /* Modifies:
  _emu->encoding[*][381] (wr @cyp.add 382): _NIOSSampleRate + interval betw. 2 starts
  _emu->encoding[*][558] (wr @cyp.add 559): _NIOSSampleRate
  So it must be run after encodePowersystem() */
  _encodeTimeOptions();
  // Determine start & stop sample (for later, results retrieval use)
  unsigned int startSample, stopSample;
  startSample = static_cast<unsigned int>( ceil(sce.startTime()/_timeStep) );
  stopSample  = static_cast<unsigned int>( floor(sce.stopTime()/_timeStep) );
  cout << "TDEmulator::do_simulate(): ";
  cout << "encode time options " << timer.Stop() << " s" << endl;
  timer.Start();

  // Encode the scenario
  /* Modifies:
  For slice k containing the perturbation:
    _emu->encoding[k][551] (cyp.add 552) = pertStopStart
    _emu->encoding[k][552] (cyp.add 553) = not used
    _emu->encoding[k][553] (cyp.add 554) = switchConf
  For all slices:
    _emu->encoding[*][554] (cyp.add 555) = NIOSconfirm + stopSample
    _emu->encoding[*][559] (cyp.add 560) = pertStopStart
    For spy0: attached to gen angles
    _emu->encoding[*][331] (cyp.add 332): 0x00.0bxxxxxxx87654321x where positions 1-8
                                     are 1 (true) when there is a respective gen
                                     pipeline element
    _emu->encoding[*][332] (cyp.add 333): spy0 start iteration
    _emu->encoding[*][333] (cyp.add 334): spy0 stop iteration
    _emu->encoding[*][334] (cyp.add 335): spy0 sample rate }
    For spy1: attached to gen angles
    _emu->encoding[*][335] (cyp.add 332): 0x00.0bxxxxxxx87654321x where positions 1-8
                                     are 1 (true) when there is a respective gen
                                     pipeline element
    _emu->encoding[*][336] (cyp.add 333): spy1 start iteration
    _emu->encoding[*][337] (cyp.add 334): spy1 stop iteration
    _emu->encoding[*][338] (cyp.add 335): spy1 sample rate
  So it must be run after encodePowersystem() */
  ans = _encodeScenario( sce );
  if ( ans ) return 41;
  cout << "TDEmulator::do_simulate(): ";
  cout << "encode scenario " << timer.Stop() << " s" << endl;
  timer.Start();

  // Finally write the modified _emu->encoding vector to the devices
  ans = _emu->writeEncoding( false );
  if ( ans ) return 42;
  cout << "TDEmulator::do_simulate(): ";
  cout << "write encoding " << timer.Stop() << " s" << endl;
  timer.Start();

  // --------------- Verify that results requested can be provided -------------
  vector<double> resultsData; // temp results data vector
  vector<TDResultIdentifier> resultsIdentifiers = res.identifiers();
  vector<bool> resultsRequested = res.dataExists();
  vector<bool> resultsReady( resultsRequested.size(), false );
  for ( size_t k = 0 ; k != res.dataExists().size() ; ++k ){
    // Check whether the results are requested
    if( !resultsRequested[k] )
      continue;

    // Check whether the requested results are supported by the emulator
    if( _supportedResults.count(make_pair(resultsIdentifiers[k].elementType,
                                          resultsIdentifiers[k].variable))==0 ){
      // Mark result as non available and continue
      resultsRequested[k] = false;
      continue;
    }

    // For bus voltages see whether there is a generator or a load mapped onto
    // them, otherwise the bus voltage cannot be retrieved (ie. of fork buses)
    if (    resultsIdentifiers[k].elementType == TDRESULTS_BUS
         && (    resultsIdentifiers[k].variable == TDRESULTS_VOLTAGE_REAL
              || resultsIdentifiers[k].variable == TDRESULTS_VOLTAGE_IMAG ) ){
      unsigned int busExtId = resultsIdentifiers[k].extId;
      int busIntId = _pws->getBus_intId( busExtId );
      if ( busIntId < 0){
        // Bus does not exist in the bus set! Should never happen
        // Mark results as non available and continue
        resultsRequested[k] = false;
        continue;
      }
      bool genMapped = false;
      bool loadMapped = false;
      if ( _pws->getBusGenMap().at( busIntId ).size() == 1 ){
        genMapped = true;
      } else if ( _pws->getBusLoadMap().at( busIntId ).size() == 1 ){
        loadMapped = true;
      }
      if ( !genMapped && !loadMapped ){
        // No observable (pipeline) element is mapped onto the node
        // Mark results as non available and continue
        resultsRequested[k] = false;
        continue;
      }
    }

    // ----- Provide results that do not need emulation runs -----
    // Results requested == time: No emulation required for time
    if (    resultsIdentifiers[k].elementType == TDRESULTS_OTHER
         && resultsIdentifiers[k].variable == TDRESULTS_TIME ){
      resultsData.clear();
      for ( size_t m = startSample ; m != stopSample ; ++m )
        resultsData.push_back( m*_timeStep);
      res.set_data( k, resultsData );
      resultsReady[k] = true;
      continue;
    }

    // Check for generator particularities: if the mechanical starting time of
    // a generator is greater than GEN_MECHSTARTTIME_THRESHOLD, then instead of
    // the generator pipeline of the correspoding slice, it is placed in the
    // const I load pipeline; for those gens a special treatment for results is
    // required
    if ( resultsIdentifiers[k].elementType == TDRESULTS_GENERATOR ){

      Generator const* pGen;
      ans = _pws->getGenerator( resultsIdentifiers[k].extId, pGen );
      if ( pGen->M() < GEN_MECHSTARTTIME_THRESHOLD )
        // Gen is a 'normal' gen, treated in the gen pipeline
        continue;

      // else { // The gen is treated in the const I pipeline
      resultsData.clear();

      // The following is not needed any more
//      complex<double> S( pGen->pgen(), pGen->qgen() );
//      complex<double> I = conj(S) / conj(pGen->Uss());
//      PwsMapperModelElement const*
//      el = _emu->mmd()->element( PWSMODELELEMENTTYPE_GEN, pGen->extId() );
//      int tab = floor( static_cast<double>(el->tab)/2.0 );
//      int row = floor( static_cast<double>(el->row)/2.0 );
//      int col = floor( static_cast<double>(el->col)/2.0 );
//      int pipelineId = _emu->emuhw()->sliceSet[tab].dig.pipe_iload.search_element(row,col);
//      if ( pipelineId < 0 ) return 50;

      switch ( resultsIdentifiers[k].variable ){

      case TDRESULTS_ANGLE:
        resultsData.resize( stopSample-startSample, pGen->deltass() );
        break;

      case TDRESULTS_SPEED:
        resultsData.resize( stopSample-startSample, 0 ); // 0 == omegass
        break;

//      case TDRESULTS_CURRENT_REAL:
//        resultsData.resize( stopSample-startSample, I.real() );
//        break;

//      case TDRESULTS_CURRENT_IMAG:
//        resultsData.resize( stopSample-startSample, I.imag() );
//        break;

      case TDRESULTS_ACCELERATING_POWER:
        resultsRequested[k] = false; // Pacc data cannot be provided for gens in
                                     // the const I load pipeline
      case TDRESULTS_VOLTAGE_REAL: // handled by special logic in _getStartCode()
      case TDRESULTS_VOLTAGE_IMAG: // handled by special logic in _getStartCode()
      default:
        continue;                  // ! before setting resultsReady[k] = true
      }
      res.set_data( k, resultsData );
      resultsReady[k] = true;
      continue;
    }
  }

  // --------------- Determine emulation runs needed ----------------
  // --------------- for results requested           ----------------
  vector<vector<unsigned int> > _NIOSStartCodes;
  // ( number_of_slices x number_of_emulation_runs )
  // contains all the start codes corresponding to each slice for all the
  // emulation runs required to retrieve all the requested results. e.g.
  // In the following example 3 runs in total are required on a full four slices
  // system, in order to retrieve 8 different results (time excluded)
  //
  //          run1 run2 run3
  //  slice0  1111 1112 XXXX(1)
  //  slice1  1112 1114 1115
  //  slice2  1130 1131 XXXX
  //  slice3  1124 XXXX XXXX
  // In the previous example XXXX are indifferent values, in the sense that no
  // any more results are required from the slice in question. So in order to
  // have the slice started NEUTRAL_STARTCODE code should be written

  _NIOSStartCodes.clear();
  _NIOSStartCodes.resize( _emu->getHwSliceCount() );
  // The retrievalInfo map contains information on which results correspond
  // to which request, in the retrieval phase (follows shortly after).
  map<size_t,TDResultsRetrievalInfo> retrievalInfo;
  map<size_t,TDResultsRetrievalInfo>::const_iterator riit;
  for ( size_t k = 0; k != resultsRequested.size() ; ++k ){

    // Skip for non-requested or already provided results
    if ( !resultsRequested[k] || resultsReady[k] )
      continue;

    size_t sliceId;
    unsigned int startCode;
    ans = _getStartCode( resultsIdentifiers[k], sliceId, startCode );
    if ( ans ){
      // There was a problem retrieving the correct start code for the results
      // request k
      // Mark results as non available and continue
      continue;
    }
    // Check whether start code has already been registered for the slice in
    // question
    bool startCodeFound = false;
    size_t startCodeFoundRun;
    for ( size_t m = 0; m != _NIOSStartCodes[sliceId].size() ; ++m ){
      if ( _NIOSStartCodes[sliceId][m] == startCode ){
        // Respective request was found at position [m]
        startCodeFound = true;
        startCodeFoundRun = m;
        break;
      }
    }
    TDResultsRetrievalInfo temp;
    if ( !startCodeFound ){
      _NIOSStartCodes[sliceId].push_back(startCode);
      temp.sliceId = sliceId;
      temp.runId = _NIOSStartCodes[sliceId].size()-1;
    } else { // startCodeFound
      temp.sliceId = sliceId;
      temp.runId = startCodeFoundRun;
    }
    retrievalInfo.insert(pair<size_t,TDResultsRetrievalInfo>(k,temp));
  }
  // Precondition _NIOSStartCodes as explained
  size_t numberOfRuns = 0;
  for ( size_t k = 0 ; k != _NIOSStartCodes.size() ; ++k ){
    if ( _NIOSStartCodes[k].size() > numberOfRuns )
      numberOfRuns = _NIOSStartCodes[k].size();
    if ( _NIOSStartCodes[k].size() == 0 ){
      Slice* sl = &_emu->emuhw()->sliceSet[k];
      if (    sl->dig.pipe_gen.element_count()   == 0   // if all sl pipelines are
           && sl->dig.pipe_zload.element_count() == 0   // empty then start code is
           && sl->dig.pipe_iload.element_count() == 0   // 6666 == slice in HiZ
           && sl->dig.pipe_pload.element_count() == 0 )
        _NIOSStartCodes[k].push_back( 6666 );
      else                                          // if sl is not empty then
        _NIOSStartCodes[k].push_back( NEUTRAL_STARTCODE ); // put a placeholder
                                              // start code to have the sl start
    }
  }
  for ( size_t k = 0 ; k != _NIOSStartCodes.size() ; ++k )
    for ( size_t m = _NIOSStartCodes[k].size() ; m < numberOfRuns ; ++m )
      _NIOSStartCodes[k].push_back( NEUTRAL_STARTCODE );

  // ---- DEBUG COUT
//  for ( size_t k = 0 ; k != _NIOSStartCodes.size() ; ++k ){
//    cout << "_NIOSStartCode[" << k << "]: ";
//    for ( size_t m = 0 ; m != _NIOSStartCodes[k].size() ; ++m ){
//      cout << _NIOSStartCodes[k][m] << " ";
//    }
//    cout << endl;
//  }
//  cout << endl;
  // ---- END OF DEBUG COUT
  cout << "TDEmulator::do_simulate(): ";
  cout << "result requests validation " << timer.Stop() << " s" << endl;
  timer.Start();

  // --------------- Write start commands and Retrieve results ---------------
  vector<uint32_t> encoding556(1);
  vector<uint32_t> readBuffer, runCodes;
  size_t runId_, sliceId_; // counters
  for ( runId_ = 0 ; runId_ != numberOfRuns ; ++runId_ ){

    // Reset emulator
    ans = resetEmulation(false); // DEBUG: change to resetEmulation(true);
    if ( ans ) return 51;
    cout << "TDEmulator::do_simulate(): ";
    cout << "(run " << runId_ << ") ";
    cout << "reset " << timer.Stop() << " s" << endl;
    timer.Start();

    // Write start commands to all slices
    runCodes.clear();
    for ( sliceId_ = 0 ; sliceId_ != _emu->getHwSliceCount() ; ++sliceId_ ){
      encoding556[0] = _NIOSStartCodes[sliceId_][runId_];
      runCodes.push_back( _NIOSStartCodes[sliceId_][runId_] );
      int devId = _emu->sliceDeviceMap(sliceId_);
      ans = _emu->usbWrite( devId, 556, encoding556 );
//      if ( ans ) // TODO
    }
    cout << "TDEmulator::do_simulate(): ";
    cout << "(run " << runId_ << ") ";
    cout << "issue start commands " << timer.Stop() << " s" << endl;
    timer.Start();

    // Wait for results
    ans = _waitForResults( runCodes, 60.0 ); // Blocking!
    if ( ans ){
      cout << "Emulator hardware CRASHED IN A FATAL WAY! ";
      cout << "Please switch off the lights in your office,";
      cout << "take a deep breath and hard-reset it ..." << endl;
      return 52;
    }
//    auxiliary::sleep(500000);
    cout << "TDEmulator::do_simulate(): ";
    cout << "(run " << runId_ << ") ";
    cout << "wait for stop codes " << timer.Stop() << " s" << endl;
    timer.Start();

    // Read and parse results according to retrievalInfo
    vector<double> resultsBuffer;
    for ( sliceId_ = 0 ; sliceId_ != _emu->getHwSliceCount() ; ++sliceId_ ){

      // Check to see whether results read from the emulator correspond to any
      // requested results
      for (riit = retrievalInfo.begin() ; riit != retrievalInfo.end() ; ++riit){
        TDResultsRetrievalInfo tdrri = riit->second;
        if ( tdrri.runId == runId_ && tdrri.sliceId == sliceId_ ){
          // Results for resId correspond to this runId_ and sliceId_
          size_t resId = riit->first;

          // So perform a read operation on the slice
          int devId = _emu->sliceDeviceMap(sliceId_);

          // Determine how results should be read from the boards
          if (    resultsIdentifiers[resId].elementType == TDRESULTS_GENERATOR
               && (   resultsIdentifiers[resId].variable == TDRESULTS_ANGLE
                   || resultsIdentifiers[resId].variable == TDRESULTS_SPEED ) ){
            // Provide the results throught the spies mechanism
            // Ask the spy to fetch the results
            unsigned int spyCode;
            ans = _getSpyCode( resultsIdentifiers[resId], spyCode );
            if ( ans ) return 60;
            vector<uint32_t> encoding558(1, static_cast<uint32_t>(spyCode));
            ans = _emu->usbWrite(devId, 558, encoding558);
//            if ( ans ) // todo
            // Wait for the spy to fetch the results
            ans = _waitForSpy(sliceId_, spyCode, 1);
            if ( ans ) return 61;

          } else {
            // Provide the results by directly reading the results addresses of
            // the shared memory - they have already been fetched there by NIOS
          }
          // Either way read the results back;  561 is the address that the FPGA
          // starts writing the requested results to
          ans = _emu->usbRead(devId, 561+startSample, stopSample, readBuffer);
          cout << "TDEmulator::do_simulate(): ";
          cout << "(run " << runId_ << ") ";
          cout << "read dev" <<  devId << " " << timer.Stop() << " s" << endl;
          timer.Start();

          switch ( resultsIdentifiers[resId].variable ){
          case TDRESULTS_ANGLE:
            _parseGenAngles( readBuffer, resultsBuffer, true );
            break;
          case TDRESULTS_SPEED:
            _parseGenSpeeds( readBuffer, resultsBuffer );
            break;
          case TDRESULTS_ACCELERATING_POWER:
            _parseGenPmec( readBuffer, resultsBuffer );
            break;
          case TDRESULTS_VOLTAGE_REAL:
            _parseVoltage( readBuffer, true, resultsBuffer );
            break;
          case TDRESULTS_VOLTAGE_IMAG:
            _parseVoltage( readBuffer, false, resultsBuffer );
            break;
  //        /* Current in the resultsBuffer represents the current that comes out
  //        of the amplifier. In the case of generators, this current is not the
  //        one injected at the generator bus. In the following schema is is I',
  //        whereas I is really needed.
  //
  //        Generator Norton equivalent:
  //
  //                   I to the grid
  //            -------->-----
  //            |   |
  //         I' ^   | \/ I''
  //            |   |
  //            |  [ ]
  //            O  [ ] xd_1
  //            |  [ ]
  //            |   |
  //            |   |
  //           --- ---
  //            -   -
  //
  //        So I'' (flowing through the xd_1) has to be subtracted
  //        */
  //        case TDRESULTS_CURRENT_REAL:
  //          _parseCurrent( readBuffer, true, resultsBuffer );
  //          break;
  //        case TDRESULTS_CURRENT_IMAG:
  //          _parseCurrent( readBuffer, false, resultsBuffer );
  //          break;
          default:
            break;
          } // end switch ( resultsIdentifiers[resId].variable )

          // Store results to res struct
          res.set_data( resId, resultsBuffer );
          resultsReady[resId] = true;
          cout << "TDEmulator::do_simulate(): ";
          cout << "(run " << runId_ << ") ";
          cout << "parse results from dev" <<  devId << " " << timer.Stop() << " s" << endl;
          timer.Start();

        } // end if ( tdrri.runId == runId_ && tdrri.sliceId == sliceId_ )
      } // retrievalInfo loop
    } // number of slices read & parse results loop
  } // number of runs loop

  // Update available results vector
  for ( size_t k = 0; k != resultsRequested.size() ; ++k )
    res.set_dataExists( k, resultsRequested[k] & resultsReady[k] );

  cout << "TDEmulator::do_simulate(): ";
  cout << "simulation (" << numberOfRuns << " runs) completed!" << endl;

  // Additional action: Reset encoding positions corresponding to spies
  /* For spy0: attached to gen angles
  _emu->encoding[*][331] (cyp.add 332) -> 0
  _emu->encoding[*][332] (cyp.add 333) -> 0
  _emu->encoding[*][333] (cyp.add 334) -> 0
  _emu->encoding[*][334] (cyp.add 335) -> 0
  For spy1: attached to gen angles
  _emu->encoding[*][335] (cyp.add 332) -> 0
  _emu->encoding[*][336] (cyp.add 333) -> 0
  _emu->encoding[*][337] (cyp.add 334) -> 0
  _emu->encoding[*][338] (cyp.add 335) -> 0 */
  vector<uint32_t> encoding331( 2*4, 0 );
  for ( sliceId_ = 0 ; sliceId_ != _emu->getHwSliceCount() ; ++sliceId_ ){
    int devId = _emu->sliceDeviceMap(sliceId_);
    ans = _emu->usbWrite(devId, 331, encoding331);
//    if ( ans ) // todo
  }
  cout << "TDEmulator::do_simulate(): ";
  cout << "reset spy encoding " << timer.Stop() << " s" << endl;

  return 0;
}

#endif // BUILD_NOSPIES

int TDEmulator::do_checkStability( vector<Scenario> const& scenarios,
                         vector<map<unsigned int,bool> >& genStable,
                         vector<map<unsigned int,double> >* genInstabilityTime){

  // Check for trivial run
  if ( scenarios.size() == 0 ){
    genStable.clear();
    if ( genInstabilityTime )
      genInstabilityTime->clear();
    return 0;
  }

  PrecisionTimer timer; // counts in seconds
  timer.Start();
  // --------------- Map & fit & encode pws & end calib. mode ---------------
  int ans = _emu->preconditionEmulator();
  if (ans) return ans;
  // _emu->encoding has been initialized to a size: [slicecount]x[560]
  cout << "TDEmulator::do_checkStability(): ";
  cout << "preconditioning proper " << timer.Stop() << " s" << endl;
  timer.Start();

  // --------------- Update encoding ---------------
  // ---- Set the time options : Determine the start and stop reading samples
  // according to the timeStep and the start/stop time requested by the scenario
  /* Modifies:
  _emu->encoding[*][381] (wr @cyp.add 382): _NIOSSampleRate + interval betw. 2 starts
  _emu->encoding[*][558] (wr @cyp.add 559): _NIOSSampleRate
  So it must be run after encodePowersystem() */
  _encodeTimeOptions();
  cout << "TDEmulator::do_checkStability(): ";
  cout << "encode time options " << timer.Stop() << " s" << endl;
  timer.Start();

  /* ---- Encode the scenarios
    _emu->encoding[*][551] (wr @cyp.add 552) = sce start for slice 2
    _emu->encoding[*][552] (wr @cyp.add 553) = sce start for slice 3
    _emu->encoding[*][553] (wr @cyp.add 554) = sce start for slice 4
    _emu->encoding[*][559] (wr @cyp.add 560) = sce count

    Starting from cyp.add 561 the following scenario description block is appended
    to _emu->encoding[*][...]:
      enc add sce#          word
    ----------------------------
    [560] 561    1 stopStartIter
    [561] 562    1    switchConf
    ----------------------------
    [562] 563    2 stopStartIter
    [563] 564    2    switchConf
    ----------------------------
    ...
    ----------------------------
    [A-1]   A    N stopStartIter  where N the Nth scenario (start count from 1)
    [  A] A+1    N    switchConf  and A starting address of the conf info for sceN
                                  A = 561 + 2*(N-1) */
  size_t sceRunCount; // scenarios that were actually encoded (the other
                            // were found incompatible with this analysis)
  map<size_t, size_t> newSceIndex;
  ans = _encodeMultiScenarios(scenarios, sceRunCount, newSceIndex);
  if ( ans ) return 41;
//  // DEBUG ---------------
//  cout << "newSceIndex:" << endl;
//  for ( map<size_t,size_t>::const_iterator it = newSceIndex.begin();
//        it != newSceIndex.end() ; ++it )
//    cout << it->first << " -> " << it->second << endl;
//  cout << endl;
//  // END OF DEBUG --------
  cout << "TDEmulator::do_checkStability(): ";
  cout << "encode scenarios " << timer.Stop() << " s" << endl;
  timer.Start();

  // Reset emulator before giving the new start commands
  ans = resetEmulation(true);
  if ( ans ) return 50;
  cout << "TDEmulator::do_checkStability(): ";
  cout << "reset emulator " << timer.Stop() << " s" << endl;
  timer.Start();

  ans = _emu->writeEncoding(false);
  if ( ans ) return 42;
  cout << "TDEmulator::do_checkStability(): ";
  cout << "write encoding " << timer.Stop() << " s" << endl;
  timer.Start();

  // --------------- Write start commands ---------------
  vector<int> stopCodes(_emu->getHwSliceCount(), 9000+sceRunCount);
  vector<uint32_t> encoding556(1, 0);
  for (size_t k = 0 ; k != _emu->getHwSliceCount() ; ++k ){
    // Determine start code for slice k
    Slice* sl = &_emu->emuhw()->sliceSet[k];
    if (    sl->dig.pipe_gen.element_count()   == 0   // if all sl pipelines are
         && sl->dig.pipe_zload.element_count() == 0   // empty then start code is
         && sl->dig.pipe_iload.element_count() == 0   // 6666 == slice in HiZ
         && sl->dig.pipe_pload.element_count() == 0 ){
      encoding556[0] = 6666;
      stopCodes[k] = -1; // for a hi-Z slice, do not expect any stopCode
    } else {
      encoding556[0] = 2001+k; // multi-scenario start command for each slice
    }

    // Write start code to slice
    int devId = _emu->sliceDeviceMap(k);
    ans = _emu->usbWrite( devId, 556, encoding556 );
//      if ( ans ) // TODO
  }
  cout << "TDEmulator::do_checkStability(): ";
  cout << "issue start commands " << timer.Stop() << " s" << endl;
  timer.Start();

  // --------------- Read and parse results ---------------
  ans = _waitForStopCodes( stopCodes, 15 );
  if ( ans ){
    cout << "Emulator hardware CRASHED IN A FATAL WAY! ";
    cout << "Please switch off the lights in your office,";
    cout << "take a deep breath and hard-reset it ..." << endl;
    return 51;
  }
  cout << "TDEmulator::do_checkStability(): ";
  cout << "wait for stop codes " << timer.Stop() << " s" << endl;
  timer.Start();

  genStable.clear();
  genStable.resize( scenarios.size() );
  if (genInstabilityTime)
    genInstabilityTime->clear();
  vector<uint32_t> readBuffer;
  uint32_t mask8 = (1<<8)-1;
//  uint32_t mask16 = (1<<16)-1; // TODO: use to extract time to first instabil
  for ( size_t sliceId = 0 ; sliceId != _emu->getHwSliceCount() ; ++sliceId ){
    // Read results
    int devId = _emu->sliceDeviceMap(sliceId);
    // 761 is the address that the FPGA starts writing the requested results to
    ans = _emu->usbRead(devId, 761, sceRunCount, readBuffer);
    if (ans) return 52;

    // Parse results
    Slice* sl = &_emu->emuhw()->sliceSet[sliceId];
    for ( size_t sceRunId = 0 ; sceRunId != sceRunCount ; ++sceRunId ){
      // Scenario that run sceRunId'th (as encoded by _encodeMultiScenarios)
      // corresponds to input argument 'scenarios' scenario index as per
      // newSceIndex
      size_t sceId = newSceIndex.at(sceRunId);

      // Get gen stability flags returned by the emulator; tempGenStab contains
      // boolean flags (0/1) denoting stability for the generator present in the
      // dig.pipe_gen of slice sliceId indexed according to their pipeline indexing
      bitset<8> tempGenStab =
          bitset<8>(static_cast<unsigned long>(mask8 & readBuffer[sceRunId]));
//      cout << "sliceId=" << sliceId
//           << " sceId=" << sceId
//           << " tempGenStab=" << tempGenStab.to_string() << endl;
      for ( size_t genPipeId = 0 ;
            genPipeId != sl->dig.pipe_gen.element_count() ;
            ++genPipeId ){

        // So for each one of the gens in the pipeline retrieve its mapping
        // position
        pair<int,int> genPipePos = sl->dig.pipe_gen.position().at( genPipeId );
        MappingPosition genMapPos( 2*sliceId,
                                   2*genPipePos.first,
                                   2*genPipePos.second );

        // From the mapping model find the extId of the generator mapped at that
        // position
        vector<pair<int,unsigned int> > elementsMapped = _emu->mmd()->elementsMapped(genMapPos);
        bool genExtIdFound = false;
        size_t elId;
        for ( elId = 0 ; elId != elementsMapped.size() ; ++elId ){
          if ( elementsMapped[elId].first == PWSMODELELEMENTTYPE_GEN ){
            genExtIdFound = true;
            break;
          }
        }
        if ( !genExtIdFound ) return 53; // Weird error!
        unsigned int genExtId = elementsMapped[elId].second;

        // And update the result stab map 'genStable'
        genStable[sceId][genExtId] = !tempGenStab[genPipeId];
      }
    }
  }

  // Update results stability maps with results for the slack gen
  for ( size_t sceRunId = 0 ; sceRunId != sceRunCount ; ++sceRunId ){
    // Scenario that run sceRunId'th (as encoded by _encodeMultiScenarios)
    // corresponds to input argument 'scenarios' scenario index as per
    // newSceIndex
    size_t sceId = newSceIndex.at(sceRunId);
    // Unless there is already a stability entry for the slack gen (it was found
    // in the generators pipeline) assume stable
    if (genStable[sceId].count(_pws->slackGenExtId()) <= 0)
      genStable[sceId][_pws->slackGenExtId()] = true;
  }
  cout << "TDEmulator::do_checkStability(): ";
  cout << "parse results " << timer.Stop() << " s" << endl;

  // --------------- Restore encoding back to normal size ---------------
  for ( size_t k = 0 ; k != _emu->getHwSliceCount() ; ++k ){
    // Scenario change options set back to 0
    _emu->encoding[k][551] = static_cast<uint32_t>(0);
    _emu->encoding[k][552] = static_cast<uint32_t>(0);
    _emu->encoding[k][553] = static_cast<uint32_t>(0);
    // Truncate _emu->encoding to 560 words
    if ( _emu->encoding[k].size() > 560 ) // normally true
      _emu->encoding[k].erase( _emu->encoding[k].begin()+560, _emu->encoding[k].end() );
  }

  return 0;
}

#define REALTIME_PER_ITERATION 1.586e-6
#define ALL_UNSTABLE_CODE 0xf000
#define ALL_STABLE_CODE 0x000f

int TDEmulator::do_checkCCT( vector<Scenario> const& scenarios,
                             double precision,
                             vector<double>& cctMin, vector<double>& cctMax ){

  // Input argument validation
  for ( size_t k = 0 ; k != scenarios.size() ; ++k ){
    if ( !scenarios[k].checkCCTCompatibility() )
      return 1;
  }
  /* If so the following event structure is assumed for each scenario k:
  scenario[k]._eventSet[0]: branch 3ph fault ON event
  scenario[k]._eventSet[1]: branch 3ph fault OFF event
  optional{scenario[k]._eventSet[2]: branch trip(on) event} */

  PrecisionTimer timer; // counts in seconds
  timer.Start();
  // --------------- Map & fit & encode pws & end calib. mode ---------------
  int ans = _emu->preconditionEmulator();
  if (ans) return ans;
  // _emu->encoding has been initialized to a size: [slicecount]x[560]
  cout << "TDEmulator::do_checkCCT(): ";
  cout << "preconditioning proper " << timer.Stop() << " s" << endl;
  timer.Start();

  // --------------- Update encoding ---------------

  // ---- Set the time options : Determine the start and stop reading samples
  // according to the timeStep and the start/stop time requested by the scenario
  /* Modifies:
  _emu->encoding[*][381] (wr @cyp.add 382): NIOS confirm + _NIOSSampleRate + interval betw. 2 starts
  _emu->encoding[*][558] (wr @cyp.add 559): _NIOSSampleRate
  So it must be run after encodePowersystem() */
  _encodeTimeOptions();
  cout << "TDEmulator::do_checkCCT(): ";
  cout << "encode time options " << timer.Stop() << " s" << endl;
  timer.Start();

  /* ---- Encode the scenarios
    _emu->encoding[*][551] (cyp.add 552) = sce start for slice 2
    _emu->encoding[*][552] (cyp.add 553) = sce start for slice 3
    _emu->encoding[*][553] (cyp.add 554) = sce start for slice 4
    _emu->encoding[*][554] (cyp.add 555) = NIOSconfirm + stopSample

    Starting from cyp.add 561 the following scenario description block is appended
    to _emu->encoding[*][...]:
      enc add sce#          word
    ----------------------------
    [560] 561       maxFaultTime
    [561] 562          precision
    ----------------------------
    [562] 563    1    switchConf
    [563] 564    2    switchConf
    ...

    [661] 662  100    switchConf  */
  map<size_t, size_t> newSceIndex;
  size_t sceRunCount;
  vector<size_t> scePerSlice;
  double maxFault, stopTime;
  ans = _encodeMultiCCT(scenarios,precision,
                        newSceIndex,sceRunCount,scePerSlice,maxFault,stopTime);
  if ( ans ) return 41;
  // DEBUG ---------------
//  cout << "newSceIndex:" << endl;
//  for ( map<size_t,size_t>::const_iterator it = newSceIndex.begin();
//        it != newSceIndex.end() ; ++it )
//    cout << it->first << " -> " << it->second << endl;
//  cout << endl;
  // END OF DEBUG --------
  cout << "TDEmulator::do_checkCCT(): ";
  cout << "encode scenarios " << timer.Stop() << " s" << endl;
  timer.Start();

  // Reset emulator before giving the new start commands
  ans = resetEmulation(true);
  if ( ans ) return 42;
  cout << "TDEmulator::do_checkCCT(): ";
  cout << "reset emulator " << timer.Stop() << " s" << endl;
  timer.Start();

  // ---- Write encoding to boards
  ans = _emu->writeEncoding(false);
  if ( ans ) return 50;
  cout << "TDEmulator::do_checkCCT(): ";
  cout << "write encoding " << timer.Stop() << " s" << endl;
  timer.Start();

  // --------------- Write start commands ---------------
  /* Start code written at address 556 expected:
      3200: when only one slice (the master) is physically present
      3001+k: for slice k {k = 0,1,2,3 WARNING: c++ indexing}
      3001: for a board containing pipeline elements
      6666: when no elements are present on the pipelines of the slice
    Notice: start code 3000 has to be written first, ie. before 3001 codes */

  // Check active slices
  vector<bool> activeSlices(_emu->getHwSliceCount());
  for ( size_t k = 0 ; k != _emu->getHwSliceCount() ; ++k ){
    Slice* sl = &_emu->emuhw()->sliceSet[k];
    if (    sl->dig.pipe_gen.element_count()   == 0   // if all sl pipelines are
         && sl->dig.pipe_zload.element_count() == 0   // empty then start code is
         && sl->dig.pipe_iload.element_count() == 0   // 6666 == slice in HiZ
         && sl->dig.pipe_pload.element_count() == 0 ){
      activeSlices[k] = false;
    } else {
      activeSlices[k] = true;
    }
  }
  // The simulation cannot start anyway if the master slice is not active
  if ( _emu->getHwSliceCount() > 0 )
    if ( !activeSlices[0] )
      return 51;
  // Check whether we have a situation like: active, active, inactive, active
  // which is not allowed
  int lastActive = 0;
  for ( int k = activeSlices.size()-1 ; k > 0 ; --k ){
    if ( activeSlices[k] ){
      lastActive = k;
    } else {
      if ( k < lastActive )
        return 52;
    }
  }
  // Write start codes to slices
  vector<uint32_t> encoding556(1, 0);
  if ( _emu->getHwSliceCount() == 1 ){
    // Case when only one slice is physically present in the system
    encoding556[0] = 3200;
    int devId = _emu->sliceDeviceMap(0);
    ans = _emu->usbWrite( devId, 556, encoding556 );
//    if ( ans ) // TODO
  } else {
    // Case when there are multiple slices
    for ( size_t k = 0; k != _emu->getHwSliceCount() ; ++k ){
      encoding556[0] = activeSlices[k] ? 3001+k : 6666;
      int devId = _emu->sliceDeviceMap(k);
      ans = _emu->usbWrite( devId, 556, encoding556 );
//      if ( ans ) // TODO
    }
  }
  cout << "TDEmulator::do_checkCCT(): ";
  cout << "issue start commands " << timer.Stop() << " s" << endl;
  timer.Start();

  // --------------- Read and parse results ---------------
  // Wait for stop codes (ACK that multiCCT analysis has been finished)
  vector<int> stopCodes( _emu->getHwSliceCount(), -1 );
  size_t lastPerturbed(0);
  for ( int k = scePerSlice.size()-1 ; k > 0  ; --k ){
    if ( scePerSlice[k] > 0 ){
      lastPerturbed = k;
      break;
    }
  }
  stopCodes[lastPerturbed] = 8777; // wait for the stop code only at the last
                                   // slice that was concerned by the multiCCT
  size_t intTimeStepOption = _getIntTStepOption();
  double intTimeStepValue = _int_tstep_values[intTimeStepOption];
  double itCount = stopTime/intTimeStepValue;
  double timePerRun = itCount * (2*REALTIME_PER_ITERATION); // safety factor of *2
  double runsPerSce = ceil(log2(maxFault/precision));
  double timeout = sceRunCount*runsPerSce*timePerRun;
//  cout << "TDEmulator::do_checkCCT(): timeout = " << timeout << endl; // DEBUG
  ans = _waitForStopCodes( stopCodes, timeout );
  if ( ans ){
    cout << "Emulator hardware CRASHED IN A FATAL WAY! ";
    cout << "Please switch off the lights in your office,";
    cout << "take a deep breath and hard-reset it ..." << endl;
    return 60;
  }
  cout << "TDEmulator::do_checkCCT(): ";
  cout << "wait for stop codes " << timer.Stop() << " s" << endl;
  timer.Start();

  // Determine words to read from each slice
  vector<size_t> wordsToRead(_emu->getHwSliceCount(),0);
  for ( size_t k = 0 ; k != wordsToRead.size() ; ++k )
    wordsToRead[k] = 3*scePerSlice[k];

  // Read results proper
  vector<uint32_t> readBuffer;
  vector<uint32_t> results;
  size_t readStartOffset(0);
  for ( size_t sliceId = 0 ; sliceId != _emu->getHwSliceCount() ; ++sliceId ){
    int devId = _emu->sliceDeviceMap(sliceId);
    // 663 is the address that the FPGA starts writing the requested results to
    // Note: on each slice, results are present only for scenarios concerning
    // that particular slice; other result positions are left empty (0)
    ans = _emu->usbRead(devId, 663+readStartOffset, wordsToRead[sliceId], readBuffer);
    if (ans) return 52;
    // Read results proper
    results.insert( results.end(), readBuffer.begin(), readBuffer.end() );
    // Update the read start offset
    readStartOffset += wordsToRead[sliceId];
  }

  // Parse results
  cctMin.clear();
  cctMax.clear();
  cctMin.resize( scenarios.size(), -1.0 );
  cctMax.resize( scenarios.size(), -1.0 );
  for ( size_t k = 0 ; k != sceRunCount ; ++k ){
    if ( results[3*k] == ALL_UNSTABLE_CODE && results[3*k+1] == ALL_UNSTABLE_CODE ){
      // Simulation unstable even for min time
      cctMin[ newSceIndex.at(k) ] = 0;
      cctMax[ newSceIndex.at(k) ] = 0 + precision;
    } else if ( results[3*k] == ALL_STABLE_CODE && results[3*k+1] == ALL_STABLE_CODE ){
      // Simulation stable even for max time
      cctMin[ newSceIndex.at(k) ] = maxFault - precision;
      cctMax[ newSceIndex.at(k) ] = maxFault;
    } else {
      // Normal case
      cctMin[ newSceIndex.at(k) ] = intTimeStepValue * static_cast<double>(results[3*k]);
      cctMax[ newSceIndex.at(k) ] = intTimeStepValue * static_cast<double>(results[3*k + 1]);
    }
  }
  cout << "TDEmulator::do_checkCCT(): ";
  cout << "parse results " << timer.Stop() << " s" << endl;

  return 0;
}

void TDEmulator::updateProperties( list<property> const& properties ){
  PropertyHolder::updateProperties(properties);
  // in addition update the time step
  _updateTimeStep();
}

int TDEmulator::updateProperty( property const& property_ ){
  int ans = PropertyHolder::updateProperty(property_);
  if ( ans )
    return ans;
  _updateTimeStep();
  return 0;
}

int TDEmulator::_getIntTStepOption() const{
  boost::any anyIntTStepOption = _getPropertyValueFromKey(TDEHWE_PROPERTY_INTOPT);
  int intTStepOption = boost::any_cast<int>( anyIntTStepOption );
  return intTStepOption;
}
int TDEmulator::_getNIOSSampleRate() const{
  boost::any anyNIOSSampleRate = _getPropertyValueFromKey(TDEHWE_PROPERTY_NIOSS);
  int NIOSSampleRate = boost::any_cast<int>( anyNIOSSampleRate );
  return NIOSSampleRate;
}

int TDEmulator::_getDampIt() const{
  boost::any anyDampIt = _getPropertyValueFromKey(TDEHWE_PROPERTY_DAMPIT);
  int dampIt = boost::any_cast<int>( anyDampIt );
  return dampIt;
}

int TDEmulator::_getTrigDelay() const{
  boost::any anyTrigDelay = _getPropertyValueFromKey(TDEHWE_PROPERTY_TRIGD);
  int trigDelay = boost::any_cast<int>( anyTrigDelay );
  return trigDelay;
}

int TDEmulator::_setIntTStepOption( int val ){
  // Update property proper
  property prop(TDEHWE_PROPERTY_INTOPT, val);
  int ans = updateProperty( prop );
  if ( ans )
    return ans;

//  // Update _timeStep
//  double intTStepValue = _int_tstep_values[val];
//  int NIOSSampleRate = _getNIOSSampleRate();
//  _timeStep = intTStepValue * static_cast<double>(NIOSSampleRate);

  return 0;
}

int TDEmulator::_setNIOSSampleRate( int val ){
  // Update property proper
  property prop(TDEHWE_PROPERTY_NIOSS, val);
  int ans = updateProperty( prop );
  if ( ans )
    return ans;

//  // Update _timeStep
//  int intTStepOption = _getIntTStepOption();
//  double intTStepValue = _int_tstep_values[intTStepOption];
//  int NIOSSampleRate = val;
//  _timeStep = intTStepValue * static_cast<double>(NIOSSampleRate);

  return 0;
}

int TDEmulator::_setDampIt( int val ){
  property prop(TDEHWE_PROPERTY_DAMPIT, val);
  return updateProperty( prop );
}

int TDEmulator::_setTrigDelay( int val ){
  property prop(TDEHWE_PROPERTY_TRIGD, val);
  return updateProperty( prop );
}

void TDEmulator::_updateTimeStep(){
  int intTStepOption = _getIntTStepOption();
  double intTStepValue = _int_tstep_values[intTStepOption];
  int NIOSSampleRate = _getNIOSSampleRate();
  _timeStep = intTStepValue * static_cast<double>(NIOSSampleRate);
//  cout << "Time step is set to: " << _timeStep << endl;
}


/* Determine the start and stop reading samples according to the timeStep and
   the start/stop time requested by the scenario

  Modifies:
  _emu->encoding[*][217] (wr @cyp.add 218): timeStep
  _emu->encoding[*][218] (wr @cyp.add 219): intTStepOption
  _emu->encoding[*][381] (wr @cyp.add 382): NIOSSampleRate + trigDelay
  _emu->encoding[*][558] (wr @cyp.add 559): NIOSSampleRate
*/
void TDEmulator::_encodeTimeOptions(){

  // ----- intTStepOption -----
  int intTStepOption = _getIntTStepOption();
  for ( size_t k = 0 ; k != _emu->getHwSliceCount() ; ++k ){
    _emu->encoding[k][217] = intTStepOption;
    _emu->encoding[k][218] = intTStepOption;
  }

  // ----- NIOSSampleRate + trigDelay -----
  int NIOSSampleRate = _getNIOSSampleRate();
  int trigDelaySet = _getTrigDelay();
  int maxMetric = 0;
  if ( trigDelaySet < 0 ){
    // Determine the wait time (metric) required between start signals for every
    // board due to pipeline elements latency; for every slice it is the max among
    // all pipelines of any slice
    for ( size_t k = 0 ; k != _emu->getHwSliceCount() ; ++k ){
      Slice* sl = &_emu->emuhw()->sliceSet[k];
      int genMetric = 107 + sl->dig.pipe_gen.element_count();
      if ( genMetric > maxMetric )
        maxMetric = genMetric;
      int iloadMetric = 76 + sl->dig.pipe_zload.element_count();
      if ( iloadMetric > maxMetric )
        maxMetric = iloadMetric;
    }
  } else {
    maxMetric = trigDelaySet;
  }
  for ( size_t k = 0 ; k != _emu->getHwSliceCount() ; ++k ){
    uint32_t mask15 = (1 << 15) - 1;
    uint32_t temp = static_cast<uint32_t>( (NIOSSampleRate & mask15) << 15 );
    Slice* sl = &_emu->emuhw()->sliceSet[k];
    if (    sl->dig.pipe_gen.element_count()   != 0   // if the slice pipelines are
         || sl->dig.pipe_zload.element_count() != 0   // empty then the LSB of
         || sl->dig.pipe_iload.element_count() != 0   // _emu->encoding[k][381] is left 0
         || sl->dig.pipe_pload.element_count() != 0 )
      temp |= maxMetric;
    encoder::stamp_NIOS_confirm( temp );
    _emu->encoding[k][381] = temp;
  }

  // ----- NIOSSampleRate -----
  for ( size_t k = 0 ; k != _emu->getHwSliceCount() ; ++k )
    _emu->encoding[k][558] = NIOSSampleRate;

  return;
}

int TDEmulator::_encodeScenarioBase( Scenario const& sce_,
                                     size_t& sliceId,
                                     unsigned int& stopStartIter,
                                     unsigned int& dampIter,
                                     unsigned int& switchConf,
                                     unsigned int& stopSample,
                                     bool& refittingPerformed ){
  Scenario sce(sce_);
  if ( !isEngineCompatible( sce ) )
    // Scenario not compatible with the engine
    return 1;

  sce.reduce();
  sce.sort_t();
  size_t k;
  int ans;
  refittingPerformed = false;

  // --------------- Determine the number of samples require to ---------------
  // --------------- the stop of the emulation --------------------------------
  stopSample = static_cast<unsigned int>(ceil(sce.stopTime()/getTimeStep()));

  // --------------- Determine the type of the possible error ---------------
  int error_type;
  // error_type=0 * bus-3ph_fault-ON
  // error_type=1 * bus-3ph_fault-ON followed by bus-3ph_fault-OFF
  // error_type=2 * branch-3ph_fault-ON
  // error_type=3 * branch-3ph_fault-ON followed by a branch-3ph_fault-OFF
  // error_type=4 * branch-3ph_fault-ON followed by a branch-3ph_fault-OFF and
  //                a concurrent branch-trip-ON
  // error_type=5 * branch-short-ON
  // error_type=6 * branch-short-ON followed by a branch-short-OFF
  // error_type=7 * branch-short-ON followed by a branch-short-OFF and a
  //                concurrent branch-trip-ON
  vector<Event> events = sce.getEventSet();
  vector<Event> on_events;  // at most 2 expected (one fault and one trip)
  vector<Event> off_events; // at most 1 expected (fault off )
  for ( k = 0; k != events.size() ; ++k ){
    if ( !events[k].status() ) continue;

    if ( events[k].bool_arg() ){
      on_events.push_back( events[k] );
      if ( on_events.size() > 2 )
        // Should never happen due to isEngineCompatible() check
        return 2;
    } else { // !events[k].bool_arg()
      off_events.push_back( events[k] );
      if ( off_events.size() > 1 )
        // Should never happen due to isEngineCompatible() check
        return 3;
    }
  }

  if (    on_events.size()==1 && off_events.size()==0
       && on_events[0].element_type()==EVENT_ELEMENT_TYPE_BUS
       && on_events[0].event_type()==EVENT_EVENTTYPE_BUSFAULT ){
    error_type = 0;
  } else if (    on_events.size()==1 && off_events.size()==0
              && on_events[0].element_type()==EVENT_ELEMENT_TYPE_BRANCH
              && on_events[0].event_type()==EVENT_EVENTTYPE_BRFAULT ){
    error_type = 2;
//  } else if (    on_events.size()==1 && off_events.size()==0
//              && on_events[0].element_type()==EVENT_ELEMENT_TYPE_BRANCH
//              && on_events[0].event_type()==EVENT_EVENTTYPE_BRTRIP){
//    error_type = 8;
  } else if (    on_events.size()==1 && off_events.size()==0
              && on_events[0].element_type()==EVENT_ELEMENT_TYPE_BRANCH
              && on_events[0].event_type()==EVENT_EVENTTYPE_BRSHORT ){
    error_type = 5;
  } else if (    on_events.size()==1 && off_events.size()==1
              && on_events[0].element_type()==EVENT_ELEMENT_TYPE_BUS
              && on_events[0].event_type()==EVENT_EVENTTYPE_BUSFAULT
              && off_events[0].element_type()==EVENT_ELEMENT_TYPE_BUS
              && off_events[0].event_type()==EVENT_EVENTTYPE_BUSFAULT){
    error_type = 1;
  } else if (    on_events.size()==1 && off_events.size()==1
              && on_events[0].element_type()==EVENT_ELEMENT_TYPE_BRANCH
              && on_events[0].event_type()==EVENT_EVENTTYPE_BRFAULT
              && off_events[0].element_type()==EVENT_ELEMENT_TYPE_BRANCH
              && off_events[0].event_type()==EVENT_EVENTTYPE_BRFAULT){
    error_type = 3;
  } else if (    on_events.size()==1 && off_events.size()==1
              && on_events[0].element_type()==EVENT_ELEMENT_TYPE_BRANCH
              && on_events[0].event_type()==EVENT_EVENTTYPE_BRSHORT
              && off_events[0].element_type()==EVENT_ELEMENT_TYPE_BRANCH
              && off_events[0].event_type()==EVENT_EVENTTYPE_BRSHORT){
    error_type = 6;
  } else if (    on_events.size()==2 && off_events.size()==1
              && on_events[0].element_type()==EVENT_ELEMENT_TYPE_BRANCH
              && on_events[0].event_type()==EVENT_EVENTTYPE_BRFAULT
              && off_events[0].element_type()==EVENT_ELEMENT_TYPE_BRANCH
              && off_events[0].event_type()==EVENT_EVENTTYPE_BRFAULT
              && on_events[1].element_type()==EVENT_ELEMENT_TYPE_BRANCH
              && on_events[1].event_type()==EVENT_EVENTTYPE_BRTRIP){
    error_type = 4;
  } else if (    on_events.size()==2 && off_events.size()==1
              && on_events[0].element_type()==EVENT_ELEMENT_TYPE_BRANCH
              && on_events[0].event_type()==EVENT_EVENTTYPE_BRSHORT
              && off_events[0].element_type()==EVENT_ELEMENT_TYPE_BRANCH
              && off_events[0].event_type()==EVENT_EVENTTYPE_BRSHORT
              && on_events[1].element_type()==EVENT_ELEMENT_TYPE_BRANCH
              && on_events[1].event_type()==EVENT_EVENTTYPE_BRTRIP){
    error_type = 7;
  } else {
    return 4;
  }

  // --------------- Determine scenario time options ---------------
  // Find the start and stop time
  double startTime = numeric_limits<double>::min();
  double stopTime = numeric_limits<double>::min();
  if ( on_events.size() > 0 )
    startTime = on_events[0].time();
  if ( off_events.size() > 0 )
    stopTime = off_events[0].time();
  // Saturate times to respect limits
  if ( startTime < 0 )
    startTime = 0; // saturate start time to 0
  if ( stopTime < startTime )
    stopTime = startTime; // saturate stopTime to startTime

  // Determine start and stop iterations
  size_t intTimeStepOption = _getIntTStepOption();
  double intTimeStepValue = _int_tstep_values[intTimeStepOption];
  double startIterD = startTime/intTimeStepValue;
  if ( startIterD < 1 )
    startIterD = 1; // saturate startIterD at 1
  double stopIterD = stopTime/intTimeStepValue;
  if ( stopIterD < startIterD )
    stopIterD = startIterD; // staturate stopIterD to startIterD
  uint32_t startIter = static_cast<uint32_t>(auxiliary::round(startIterD));
  uint32_t stopIter = static_cast<uint32_t>(auxiliary::round(stopIterD));
  stopStartIter = startIter;
  uint32_t mask16 = (1 << 16) - 1;
  stopIter &= mask16;
  stopStartIter |= ( stopIter << 16);

  // Retrieve mapping of the faulted element
  int el_type = static_cast<int>(on_events[0].element_type());
  int el_extId = static_cast<int>(on_events[0].element_extId());
  int tab, row, col;
  size_t embr_pos;
  ans = _emu->mmd()->getElementMapping( el_type, el_extId, &tab, &row, &col, &embr_pos);
  if ( ans ) return 5;
  // Determine whether the branch in question is on one of the extensions
  // (left or bottom)
  bool isOnLeftExtension = false;
  bool isOnBottomExtension = false;
  if ( row == 0 && col >= 1 && col <= 4 && embr_pos == EMBRPOS_D )
    isOnBottomExtension = true;
  else if ( col == 0 && row <= 2 && embr_pos == EMBRPOS_L )
    isOnLeftExtension = true;

  // --------------- Determine any branch refitting required due ---------------
  // --------------- to fault location                           ---------------
  // Refitting is required only for 3ph faults
  if ((error_type == 2 || error_type == 3 || error_type == 4) // br 3ph faults
      && on_events[0].double_arg() != 0.5 ){            // and fault loc is not
    // the default one
    if ( el_type != EVENT_ELEMENT_TYPE_BRANCH )
      return 10; // weirdo!

    // Retrieve branch to be fitted
    Branch const* pBr;
    ans = _pws->getBranch( el_extId, pBr );
    if ( ans )
      // Branch with ext id el_extId cannot be retrieved!
      return 11;

    // Determine location of the fault in embr terms
    double embr_loc;
    // In embr terms the location parameter [0,1] splits the resistance of the
    // embr between the two resistors, near and far. If the powersystem branch
    // is mapped with the from bus mapped to the node in question (tab,row,col)
    // then the embr location coincides with the location of the fault
    // location. Otherwise (to bus of the powersystem branch mapped to node in
    // question) it holds: embr_loc = 1 - fault_loc
    double fault_loc = on_events[0].double_arg();
    // Find out which elements are mapped to the node in question
    vector<pair<int,unsigned int> >
        nodeElements = _emu->mmd()->elementsMapped(tab,row,col);
    // Retrieve from and to bus of the branch
    Bus const* pFromBus;
    Bus const* pToBus;
    ans = _pws->getBus( pBr->fromBusExtId(), pFromBus );
    if ( ans )
      return 10;
    ans = _pws->getBus( pBr->toBusExtId(), pToBus );
    if ( ans )
      return 11;
    bool nodeFound = false;
    for ( k = 0 ; k != nodeElements.size() ; ++k ){
      if ( nodeElements[k].first == PWSMODELELEMENTTYPE_BUS
           && nodeElements[k].second == pFromBus->extId ){
        embr_loc = fault_loc;
        nodeFound = true;
        break;
      }
      if ( nodeElements[k].first == PWSMODELELEMENTTYPE_BUS
           && nodeElements[k].second == pToBus->extId ){
        embr_loc = 1 - fault_loc;
        nodeFound = true;
        break;
      }
    }
    if ( !nodeFound )
      return 12;

    // Refitting proper
    ans = _emu->embr_set(tab, row, col, embr_pos, *pBr, _emu->ratioZ(), embr_loc);
    if ( ans )
      cout << "Fitting br " << el_extId << " failed with code: " << ans << endl;
    refittingPerformed = true;
  }

  // --------------- Transform bus 3ph fault to branch 3ph fault ---------------
  else if ( error_type == 0 || error_type == 1 ){ // bus 3ph faults

    if ( el_type != EVENT_ELEMENT_TYPE_BUS )
      return 13; // weirdo!

    // Retrieve powersystem fault bus
    //    Bus const* pBus;
    //    ans = _pws->getBus( el_extId, pBus );
    //    if ( ans )
    //      return 14;
    int el_intId = _pws->getBus_intId( el_extId );
    if ( el_intId < 0 )
      return 15;

    // Retrieve the branch incident to the fault bus that has the largest x val
    set<unsigned int> incidentBrExtIds = _pws->getBusBrMap().at(el_intId);
    set<unsigned int>::const_iterator it;
    double maxBrX = -1;
    unsigned int maxBrX_brExtId;
    Branch const* pBr;
    for ( it = incidentBrExtIds.begin() ; it != incidentBrExtIds.end() ; it++ ){
      unsigned int brExtId = *it;
      ans = _pws->getBranch( brExtId, pBr );
      if ( ans )
        return 16;
      if ( pBr->x() > maxBrX ){
        maxBrX = pBr->x();
        maxBrX_brExtId = brExtId;
      }
    }
    ans = _pws->getBranch( maxBrX_brExtId, pBr );
    if ( ans )
      return 17;
//    // Alternatively retrieve the first incident branch to the fault bus
//    unsigned int brExtId = *(tde_hwe->getPws()->busBrMap().at(el_intId).begin());
//    Branch const* pBr;
//    int ans = tde_hwe->getPws()->getBranch( brExtId, pBr );
//    if ( ans )
//      // Branch with ext id brExtId cannot be retrieved!
//      return 11;

    // Determine embr loc
    double embr_loc;
    if ( pBr->fromBusExtId() == static_cast<unsigned int>(el_extId) )
      embr_loc = 0; // set fault at from end of the embr
    else if ( pBr->toBusExtId() == static_cast<unsigned int>(el_extId) )
      embr_loc = 1; // set fault at to end of the embr
    else
      return 18;

    // Determine the mapping position of the branch
    ans = _emu->mmd()->getElementMapping( PWSMODELELEMENTTYPE_BR, pBr->extId(),
                                   &tab, &row, &col, &embr_pos );
    if ( ans ) return 19;
    // Determine whether the branch in question is on one of the extensions
    // (left or bottom)
    isOnLeftExtension = false;
    isOnBottomExtension = false;
    if ( row == 0 && col >= 1 && col <= 4 && embr_pos == EMBRPOS_D )
      isOnBottomExtension = true;
    else if ( col == 0 && row <= 2 && embr_pos == EMBRPOS_L )
      isOnLeftExtension = true;

    // Refitting proper
    ans = _emu->embr_set(tab, row, col, embr_pos, *pBr, _emu->ratioZ(), embr_loc);
    if( ans )
      cout << "Fitting br " << el_extId << " failed with code: " << ans << endl;
    refittingPerformed = true;
  }

  // --------------- Determine the switch configuration ---------------
  switchConf = 0;
  size_t maxrow, maxcol;
  _emu->emuhw()->sliceSet[tab].ana.size(maxrow, maxcol);
  unsigned int nodeAddress = row * maxcol + col;
  unsigned int switchAddress = 0;
  /* The switch numbering for a normal (non-extension) node follows here-under:
    (Real and imaginary subnetworks share the switch numbering)

         U                UR
        /|                /
       / |            ---/
      /  |           /  /
     |   |-sw3      /  /
     |   |  |      /  /
    sw2  | gnd   sw4 /-sw5
     |   | ---   /  /   |
     |   |  -   /  /   gnd
     |   |     /  /    ---
      \  |    /  /      -
       \ |   ----
        \|  /
         | /
         |/  ------sw1------
         | /                 \
         O--------------------- R
                    |
                   sw0
                    |
                   gnd
                   ---
                    -

    Switch numbering for left extensions:
    (Real and imaginary subnetworks for the left extensions have different switch
    numbering but only the real part is taken into account; hereunder the imag
    numbering appears in parentheses)
    Left extension branches are known collectively as node 24

       L   ----sw9(11)---
    up   /                \   current board
    12 O--------------------O node 12
                 |
              sw10(12)
                 |
                ---
                 -

       L   ----sw5(7)----
    up   /                \   current board
     6 O--------------------O node 6
                 |
               sw6(8)
                 |
                ---
                 -

       L   ----sw1(3)----
    up   /                \   current board
     0 O--------------------O node 0
                 |
               sw2(4)
                 |
                ---
                 -


    Switch numbering for bottom extensions:
    (Real and imaginary subnetworks for the bottom extensions have different sw
    numbering but only the real part is taken into account)
    Bottom extension branches are known collectively as node 25

        cur. board        cur. board          cur. board           cur. board
          node 1            node 2              node 3               node 4
            O                 O                   O                    O
            |                 |                   |                    |
            |\                |\                  |\                   |\
            | \               | \                 | \                  | \
            |  \              |  \                |  \                 |  \
            |   |             |   |               |   |                |   |
     sw5(7)-|  sw4(6) sw9(11)-|  sw8(10) sw13(15)-|  sw12(14) sw17(19)-| sw16(15)
      |     |   |       |     |   |         |     |   |          |     |   |
     ---    |  /       ---    |  /         ---    |  /          ---    |  /
      -     | /         -     | /           -     | /            -     | /
            |/                |/                  |/                   |/
            |   D             |   D               |   D                |   D
            O                 O                   O                    O
         up board          up board            up board             up board
          node 1            node 2              node 3               node 4
    */

  // Fault is on a left extension emulator branch
  if ( isOnLeftExtension ){
    switch ( error_type ){
    case 4: // branch 3ph fault with trip
      switchConf |= ( 1 << 10 ); // branch tripped
    case 0: // bus 3ph fault translated to branch 3 ph fault
    case 1: // bus 3ph fault translated to branch 3 ph fault
    case 2: // branch 3 ph fault
    case 3: // branch 3 ph fault
      switch ( row ){
      case 0: switchAddress = 2; break;
      case 1: switchAddress = 6; break;
      case 2: switchAddress = 10; break;
      default: return 20; } // Weird!
      break;

    case 7: // branch short-circuit with trip
      switchConf |= ( 1 << 10 ); // branch tripped
    case 5: // branch short-circuit
    case 6: // branch short-circuit
      switch ( row ){
      case 0: switchAddress = 1; break;
      case 1: switchAddress = 5; break;
      case 2: switchAddress = 9; break;
      default: return 21; } // Weird!
      break;

    default:
      return 22;
    }
  }

  // Fault is on a bottom extension emulator branch
  else if ( isOnBottomExtension ){
    switch ( error_type ){
    case 4: // branch 3ph fault with trip
      switchConf |= ( 1 << 10 ); // branch tripped
    case 0: // bus 3ph fault translated to branch 3 ph fault
    case 1: // bus 3ph fault translated to branch 3 ph fault
    case 2: // branch 3 ph fault
    case 3: // branch 3 ph fault
      switch ( col ){
      case 1: switchAddress = 5; break;
      case 2: switchAddress = 9; break;
      case 3: switchAddress = 13; break;
      case 4: switchAddress = 17; break;
      default: return 23; } // Weird!
      break;

    case 7: // branch short-circuit with trip
      switchConf |= ( 1 << 10 ); // branch tripped
    case 5: // branch short-circuit
    case 6: // branch short-circuit
      switch ( col ){
      case 1: switchAddress = 4; break;
      case 2: switchAddress = 8; break;
      case 3: switchAddress = 12; break;
      case 4: switchAddress = 16; break;
      default: return 24; } // Weird!
      break;

    default:
      return 25;
    }
  }

  // Fault is on a non-extension emulator branch
  else {
    switch ( error_type ){
    case 4: // branch 3ph fault
      switchConf |= ( 1 << 10 ); // branch tripped
    case 0: // bus 3ph fault translated to branch 3 ph fault
    case 1: // bus 3ph fault translated to branch 3 ph fault
    case 2: // branch 3ph fault
    case 3: // branch 3ph fault
      if ( embr_pos == EMBRPOS_U )
        switchAddress = 3;
      else if ( embr_pos == EMBRPOS_UR )
        switchAddress = 5;
      else if ( embr_pos == EMBRPOS_R )
        switchAddress = 0;
      break;

    case 7: // branch short-circuit
      switchConf |= ( 1 << 10 ); // branch tripped
    case 5: // branch short-circuit
    case 6: // branch short-circuit
      if ( embr_pos == EMBRPOS_U )
        switchAddress = 2;
      else if ( embr_pos == EMBRPOS_UR )
        switchAddress = 4;
      else if ( embr_pos == EMBRPOS_R )
        switchAddress = 1;
      break;

    default:
      // unknown error_type
      return 23;
    }
  }

  uint32_t mask5 = (1 << 5) - 1;
  nodeAddress &= mask5;
  switchConf |= static_cast<uint32_t>(nodeAddress << 1);
  uint32_t mask4 = (1 << 4) - 1;
  switchAddress &= mask4;
  switchConf |= static_cast<uint32_t>(switchAddress << 6);

  // --- Damping ---
  int dampIterSet = _getDampIt();
  if ( dampIterSet >= 0 ){
    dampIter = static_cast<unsigned int>(dampIterSet);
  } else {
    uint32_t mask16 = (1 << 16) - 1;
    dampIter = stopStartIter & mask16; // damp till the beginning of the first event
  }

  // --------------- Output arguments ---------------
  sliceId = static_cast<size_t>(tab);
//  stopStartIter
//  dampIter
//  switchConf
//  stopSample

  return 0;
}

/* Modifies:
  For slice k containing the perturbation:
    _emu->encoding[k][551] (cyp.add 552) = pertStopStart
    _emu->encoding[k][552] (cyp.add 553) = not used
    _emu->encoding[k][553] (cyp.add 554) = switchConf

  For all slices:
    _emu->encoding[*][168] (cyp.add 169) = dampIt
    - ... --
    _emu->encoding[*][554] (cyp.add 555) = NIOSconfirm + stopSample
    _emu->encoding[*][559] (cyp.add 560) = pertStopStart

    For spy0: attached to gen angles
    _emu->encoding[*][331] (cyp.add 332): 0x00.0bxxxxxxx87654321x where positions 1-8
                                     are 1 (true) when there is a respective gen
                                     pipeline element
    _emu->encoding[*][332] (cyp.add 333): spy0 start iteration
    _emu->encoding[*][333] (cyp.add 334): spy0 stop iteration
    _emu->encoding[*][334] (cyp.add 335): spy0 sample rate }
    For spy1: attached to gen angles
    _emu->encoding[*][335] (cyp.add 332): 0x00.0bxxxxxxx87654321x where positions 1-8
                                     are 1 (true) when there is a respective gen
                                     pipeline element
    _emu->encoding[*][336] (cyp.add 333): spy1 start iteration
    _emu->encoding[*][337] (cyp.add 334): spy1 stop iteration
    _emu->encoding[*][338] (cyp.add 335): spy1 sample rate */
int TDEmulator::_encodeScenario( Scenario sce ){

  // If the state of the TDEmulator is not at least EMU_STATE_PWSENCODED
  // then the function is not executed (returns non-zero)
  if ( _emu->state() < EMU_STATE_PWSENCODED )
    return -1;

  // Assert that the scenario is compatible with TDEmulator
  if ( !isEngineCompatible(sce) )
    return 1;

  size_t sliceId;
  unsigned int stopStartIter;
  unsigned int dampIter;
  unsigned int switchConf;
  unsigned int stopSample;
  bool refittingPerformed;
  int ans = _encodeScenarioBase( sce, sliceId, stopStartIter, dampIter,
                                 switchConf, stopSample, refittingPerformed );
  if ( ans ) return 2;
  // ---- DEBUG ----
//  cout << "Scenario{"
//       << " sliceId:" << sliceId
//       << " stopStartIter:" << stopStartIter
//       << " switchConf:" << switchConf
//       << " stopSample:" << stopSample
//       << " refit:"<<refittingPerformed
//       << "}" << endl;
  // ---- DEBUG ----
  if ( refittingPerformed ){
    ans = _emu->encodePowersystem();
    if ( ans ) return 3;
  }
  size_t intTimeStepOption = _getIntTStepOption();
  double intTimeStepValue = _int_tstep_values[intTimeStepOption];
  int NIOSSampleRate = _getNIOSSampleRate();


  // --- Determine spy-specific variables ---
  // Determine start and stop iterations
  double startIterD = sce.startTime()/intTimeStepValue;
  if ( startIterD < 1 )
    startIterD = 1; // saturate startIterD at 1
  double stopIterD = sce.stopTime()/intTimeStepValue;
  if ( stopIterD < startIterD )
    stopIterD = startIterD; // staturate stopIterD to startIterD
  uint32_t startIt = static_cast<uint32_t>(auxiliary::round(startIterD));
  uint32_t stopIt = static_cast<uint32_t>(auxiliary::round(stopIterD));

  for ( size_t k = 0 ; k != _emu->encoding.size() ; ++k ){

    // *** DAMPING ***
    _emu->encoding[k][168] = dampIter;

    // *** SPIES ***
    Slice const* sl = &_emu->emuhw()->sliceSet[k];
    // Directive to spy all elements in the gen pipeline of the slice
    uint32_t spyDirective = ((1 << sl->dig.pipe_gen.element_count()) - 1) << 1;
    // For spy0 attached to gen angles
    _emu->encoding[k][331] = spyDirective;
    _emu->encoding[k][332] = startIt;
    _emu->encoding[k][333] = stopIt;
    _emu->encoding[k][334] = NIOSSampleRate;
    // For spy1 attached to gen speeds
    _emu->encoding[k][335] = spyDirective;
    _emu->encoding[k][336] = startIt;
    _emu->encoding[k][337] = stopIt;
    _emu->encoding[k][338] = NIOSSampleRate;

    // *** SCENARIO PROPER ***
    if ( k == sliceId ){
      // Fill in perturbation details only for the board which actually features
      // the perturbation
      _emu->encoding[k][551] = static_cast<uint32_t>(stopStartIter);
      _emu->encoding[k][552] = static_cast<uint32_t>(0);
      _emu->encoding[k][553] = static_cast<uint32_t>(switchConf);
    } else {
      // Fill in 0 to perturbation encoding positions for the rest of the boards
      // in order to prevent them from being perturbed
      _emu->encoding[k][551] = static_cast<uint32_t>(0);
      _emu->encoding[k][552] = static_cast<uint32_t>(0);
      _emu->encoding[k][553] = static_cast<uint32_t>(0);
    }
    // Fill in encoding position [554] for every board with the number of
    // samples requested for the whole emulation run
    _emu->encoding[k][554] = static_cast<uint32_t>(stopSample);
    encoder::stamp_NIOS_confirm( _emu->encoding[k][554] );
    // Fill in encoding position [559] for every board
    _emu->encoding[k][559] = static_cast<uint32_t>(stopStartIter);
  }

  return 0;
}

/* Modifies (for all boards *):
    _emu->encoding[*][551] (cyp.add 552) = maxFaultIter
    _emu->encoding[*][552] (cyp.add 553) = precisionIter
    _emu->encoding[*][553] (cyp.add 554) = switchConf
    _emu->encoding[*][554] (cyp.add 555) = NIOSconfirm + stopSample
  ( _emu->encoding[*][559] (cyp.add 560) = pertStopStart )  */
int TDEmulator::_encodeScenarioCCT( Scenario sce,
                                    double maxFaultTime,
                                    double precision,
                                    size_t& sceSliceId ){
  // If the state of the TDEmulator is not at least EMU_STATE_PWSENCODED
  // then the function is not executed (returns non-zero)
  if ( _emu->state() < EMU_STATE_PWSENCODED )
    return -1;

  // Assert that the scenario is compatible with TDEmulator
  if ( !isEngineCompatible(sce) )
    return 1;

  size_t sliceId;
  unsigned int stopStartIter;
  unsigned int dampIter;
  unsigned int switchConf;
  unsigned int stopSample;
  bool refittingPerformed;
  int ans = _encodeScenarioBase( sce, sliceId, stopStartIter, dampIter,
                                 switchConf, stopSample, refittingPerformed );
  if ( ans ) return 2;
  // ---- DEBUG ----
//  cout << "Scenario{"
//       << " sliceId:" << sliceId
//       << " stopStartIter:" << stopStartIter
//       << " switchConf:" << switchConf
//       << " stopSample:" << stopSample
//       << " refit:"<<refittingPerformed
//       << "}" << endl;
  // ---- DEBUG ----
  if ( refittingPerformed ){
    ans = _emu->encodePowersystem();
    if ( ans ) return 3;
  }
//  if (_emu->getHwSliceCount()==0) return 3; // redundant
  size_t intTimeStepOption = _getIntTStepOption();
  double intTimeStepValue = _int_tstep_values[intTimeStepOption];

  // ---- Set CCT analysis specifics
  /* maxFaultIter : maximum CCT test time [in hw integrator iterations]
     precisionIter: precision [in hw integrator iterations] */
  uint32_t maxFaultIter(static_cast<uint32_t>(auxiliary::round(maxFaultTime/intTimeStepValue)));
  uint32_t precisionIter(static_cast<uint32_t>(auxiliary::round(precision/intTimeStepValue)));

  for ( size_t k = 0 ; k != _emu->encoding.size() ; ++k ){

    /* NEW IMPLEMENTATION:
      _emu->encoding[k][551, 552, 553, 554, (559)] written to all boards */
    _emu->encoding[k][551] = static_cast<uint32_t>(maxFaultIter);
    _emu->encoding[k][552] = static_cast<uint32_t>(precisionIter);
    _emu->encoding[k][553] = static_cast<uint32_t>(switchConf);
    _emu->encoding[k][554] = static_cast<uint32_t>(stopSample);
    encoder::stamp_NIOS_confirm( _emu->encoding[k][554] );
//    // Fill in encoding position [560] for every board
//    _emu->encoding[k][559] = static_cast<uint32_t>(stopStartIter);
  }

  // Update output argument
  sceSliceId = sliceId;

  return 0;
}

/* Modifies the following:
  _emu->encoding[*][551] (wr @cyp.add 552) = sce start for slice 2
  _emu->encoding[*][552] (wr @cyp.add 553) = sce start for slice 3
  _emu->encoding[*][553] (wr @cyp.add 554) = sce start for slice 4
  _emu->encoding[*][554] (wr @cyp.add 555) = NIOSconfirm + stopSample
  _emu->encoding[*][555] (wr @cyp.add 556) = 0 (clear start code for multi-sce run)
  - ... -
  _emu->encoding[*][559] (wr @cyp.add 560) = sce count

  Starting from cyp.add 561 the following scenario description block is appended
  to _emu->encoding[*][...]:
    enc add sce#          word
  ----------------------------
  [560] 561    1 stopStartIter
  [561] 562    1    switchConf
  ----------------------------
  [562] 563    2 stopStartIter
  [563] 564    2    switchConf
  ----------------------------
  ...
  ----------------------------
  [A-1]   A    N stopStartIter  where N the Nth scenario (start count from 1)
  [  A] A+1    N    switchConf  and A starting address of the conf info for sceN
                                A = 561 + 2*(N-1)

  Reindexing performed at 'map<size_t, size_t> newSceIndex' is as
  follows: _scenariosEncoding id -> scenarios id */
int TDEmulator::_encodeMultiScenarios( vector<Scenario> scenarios,
                                       size_t& sceRunCount,
                                       map<size_t, size_t>& newSceIndex){

  // If the state of the TDEmulator is not at least EMU_STATE_PWSENCODED
  // then the function is not executed (returns non-zero)
  if ( _emu->state() < EMU_STATE_PWSENCODED )
    return -1;

  // Input argument validation
  if ( scenarios.size() == 0 )
    return 1;

  // ----- Determine scenario encoding info -----
  vector<vector<size_t> > sliceScenariosIds( _emu->getHwSliceCount() );
  vector<size_t> incompatibleScenariosIds;
  vector<unsigned int> stopStartIter( scenarios.size() );
  vector<unsigned int> switchConf( scenarios.size() );
  unsigned int stopSample; // stopSample is the number of samples requested to
                           // to be computed for each multi scenario run
  // NOTE: In the way stopSample is updated hereunder, it is overwritten always
  // by the stopSample corresponding to stop time of the last scenario in
  // 'scenarios'
  for ( size_t k = 0; k != scenarios.size() ; ++k ){

    // Skip inactive scenarios
    if ( !scenarios[k].status() ){
      // Well, it's not exactly incompatible but anyway :)
      incompatibleScenariosIds.push_back(k);
      continue;
    }
    // Check whether scenarios are compatible with the engine
    if ( !isEngineCompatible(scenarios[k]) ){
      incompatibleScenariosIds.push_back(k);
      continue;
    }
    // Encode scenario proper
    size_t sliceId_;
    unsigned int stopStartIter_, dampIter_, switchConf_;
    bool refittingPerformed_;
    int ans = _encodeScenarioBase(scenarios[k],
                                  sliceId_, stopStartIter_, dampIter_,
                                  switchConf_, stopSample, refittingPerformed_);
    if ( ans ){
      // If the encoding failed to encode the scenario then mark it as
      // incompatible and continue
      incompatibleScenariosIds.push_back(k);
      continue;
    }
    if ( refittingPerformed_ ){
      // If for the current scenario a refitting is required then mark is as
      // incompatible and continue. In multiple scenarios mode, no refitting is
      // allowed between different scenarios
      incompatibleScenariosIds.push_back(k);
      continue;
    }

    // Update stores
    sliceScenariosIds[sliceId_].push_back(k);
    stopStartIter[k] = stopStartIter_;
    switchConf[k] = switchConf_;
  }

  /* Multi-scenarios are executed one after the other, with the perturbation
  being situated on different slices, in case of multi-slice emulator set-ups.
  'sceChange' holds the information when the perturbation info starts concerning
  a following board. There are 3 positions in the memory conf where the above is
  stored:
  _emu->encoding[*][551] (wr @cyp.add 552) = sce start for slice 2
  _emu->encoding[*][552] (wr @cyp.add 553) = sce start for slice 3
  _emu->encoding[*][553] (wr @cyp.add 554) = sce start for slice 4
  _emu->encoding[*][554] (wr @cyp.add 555) = NIOSconfirm + stopSample
  So scenarios starting with sceId (index count starting from 0!) stored in
  position [552] @553 should start executing their perturbation on slice #2, and
  so on. In case of a set-up with less than 4 boards, or of scenario sets that
  do not contain scenarios for all the boards, 'sceChange' for the first
  non-existing slice would be overall 'scenario count+1' (to signify a
  non-existing scenario, and thus the stop of the emulation) and '0' from then
  on.

  Eg. in a case of a two-slice setup with 13 scenarios to emulate:
  _emu->encoding[*][551] (wr@cyp.add 552) = sceChange[0] = 9
  _emu->encoding[*][552] (wr@cyp.add 553) = sceChange[1] = 14
  _emu->encoding[*][552] (wr@cyp.add 553) = - = 0
  Would denote that the first 8 scenarios (sce #1 to #8) pertain to slice 1 ,
  then the following 5 (sce #9 to #13) pertain to slice 2. Slice 3 (starting
  from the ... non-existent scenario #14) and slice 4 (0) would not be used. */
  vector<size_t> sceChange(_emu->getHwSliceCount());
  vector<uint32_t> _scenariosEncoding;
  newSceIndex.clear();
  size_t tempSceRunCount = 0;
  for ( size_t k = 0 ; k != sliceScenariosIds.size() ; ++k ){
    for ( size_t m = 0 ; m != sliceScenariosIds[k].size() ; ++m ){
      newSceIndex[ tempSceRunCount++ ] = sliceScenariosIds[k][m];
      _scenariosEncoding.push_back( stopStartIter.at(sliceScenariosIds[k][m]) );
      _scenariosEncoding.push_back( switchConf.at(sliceScenariosIds[k][m]) );
    }
    sceChange[k] = tempSceRunCount+1;
  }
  // Format sceChange to be of size 3 - append with '0' (zeros) when necessary
  if ( sceChange.size() > 3 )
    sceChange.erase( sceChange.begin()+3, sceChange.end() );
  else if ( sceChange.size() < 3 )
    for ( size_t k =  sceChange.size() ; k != 3 ; ++k )
      sceChange.push_back(0);

  // ----- DEBUG -----
//  for ( size_t k = 0 ; k != sceChange.size() ; ++k )
//    cout << "sceChange["<<k<<"]:" << sceChange[k] << endl;
//  for ( size_t k = 0 ; k != _scenariosEncoding.size() ; ++k )
//    cout << "_scenariosEncoding["<<k<<"]:" << _scenariosEncoding[k] << endl;
  // ----- DEBUG -----

  // ----- Modify encoding -----
  for ( size_t k = 0 ; k != _emu->getHwSliceCount() ; ++k ){

    // Scenario change options (sceChange is guarranteed to be of size 3)
    _emu->encoding[k][551] = static_cast<uint32_t>(sceChange[0]);
    _emu->encoding[k][552] = static_cast<uint32_t>(sceChange[1]);
    _emu->encoding[k][553] = static_cast<uint32_t>(sceChange[2]);
    _emu->encoding[k][554] = static_cast<uint32_t>(stopSample);
    encoder::stamp_NIOS_confirm( _emu->encoding[k][554] );
    // Clear start code for multi-sce run
    _emu->encoding[k][555] = static_cast<uint32_t>(0);
    // Number of scenarios
    _emu->encoding[k][559] = static_cast<uint32_t>(tempSceRunCount);

    // Multi-scenario specific encoding at the end of _emu->encoding[k]
    if ( _emu->encoding[k].size() > 560 )
      // Truncate _emu->encoding to 560 words (if larger)
      _emu->encoding[k].erase( _emu->encoding[k].begin()+560, _emu->encoding[k].end() );
    // Append proper
    _emu->encoding[k].insert( _emu->encoding[k].end(),
                         _scenariosEncoding.begin(), _scenariosEncoding.end() );
  }
  // Output sceRunCount
  sceRunCount = tempSceRunCount;

  return 0;
}

/* ---- Encode the scenarios
  _emu->encoding[*][551] (cyp.add 552) = sce start for slice 2
  _emu->encoding[*][552] (cyp.add 553) = sce start for slice 3
  _emu->encoding[*][553] (cyp.add 554) = sce start for slice 4
  _emu->encoding[*][554] (cyp.add 555) = NIOSconfirm + stopSample
  _emu->encoding[*][555] (wr @cyp.add 556) = 0 (clear start code for multi-sce run)
  - ... -
  _emu->encoding[*][559] (wr @cyp.add 560) = sce count

  Starting from cyp.add 561 the following scenario description block is appended
  to _emu->encoding[*][...]:
    enc add sce#          word
  ----------------------------
  [560] 561       maxFaultTime
  [561] 562          precision
  ----------------------------
  [562] 563    1    switchConf
  [563] 564    2    switchConf
  ...

  [661] 662  100    switchConf */
int TDEmulator::_encodeMultiCCT( vector<Scenario> const& scenarios,
                                 double precision,
                                 map<size_t,size_t>& newSceIndex,
                                 size_t& sceRunCount,
                                 vector<size_t>& scePerSlice,
                                 double& maxFault,
                                 double& stopTime ){
  // If the state of the TDEmulator is not at least EMU_STATE_PWSENCODED
  // then the function is not executed (returns non-zero)
  if ( _emu->state() < EMU_STATE_PWSENCODED )
    return -1;

  // Input argument validation
  if ( scenarios.size() == 0 )
    return 1;
  if ( precision < 0 )
    return 2;

  // ----- Determine precisionIt -----
  size_t intTimeStepOption = _getIntTStepOption();
  double intTimeStepValue = _int_tstep_values[intTimeStepOption];
  double precisionItD = precision/intTimeStepValue;
  uint32_t precisionIt = static_cast<uint32_t>(auxiliary::round(precisionItD));

  // ----- Determine scenario encoding info -----
  maxFault = -1.0;
  vector<vector<size_t> > sliceScenariosIds( _emu->getHwSliceCount() );
  vector<size_t> incompatibleScenariosIds;
  vector<unsigned int> stopStartIter( scenarios.size() );
  vector<unsigned int> switchConf( scenarios.size() );
  unsigned int stopSample; // stopSample is the number of samples requested to
                           // to be computed for each multi scenario run
  // NOTE: In the way stopSample is updated hereunder, it is overwritten always
  // by the stopSample corresponding to stop time of the last scenario in
  // 'scenarios'
  for ( size_t k = 0; k != scenarios.size() ; ++k ){

    // Skip inactive scenarios
    if ( !scenarios[k].status() ){
      // Well, it's not exactly incompatible but anyway :)
      incompatibleScenariosIds.push_back(k);
      continue;
    }
    // Check whether scenarios is compatible with the engine
    if ( !isEngineCompatible(scenarios[k]) ){
      incompatibleScenariosIds.push_back(k);
      continue;
    }
    // Check whether scenario is compatible with CCT analysis
    if ( !scenarios[k].checkCCTCompatibility() ){
      incompatibleScenariosIds.push_back(k);
      continue;
    }

    // Find maxFault - unless already found (maxFault>0)
    if ( maxFault < 0 ){
      Event tempEvent = scenarios[k].getEvent(1);
      maxFault = tempEvent.time();
    }

    // Encode scenario proper
    unsigned int sliceId_;
    unsigned int stopStartIter_, dampIter_, switchConf_;
    bool refittingPerformed_;
    int ans = _encodeScenarioBase(scenarios[k],
                                  sliceId_, stopStartIter_, dampIter_,
                                  switchConf_, stopSample, refittingPerformed_);
    stopTime = static_cast<double>(stopSample)*getTimeStep();

    if ( ans ){
      // If the encoding failed to encode the scenario then mark it as
      // incompatible and continue
      incompatibleScenariosIds.push_back(k);
      continue;
    }
    if ( refittingPerformed_ ){
      // If for the current scenario a refitting is required then mark is as
      // incompatible and continue. In multiple scenarios mode, no refitting is
      // allowed between different scenarios
      incompatibleScenariosIds.push_back(k);
      continue;
    }

    // Update stores
    sliceScenariosIds[sliceId_].push_back(k);
    stopStartIter[k] = stopStartIter_;
    switchConf[k] = switchConf_;
  }
  // Determine number of scenarios per slice
  scePerSlice.clear();
  scePerSlice.resize(sliceScenariosIds.size(),0);
  for ( size_t k = 0 ; k != sliceScenariosIds.size() ; ++k ){
    scePerSlice[k] = sliceScenariosIds[k].size();
  }

  // ----- Determine maxFaultIt -----
  if ( maxFault < 0 )
    // maxFault has not been succesfully determined so quit!
    return 3;
  double maxFaultItD = maxFault/intTimeStepValue;
  uint32_t maxFaultIt = static_cast<uint32_t>(auxiliary::round(maxFaultItD));

  /* Multi-scenarios are executed one after the other, with the perturbation
  being situated on different slices, in case of multi-slice emulator set-ups.
  'sceChange' holds the information when the perturbation info starts concerning
  a following board. There are 3 positions in the memory conf where the above is
  stored:
  _emu->encoding[*][551] (wr @cyp.add 552) = sce start for slice 2
  _emu->encoding[*][552] (wr @cyp.add 553) = sce start for slice 3
  _emu->encoding[*][553] (wr @cyp.add 554) = sce start for slice 4
  _emu->encoding[*][554] (wr @cyp.add 555) = NIOSconfirm + stopSample
  So scenarios starting with sceId (index count starting from 0!) stored in
  position [552] @553 should start executing their perturbation on slice #2, and
  so on. In case of a set-up with less than 4 boards, or of scenario sets that
  do not contain scenarios for all the boards, 'sceChange' for the first
  non-existing slice would be overall 'scenario count+1' (to signify a
  non-existing scenario, and thus the stop of the emulation) and '0' from then
  on.

  Eg. in a case of a two-slice setup with 13 scenarios to emulate:
  _emu->encoding[*][551] (wr@cyp.add 552) = sceChange[0] = 9
  _emu->encoding[*][552] (wr@cyp.add 553) = sceChange[1] = 14
  _emu->encoding[*][552] (wr@cyp.add 553) = - = 0
  Would denote that the first 8 scenarios (sce #1 to #8) pertain to slice 1 ,
  then the following 5 (sce #9 to #13) pertain to slice 2. Slice 3 (starting
  from the ... non-existent scenario #14) and slice 4 (0) would not be used. */
  vector<size_t> sceChange(_emu->getHwSliceCount());
  vector<uint32_t> _scenariosEncoding;
  newSceIndex.clear();
  size_t tempSceRunCount = 0;
  for ( size_t k = 0 ; k != sliceScenariosIds.size() ; ++k ){
    for ( size_t m = 0 ; m != sliceScenariosIds[k].size() ; ++m ){
      newSceIndex[ tempSceRunCount++ ] = sliceScenariosIds[k][m];
      _scenariosEncoding.push_back( switchConf.at(sliceScenariosIds[k][m]) );
    }
    sceChange[k] = tempSceRunCount+1;
  }
  // Format sceChange to be of size 3 - append with '0' (zeros) when necessary
  if ( sceChange.size() > 3 )
    sceChange.erase( sceChange.begin()+3, sceChange.end() );
  else if ( sceChange.size() < 3 )
    for ( size_t k =  sceChange.size() ; k != 3 ; ++k )
      sceChange.push_back(0);

  // ----- DEBUG -----
//  for ( size_t k = 0 ; k != sceChange.size() ; ++k )
//    cout << "sceChange["<<k<<"]:" << sceChange[k] << endl;
//  for ( size_t k = 0 ; k != _scenariosEncoding.size() ; ++k )
//    cout << "_scenariosEncoding["<<k<<"]:" << _scenariosEncoding[k] << endl;
  // ----- DEBUG -----

  // ----- Modify encoding -----
  for ( size_t k = 0 ; k != _emu->getHwSliceCount() ; ++k ){

    // Scenario change options (sceChange is guarranteed to be of size 3)
    _emu->encoding[k][551] = static_cast<uint32_t>(sceChange[0]);
    _emu->encoding[k][552] = static_cast<uint32_t>(sceChange[1]);
    _emu->encoding[k][553] = static_cast<uint32_t>(sceChange[2]);
    _emu->encoding[k][554] = static_cast<uint32_t>(stopSample);
    encoder::stamp_NIOS_confirm( _emu->encoding[k][554] );

    // Clear start code for multi-sce run
    _emu->encoding[k][555] = static_cast<uint32_t>(0);

    // Clear position [557] 558 - not used in multiCCT
    _emu->encoding[k][557] = static_cast<uint32_t>(0);

    // Number of scenarios
    _emu->encoding[k][559] = static_cast<uint32_t>(tempSceRunCount);

    // Multi-scenario specific encoding at the end of _emu->encoding[k]
    if ( _emu->encoding[k].size() > 560 )
      // Truncate _emu->encoding to 560 words (if larger)
      _emu->encoding[k].erase( _emu->encoding[k].begin()+560, _emu->encoding[k].end() );

    // At position [560] 561 push back maxFaultIt
    _emu->encoding[k].push_back(maxFaultIt);
    // At position [561] 562 push back precisionIt
    _emu->encoding[k].push_back(precisionIt);

    // Append _scenariosEncoding - containing switchConf info
    _emu->encoding[k].insert( _emu->encoding[k].end(),
                         _scenariosEncoding.begin(), _scenariosEncoding.end() );
  }
  sceRunCount = tempSceRunCount;

  return 0;
}


#if BUILD_NOSPIES

int TDEmulator::_getStartCode( TDResultIdentifier const& tdri,
                               size_t& sliceId,
                               unsigned int& startCode ) const{

  PwsMapperModelElement const* el;
  int tab, row, col;
  int pipelineId;
  unsigned int startCodeBase;
  int ans;

  Generator const* pGen;
  bool isInGenPipeline;
  Load const* pLoad;
  Pipeline const* pipe;

  unsigned int busExtId;
  int busIntId;
  bool genMapped = false;
  size_t genIntId;
  unsigned int genExtId;
  bool loadMapped = false;
  size_t loadIntId;
  unsigned int loadExtId;
  TDResultIdentifier newTdri;

  switch ( tdri.elementType ){

  case TDRESULTS_GENERATOR:
    el = _emu->mmd()->element( PWSMODELELEMENTTYPE_GEN, tdri.extId );
    tab = floor( static_cast<double>(el->tab)/2.0 );
    row = floor( static_cast<double>(el->row)/2.0 );
    col = floor( static_cast<double>(el->col)/2.0 );

    // Determine whether the gen is in the gen pipeline or in the const I load
    // pipeline
    ans = _pws->getGenerator( tdri.extId, pGen );
    if ( ans ) return 10;

    // Determine pipelineId accordingly
    if ( pGen->M() < GEN_MECHSTARTTIME_THRESHOLD ){
      pipe = &_emu->emuhw()->sliceSet[tab].dig.pipe_gen;
      isInGenPipeline = true;
    } else { // ( pGen->M() >= GEN_MECHSTARTTIME_THRESHOLD )
      pipe = &_emu->emuhw()->sliceSet[tab].dig.pipe_iload;
      isInGenPipeline = false;
    }
    pipelineId = pipe->search_element(row,col);
    if ( pipelineId < 0 )
      // TODO something better!
      return 11;

    switch ( tdri.variable ){
    case TDRESULTS_ANGLE:
      if ( isInGenPipeline ) startCodeBase = 1111;
      else return 12; // gen delta for gens in iloadpipe does not need a TD run
      break;
    case TDRESULTS_SPEED:
      if ( isInGenPipeline ) startCodeBase = 1119;
      else return 13; // gen speed for gens in iloadpipe does not need a TD run
      break;
    case TDRESULTS_ACCELERATING_POWER:
      if ( isInGenPipeline ) startCodeBase = 1127;
      else return 14;  // gen Pacc for gens in iloadpipe does not need a TD run
      break;
    case TDRESULTS_VOLTAGE_REAL:
      if ( isInGenPipeline ) startCodeBase = 1135;
      else startCodeBase = 1175;
      break;
    case TDRESULTS_VOLTAGE_IMAG:
      if ( isInGenPipeline ) startCodeBase = 1135;
      else startCodeBase = 1175;
      break;
    case TDRESULTS_CURRENT_REAL:
      if ( isInGenPipeline ) startCodeBase = 1143;
      else return 15; // Re{I} for constI load pipeline does not need a TD run
      break;
    case TDRESULTS_CURRENT_IMAG:
      if ( isInGenPipeline ) startCodeBase = 1143;
      else return 16; // Im{I} for constI load pipeline does not need a TD run
      break;
    default:
      return 2;
    }
    break;

  case TDRESULTS_LOAD:
    el = _emu->mmd()->element( PWSMODELELEMENTTYPE_LOAD, tdri.extId );
    tab = floor( static_cast<double>(el->tab)/2.0 );
    row = floor( static_cast<double>(el->row)/2.0 );
    col = floor( static_cast<double>(el->col)/2.0 );

    // Determine whether the load is a constant current or a constant
    // impedance one
    ans = _pws->getLoad(tdri.extId,pLoad);
    if ( ans )
      // TODO something better!
      return 3;

    // Determine pipelineId accordingly
    switch ( pLoad->type() ){
    case LOADTYPE_CONSTZ:
      pipe = &_emu->emuhw()->sliceSet[tab].dig.pipe_zload;
      break;
    case LOADTYPE_CONSTI:
      pipe = &_emu->emuhw()->sliceSet[tab].dig.pipe_iload;
      break;
    case LOADTYPE_CONSTP:
    default:
      // TODO something better!
      return 4;
    }
    pipelineId = pipe->search_element(row,col);
    if ( pipelineId < 0 )
      // TODO something better!
      return 5;


    switch ( tdri.variable ){
    case TDRESULTS_VOLTAGE_REAL:
      switch ( pLoad->type() ){
      case LOADTYPE_CONSTZ:
        startCodeBase = 1151;
        break;
      case LOADTYPE_CONSTI:
        startCodeBase = 1175;
        break;
      case LOADTYPE_CONSTP:
      default:
        // TODO something better!
        return 6;
      }
      break;

    case TDRESULTS_VOLTAGE_IMAG:
      switch ( pLoad->type() ){
      case LOADTYPE_CONSTZ:
        startCodeBase = 1151;
        break;
      case LOADTYPE_CONSTI:
        startCodeBase = 1175;
        break;
      case LOADTYPE_CONSTP:
      default:
        // TODO something better!
        return 7;
      }
      break;

    case TDRESULTS_CURRENT_REAL:
      switch ( pLoad->type() ){
      case LOADTYPE_CONSTZ:
        startCodeBase = 1199;
        break;
      case LOADTYPE_CONSTI:
      case LOADTYPE_CONSTP:
      default:
        // TODO something better!
        return 8;
      }
      break;

    case TDRESULTS_CURRENT_IMAG:
      switch ( pLoad->type() ){
      case LOADTYPE_CONSTZ:
        startCodeBase = 1199;
        break;
      case LOADTYPE_CONSTI:
      case LOADTYPE_CONSTP:
      default:
        // TODO something better!
        return 9;
      }
      break;

    default:
      return 10;
    }
    break;

  case TDRESULTS_BUS:

    // Find whether a generator or a load is mapped to the bus in question
    busExtId = tdri.extId;
    busIntId = _pws->getBus_intId( busExtId );
    if ( busIntId < 0)
      // Bus does not exist in the bus set! Should never happen
      return 11;
    if ( _pws->busGenMap().at( busIntId ).size() == 1 ){
      genMapped = true;
      genIntId = *_pws->busGenMap().at( busIntId ).begin();
      genExtId = _pws->getGen_extId( genIntId );
    } else if ( _pws->busLoadMap().at( busIntId ).size() == 1 ){
      loadMapped = true;
      loadIntId = *_pws->busLoadMap().at( busIntId ).begin();
      loadExtId = _pws->getLoad_extId( loadIntId );
    }
    if ( !genMapped && !loadMapped )
      // No observable (pipeline) element is mapped onto the node
      return 12;

    if ( genMapped ){
      newTdri.elementType = TDRESULTS_GENERATOR;
      newTdri.extId = genExtId;
    } else if ( loadMapped) {
      newTdri.elementType = TDRESULTS_LOAD;
      newTdri.extId = loadExtId;
    }
    newTdri.variable = tdri.variable;
    return _getStartCode( newTdri, sliceId, startCode );
    break;

  default:
    return 11;
  }

  // Write results to output arguments
  sliceId = tab;
  startCode = startCodeBase + pipelineId;

  return 0;
}

#else

int TDEmulator::_getStartCode( TDResultIdentifier const& tdri,
                               size_t& sliceId,
                               unsigned int& startCode ) const{

  PwsMapperModelElement const* el;
  int tab, row, col;
  int pipelineId;
  unsigned int startCodeBase(NEUTRAL_STARTCODE); // startCodeBase for empty run
  int ans;

  Generator const* pGen;
  bool isInGenPipeline;
  Load const* pLoad;
  Pipeline const* pipe;

  unsigned int busExtId;
  int busIntId;
  bool genMapped = false;
  size_t genIntId;
  unsigned int genExtId;
  bool loadMapped = false;
  size_t loadIntId;
  unsigned int loadExtId;
  TDResultIdentifier newTdri;

  switch ( tdri.elementType ){

  case TDRESULTS_GENERATOR:
    el = _emu->mmd()->element( PWSMODELELEMENTTYPE_GEN, tdri.extId );
    tab = floor( static_cast<double>(el->tab)/2.0 );
    row = floor( static_cast<double>(el->row)/2.0 );
    col = floor( static_cast<double>(el->col)/2.0 );

    // Determine whether the gen is in the gen pipeline or in the const I load
    // pipeline
    ans = _pws->getGenerator( tdri.extId, pGen );
    if ( ans ) return 10;

    // Determine pipelineId accordingly
    if ( pGen->M() < GEN_MECHSTARTTIME_THRESHOLD ){
      pipe = &_emu->emuhw()->sliceSet[tab].dig.pipe_gen;
      isInGenPipeline = true;
    } else { // ( pGen->M() >= GEN_MECHSTARTTIME_THRESHOLD )
      pipe = &_emu->emuhw()->sliceSet[tab].dig.pipe_iload;
      isInGenPipeline = false;
    }
    pipelineId = pipe->search_element(row,col);
    if ( pipelineId < 0 )
      // TODO something better!
      return 11;

    switch ( tdri.variable ){
    case TDRESULTS_ANGLE:
      if ( isInGenPipeline ) // gen angles are handled by the spies; so leave
        {;}                  // startCodeBase as is;
      else return 12; // gen delta for gens in iloadpipe does not need a TD run
      break;
    case TDRESULTS_SPEED:
      if ( isInGenPipeline ) // gen speeds are handled by the spies; so leave
        {;}                  // startCodeBase as is;
      else return 13; // gen speed for gens in iloadpipe does not need a TD run
      break;
    case TDRESULTS_ACCELERATING_POWER:
      if ( isInGenPipeline ) startCodeBase = 1127;
      else return 14;  // gen Pacc for gens in iloadpipe does not need a TD run
      break;
    case TDRESULTS_VOLTAGE_REAL:
      if ( isInGenPipeline ) startCodeBase = 1135;
      else startCodeBase = 1175;
      break;
    case TDRESULTS_VOLTAGE_IMAG:
      if ( isInGenPipeline ) startCodeBase = 1135;
      else startCodeBase = 1175;
      break;
    case TDRESULTS_CURRENT_REAL:
      if ( isInGenPipeline ) startCodeBase = 1143;
      else return 15; // Re{I} for constI load pipeline does not need a TD run
      break;
    case TDRESULTS_CURRENT_IMAG:
      if ( isInGenPipeline ) startCodeBase = 1143;
      else return 16; // Im{I} for constI load pipeline does not need a TD run
      break;
    default:
      return 2;
    }
    break;

  case TDRESULTS_LOAD:
    el = _emu->mmd()->element( PWSMODELELEMENTTYPE_LOAD, tdri.extId );
    tab = floor( static_cast<double>(el->tab)/2.0 );
    row = floor( static_cast<double>(el->row)/2.0 );
    col = floor( static_cast<double>(el->col)/2.0 );

    // Determine whether the load is a constant current or a constant
    // impedance one
    ans = _pws->getLoad(tdri.extId,pLoad);
    if ( ans )
      // TODO something better!
      return 3;

    // Determine pipelineId accordingly
    switch ( pLoad->type() ){
    case LOADTYPE_CONSTZ:
      pipe = &_emu->emuhw()->sliceSet[tab].dig.pipe_zload;
      break;
    case LOADTYPE_CONSTI:
      pipe = &_emu->emuhw()->sliceSet[tab].dig.pipe_iload;
      break;
    case LOADTYPE_CONSTP:
    default:
      // TODO something better!
      return 4;
    }
    pipelineId = pipe->search_element(row,col);
    if ( pipelineId < 0 )
      // TODO something better!
      return 5;


    switch ( tdri.variable ){
    case TDRESULTS_VOLTAGE_REAL:
      switch ( pLoad->type() ){
      case LOADTYPE_CONSTZ:
        startCodeBase = 1151;
        break;
      case LOADTYPE_CONSTI:
        startCodeBase = 1175;
        break;
      case LOADTYPE_CONSTP:
      default:
        // TODO something better!
        return 6;
      }
      break;

    case TDRESULTS_VOLTAGE_IMAG:
      switch ( pLoad->type() ){
      case LOADTYPE_CONSTZ:
        startCodeBase = 1151;
        break;
      case LOADTYPE_CONSTI:
        startCodeBase = 1175;
        break;
      case LOADTYPE_CONSTP:
      default:
        // TODO something better!
        return 7;
      }
      break;

    case TDRESULTS_CURRENT_REAL:
      switch ( pLoad->type() ){
      case LOADTYPE_CONSTZ:
        startCodeBase = 1199;
        break;
      case LOADTYPE_CONSTI:
      case LOADTYPE_CONSTP:
      default:
        // TODO something better!
        return 8;
      }
      break;

    case TDRESULTS_CURRENT_IMAG:
      switch ( pLoad->type() ){
      case LOADTYPE_CONSTZ:
        startCodeBase = 1199;
        break;
      case LOADTYPE_CONSTI:
      case LOADTYPE_CONSTP:
      default:
        // TODO something better!
        return 9;
      }
      break;

    default:
      return 10;
    }
    break;

  case TDRESULTS_BUS:

    // Find whether a generator or a load is mapped to the bus in question
    busExtId = tdri.extId;
    busIntId = _pws->getBus_intId( busExtId );
    if ( busIntId < 0)
      // Bus does not exist in the bus set! Should never happen
      return 11;
    if ( _pws->getBusGenMap().at( busIntId ).size() == 1 ){
      genMapped = true;
      genIntId = *_pws->getBusGenMap().at( busIntId ).begin();
      genExtId = _pws->getGen_extId( genIntId );
    } else if ( _pws->getBusLoadMap().at( busIntId ).size() == 1 ){
      loadMapped = true;
      loadIntId = *_pws->getBusLoadMap().at( busIntId ).begin();
      loadExtId = _pws->getLoad_extId( loadIntId );
    }
    if ( !genMapped && !loadMapped )
      // No observable (pipeline) element is mapped onto the node
      return 12;

    if ( genMapped ){
      newTdri.elementType = TDRESULTS_GENERATOR;
      newTdri.extId = genExtId;
    } else if ( loadMapped) {
      newTdri.elementType = TDRESULTS_LOAD;
      newTdri.extId = loadExtId;
    }
    newTdri.variable = tdri.variable;
    return _getStartCode( newTdri, sliceId, startCode );
    break;

  default:
    return 11;
  }

  // Write results to output arguments
  sliceId = tab;
  if ( startCodeBase == NEUTRAL_STARTCODE ){
    // results to be handled by spies - no pipelineId offset required
    startCode = startCodeBase;
  } else {
    // results to be handled by the NIOS - pipelineId offset required
    startCode = startCodeBase + pipelineId;
  }

  return 0;
}

#endif // BUILD_NOSPIES

int TDEmulator::_getSpyCode( TDResultIdentifier const& tdri,
                             unsigned int& spyCode ) const{

  switch ( tdri.elementType ){

  case TDRESULTS_GENERATOR:{
    PwsMapperModelElement const* el
        = _emu->mmd()->element( PWSMODELELEMENTTYPE_GEN, tdri.extId );
    int tab = floor( static_cast<double>(el->tab)/2.0 );
    int row = floor( static_cast<double>(el->row)/2.0 );
    int col = floor( static_cast<double>(el->col)/2.0 );

    // Determine whether the gen is in the gen pipeline or in the const I load
    // pipeline
    Generator const* pGen;
    int ans = _pws->getGenerator( tdri.extId, pGen );
    if ( ans ) return 10;

    // Determine pipelineId accordingly
    bool isInGenPipeline;
    Pipeline const* pipe;
    if ( pGen->M() < GEN_MECHSTARTTIME_THRESHOLD ){
      pipe = &_emu->emuhw()->sliceSet[tab].dig.pipe_gen;
      isInGenPipeline = true;
    } else { // ( pGen->M() >= GEN_MECHSTARTTIME_THRESHOLD )
      pipe = &_emu->emuhw()->sliceSet[tab].dig.pipe_iload;
      isInGenPipeline = false;
    }
    int pipelineId = pipe->search_element(row,col);
    if ( pipelineId < 0 )
      // TODO something better!
      return 11;

    switch ( tdri.variable ){
    case TDRESULTS_ANGLE:
      if ( isInGenPipeline )
        spyCode = 2 + 2*pipelineId;
      else return 12; // gen delta for gens in iloadpipe does not need a TD run
      break;
    case TDRESULTS_SPEED:
      if ( isInGenPipeline )
        spyCode = 32 + 2*pipelineId;
      else return 13; // gen speed for gens in iloadpipe does not need a TD run
      break;
    default:
      return 14;
    }
    break;}
  default:
    return 15;
  }

  return 0;
}

// stopCodes are given with sliceId indexing!
int TDEmulator::_waitForStopCodes( vector<int> const& stopCodes,
                                   double timeout_ ){

  if ( stopCodes.size() != _emu->getHwSliceCount() )
    return 1;

  // If timeout negative then default to 5 seconds
  double timeout = (timeout_>0) ? timeout_ : 5.0 ;

  // Retrieve device ids for slices
  int ans;
  size_t k; // counter
  vector<int> devId;
  for( k = 0 ; k != _emu->getHwSliceCount(); ++k ){
    // Limit checking
    devId.push_back( _emu->sliceDeviceMap(k) );
    if ( devId[k] < 0 || devId[k] >= static_cast<int>(_emu->getUSBDevicesCount()) )
      return 2;
  }

  // Allocate result variables
  vector<uint32_t> readBuffer;
  vector<bool> stopCodesReady(devId.size(), false);
  // stopCodesReady flag set to true for slices with a negative requested
  // stopCode (can't be). Subsequent tests for those slices are to be skipped
  for ( k = 0 ; k != _emu->getHwSliceCount() ; ++k )
    if ( stopCodes[k] < 0 )
      stopCodesReady[k] = true;
  bool allStopCodesReady; // global ready flag

  // Set a time threshold of 'timeout' seconds for the operation
  double elapsedTime = 0;
  PrecisionTimer timer; // counts in seconds
  timer.Start();

  vector<uint32_t> debugBuffer;
  while ( elapsedTime < timeout ){
    // Read stop code for each slice and update stopCodesReady accordingly
    for ( k = 0 ; k != _emu->getHwSliceCount(); ++k ){
      // Skip for read slices
      if ( stopCodesReady[k] ) continue;

      ans = _emu->usbRead(devId[k], 557, 1, readBuffer);
//      // DEBUG
//      cout << "DEBUG >> 0557: " << readBuffer[0] << endl;
//      _emu->usbRead(devId[k], 1000, 5, debugBuffer);
//      cout << "DEBUG >> 1000: " << debugBuffer[0] << endl;
//      cout << "DEBUG >> 1001: " << debugBuffer[1] << endl;
//      cout << "DEBUG >> 1002: " << debugBuffer[2] << endl;
//      cout << "DEBUG >> 1003: " << debugBuffer[3] << endl;
//      cout << "DEBUG >> 1004: " << debugBuffer[4] << endl;
//      // END OF DEBUG
      if ( ans ) return 2;
      if ( stopCodes[k] < 0 ) continue; // skip test for negative 'stopCodes'
      if ( readBuffer[0] == static_cast<uint32_t>(stopCodes[k]) )
        stopCodesReady[k] = true;
    }

    // Check whether results from all slices are ready
    allStopCodesReady = true;
    for ( k = 0 ; k != stopCodesReady.size() ; ++k ){
      if ( !stopCodesReady[k] ){
        allStopCodesReady = false;
        break;
      }
    }
    if ( allStopCodesReady )
      return 0;

    // Update timer
    elapsedTime += timer.Stop();
    timer.Start();
  }
  return 1;
}

// runCodes are given with sliceId indexing!
int TDEmulator::_waitForResults( vector<uint32_t> const& runCodes,
                                 double timeout ){

  // Prepare stopCodes vector to wait for
  vector<int> stopCodes( runCodes.size() );
  // Slices for which the runCode is 6666 (HiZ) are skipped from the test by
  // asking for a negative stopCode
  for ( size_t k = 0 ; k != stopCodes.size() ; ++k )
    stopCodes[k] = (runCodes[k]!=6666) ? 9999 : -1 ;

  return _waitForStopCodes( stopCodes, timeout );
}

// According to Guillaume Lanz as of July 2012 this takes approximately 200us
// from the FPGA to copmlete
int TDEmulator::_waitForSpy( size_t sliceId,
                             uint32_t spyCode,
                             double timeout ){
  vector<int> stopCodes( _emu->getHwSliceCount(), -1 );
  stopCodes[sliceId] = spyCode;
  return _waitForStopCodes( stopCodes, timeout );
}

#define ANGLERES_FOLDLIMIT 2.0

void TDEmulator::_parseGenAngles( vector<uint32_t> const& resultsRead,
                                  vector<double>& ys, bool unfold ){

  ys.clear();

  // Vector a contains signed 0x0000Q2.20 format numbers
  // To convert a number from Qm.n format to floating point:
  //   - Convert the number to floating point as if it were an integer
  //   - Multiply by 2-n
  unsigned int bit22 = (1<<21);
  unsigned int signExpansion = ~( (1<<22) - 1 );
  double last_res(0.0);
  double curr_res;
  double angle;
  double base_angle = 0.0;
  for ( size_t k = 0 ; k != resultsRead.size() ; ++k ){
    int32_t temp_int32 = static_cast<int32_t>( resultsRead[k] );
    if ( temp_int32 & bit22 ) // get sign
      temp_int32 |= signExpansion;
    curr_res = static_cast<double>(temp_int32) / static_cast<double>(1<<20);
    if ( unfold ){
      if ( (curr_res-last_res) <= -ANGLERES_FOLDLIMIT )
        // Positive fold (eg. from last_res=1.9 -> curr_res=-1.9)
        base_angle += 2*ANGLERES_FOLDLIMIT;
      else if ( (curr_res-last_res) >= ANGLERES_FOLDLIMIT )
        // Negative fold (eg. from last_res=-1.9 -> curr_res=1.9)
        base_angle -= 2*ANGLERES_FOLDLIMIT;
      last_res = curr_res;
      curr_res += base_angle;
    }
    angle = curr_res * M_PI / 2;
    ys.push_back( angle );
  }

  return;
}

void TDEmulator::_parseGenSpeeds( vector<uint32_t> const& resultsRead,
                                  vector<double>& ys ){

  ys.clear();

  // Vector a contains signed 0x0000Q2.12 format numbers
  // To convert a number from Qm.n format to floating point:
  //   - Convert the number to floating point as if it were an integer
  //   - Multiply by 2-n
  unsigned int bit14 = (1<<13);
  unsigned int signExpansion = ~( (1<<14) - 1 );
  for ( size_t k = 0 ; k != resultsRead.size() ; ++k ){
    int32_t temp_int32 = static_cast<int32_t>( resultsRead[k] );
    if ( temp_int32 & bit14 ) // get sign
      temp_int32 |= signExpansion;
    double temp_double = static_cast<double>(temp_int32) / static_cast<double>(1<<12);
    ys.push_back( temp_double );
  }

  return;
}

void TDEmulator::_parseGenPmec( vector<uint32_t> const& resultsRead,
                                vector<double>& ys ){

  ys.clear();

  // Vector a contains signed 0x0000Q2.13 format numbers
  // To convert a number from Qm.n format to floating point:
  //   - Convert the number to floating point as if it were an integer
  //   - Multiply by 2-n
  unsigned int bit18 = (1<<17);
  unsigned int signExpansion = ~( (1<<18) - 1 );
  for ( size_t k = 0 ; k != resultsRead.size() ; ++k ){
    int32_t temp_int32 = static_cast<int32_t>( resultsRead[k] );
    if ( temp_int32 & bit18 ) // get sign
      temp_int32 |= signExpansion;
    double temp_double = static_cast<double>(temp_int32) / static_cast<double>(1<<13);
    ys.push_back( temp_double );
  }

  return;
}

void TDEmulator::_parseVoltage( vector<uint32_t> const& resultsRead,
                                vector<double>& ys1, vector<double>& ys2 ){

  ys1.clear();
  ys2.clear();

  // Vector a contains signed 0x0000Q2.14 format numbers
  // To convert a number from Qm.n format to floating point:
  //   - Convert the number to floating point as if it were an integer
  //   - Multiply by 2-n
  unsigned int bit16 = (1<<15);
  unsigned int signExpansionL = ~( (1<<16) - 1 );
  for ( size_t k = 0 ; k != resultsRead.size() ; ++k ){
    int32_t LSB = static_cast<int32_t>( resultsRead[k] & 0x0000FFFF );
    int32_t MSB = static_cast<int32_t>( resultsRead[k] & 0xFFFF0000 );
    if ( LSB & bit16 ) // get sign
      LSB |= signExpansionL;
    double temp_double = static_cast<double>(LSB) / static_cast<double>(1<<14);
    ys1.push_back( temp_double );
    temp_double = static_cast<double>(MSB) / static_cast<double>(1<<30);
    ys2.push_back( temp_double );
  }

  return;
}

void TDEmulator::_parseVoltage( vector<uint32_t> const& resultsRead,
                                bool realPart,
                                vector<double>& ys){
  ys.clear();

  // Vector a contains signed 0x0000Q2.14 format numbers
  // To convert a number from Qm.n format to floating point:
  //   - Convert the number to floating point as if it were an integer
  //   - Multiply by 2-n
  unsigned int bit16 = (1<<15);
  unsigned int signExpansionL = ~( (1<<16) - 1 );
  for ( size_t k = 0 ; k != resultsRead.size() ; ++k ){
    int32_t LSB = static_cast<int32_t>( resultsRead[k] & 0x0000FFFF );
    int32_t MSB = static_cast<int32_t>( resultsRead[k] & 0xFFFF0000 );
    if ( LSB & bit16 ) // get sign
      LSB |= signExpansionL;

    double temp_double;
    if ( realPart )
      temp_double = static_cast<double>(LSB) / static_cast<double>(1<<14);
    else
      temp_double = static_cast<double>(MSB) / static_cast<double>(1<<30);
    ys.push_back( temp_double );
  }

  return;
}

void TDEmulator::_parseCurrent( vector<uint32_t> const& resultsRead,
                                vector<double>& ys1, vector<double>& ys2 ){

  ys1.clear();
  ys2.clear();

  // Vector a contains signed 0x0000Q2.11 format numbers
  // To convert a number from Qm.n format to floating point:
  //   - Convert the number to floating point as if it were an integer
  //   - Multiply by 2-n
  unsigned int bit16 = (1<<15);
  unsigned int signExpansionL = ~( (1<<16) - 1 );
  for ( size_t k = 0 ; k != resultsRead.size() ; ++k ){
    int32_t LSB = static_cast<int32_t>( resultsRead[k] & 0x0000FFFF );
    int32_t MSB = static_cast<int32_t>( resultsRead[k] & 0xFFFF0000 );
    if ( LSB & bit16 ) // get sign
      LSB |= signExpansionL;

    double temp_double = static_cast<double>(LSB) / static_cast<double>(1<<11);
    ys1.push_back( temp_double );
    temp_double = static_cast<double>(MSB) / static_cast<double>(1<<27);
    ys2.push_back( temp_double );
  }

  return;
}

void TDEmulator::_parseCurrent( vector<uint32_t> const& resultsRead,
                                bool realPart,
                                vector<double>& ys ){

  ys.clear();

  // Vector a contains signed 0x0000Q2.11 format numbers
  // To convert a number from Qm.n format to floating point:
  //   - Convert the number to floating point as if it were an integer
  //   - Multiply by 2-n
  unsigned int bit16 = (1<<15);
  unsigned int signExpansionL = ~( (1<<16) - 1 );
  for ( size_t k = 0 ; k != resultsRead.size() ; ++k ){
    int32_t LSB = static_cast<int32_t>( resultsRead[k] & 0x0000FFFF );
    int32_t MSB = static_cast<int32_t>( resultsRead[k] & 0xFFFF0000 );
    if ( LSB & bit16 ) // get sign
      LSB |= signExpansionL;

    double temp_double;
    if ( realPart )
      temp_double = static_cast<double>(LSB) / static_cast<double>(1<<11);
    else
      temp_double = static_cast<double>(MSB) / static_cast<double>(1<<27);
    ys.push_back( temp_double );
  }

  return;
}
