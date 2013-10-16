
#include "importerexporter.h"
using namespace elabtsaot;

#include "messagehandler.h"
#include "powersystemxmlhandler.h"
#include "schematicxmlhandler.h"
#include "mappingxmlhandler.h"
#include "scenariosetxmlhandler.h"
#include "pwsschematicmodel.h"
#include "pwsmappermodel.h"
#include "powersystem.h"
#include "emulatorhw.h"
#include "scenarioset.h"
#include "tdresults.h"
#include "auxiliary.h"

#include <QFileDialog>
#include <QFile>
#include <QtXmlPatterns>
#include <QXmlInputSource>

#include <string>
using std::string;
#include <iostream>
using std::cout;
using std::endl;
#include <limits>
using std::numeric_limits;

using std::vector;

int io::importProject( string filename_,
                       string& pwsfilename,
                       string& schfilename,
                       string& mapfilename,
                       string& scsfilename ){
  QString filename(QString::fromStdString(filename_));
  QFile file( filename );
  if ( !file.open(QFile::ReadOnly | QFile::Text) ) // open() returns true if ok
    return 1;
  QTextStream fin(&file);
  fin.setCodec("UTF-8");

  QString temp;
  QString tempBuffer;
  // ----- start '$$' delimeter -----
  bool startDelimeterRead = false;
  while ( !startDelimeterRead ){
    temp = fin.readLine();
    if ( temp.isNull() )
      return 2; // EOF reach prematurely
    if ( temp.startsWith("$$") )
      startDelimeterRead = true;
  }

  // ----- title/description section -----
  string title;
  bool mainDelimeterRead = false;
  tempBuffer.clear();
  while ( !mainDelimeterRead ){
    temp = fin.readLine();
    if ( temp.isNull() )
      return 5; // EOF reach prematurely
    if ( temp.startsWith("$$") )
      mainDelimeterRead = true;
    else
      tempBuffer.append( temp );
  }
  title = tempBuffer.toStdString();

  // ----- main part -----
  QFileInfo fileinfo(filename);
  QDir dir( fileinfo.dir() );
  bool footerDelimeterRead = false;
  while ( !footerDelimeterRead ){
    temp = fin.readLine();
    temp = temp.simplified(); // change all whitespace to ASCII #32 (space)
    temp.replace( " ", "" );  // delete all whitespace
    if ( temp.isNull() ){
      return 7; // EOF reach prematurely
    } else if ( temp.startsWith("$$") ){
      footerDelimeterRead = true;
    } else {
      if ( temp.startsWith("pws") ){
        temp.remove(0,4);
        if ( temp.endsWith(".xml") )
          pwsfilename = dir.absoluteFilePath(temp).toStdString();
      } else if ( temp.startsWith("sch") ){
        temp.remove(0,4);
        if ( temp.endsWith(".xml") )
          schfilename = dir.absoluteFilePath(temp).toStdString();
      } else if ( temp.startsWith("map") ){
        temp.remove(0,4);
        if ( temp.endsWith(".xml") )
          mapfilename = dir.absoluteFilePath(temp).toStdString();
      } else if ( temp.startsWith("scs") ){
        temp.remove(0,4);
        if ( temp.endsWith(".xml") )
          scsfilename = dir.absoluteFilePath(temp).toStdString();
      }
    }
  }
  return 0;
}

int io::importPowersystem(string filename_, Powersystem* pws){
  QString filename(QString::fromStdString(filename_));

  if ( pws == NULL )
    return -1;

  // Check xml's consistency according to schema
  if( validateSchema( ":/xml_schemas/powersystem.xsd", filename_ ) ){
    cout << "The XML file is not built according to the powersystem schema."
         << endl;
    return 1;
  }

  QFile xmlFile( filename );
  QXmlInputSource source( &xmlFile );
  QXmlSimpleReader reader;
  Powersystem tempws("xml import power system", 100, 50);
  PowersystemXMLHandler* handler = new PowersystemXMLHandler( &tempws );
  reader.setContentHandler( handler );
  if ( !reader.parse( source )){
    cout << "Error parsing xml file!" << endl;
    return 2;
  }

  if( int ans = tempws.validate() ){
    cout << "Powersystem contained in the XML file cannot be validated" << endl;
    cout << "Exit code: " << ans << " Aborting ... " << endl;
    return 3;
  } else{
    // Powersystem properly validated - copy to the provided argument
    *pws = tempws;
  }

  return 0;
}

int io::importSchematic( string filename_, PwsSchematicModel* smd ){
  QString filename(QString::fromStdString(filename_));

  if ( smd == NULL )
    return -1;

  // Check xml's consistency according to schema
  if( validateSchema( ":/xml_schemas/schematic.xsd", filename_ ) ){
    cout << "The XML file is not built according to the schematic schema."
         << endl;
    return 1;
  }

  QFile xmlFile( filename );
  QXmlInputSource source( &xmlFile );
  QXmlSimpleReader reader;
  PwsSchematicModel temp_smd;
  SchematicXMLHandler* handler = new SchematicXMLHandler( &temp_smd );
  reader.setContentHandler( handler );
  if ( !reader.parse( source )){
    cout << "Error parsing xml file!" << endl;
    return 2;
  }

  return smd->copy( temp_smd );
}

int io::importMapping( string filename_,
                       PwsMapperModel* mmd,
                       EmulatorHw const* emuhw,
                       Powersystem const* pws){
  QString filename(QString::fromStdString(filename_));

  if ( mmd == NULL )
    return -1;

  // Check xml's consistency according to schema
  if( validateSchema( ":/xml_schemas/mapping.xsd", filename_ ) ){
    cout << "The XML file is not built according to the mapping schema."
         << endl;
    return 1;
  }

  QFile xmlFile( filename );
  QXmlInputSource source( &xmlFile );
  QXmlSimpleReader reader;
  PwsMapperModel* temp_mmd = new PwsMapperModel( pws, emuhw );
  MappingXMLHandler* handler = new MappingXMLHandler( temp_mmd );
  reader.setContentHandler( handler );
  if ( !reader.parse( source )){
    cout << "Error parsing xml file!" << endl;
    return 2;
  }

  // Validate the imported mapping model against the emulator and the
  // powersystem provided as arguments
  int ans = temp_mmd->validate();
  if( ans ){
    cout << "Mapping contained in the XML file cannot be validated" << endl;
    cout << "Exit code: " << ans << " Aborting ... " << endl;
    return 3;
  }

  return mmd->copy( *temp_mmd );
}

int io::importScenarioSet(string filename_, ScenarioSet* scs){
  QString filename(QString::fromStdString(filename_));

  if ( scs == NULL )
    return -1;

  // Check xml's consistency according to schema
  if( validateSchema( ":/xml_schemas/scenariosset.xsd", filename_ ) ){
    cout << "The XML file is not built according to the scenario set schema."
         << endl;
    return 1;
  }

  QFile xmlFile( filename );
  QXmlInputSource source( &xmlFile );
  QXmlSimpleReader reader;
  ScenarioSet* temp_scs = new ScenarioSet();
  ScenarioSetXMLHandler* handler = new ScenarioSetXMLHandler( temp_scs );
  reader.setContentHandler( handler );
  if ( !reader.parse( source ) ){
    cout << "Error parsing xml file!" << endl;
    return 2;
  }

  *scs = *temp_scs;
  return 0;
}

int io::importTDResults( string filename_, TDResults* res ){
  QString filename(QString::fromStdString(filename_));

  if ( res == NULL )
    return -1;

  QFile file( filename );
  if ( !file.open(QFile::ReadOnly | QFile::Text) ) // open() returns true if ok
    return 1;
  QTextStream fin(&file);
  fin.setCodec("UTF-8");

  QString temp;
  QString tempBuffer;
  // Temp variables that correspond to a TDResults structure
  string sha1Digest;
  string title;
  string description;
  vector<bool> dataExists;
  vector<TDResultIdentifier> identifiers;
  vector<string> names;
  vector<string> units;
  vector<vector<double> > data;
  string footer;

  // ----- File beginning -----
//  bool sha1DigestDelimeterRead = false;
  temp = fin.readLine();
  if ( !temp.startsWith("$$") )
    return 2;
//  sha1DigestDelimeterRead = true;

  // ----- sha1digest section -----
  bool titleDelimeterRead = false;
  temp = fin.readLine();
  if ( !temp.contains("$$") ){
    sha1Digest = temp.toStdString();
  } else {
    if ( temp.startsWith("$$") ){
      sha1Digest = "UNKNOWN";
      titleDelimeterRead = true;
    } else {
      // Weird sha1Digest read!
      return 3;
    }
  }
  if ( !titleDelimeterRead ){
    temp = fin.readLine();
    if ( !temp.contains("$$") )
      return 4;
  }

  // ----- title section -----
  bool descDelimeterRead = false;
  tempBuffer.clear();
  while ( !descDelimeterRead ){
    temp = fin.readLine();
    if ( temp.isNull() )
      return 5; // EOF reach prematurely
    if ( temp.startsWith("$$") )
      descDelimeterRead = true;
    else
      tempBuffer.append( temp );
  }
  title = tempBuffer.toStdString();

  // ----- desc section -----
  bool formatDelimeterRead = false;
  tempBuffer.clear();
  while ( !formatDelimeterRead ){
    temp = fin.readLine();
    if ( temp.isNull() )
      return 6; // EOF reach prematurely
    if ( temp.startsWith("$$") )
      formatDelimeterRead = true;
    else
      tempBuffer.append( temp );
  }
  description = tempBuffer.toStdString();

  // ----- format section -----
  bool dataDelimeterRead = false;
  while ( !dataDelimeterRead ){
    temp = fin.readLine();
    if ( temp.isNull() ){
      return 7; // EOF reach prematurely
    } else if ( temp.startsWith("$$") ){
      dataDelimeterRead = true;
    } else {
      // Parse format section proper
      QStringList list = temp.split( QRegExp("\\s*\\$\\s*") );
      if ( list.size() < 7 )
        return 8; // inconsistent format-line format
      dataExists.push_back( true );
      names.push_back( list.at(0).toStdString() );
      units.push_back( list.at(1).toStdString() );
      TDResultIdentifier tempTdri;
      bool ok = true;
      tempTdri.elementType = list.at(2).toInt(&ok);
      if ( !ok ) return 81;
      tempTdri.variable = list.at(3).toInt(&ok);
      if ( !ok ) return 82;
      tempTdri.extId = list.at(4).toInt(&ok);
      if ( !ok ) return 83;
      identifiers.push_back( tempTdri );
    }
  }
  int numberOfVariables = dataExists.size();
  data.resize( numberOfVariables );

  // ----- data section -----
  bool footerDelimeterRead = false;
  while ( !footerDelimeterRead ){
    temp = fin.readLine();
    if ( temp.isNull() ){
      return 9; // EOF reached prematurely
    } else if ( temp.startsWith("$$") ){
      footerDelimeterRead = true;
    } else {
      // Parse data section proper
      QStringList list = temp.split( QRegExp("\\s*\\$\\s*") );
      if ( list.size() < numberOfVariables )
        return 10; // malformed data format line
      bool ok = true;
      for ( int k = 0 ; k != numberOfVariables ; ++k ){
        data[k].push_back( list[k].toDouble(&ok) );
        if ( !ok ) return 11; // malformed data format line
      }
    }
  }

  // ----- footer section -----
  temp = fin.readLine();
  while ( !temp.isNull() ){
    footer.append( temp.toStdString() );
    temp = fin.readLine();
  }

  // Close file
  file.close();

  // Write temp variables read from the file into the TDResults structure
  TDResults tempRes( sha1Digest, title, description, dataExists, identifiers,
                     names, units, data, footer );
  *res = tempRes;

  return 0;
}

int io::exportPowersystem( string filename, Powersystem const* pws){

  FILE *f;
  char name[200];
  strcpy(name, filename.c_str());
  f = fopen( name, "w" );

  fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
  fprintf(f, "<powersystem xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n");

  // --------------- Output network's info ---------------
  fprintf(f, "<info>\n");
  fprintf(f, "\t<name> %s </name>\n",
          pws->name().c_str());
  fprintf(f, "\t<description> %s </description>\n",
          pws->description().c_str());
  fprintf(f, "\t<baseMVA> %.12f </baseMVA>\n",
          pws->baseS());
  fprintf(f, "\t<baseFreq> %.12f </baseFreq>\n",
          pws->baseF());
  fprintf(f, "\t<slackBus> %d </slackBus>\n",
          pws->slackBusExtId());
  fprintf(f, "\t<slackGen> %d </slackGen>\n",
          pws->slackGenExtId());
  fprintf(f, "</info>\n");
  fflush(f);

  // --------------- Output buses ---------------
  fprintf(f, "<buses>\n");
  for( size_t k = 0 ; k != pws->getBusSet_size(); ++k ){

    // Get bus at pos (intId) k
    Bus const* bus = pws->getBus(k);
    if( !bus )
      break;

    // Write its parameters to the xml file
    fprintf(f, "\t<bus>\n");
    fprintf(f, "\t\t<extId> %d </extId>\n", bus->extId);
    fprintf(f, "\t\t<name> %s </name>\n", bus->name.c_str());
    fprintf(f, "\t\t<Gsh> %.12f </Gsh>\n", bus->Bsh);
    fprintf(f, "\t\t<Bsh> %.12f </Bsh>\n", bus->Gsh);
    fprintf(f, "\t\t<baseKV> %.12f </baseKV>\n", bus->baseKV);
    fprintf(f, "\t\t<V> %.12f </V>\n", bus->V);
    fprintf(f, "\t\t<theta> %.12f </theta>\n", bus->theta);
    fprintf(f, "\t</bus>\n");
    fflush(f);
  }
  fprintf(f, "</buses>\n\n");

  // --------------- Output branches --------------------
  fprintf(f, "<branches>\n");
  for( size_t k = 0 ; k != pws->getBrSet_size() ; ++k ){

    // Get branch at pos (intId) k
    Branch const* br = pws->getBranch(k);
    if( !br )
      break;

    // Write its parameters to the xml file
    fprintf(f, "\t<branch>\n");
    fprintf(f, "\t\t<id> %d </id>\n", br->extId);
    fprintf(f, "\t\t<status> %d </status>\n", static_cast<int>( br->status ) );
    fprintf(f, "\t\t<fromBusExtId> %d </fromBusExtId>\n", br->fromBusExtId);
    fprintf(f, "\t\t<toBusExtId> %d </toBusExtId>\n", br->toBusExtId);
    fprintf(f, "\t\t<R> %.12f </R>\n", br->R);
    fprintf(f, "\t\t<X> %.12f </X>\n", br->X);
    fprintf(f, "\t\t<Bfrom> %.12f </Bfrom>\n", br->Bfrom);
    fprintf(f, "\t\t<Bto> %.12f </Bto>\n", br->Bto);
    fprintf(f, "\t\t<Gfrom> %.12f </Gfrom>\n", br->Gfrom);
    fprintf(f, "\t\t<Gto> %.12f </Gto>\n", br->Gto);
    fprintf(f, "\t\t<Xratio> %.12f </Xratio>\n", br->Xratio);
    fprintf(f, "\t\t<Xshift> %.12f </Xshift>\n", br->Xshift);
    fprintf(f, "\t</branch>\n");
    fflush(f);
  }
  fprintf(f, "</branches>\n\n");

  // -------------------- Output generators --------------------
  fprintf(f, "<generators>\n");
  for( size_t k = 0 ; k != pws->getGenSet_size() ; ++k ){

    // Get generator at pos (intId) k
    Generator const* gen = pws->getGenerator(k);
    if( !gen )
      break;

    // Write its parameters to the xml file
    fprintf(f, "\t<generator>\n");

    fprintf(f, "\t\t<id> %d </id>\n", gen->extId());
    fprintf(f, "\t\t<name> %s </name>\n", gen->name().c_str());
    fprintf(f, "\t\t<atbus> %d </atbus>\n", gen->busExtId() );
    fprintf(f, "\t\t<avr> %d </avr>\n", gen->avr());
    fprintf(f, "\t\t<vsetpoint>%.12f</vsetpoint>\n", gen->voltageSetpoint());
    fprintf(f, "\t\t<status>%d</status>\n", static_cast<int>( gen->status() ) );

    fprintf(f, "\t\t<generation>\n");
    fprintf(f, "\t\t\t<pgen> %.12f </pgen>\n", gen->pgen());
    fprintf(f, "\t\t\t<qgen> %.12f </qgen>\n", gen->qgen());
    fprintf(f, "\t\t\t<pmin> %.12f </pmin>\n", gen->pmin());
    fprintf(f, "\t\t\t<pmax> %.12f </pmax>\n", gen->pmax());
    fprintf(f, "\t\t\t<qmin> %.12f </qmin>\n", gen->qmin());
    fprintf(f, "\t\t\t<qmax> %.12f </qmax>\n", gen->qmax());
    fprintf(f, "\t\t</generation>\n");

    fprintf(f, "\t\t<dynamic>\n");
    fprintf(f, "\t\t\t<model> %d ", gen->model());
    fprintf(f, "</model>\n");
    fprintf(f, "\t\t\t<leakage_reactance> %.12f ", gen->xl());
    fprintf(f, "</leakage_reactance>\n");
    fprintf(f, "\t\t\t<armature_resistance> %.12f ", gen->ra());
    fprintf(f, "</armature_resistance>\n");
    fprintf(f, "\t\t\t<synchronous_direct_reactance> %.12f ", gen->xd());
    fprintf(f, "</synchronous_direct_reactance>\n");
    fprintf(f, "\t\t\t<transient_direct_reactance> %.12f ", gen->xd_1());
    fprintf(f, "</transient_direct_reactance>\n");
    fprintf(f, "\t\t\t<subtransient_direct_reactance> %.12f ", gen->xd_2());
    fprintf(f, "</subtransient_direct_reactance>\n");
    fprintf(f, "\t\t\t<transient_direct_time_const> %.12f ", gen->Td0_1());
    fprintf(f, "</transient_direct_time_const>\n");
    fprintf(f, "\t\t\t<subtransient_direct_time_const> %.12f ", gen->Td0_2());
    fprintf(f, "</subtransient_direct_time_const>\n");
    fprintf(f, "\t\t\t<synchronous_quadrature_reactance> %.12f ", gen->xq());
    fprintf(f, "</synchronous_quadrature_reactance>\n");
    fprintf(f, "\t\t\t<transient_quadrature_reactance> %.12f ", gen->xq_1());
    fprintf(f, "</transient_quadrature_reactance>\n");
    fprintf(f, "\t\t\t<subtransient_quadrature_reactance> %.12f ", gen->xq_2());
    fprintf(f, "</subtransient_quadrature_reactance>\n");
    fprintf(f, "\t\t\t<transient_quadrature_time_const> %.12f ", gen->Tq0_1());
    fprintf(f, "</transient_quadrature_time_const>\n");
    fprintf(f, "\t\t\t<subtransient_quadrature_time_const> %.12f ", gen->Tq0_2());
    fprintf(f, "</subtransient_quadrature_time_const>\n");
    fprintf(f, "\t\t\t<mechanical_starting_time> %.12f ", gen->M());
    fprintf(f, "</mechanical_starting_time>\n");
    fprintf(f, "\t\t\t<damping_coefficient> %.12f ", gen->D());
    fprintf(f, "</damping_coefficient>\n");
    fprintf(f, "\t\t</dynamic>\n");

    fprintf(f, "\t</generator>\n");
    fflush(f);
  }
  fprintf(f, "</generators>\n\n");

  // --------------- Output loads ---------------
  fprintf(f, "<loads>\n");
  for( size_t k = 0 ; k != pws->getLoadSet_size() ; ++k ){

    // Get load at pos (intId) k
    Load const* load = pws->getLoad(k);
    if( !load )
      break;

    // And write its parameters to the xml file
    fprintf(f, "\t<load>\n");

    fprintf(f, "\t\t<id> %d </id>\n", load->extId());
    fprintf(f, "\t\t<name> %s </name>\n", load->name().c_str());
    fprintf(f, "\t\t<atbus> %d </atbus>\n", load->busExtId() );
    fprintf(f, "\t\t<pdemand> %.12f </pdemand>\n", load->pdemand());
    fprintf(f, "\t\t<qdemand> %.12f </qdemand>\n", load->qdemand());
    fprintf(f, "\t\t<status> %d </status>\n", static_cast<int>( load->status() ) );
    fprintf(f, "\t\t<v_exp_a> %.12f </v_exp_a>\n", load->v_exp_a());
    fprintf(f, "\t\t<v_exp_b> %.12f </v_exp_b>\n", load->v_exp_b());
    fprintf(f, "\t\t<k_p_f> %.12f </k_p_f>\n", load->k_p_f());
    fprintf(f, "\t\t<k_q_f> %.12f </k_q_f>\n", load->k_q_f());

    fprintf(f, "\t</load>\n");
    fflush(f);
  }
  fprintf(f, "</loads>\n\n");
  fprintf(f, "</powersystem>\n");
  fflush(f);
  fclose(f);

  return 0;
}

int io::exportSchematic( string filename, PwsSchematicModel const* smd ){

  FILE *f;
  char name[200];
  strcpy(name, filename.c_str());
  f = fopen( name, "w" );

  fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
  fprintf(f, "<schematic xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n");
  fflush(f);

  size_t mdlId;
  PwsSchematicModelElement const* cd;
  // --------------- Write bus information ---------------
  for( mdlId = 0; mdlId != smd->busElements_size(); ++mdlId ){
    cd = smd->elementByIndex(PWSMODELELEMENTTYPE_BUS, mdlId);

    fprintf(f, "<element>\n");
    fprintf(f, "\t<type> %d </type>\n", cd->type );
    fprintf(f, "\t<id> %d </id>\n", cd->extId);
    fprintf(f, "\t<x1> %.12f </x1>\n", cd->x1);
    fprintf(f, "\t<y1> %.12f </y1>\n", cd->y1);
    fprintf(f, "\t<x2> %.12f </x2>\n", cd->x2);
    fprintf(f, "\t<y2> %.12f </y2>\n", cd->y2);
    fprintf(f, "</element>\n");
    fflush(f);
  }

  // --------------- Write branch information ---------------
  for( mdlId = 0; mdlId!=smd->branchElements_size(); ++mdlId ){
    cd = smd->elementByIndex(PWSMODELELEMENTTYPE_BR, mdlId);

    fprintf(f, "<element>\n");
    fprintf(f, "\t<type> %d </type>\n", cd->type);
    fprintf(f, "\t<id> %d </id>\n", cd->extId);
    fprintf(f, "\t<x1> %.12f </x1>\n", cd->x1);
    fprintf(f, "\t<y1> %.12f </y1>\n", cd->y1);
    fprintf(f, "\t<x2> %.12f </x2>\n", cd->x2);
    fprintf(f, "\t<y2> %.12f </y2>\n", cd->y2);
    fprintf(f, "</element>\n");
    fflush(f);
  }

  // --------------- Write generator information ---------------
  for( mdlId = 0; mdlId!=smd->genElements_size(); ++mdlId ){
    cd = smd->elementByIndex(PWSMODELELEMENTTYPE_GEN, mdlId);

    fprintf(f, "<element>\n");
    fprintf(f, "\t<type> %d </type>\n", cd->type);
    fprintf(f, "\t<id> %d </id>\n", cd->extId);
    fprintf(f, "\t<x1> %.12f </x1>\n", cd->x1);
    fprintf(f, "\t<y1> %.12f </y1>\n", cd->y1);
    fprintf(f, "\t<x2> %.12f </x2>\n", cd->x2);
    fprintf(f, "\t<y2> %.12f </y2>\n", cd->y2);
    fprintf(f, "</element>\n");
    fflush(f);
  }

  // --------------- Write load information ---------------
  for( mdlId = 0; mdlId!=smd->loadElements_size(); ++mdlId ){
    cd = smd->elementByIndex(PWSMODELELEMENTTYPE_LOAD, mdlId);

    fprintf(f, "<element>\n");
    fprintf(f, "\t<type> %d </type>\n", cd->type);
    fprintf(f, "\t<id> %d </id>\n", cd->extId);
    fprintf(f, "\t<x1> %.12f </x1>\n", cd->x1);
    fprintf(f, "\t<y1> %.12f </y1>\n", cd->y1);
    fprintf(f, "\t<x2> %.12f </x2>\n", cd->x2);
    fprintf(f, "\t<y2> %.12f </y2>\n", cd->y2);
    fprintf(f, "</element>\n");
    fflush(f);
  }

  fprintf(f, "</schematic>\n");
  fflush(f);
  fclose(f);

  return 0;
}

int io::exportMapping( string filename, PwsMapperModel const* mmd ){

  FILE *f;
  char name[200];
  strcpy(name, filename.c_str());
  f = fopen( name, "w" );

  fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
  fprintf(f, "<mapping xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n");
  fflush(f);

  size_t mdlId;
  PwsMapperModelElement const* cd;
  // --------------- Write bus information ---------------
  for( mdlId = 0; mdlId != mmd->busElements_size(); ++mdlId ){
    cd = mmd->elementByIndex(PWSMODELELEMENTTYPE_BUS, mdlId);

    fprintf(f, "<element>\n");
    fprintf(f, "\t<type> %d </type>\n", cd->type );
    fprintf(f, "\t<id> %d </id>\n", cd->extId);
    fprintf(f, "\t<mapped> %d </mapped>\n", static_cast<int>(cd->mapped));
    fprintf(f, "\t<tab> %d </tab>\n", cd->tab);
    fprintf(f, "\t<row> %d </row>\n", cd->row);
    fprintf(f, "\t<col> %d </col>\n", cd->col);
    fprintf(f, "</element>\n");
    fflush(f);
  }

  // --------------- Write branch information ---------------
  for( mdlId = 0; mdlId!=mmd->branchElements_size(); ++mdlId ){
    cd = mmd->elementByIndex(PWSMODELELEMENTTYPE_BR, mdlId);

    fprintf(f, "<element>\n");
    fprintf(f, "\t<type> %d </type>\n", cd->type);
    fprintf(f, "\t<id> %d </id>\n", cd->extId);
    fprintf(f, "\t<mapped> %d </mapped>\n", static_cast<int>(cd->mapped));
    fprintf(f, "\t<tab> %d </tab>\n", cd->tab);
    fprintf(f, "\t<row> %d </row>\n", cd->row);
    fprintf(f, "\t<col> %d </col>\n", cd->col);
    fprintf(f, "</element>\n");
    fflush(f);
  }

  // --------------- Write generator information ---------------
  for( mdlId = 0; mdlId!=mmd->genElements_size(); ++mdlId ){
    cd = mmd->elementByIndex(PWSMODELELEMENTTYPE_GEN, mdlId);

    fprintf(f, "<element>\n");
    fprintf(f, "\t<type> %d </type>\n", cd->type);
    fprintf(f, "\t<id> %d </id>\n", cd->extId);
    fprintf(f, "\t<mapped> %d </mapped>\n", static_cast<int>(cd->mapped));
    fprintf(f, "\t<tab> %d </tab>\n", cd->tab);
    fprintf(f, "\t<row> %d </row>\n", cd->row);
    fprintf(f, "\t<col> %d </col>\n", cd->col);
    fprintf(f, "</element>\n");
    fflush(f);
  }

  // --------------- Write load information ---------------
  for( mdlId = 0; mdlId!=mmd->loadElements_size(); ++mdlId ){
    cd = mmd->elementByIndex(PWSMODELELEMENTTYPE_LOAD, mdlId);

    fprintf(f, "<element>\n");
    fprintf(f, "\t<type> %d </type>\n", cd->type);
    fprintf(f, "\t<id> %d </id>\n", cd->extId);
    fprintf(f, "\t<mapped> %d </mapped>\n", static_cast<int>(cd->mapped));
    fprintf(f, "\t<tab> %d </tab>\n", cd->tab);
    fprintf(f, "\t<row> %d </row>\n", cd->row);
    fprintf(f, "\t<col> %d </col>\n", cd->col);
    fprintf(f, "</element>\n");
    fflush(f);
  }

  fprintf(f, "</mapping>\n");
  fflush(f);
  fclose(f);

  return 0;
}

int io::exportScenarioSet( string filename, ScenarioSet const* scs ){

  FILE *f;
  char name[200];
  strcpy(name, filename.c_str());
  f = fopen( name, "w" );

  fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
  fprintf(f, "<scenarioSet xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n");
  fprintf(f, "\n");

  // --------------- Output scenarioSet's general info ---------------
  fprintf(f, "<extId> %d </extId>\n", scs->extId() );
  fprintf(f, "<name> %s </name>\n", scs->name().c_str() );
  fprintf(f, "<description> %s </description>\n", scs->description().c_str() );
  fprintf(f, "\n");
  fflush(f);

  // --------------- Output scenarios ---------------
  for ( size_t k = 0 ; k != scs->scenarios.size() ; ++k ){
    fprintf(f, "<scenario>\n");

    fprintf(f, "\t<extId> %d </extId>\n", scs->scenarios[k].extId() );
    fprintf(f, "\t<name> %s </name>\n", scs->scenarios[k].name().c_str() );
    fprintf(f, "\t<description> %s </description>\n", scs->scenarios[k].description().c_str() );
    fprintf(f, "\t<startTime> %.12f </startTime>\n", scs->scenarios[k].startTime() );
    fprintf(f, "\t<stopTime> %.12f </stopTime>\n", scs->scenarios[k].stopTime() );
    fprintf(f, "\t<status> %d </status>\n", scs->scenarios[k].status() );
    fflush(f);

    for ( size_t m = 0 ; m != scs->scenarios[k].getEventSetSize() ; ++m ){
      fprintf(f, "\t<event>\n");
      Event evn = scs->scenarios[k].getEvent(m);

      fprintf(f, "\t\t<name> %s </name>\n", evn.name().c_str() );
      fprintf(f, "\t\t<time> %.12f </time>\n", evn.time() );
      fprintf(f, "\t\t<description> %s </description>\n", evn.description().c_str() );
      fprintf(f, "\t\t<status> %d </status>\n", evn.status() );
      fprintf(f, "\t\t<element_type> %d </element_type>\n", evn.element_type() );
      fprintf(f,"\t\t<element_extId> %d </element_extId>\n", evn.element_extId());
      fprintf(f, "\t\t<event_type> %d </event_type>\n", evn.event_type() );
      fprintf(f, "\t\t<bool_arg> %d </bool_arg>\n", evn.bool_arg() );
      fprintf(f, "\t\t<double_arg> %.12f </double_arg>\n", evn.double_arg() );
      fprintf(f,"\t\t<double_arg_1> %.12f </double_arg_1>\n", evn.double_arg_1());
      fprintf(f,"\t\t<double_arg_2> %.12f </double_arg_2>\n", evn.double_arg_2());
      fprintf(f,"\t\t<uint_arg> %d </uint_arg>\n", evn.uint_arg());

      fprintf(f, "\t</event>\n");
      fprintf(f, "\n");
      fflush(f);
    }

    fprintf(f, "</scenario>\n");
    fprintf(f, "\n");
    fflush(f);
  }
  fprintf(f, "\n");

  fprintf(f, "</scenarioSet>\n");
  fflush(f);
  fclose(f);

  return 0;
}

int io::exportTDResults( string filename, TDResults const* res ){

  // Open file
  FILE *f;
  char name[200];
  strcpy(name, filename.c_str());
  f = fopen( name, "w" );

  // Retrieve references to TDResults members
  vector<bool> const& dataExists( res->dataExists() );
  vector<vector<double> > const& data( res->data() );

  // ----- SHA1 digest -----
  fprintf(f, "$$\n");
//  fprintf(f, "%s\n", res->sha1Digest().c_str() );
  fprintf(f, "%s", res->sha1Digest().c_str() ); // sha1Digest() string has \n

  // ----- Title -----
  fprintf(f, "$$\n");
  fprintf(f, "%s\n", res->title().c_str() );

  // ----- Header section -----
  fprintf(f, "$$\n");
  fprintf(f, "Description %s\n", res->description().c_str() );

  // ----- Format section -----
  fprintf(f, "$$\n");
  fflush(f);
  for ( size_t k = 0 ; k != dataExists.size() ; ++k ){
    if ( !dataExists.at(k) )
      continue;
    fprintf(f, "%s $ ", res->names().at(k).c_str());
    fprintf(f, "%s $ ", res->units().at(k).c_str());
    fprintf(f, "%d $ ", res->identifiers().at(k).elementType);
    fprintf(f, "%d $ ", res->identifiers().at(k).variable);
    fprintf(f, "%d $ ", res->identifiers().at(k).extId);
    fprintf(f, "2 $ 9\n"); // Q2.9 format for all results // TODO change!
    fflush(f);
    auxiliary::stayAlive();
  }

  // ----- Data section -----
  fprintf(f, "$$\n");
  // Find the minimum length of the data vectors
  // Actually all vectors should normally be of the same length
  size_t minDataLength = numeric_limits<int>::max();
  for ( size_t k = 0 ; k != data.size() ; ++k ){
    if ( !dataExists.at(k) )
      continue;
    if ( data.at(k).size() < minDataLength )
      minDataLength = data.at(k).size();
  }
  // Output results to file
  for ( size_t k = 0 ; k != minDataLength ; ++k ){
    for ( size_t m = 0 ; m != data.size() ; ++m ){
      if ( !dataExists.at(m) )
        continue;
      fprintf(f, "%.9f $", data[m][k]);
      fflush(f);
      auxiliary::stayAlive();
    }
    fprintf(f, "\n");
  }

  // ----- Footer section -----
  fprintf(f, "$$\n");
  fprintf(f, "%s\n", res->footer().c_str());
  fflush(f);
  fclose(f);

  return 0;
}

int io::validateSchema(string xsdPath_, string xmlPath_){
  QString xsdPath(QString::fromStdString(xsdPath_));
  QString xmlPath(QString::fromStdString(xmlPath_));

  // Setup schema object
  QXmlSchema schema;
  MessageHandler messenger;
  schema.setMessageHandler(&messenger);

  // Retrieve schema data
  QFile xsdFile(xsdPath);
  xsdFile.open(QIODevice::ReadOnly);
  QByteArray const schemaData = xsdFile.readAll();

  // Exec schema validation
  schema.load(schemaData);
  if( schema.isValid() ){
    // XML instance validator setup
    QXmlSchemaValidator validator(schema);
    QFile xmlFile(xmlPath);
    xmlFile.open(QIODevice::ReadOnly);
    QByteArray const instanceData = xmlFile.readAll();
    // Exec XML instance validator
    if( validator.validate(instanceData) ){
      cout << "XML instance validation was successful." << endl;
      return 0;
    }else{
      cout << "XML instance is invalid." << endl;
      cout << "Error '" << messenger.statusMessage().toStdString()
           << "' at line " << messenger.line()
           << ", column " << messenger.column() << "." << endl;
      return 1;
    }
  }else{
    cout << "XML schema is invalid." << endl;
    cout << "Error '" << messenger.statusMessage().toStdString()
         << "' at line " << messenger.line()
         << ", column " << messenger.column() << "." << endl;
    return 1;
  }
}
