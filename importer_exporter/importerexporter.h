/*!
\filename importerexporter.h
\brief Definition filename for class ImporterExporter

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Fragkiskos Koufogiannis, fkoufis at gmail dot com
\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef IMPORTEREXPORTER_H
#define IMPORTEREXPORTER_H

#include <string>

namespace elabtsaot{

class Powersystem;
class PwsSchematicModel;
class PwsMapperModel;
class EmulatorHw;
class Emulator;
class ScenarioSet;
class TDResults;

namespace io{

// Note: In the following, 'filename' param should be passed with its extension
int importProject(std::string filename,
                  std::string& pwsfilename,
                  std::string& schfilename,
                  std::string& mapfilename,
                  std::string& scsfilename);
int importPowersystem(std::string filename, Powersystem* pws);
int logPowerSystem(Powersystem const& pws, std::ostream& ostr);
int logPowerSystem(Powersystem const& pws, std::string const& filename);
int importSchematic(std::string filename, PwsSchematicModel* smd);
int importMapping(std::string filename,
                  PwsMapperModel* mmd,
                  EmulatorHw const* emuhw = NULL,
                  Powersystem const* pws = NULL);
int importEncoding(std::string filename, size_t sliceId, Emulator& emu, bool force);
int importScenarioSet(std::string filename, ScenarioSet* scs);
int importTDResults(std::string filename, TDResults* res);
int exportPowersystem(std::string filename, Powersystem const* pws);
int exportSchematic(std::string filename, PwsSchematicModel const* smd);
int exportMapping(std::string filename, PwsMapperModel const* mmd);
int exportScenarioSet(std::string filename, ScenarioSet const* scs);
int exportTDResults(std::string filename, TDResults const* res); //!< Exports a set of TDResults

int validateSchema(std::string xsdPath, std::string xmlPath);

} // end of namespace io
} // end of namespace elabtsaot

#endif // IMPORTEREXPORTER_H
