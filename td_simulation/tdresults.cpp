
#include "tdresults.h"
using namespace elabtsaot;

#include "scenario.h"
#include "sha1.h"
#include "auxiliary.h"
#include "powersystem.h"

//#include <vector>
using std::vector;
//#include <string>
using std::string;
#include <ctime>
#include <string>
using std::string;

TDResults::TDResults(){
  unsigned int error_digest[5];
  for ( size_t k = 0 ; k != 5 ; ++k ) error_digest[k] = 0;
  _sha1Digest = auxiliary::SHA1toStr(error_digest);
}

TDResults::TDResults( Powersystem const* pws, Scenario const* sce ){

  // ----- _sha1Digest -----
  if ( sce != NULL ){
    string strInp = pws->serialize() + sce->serialize();
    SHA1 sha;
    sha.Reset();
    sha << auxiliary::str2chararr(strInp);
    unsigned int message_digest[5];
    if(!sha.Result(message_digest)) {
//    cerr << "ERROR-- could not compute message digest" << endl;
      unsigned int error_digest[5];
      for ( size_t k = 0 ; k != 5 ; ++k ) error_digest[k] = 0;
      _sha1Digest = auxiliary::SHA1toStr(error_digest);
    } else  {
      _sha1Digest = auxiliary::SHA1toStr(message_digest);
    }
  } else { // sce == NULL
    unsigned int error_digest[5];
    for ( size_t k = 0 ; k != 5 ; ++k ) error_digest[k] = 0;
    _sha1Digest = auxiliary::SHA1toStr(error_digest);
  }

  // ----- title & description -----
  // ----- footer -----
  time_t unixTime = time(NULL);
  _footer = "TDResults struct created at "
           + auxiliary::to_string(unixTime) + " (unix time)";

  // --- _dataExists & _identifiers & _names & _units ---
//  _dataExists.clear();
//  _identifiers.clear();
//  _names.clear();
//  _units.clear();

  // Time
  TDResultIdentifier tdri;
  tdri.elementType = TDRESULTS_OTHER;
  tdri.variable = TDRESULTS_TIME;
  tdri.extId = 0;
  _dataExists.push_back( true );
  _identifiers.push_back( tdri );
  _names.push_back( TDResultIdentifier::defaultName(tdri) );
  _units.push_back( TDResultIdentifier::defaultUnits(tdri) );

  // Dynamic variables of generators
  for ( size_t k = 0 ; k != pws->getGenSet_size() ; ++k ){
    Generator const* p_gen = pws->getGenerator( k );
    unsigned int gen_extId = p_gen->extId();
    int gen_model = p_gen->model();
    tdri.elementType = TDRESULTS_GENERATOR;
    tdri.extId = gen_extId;

    // Model 0.0 "classical"
    if ( gen_model == GENMODEL_0p0 ){
      // Internal angle (delta) in [rad]
      tdri.variable = TDRESULTS_ANGLE;
      _dataExists.push_back( false );
      _identifiers.push_back( tdri );
      _names.push_back(TDResultIdentifier::defaultName(tdri));
      _units.push_back(TDResultIdentifier::defaultUnits(tdri));

      // Electrical speed (omega) in [pu]
      tdri.variable = TDRESULTS_SPEED;
      _dataExists.push_back( false );
      _identifiers.push_back( tdri );
      _names.push_back(TDResultIdentifier::defaultName(tdri));
      _units.push_back(TDResultIdentifier::defaultUnits(tdri));

      // Accelerating power (pa) in [pu]
      tdri.variable = TDRESULTS_ACCELERATING_POWER;
      _dataExists.push_back( false );
      _identifiers.push_back( tdri );
      _names.push_back(TDResultIdentifier::defaultName(tdri));
      _units.push_back(TDResultIdentifier::defaultUnits(tdri));

      // Voltage (real) at the bus of the generator in [pu]
      tdri.variable = TDRESULTS_VOLTAGE_REAL;
      _dataExists.push_back( false );
      _identifiers.push_back( tdri );
      _names.push_back(TDResultIdentifier::defaultName(tdri));
      _units.push_back(TDResultIdentifier::defaultUnits(tdri));

      // Voltage (imag) at the bus of the generator in [pu]
      tdri.variable = TDRESULTS_VOLTAGE_IMAG;
      _dataExists.push_back( false );
      _identifiers.push_back( tdri );
      _names.push_back(TDResultIdentifier::defaultName(tdri));
      _units.push_back(TDResultIdentifier::defaultUnits(tdri));

      // Current (real) at the bus of generator in [pu]
      tdri.variable = TDRESULTS_CURRENT_REAL;
      _dataExists.push_back( false );
      _identifiers.push_back( tdri );
      _names.push_back(TDResultIdentifier::defaultName(tdri));
      _units.push_back(TDResultIdentifier::defaultUnits(tdri));

      // Current (imag) at the bus of generator in [pu]
      tdri.variable = TDRESULTS_CURRENT_IMAG;
      _dataExists.push_back( false );
      _identifiers.push_back( tdri );
      _names.push_back(TDResultIdentifier::defaultName(tdri));
      _units.push_back(TDResultIdentifier::defaultUnits(tdri));
    }

    // Model 1.1
    else if ( gen_model == GENMODEL_1p1 ){
      // Internal angle (delta) in [rad]
      tdri.variable = TDRESULTS_ANGLE;
      _dataExists.push_back( false );
      _identifiers.push_back( tdri );
      _names.push_back(TDResultIdentifier::defaultName(tdri));
      _units.push_back(TDResultIdentifier::defaultUnits(tdri));

      // Electrical speed (omega) in [pu]
      tdri.variable = TDRESULTS_SPEED;
      _dataExists.push_back( false );
      _identifiers.push_back( tdri );
      _names.push_back(TDResultIdentifier::defaultName(tdri));
      _units.push_back(TDResultIdentifier::defaultUnits(tdri));

      // Accelerating power (pa) in [pu]
      tdri.variable = TDRESULTS_ACCELERATING_POWER;
      _dataExists.push_back( false );
      _identifiers.push_back( tdri );
      _names.push_back(TDResultIdentifier::defaultName(tdri));
      _units.push_back(TDResultIdentifier::defaultUnits(tdri));

      // Voltage (real) at the bus of the generator in [pu]
      tdri.variable = TDRESULTS_VOLTAGE_REAL;
      _dataExists.push_back( false );
      _identifiers.push_back( tdri );
      _names.push_back(TDResultIdentifier::defaultName(tdri));
      _units.push_back(TDResultIdentifier::defaultUnits(tdri));

      // Voltage (imag) at the bus of the generator in [pu]
      tdri.variable = TDRESULTS_VOLTAGE_IMAG;
      _dataExists.push_back( false );
      _identifiers.push_back( tdri );
      _names.push_back(TDResultIdentifier::defaultName(tdri));
      _units.push_back(TDResultIdentifier::defaultUnits(tdri));

      // q-axis voltage behind transient reactance in [pu]
      tdri.variable = TDRESULTS_VOLTAGE_Q_TR;
      _dataExists.push_back( false );
      _identifiers.push_back( tdri );
      _names.push_back(TDResultIdentifier::defaultName(tdri));
      _units.push_back(TDResultIdentifier::defaultUnits(tdri));

      // d-axis voltage behind transient reactance in [pu]
      tdri.variable = TDRESULTS_VOLTAGE_D_TR;
      _dataExists.push_back( false );
      _identifiers.push_back( tdri );
      _names.push_back(TDResultIdentifier::defaultName(tdri));
      _units.push_back(TDResultIdentifier::defaultUnits(tdri));

    }

    // Unsupported generator model!
    else {}
  }

  // Algebraic variables of buses
  for ( size_t k = 0 ; k != pws->getBusSet_size() ; ++k ){
    Bus const* p_bus = pws->getBus(k);
    unsigned int bus_extId = p_bus->extId;
    tdri.elementType = TDRESULTS_BUS;
    tdri.extId = bus_extId;

    // Bus voltage (real) in [pu]
    tdri.variable =  TDRESULTS_VOLTAGE_REAL;
    _dataExists.push_back( false );
    _identifiers.push_back( tdri );
    _names.push_back(TDResultIdentifier::defaultName(tdri));
    _units.push_back(TDResultIdentifier::defaultUnits(tdri));

    // Bus voltage (imag) in [rad]
    tdri.variable = TDRESULTS_VOLTAGE_IMAG;
    _dataExists.push_back( false );
    _identifiers.push_back( tdri );
    _names.push_back(TDResultIdentifier::defaultName(tdri));
    _units.push_back(TDResultIdentifier::defaultUnits(tdri));
  }

  // Algebraic variables for loads
  for ( size_t k = 0 ; k != pws->getLoadSet_size() ; ++k ){
    Load const* p_load = pws->getLoad( k );
    unsigned int load_extId = p_load->extId();
    tdri.elementType = TDRESULTS_LOAD;
    tdri.extId = load_extId;

    // Voltage (real) of the load bus in [pu]
    tdri.variable = TDRESULTS_VOLTAGE_REAL;
    _dataExists.push_back( false );
    _identifiers.push_back( tdri );
    _names.push_back(TDResultIdentifier::defaultName(tdri));
    _units.push_back(TDResultIdentifier::defaultUnits(tdri));

    // Voltage (imag) of the load bus in [pu]
    tdri.variable = TDRESULTS_VOLTAGE_IMAG;
    _dataExists.push_back( false );
    _identifiers.push_back( tdri );
    _names.push_back(TDResultIdentifier::defaultName(tdri));
    _units.push_back(TDResultIdentifier::defaultUnits(tdri));
  }

  // --- _data ---
  _data.resize( _dataExists.size() );
  // Note: the second dimension of the result._data vectors is left to the
  // discretion of the td-engine
}

TDResults::TDResults( string const& sha1Digest,
                      string const& title,
                      string const& description,
                      vector<bool> const& dataExists,
                      vector<TDResultIdentifier> const& identifiers,
                      vector<string> const& names,
                      vector<string> const& units,
                      vector<vector<double> > const& data,
                      string const& footer ) :
    _sha1Digest(sha1Digest),
    _title(title),
    _description(description),
    _dataExists(dataExists),
    _identifiers(identifiers),
    _names(names),
    _units(units),
    _data(data),
    _footer(footer) {}

TDResults::TDResults( vector<TDResultIdentifier> const& tdris ){
  string name, units;
  for ( size_t k = 0 ; k != tdris.size() ; ++k ){
    name = TDResultIdentifier::defaultName( tdris[k] );
    units = TDResultIdentifier::defaultUnits( tdris[k] );
    _dataExists.push_back( true );
    _identifiers.push_back( tdris[k] );
    _names.push_back( name );
    _units.push_back( units );
  }
  _data.resize( _dataExists.size() ); // resize data to null vectors
}

int TDResults::storeResultsData( TDResultIdentifier const& tdri,
                                 vector<double> const& data ){
  if ( !TDResultIdentifier::isValid( tdri.elementType, tdri.variable ) )
    return 1;
  string name, units;

  name = TDResultIdentifier::defaultName( tdri );
  units = TDResultIdentifier::defaultUnits( tdri );
  _dataExists.push_back( true );
  _identifiers.push_back( tdri );
  _data.push_back( data );
  _names.push_back( name );
  _units.push_back( units );

  return 0;
}

void TDResults::set_title( string const& val ){ _title = val; }
void TDResults::set_description( string const& val ){ _description = val; }
int TDResults::set_dataExists( size_t pos,
                               bool newDataExists ){
  if ( pos >= _dataExists.size() )
    return 1;
  _dataExists[pos] = newDataExists;
  return 0;
}
int TDResults::set_name( size_t pos, string const& newName ){
  if ( pos >= _names.size() )
    return 1;
  _names[pos] = newName;
  return 0;
}
int TDResults::set_units( size_t pos, string const& newUnit ){
  if ( pos >= _units.size() )
    return 1;
  _units[pos] = newUnit;
  return 0;
}
int TDResults::set_data( size_t pos, vector<double> const& newData){
//  if ( pos >= _data.size() )
  if ( pos >= _dataExists.size() )
    return 1;
  _data[pos] = newData;
  return 0;
}
void TDResults::set_footer( string const& val){ _footer = val; }

int TDResults::getTimePosition() const{
  int ans = -1;
  for ( size_t k = 0 ; k != dataExists().size() ; ++k ){
    if ( identifiers().at(k).elementType == TDRESULTS_OTHER
         && identifiers().at(k).variable == TDRESULTS_TIME ){
      ans = k;
      break;
    }
  }
  return ans;
}
string TDResults::sha1Digest() const{ return _sha1Digest; }
string TDResults::title() const{ return _title; }
string TDResults::description() const{ return _description; }
vector<bool> TDResults::dataExists() const{ return _dataExists; }
vector<TDResultIdentifier> TDResults::identifiers() const{ return _identifiers; }
vector<string> TDResults::names() const{ return _names; }
vector<string> TDResults::units() const{ return _units; }
vector<vector<double> > TDResults::data() const{ return _data; }
string TDResults::footer() const{ return _footer; }
