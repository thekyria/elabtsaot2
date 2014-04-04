
#include "tdengine.h"
using namespace elabtsaot;

#include "precisiontimer.h"
#include "scenario.h"
#include "tdresultsidentifier.h"
#include "powersystem.h"
#include "tdresults.h"
//#include "auxiliary.h"

//#include <map>
using std::map;
#include <iostream>
using std::cout;
using std::endl;
//#include <vector>
using std::vector;
#include <limits>
using std::numeric_limits;

#include <boost/timer/timer.hpp>

TDEngine::TDEngine( Powersystem const* pws,
                    std::string const& description,
                    double timeStep, Logger* log ) :
  _pws(pws), _TDdescription(description), _timeStep(timeStep), _TDlog(log){}

void TDEngine::setLogger(Logger* log){ _TDlog = log; }

std::string TDEngine::getDescription() const{ return _TDdescription; }

bool TDEngine::isEngineCompatible(Scenario const& sce) const{
  return do_isEngineCompatible(sce);
}

int TDEngine::setTimeStep( double seconds, double* pMismatch){
  return do_setTimeStep(seconds, pMismatch);
}

double TDEngine::getTimeStep() const{ return _timeStep; }

int TDEngine::simulate( Scenario const& sce, TDResults& res ){
  boost::timer::auto_cpu_timer t; // when t goes out of scope it prints timing info
  int ans = do_simulate(sce, res);
  return ans;
}

int TDEngine::checkStability( std::vector<Scenario> const& scenarios,
         std::vector<std::map<unsigned int,bool> >& genStable,
         std::vector<std::map<unsigned int,double> >* genInstabilityTime){
  int ans = do_checkStability(scenarios, genStable, genInstabilityTime);
  return ans;
}

int TDEngine::checkCCT(std::vector<Scenario> const& scenarios,
                       double precision,
                       std::vector<double>& cctMin, std::vector<double>& cctMax){
  int ans = do_checkCCT(scenarios, precision, cctMin, cctMax);
  return ans;
}

Powersystem const* TDEngine::getPws() const{ return do_getPws(); }

int TDEngine::_checkStability( Scenario const& sce,
                               int criterion, double threshold,
                               bool& pwsStable ){

  if ( !sce.checkPwsCompatibility(*getPws()) )
    return 1;
  if ( !isEngineCompatible(sce) )
    return 2;

  // Ask for generator angles
  vector<TDResultIdentifier> tdris;
  TDResultIdentifier tempTdri;
  tempTdri.elementType = TDRESULTS_GENERATOR;
  tempTdri.variable = TDRESULTS_ANGLE;
  for ( size_t k = 0 ; k != getPws()->getGenCount() ; ++k ){
    tempTdri.extId = getPws()->getGenerator(k)->extId;
    tdris.push_back( tempTdri );
  }
  TDResults res(tdris);

  // Simulate scenario
  int ans = simulate( sce, res );
  if ( ans ) return 2;

  // Check stability by checking generator angles
  bool tempIsStable;
  ans = _checkStability( res, criterion, threshold, tempIsStable );
  if ( ans ) return 3;
  pwsStable = tempIsStable;

  return 0;
}

int TDEngine::_checkStability( Scenario const& sce,
                               int criterion, double threshold,
                               map<unsigned int,bool>& genStable ){

  if ( !sce.checkPwsCompatibility(*getPws()) )
    return 1;
  if ( !isEngineCompatible(sce) )
    return 2;

  // Ask for generator angles
  vector<TDResultIdentifier> tdris;
  TDResultIdentifier tempTdri;
  tempTdri.elementType = TDRESULTS_GENERATOR;
  tempTdri.variable = TDRESULTS_ANGLE;
  for ( size_t k = 0 ; k != getPws()->getGenCount() ; ++k ){
    tempTdri.extId = getPws()->getGenerator(k)->extId;
    tdris.push_back(tempTdri);
  }
  TDResults res(tdris);

  // Simulate scenario
  int ans = simulate( sce, res );
  if ( ans ) return 2;

  // Check stability by checking at generator angles
  ans = _checkStability( res, criterion, threshold, genStable );

  if ( ans ) return 3;
  return 0;
}

int TDEngine::_checkStability( TDResults const& res,
                               int criterion, double threshold,
                               bool& pwsStable ){

  // Retrieve res vectors
  vector<bool> dataExists = res.dataExists();
  vector<TDResultIdentifier> identifiers = res.identifiers();
  vector<vector<double> > data = res.data();

  // Identify generator angles results in 'res'
  bool angleResultsFound = false;
  vector<bool> isAngle( dataExists.size(), false );
  for ( size_t k = 0 ; k != dataExists.size() ; ++k ){
    if ( !dataExists[k] )
      continue;
    if (    identifiers[k].elementType == TDRESULTS_GENERATOR
         && identifiers[k].variable    == TDRESULTS_ANGLE ){
      if ( !angleResultsFound )
        angleResultsFound = true;
      isAngle[k] = true;
    }
  }

  // Form generator angles vector 'deltas'
  vector<vector<double> > deltas;
  for ( size_t k = 0 ; k != data.size() ; ++k )
    if ( isAngle[k] )
      deltas.push_back( data[k] );

  // Check stability
  bool tempIsStable = _findAnglesOverallStability( deltas, criterion, threshold );

  // Output results
  if ( angleResultsFound ){
    pwsStable = tempIsStable;
    return 0;
  } else {
    return 1;
  }
  return 0; // :)
}

// Returns the stability flags for the gens of the powersystem for the input
// time domain results
// Gen stable contains a map of the extIds of gens in res and the respective
// bool stability flags
// eg. 2->false; 3->true; 6->true; 31->true
int TDEngine::_checkStability( TDResults const& res,
                               int criterion, double threshold,
                               map<unsigned int,bool>& genStable ){

  // Retrieve res vectors
  vector<bool> dataExists = res.dataExists();
  vector<TDResultIdentifier> identifiers = res.identifiers();
  vector<vector<double> > data = res.data();

  // Identify generator angles results in 'res'
  bool angleResultsFound = false;
  vector<bool> isAngle( dataExists.size(), false );
  vector<size_t> reIndexing;
  for ( size_t k = 0 ; k != dataExists.size() ; ++k ){
    if ( !dataExists[k] ) continue;
    if (    identifiers[k].elementType == TDRESULTS_GENERATOR
         && identifiers[k].variable    == TDRESULTS_ANGLE ){
      if ( !angleResultsFound )
        angleResultsFound = true;
      isAngle[k] = true;
      reIndexing.push_back(k);
    }
  }
  // No analysis can be performed unless there are generator angle results in
  // the res vector
  if ( !angleResultsFound )
    return 1;
  // Form generator angles vector 'deltas'
  vector<vector<double> > deltas;
  for ( size_t k = 0 ; k != data.size() ; ++k )
    if ( isAngle[k] )
      deltas.push_back( data[k] );

  // Perform the check proper
  vector<bool> tempGenStable;
  _findAnglesStability(deltas, criterion, threshold, tempGenStable);

  // Update the output argument 'genStable'
  genStable.clear();
  for ( size_t k = 0 ; k != reIndexing.size() ; ++k )
    genStable[ identifiers.at(reIndexing[k]).extId ] = tempGenStable[k];

  return 0;
}

int TDEngine::_findAnglesStability( vector<vector<double> > const& genAngles,
                                    int criterion,
                                    double threshold,
                                    vector<bool>& genStabilityFlag ){

  genStabilityFlag.clear();
  genStabilityFlag.resize( genAngles.size(), true );

  // Check for trivial stability
  if ( genAngles.size() == 0 )
    return 0;

  // Each of the following code blocks needs to update 'genStabilityFlag' taking
  // into account the 'genAngles' input 'threshold' parameter
  // TODO: consider seperating in functions
  switch ( criterion ){
  case TDE_STABCRITERION_ANGLEDEVIATION:{
    // Find the angle trajectory of the least advanced generator for each time
    // instant
    // Notice: Same size for each of generator angle vectors is assumed
    // i.e. genAngles[g].size() == const, for all g.
    vector<double> deltaLA; // angle of the Least Advanced generator
    for ( size_t t = 0 ; t != genAngles[0].size(); ++t ){
      double deltaMin = numeric_limits<double>::max(); // angle of the LA gen for
                                                       // this time step
      for ( size_t g = 0 ; g != genAngles.size(); ++g )
        if ( genAngles[g][t] < deltaMin )
          deltaMin = genAngles[g][t];
      deltaLA.push_back( deltaMin );
    }

    size_t t, g;
    for ( t = 0 ; t != genAngles[0].size(); ++t ){
      // Check the angle gap between each gen and the LA one
      for ( g = 0 ; g != genAngles.size(); ++g ){
        // Skip test for unstable generators; already have been identified as such
        if (!genStabilityFlag[g]) continue;
        // Check angles against least advanced gen
        if ( std::abs(genAngles[g][t]-deltaLA[t]) > threshold )
          genStabilityFlag[g] = false;
      }
      // Check termination condition: all gens have been identified as unstable
      bool allUnstable = true;
      for ( g = 0; g != genAngles.size() ; ++g ){
        if ( genStabilityFlag[g] ){
          allUnstable = false;
          break;
        }
      }
      if (allUnstable) break;
    }
    break;}

  case TDE_STABCRITERION_ANGLEDIFFFROMSLACK:{
    size_t t, g;
    for ( t = 0 ; t != genAngles[0].size(); ++t ){
      // Check the angle gap between each gen and the slack.
      // WARNING! The slack angle trajectory is assumed to be given at position
      // 0: genAngles[0][...]
      for ( g = 1 ; g != genAngles.size() ; ++g ){
        // Skip test for unstable generators; already have been identified as such
        if ( !genStabilityFlag[g] ) continue;
        // Check angles against slack
        if ( std::abs(genAngles[g][t]-genAngles[0][t]) > threshold )
          genStabilityFlag[g] = false;
      }
      // Check termination condition: all gens have been identified as unstable
      bool allUnstable = true;
      for ( g = 0; g != genAngles.size() ; ++g ){
        if ( genStabilityFlag[g] ){
          allUnstable = false;
          break;
        }
      }
      if (allUnstable) break;
    }
    break;}

  case TDE_STABCRITERION_ANGLEDIFFFROMREF:{
    size_t t, g;
    for ( t = 0 ; t != genAngles[0].size(); ++t ){
      // Check the angle gap between each gen and the ref (== 0 rad).
      for ( g = 0 ; g != genAngles.size() ; ++g ){
        // Skip test for unstable generators; already have been identified as such
        if ( !genStabilityFlag[g] ) continue;
        // Check angles against ref
        if ( std::abs(genAngles[g][t]) > threshold )
          genStabilityFlag[g] = false;
      }
      // Check termination condition: all gens have been identified as unstable
      bool allUnstable = true;
      for ( g = 0; g != genAngles.size() ; ++g ){
        if ( genStabilityFlag[g] ){
          allUnstable = false;
          break;
        }
      }
      if (allUnstable) break;
    }
    break;}

  default:
    return 1;
  }

  // Output argument genStabilityFlag has been updated

  return 0;
}

bool TDEngine::_findAnglesOverallStability( vector<vector<double> > const& genAngles,
                                            int criterion,
                                            double threshold ){

  // Check for trivial stability
  if ( genAngles.size() == 0 )
    return true;

  // Each of the following code blocks needs to update 'genStabilityFlag' taking
  // into account the 'genAngles' input 'threshold' parameter
  // TODO: consider seperating in functions
  switch ( criterion ){

  case TDE_STABCRITERION_ANGLEDEVIATION:{
    // Find the angle trajectory of the least advanced generator for each time
    // instant
    // Notice: Same size for each of generator angle vectors is assumed implicitly
    // i.e. genAngles[g].size() == const, for all g.
    vector<double> deltaLA; // angle of the Least Advanced generator
    for ( size_t t = 0 ; t != genAngles[0].size(); ++t ){
      double deltaMin = numeric_limits<double>::max(); // angle of the LA gen for
                                                       // this time step
      for ( size_t g = 0 ; g != genAngles.size(); ++g ){
        if  ( std::isnan(genAngles[g][t]) )
          return false;
        if ( genAngles[g][t] < deltaMin )
          deltaMin = genAngles[g][t];
      }
      deltaLA.push_back( deltaMin );
    }
    // Check the angle gap between each gen and the LA one
    bool isStable = true;
    for ( size_t t = 0 ; t != genAngles[0].size(); ++t ){
      for ( size_t g = 0 ; g != genAngles.size(); ++g ){
        if (    std::abs(genAngles[g][t]-deltaLA[t]) > threshold
             || std::isnan(genAngles[g][t]) ){
          isStable = false;
          break;
        }
      }
      if (!isStable) break;
    }
    return isStable;
    break;}

  case TDE_STABCRITERION_ANGLEDIFFFROMSLACK:{
    // Check the angle gap between each gen and the slack
    bool isStable = true;
    for ( size_t t = 0 ; t != genAngles[0].size(); ++t ){
      for ( size_t g = 1 ; g != genAngles.size(); ++g ){
        if (    std::abs(genAngles[g][t]-genAngles[0][t]) > threshold
             || std::isnan(genAngles[g][t]-genAngles[0][t]) ){
          isStable = false;
          break;
        }
      }
      if (!isStable) break;
    }
    return isStable;
    break;}

  case TDE_STABCRITERION_ANGLEDIFFFROMREF:{
    // Check the angle gap between each gen and the ref (== 0 rad)
    bool isStable = true;
    for ( size_t t = 0 ; t != genAngles[0].size(); ++t ){
      for ( size_t g = 0 ; g != genAngles.size(); ++g ){
        if (    std::abs(genAngles[g][t]) > threshold
             || std::isnan(genAngles[g][t]) ){
          isStable = false;
          break;
        }
      }
      if (!isStable) break;
    }
    return isStable;
    break;}

  default:
    return false;
  }

  return false;
}

int TDEngine::do_setTimeStep( double seconds,
                              double* pMismatch ){
  // Input argument validation
  if ( seconds <= 0 )
    return 1;
  _timeStep = seconds;
  if ( pMismatch )
    *pMismatch = 0.0;
  return 0;
}

// TODO: improve
int TDEngine::do_checkStability( vector<Scenario> const& scenarios,
                                 vector<map<unsigned int,bool> >& genStable,
                                 vector<map<unsigned int,double> >* genInstabilityTime){

//  // Check for trivial run
//  if ( scenarios.size() == 0 ){
//    genStable.clear();
//    if ( genInstabilityTime )
//      genInstabilityTime->clear();
//    return 0;
//  }

  // Initialize output arguments
  genStable.clear();
  genStable.resize(scenarios.size());
  if ( genInstabilityTime ){
    genInstabilityTime->clear();
    genInstabilityTime->resize( scenarios.size() );
  }

  // Check generator stability for each one of the scenarios
//  TDResults res;
  map<unsigned int,bool> tempGenStable;
//  map<unsigned int,int> tempInstabSample;
  map<unsigned int,double> tempInstabTime;
  PrecisionTimer timer;
  for ( unsigned int k = 0 ; k != scenarios.size() ; ++k ){

    int criterion(TDE_STABCRITERION_ANGLEDIFFFROMREF);
    double threshold(M_PI);
    cout << "  TDEngine::do_checkStability(): ";
    timer.Start();
    int ans = _checkStability(scenarios[k], criterion, threshold, tempGenStable);
    double elapsedTime = timer.Stop();
    cout << " " << elapsedTime << " s" << endl;
    // If generator stability could not be determined for scenario[k] then
    // leave the respective output arguments empty
    if (ans) continue;

    // Update output arguments
    genStable[k] = tempGenStable;
    if ( genInstabilityTime )
      genInstabilityTime->at(k) = tempInstabTime;
  }

  return 0;
}

/* For each entry k in scenarios the following order of events is expected:
  scenario[k]._eventSet[0]: branch 3ph fault ON event
  scenario[k]._eventSet[1]: branch 3ph fault OFF event
  optional{scenario[k]._eventSet[2]: branch trip(on) event} */
int TDEngine::do_checkCCT( vector<Scenario> const& scenarios,
                           double precision,
                           vector<double>& cctMin, vector<double>& cctMax ){

  // Input argument validation
  if ( precision < 0 )
    return 1;

  // Initialize output arguments
  cctMin.resize( scenarios.size(), -1.0 );
  cctMax.resize( scenarios.size(), -1.0 );

  PrecisionTimer timer;
  // checkCCT loop proper
  for ( unsigned int k = 0 ; k != scenarios.size() ; ++k ){
    Scenario tempSce = scenarios[k];
    if(!tempSce.status()) continue; // consider only active scenarios

    // checkCCT on a per-scenario basis
    double tempCctMin, tempCctMax;
    cout << "  TDEngine::do_checkCCT(): ";
    timer.Start();
    int ans = _do_checkCCT( tempSce, precision, tempCctMin, tempCctMax );
    double elapsedTime = timer.Stop();
    cout << " " << elapsedTime << " s" << endl;
    if ( ans ){
      // checkCCT failure
//      cout << "CCT check for scenario: " << tempSce.name() << " ";
//      cout << "failed with code " << ans << endl;
      cctMin[k] = -1.0;
      cctMax[k] = -1.0;
    } else {
      // checkCCT success
      cctMin[k] = tempCctMin;
      cctMax[k] = tempCctMax;
    }

//    auxiliary::stayAlive();
  }
  return 0;
}

Powersystem const* TDEngine::do_getPws() const{ return _pws; }

/* scenario is expected to contain the following events in the following order:
  scenario._eventSet[0]: branch 3ph fault ON event
  scenario._eventSet[1]: branch 3ph fault OFF event
  optional{scenario._eventSet[2]: branch trip(on) event} */
int TDEngine::_do_checkCCT( Scenario const& scenario,
                            double precision,
                            double& cctMin, double& cctMax ){
  /* Following 'Bisection method'
  N = 1
  While N = NMAX { limit iterations to prevent infinite loop
    c = (a + b)/2 new midpoint
    If (f(c) = 0 or (b – a)/2 < TOL then { solution found
      Output(c)
      Stop
    }
    N = N + 1 increment step counter
    If sign(f(c)) = sign(f(a)) then a = c else b = c new interval
  } */

  // Check scenario CCT compatibility
  if ( !scenario.checkCCTCompatibility() )
    return 1;

  // Initialize bisection method
  unsigned int const MAX_ITERATIONS(100);  // limit iterations to prevent
                                           // infinite loop
  unsigned int iteration(0);               // iteration count
  double min(0);                           // starting minimum CCT point;
                                           // considered stable by definition
  double max(scenario.getEvent(1).time()); // starting maximum CCT point; given
                                           // by the scenario fault OFF event
  bool success(false);                     // bisection method success flag
  Scenario sce(scenario);                  // Scenario that is manipulated for
                                           // the intermediary checkStability()
                                           // check
  while ( iteration <= MAX_ITERATIONS ){
    // Check for termination criterion
    if ( max-min < precision){
      success = true;
      break;
    }

    // Update scenario for 'mid'
    double mid = (min+max)/2;
    // all scenarios at time 'off' (fault offs, trips, etc)
    for ( unsigned int m = 1 ; m != sce.getEventSetSize() ; ++m ){
      Event atOff = sce.getEvent(m);
      atOff.set_time( mid );
      sce.editEvent( m, atOff );
    }

    bool isStable(true);
    int criterion(TDE_STABCRITERION_ANGLEDIFFFROMREF);
    double threshold(M_PI);
    int ans = _checkStability( sce, criterion, threshold, isStable );
    if (ans) return 2;

    if (isStable){
      cout << "s";
      min = mid; // In the stable case, the lower cct bound is updated
    } else {
      cout << "u";
      max = mid; // In the unstable case, the lower cct bound is updated
    }

    ++iteration; // Proceed to next iteration
  }

  // Update output arguments
  if (success){
    cctMin = min;
    cctMax = max;
    return 0;
  } else {
    return 3;
  }

  return 0;
}
