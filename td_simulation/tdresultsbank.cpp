
#include "tdresultsbank.h"
using namespace elabtsaot;

//#include <map>
using std::map;
using std::pair;
//#include <string>
using std::string;

void TDResultsBank::reset(){
  _resultsDB.clear();
}

int TDResultsBank::getResults( string const& title, TDResults* p_res ) const{

  TDResultsBankConstIterator it = _resultsDB.find( title );
  if ( it == _resultsDB.end() )
    // Element with string key 'title' not found
    return 1;

  if ( p_res == NULL )
    return 2;

  *p_res = it->second;

  return 0;
}

int TDResultsBank::getResults( TDResultsBankConstIterator pos,
                               TDResults* p_res ) const{
  if ( !p_res )
    return 1;

  *p_res = pos->second;
  return 0;
}

int TDResultsBank::storeResults( TDResults const& results,
                                 TDResultsBankConstIterator* pos ){
  pair<map<string,TDResults>::iterator,bool> ret;
  ret = _resultsDB.insert( pair<string, TDResults>(results.title(),results) );
  if ( ret.second == false )
    // Element with title results.title() already existed!
    return 1;
  if ( pos )
    *pos = ret.first;

  return 0;
}

int TDResultsBank::updateResults( string const& title, TDResults const& res ){

  TDResultsBankIterator it = _resultsDB.find( title );
  if ( it == _resultsDB.end() )
    // Element with string key 'title' not found
    return 1;
  it->second = res;

  return 0;
}

int TDResultsBank::renameResults( string const& oldTitle,
                                  string const& newTitle ){
  // Get results
  TDResultsBankIterator itOld = _resultsDB.find( oldTitle );
  if ( itOld == _resultsDB.end() )
    return 1;
  TDResults res = itOld->second;

  // Change name
  res.set_title(newTitle);

  // Store changed results
  int ans = storeResults( res );
  if ( ans )
    return 2;

  // Delete old entry
  ans = deleteResults( oldTitle );
  if ( ans )
    return 3;

  return 0;
}

int TDResultsBank::deleteResults( string const& title ){ 

  TDResultsBankIterator it;
  it = _resultsDB.find( title );
  if ( it == _resultsDB.end() )
    // Element with string key 'title' not found
    return 1;
  _resultsDB.erase( it );

  return 0;
}

size_t TDResultsBank::getResultsDBCount() const{
  return _resultsDB.size();
}
TDResultsBankConstIterator TDResultsBank::getResultsDBBegin() const{
  return _resultsDB.begin();
}
TDResultsBankConstIterator TDResultsBank::getResultsDBEnd() const{
  return _resultsDB.end();
}
TDResultsBankConstIterator
TDResultsBank::findInResultsDB( string const& title ) const{
  return _resultsDB.find( title );
}
TDResultsBankIterator
TDResultsBank::findInResultsDB( string const& title ){
  return _resultsDB.find( title );
}
