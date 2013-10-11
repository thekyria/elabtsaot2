/*!
\file tdresults.h

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef TDRESULTS_H
#define TDRESULTS_H

#include "tdresultsidentifier.h"

#include <vector>
#include <string>

namespace elabtsaot{

class Powersystem;
class Scenario;

class TDResults{

 public:

  TDResults();
  TDResults( Powersystem const* pws, Scenario const* sce = NULL );
  TDResults( std::string const& sha1Digest,
             std::string const& title,
             std::string const& description,
             std::vector<bool> const& dataExists,
             std::vector<TDResultIdentifier> const& identifiers,
             std::vector<std::string> const& names,
             std::vector<std::string> const& units,
             std::vector<std::vector<double> > const& data,
             std::string const& footer );
  TDResults( std::vector<TDResultIdentifier> const& tdris );

  int storeResultsData( TDResultIdentifier const& tdri,
                        std::vector<double> const& data );
  void set_title( std::string const& val );
  void set_description( std::string const& val );
  int set_dataExists( size_t pos, bool dataExists );
  int set_name( size_t pos, std::string const& newName );
  int set_units( size_t pos, std::string const& newUnit );
  int set_data( size_t pos, std::vector<double> const& newData );
  void set_footer( std::string const& val);

  int getTimePosition() const;
  std::string sha1Digest() const;
  std::string title() const;
  std::string description() const;
  std::vector<bool> dataExists() const;
  std::vector<TDResultIdentifier> identifiers() const;
  std::vector<std::string> names() const;
  std::vector<std::string> units() const;
  std::vector<std::vector<double> > data() const;
  std::string footer() const;

 private:

  std::string _sha1Digest;
  std::string _title;
  std::string _description;
  std::vector<bool> _dataExists;
  std::vector<TDResultIdentifier> _identifiers;
  std::vector<std::string> _names;
  std::vector<std::string> _units;
  std::vector<std::vector<double> > _data;
  std::string _footer;

};

} // end of namespace elabtsaot

#endif // TDRESULTS_H
