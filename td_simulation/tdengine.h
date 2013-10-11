/*!
\file tdengine.h

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef TDENGINE_H
#define TDENGINE_H

#include "propertyholder.h"

#include <string>
#include <vector>
#include <map>

namespace elabtsaot{

class Logger;
class Powersystem;
class Scenario;
class TDResults;

enum InstabilityCriterion{
  TDE_STABCRITERION_ANGLEDEVIATION,     //!< max angle diff between any two gens
  TDE_STABCRITERION_ANGLEDIFFFROMSLACK, //!< max angle diff from the slack gen
  TDE_STABCRITERION_ANGLEDIFFFROMREF    //!< max angle diff from the ref (0 rad)
};

class TDEngine : public PropertyHolder {

 public:

  TDEngine( Powersystem const* pws, std::string const& description,
            double timeStep, Logger* log );
  virtual ~TDEngine(){}
  void setLogger(Logger* log);
  std::string getDescription() const;
  bool isEngineCompatible(Scenario const& sce) const;
  int setTimeStep( double seconds, double* pMismatch = NULL );
  double getTimeStep() const;
  int simulate(Scenario const& sce, TDResults& res);
  int checkStability( std::vector<Scenario> const& scenarios,
           std::vector<std::map<unsigned int,bool> >& genStable,
           std::vector<std::map<unsigned int,double> >* genInstabilityTime = 0);
  int checkCCT( std::vector<Scenario> const& scenarios, double precision,
                std::vector<double>& cctMin, std::vector<double>& cctMax );
  Powersystem const* getPws() const;

 protected:

  //! Returns the overall powersystem stability flag corresponding to the input
  //! scenario
  /*! \warning Implemented using full TD run */
  virtual int _checkStability( Scenario const& sce,
                               int criterion, double threshold,
                               bool& pwsStable );
  //! Returns the stability flags for the gens of the powersystem for the input
  //! scenario
  /*! \warning Implemented using full TD run
      \param genStable contains a map of the extIds of gens in res and the
           respective bool stability flags. eg. 2->false; 3->true; 6->true; etc */
  virtual int _checkStability( Scenario const& sce,
                               int criterion, double threshold,
                               std::map<unsigned int,bool>& genStable );
  //! Returns the overall powersystem stability flag corresponding to the input
  //! time domain results
  virtual int _checkStability( TDResults const& res,
                               int criterion, double threshold,
                               bool& pwsStable );
  //! Returns the stability flags for the gens of the powersystem for the input
  //! time domain results
  /*! \param genStable contains a map of the extIds of gens in res and the
             respective bool stability flags. eg. 2->false; 3->true; 6->true;
             etc */
  virtual int _checkStability( TDResults const& res,
                               int criterion, double threshold,
                               std::map<unsigned int,bool>& genStable );
  //! Returns the per-trajectory stability flag corresponding to the input
  //! vectors, which are interpreted as generator angle trajectories; output arg
  //! indexing follows the indexing of the input vector
  virtual int _findAnglesStability( std::vector<std::vector<double> > const& genAngles,
                                    int criterion, double threshold,
                                    std::vector<bool>& genStabilityFlag );
  //! Returns the overall stability flag corresponding to the input vectors, which
  //! are interpreted as generator angle trajectories
  virtual bool _findAnglesOverallStability( std::vector<std::vector<double> > const& genAngles,
                                            int criterion, double threshold );

  Powersystem const* _pws;
  std::string _TDdescription;
  double _timeStep;
  Logger* _TDlog;

 private:

  virtual bool do_isEngineCompatible(Scenario const& sce) const = 0;
  virtual int do_setTimeStep(double seconds,
                             double* pMismatch = NULL );
  virtual int do_simulate( Scenario const& sce, TDResults& res ) = 0;
  virtual int do_checkStability( std::vector<Scenario> const& scenarios,
       std::vector<std::map<unsigned int,bool> >& genStable,
       std::vector<std::map<unsigned int,double> >* genInstabilityTime = 0);
  virtual int do_checkCCT( std::vector<Scenario> const& scenarios,
                           double precision,
                           std::vector<double>& cctMin,
                           std::vector<double>& cctMax );
  virtual Powersystem const* do_getPws() const;
  int _do_checkCCT( Scenario const& scenario, double precision,
                    double& cctMin, double& cctMax );

};

} // end of namespace elabtsaot

#endif // TDENGINE_H
