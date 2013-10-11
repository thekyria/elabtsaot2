/*!
\file tdresultsbank.h

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef TDRESULTSBANK
#define TDRESULTSBANK

#include <map>
#include <string>

#include <tdresults.h>

namespace elabtsaot{

typedef std::map<std::string,TDResults>::const_iterator TDResultsBankConstIterator;
typedef std::map<std::string,TDResults>::iterator TDResultsBankIterator;

class TDResultsBank{

 public:

  TDResultsBank(){}

  void reset();
  int getResults( std::string const& title, TDResults* p_res ) const;
  int getResults( TDResultsBankConstIterator pos, TDResults* p_res ) const;
  int storeResults( TDResults const& results,
                    TDResultsBankConstIterator* pos = 0 );
  int updateResults( std::string const& title, TDResults const& res );
  int renameResults( std::string const& oldTitle, std::string const& newTitle );
  int deleteResults( std::string const& title );

  size_t getResultsDBCount() const;
  TDResultsBankConstIterator getResultsDBBegin() const;
  TDResultsBankConstIterator getResultsDBEnd() const;
  TDResultsBankConstIterator findInResultsDB( std::string const& title ) const;
  TDResultsBankIterator findInResultsDB( std::string const& title );

 private:

  std::map<std::string,TDResults> _resultsDB;

};

} // end of namespace elabtsaot

#endif // TDRESULTSBANK
