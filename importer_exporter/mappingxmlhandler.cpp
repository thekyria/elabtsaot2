
#include "mappingxmlhandler.h"
using namespace elabtsaot;

#include "pwsmappermodel.h"

#include <QString>

#include <iostream>
using std::cout;
using std::endl;

MappingXMLHandler::MappingXMLHandler( PwsMapperModel* mmd ) :
    _el(NULL), _mmd(mmd) {
  _mmd->clear();
}

bool MappingXMLHandler::characters(QString const& ch){

  QString val = ch.trimmed();
  if ( val.length() == 0 )
    return true;

  bool ok = true;
  if ( _tags.contains("type") ){
    _el->type = val.toInt(&ok);
    if ( !ok ) cout << "'type' tag parsing error!" << endl;
  }
  ok = true; // maybe redundant?
  if ( _tags.contains("id") ){
    _el->extId = val.toInt(&ok);
    if ( !ok ) cout << "'id' tag parsing error!" << endl;
  }
  ok = true; // maybe redundant?
  if ( _tags.contains("mapped") ){
    _el->mapped = static_cast<bool>(val.toInt(&ok));
    if ( !ok ) cout << "'mapped' tag parsing error!" << endl;
  }
  ok = true; // maybe redundant?
  if ( _tags.contains("tab") ){
    _el->tab = val.toInt(&ok);
    if ( !ok ) cout << "'tab' tag parsing error!" << endl;
  }
  ok = true; // maybe redundant?
  if ( _tags.contains("row") ){
    _el->row = val.toInt(&ok);
    if ( !ok ) cout << "'row' tag parsing error!" << endl;
  }
  ok = true; // maybe redundant?
  if ( _tags.contains("col") ){
    _el->col = val.toInt(&ok);
    if ( !ok ) cout << "'col' tag parsing error!" << endl;
  }

  return true;
}

bool MappingXMLHandler::startElement(QString const& namespaceURI,
                                     QString const& localName,
                                     QString const& qName,
                                     QXmlAttributes const& atts){
  _tags.append( qName );
  if ( qName == "element" ){
    _resetState();
    _el = new PwsMapperModelElement();
  }

  return true;
}

bool MappingXMLHandler::endElement(QString const& namespaceURI,
                                   QString const& localName,
                                   QString const& qName){
  _tags.remove(_tags.size()-1);

  if ( qName == "element" ){
    int mdlId;
    int ans = _mmd->addElement( _el->type, _el->extId, true, &mdlId );
    if ( ans )
      cout << "Failed adding element of type " << _el->type
           << " and external id " << _el->extId;
    PwsMapperModelElement* el;
    el = _mmd->elementByIndex( _el->type, mdlId );
    el->mapped = _el->mapped;
    el->tab = _el->tab;
    el->row = _el->row;
    el->col = _el->col;
    _resetState();
  }

  return true;
}

int MappingXMLHandler::set_mmd( PwsMapperModel* mmd ){
  if ( mmd ){
    _mmd = mmd;
    return 0;
  } else {
    // Non-valid (NULL) argument provided as argument!
    return 1;
  }
}
PwsMapperModel* MappingXMLHandler::mmd() const{ return _mmd; }

void MappingXMLHandler::_resetState(){
  if ( _el ){
    delete _el;
    _el = NULL;
  }
}
