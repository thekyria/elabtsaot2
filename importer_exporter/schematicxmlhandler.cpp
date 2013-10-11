
#include "schematicxmlhandler.h"
using namespace elabtsaot;

#include "pwsschematicmodel.h"

#include <QString>

#include <iostream>
using std::cout;
using std::endl;

SchematicXMLHandler::SchematicXMLHandler( PwsSchematicModel* smd ) :
    _el(NULL), _smd(smd) {
  _smd->clear();
}

bool SchematicXMLHandler::characters(QString const& ch){

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
  if ( _tags.contains("x1") ){
    _el->x1 = val.toDouble(&ok);
    if ( !ok ) cout << "'x1' tag parsing error!" << endl;
  }
  ok = true; // maybe redundant?
  if ( _tags.contains("y1") ){
    _el->y1 = val.toDouble(&ok);
    if ( !ok ) cout << "'y1' tag parsing error!" << endl;
  }
  ok = true; // maybe redundant?
  if ( _tags.contains("x2") ){
    _el->x2 = val.toDouble(&ok);
    if ( !ok ) cout << "'x2' tag parsing error!" << endl;
  }
  ok = true; // maybe redundant?
  if ( _tags.contains("y2") ){
    _el->y2 = val.toDouble(&ok);
    if ( !ok ) cout << "'y2' tag parsing error!" << endl;
  }

  return true;
}

bool SchematicXMLHandler::startElement(QString const& namespaceURI,
                                       QString const& localName,
                                       QString const& qName,
                                       QXmlAttributes const& atts){
  _tags.append( qName );
  if ( qName == "element" ){
    _resetState();
    _el = new PwsSchematicModelElement();
  }

  return true;
}

bool SchematicXMLHandler::endElement(QString const& namespaceURI,
                                     QString const& localName,
                                     QString const& qName){
  _tags.remove(_tags.size()-1);

  if ( qName == "element" ){
    int mdlId;
    int ans = _smd->addElement( _el->type, _el->extId, true, &mdlId );
    if ( ans )
      cout << "Failed adding element of type " << _el->type
           << " and external id " << _el->extId;
    PwsSchematicModelElement* el;
    el = _smd->elementByIndex( _el->type, mdlId );
    el->x1 = _el->x1;
    el->x2 = _el->x2;
    el->y1 = _el->y1;
    el->y2 = _el->y2;
    _resetState();
  }

  return true;
}

int SchematicXMLHandler::set_smd( PwsSchematicModel* smd ){
  if ( smd ){
    _smd = smd;
    return 0;
  } else {
    // Non-valid (NULL) argument provided as argument!
    return 1;
  }
}
PwsSchematicModel* SchematicXMLHandler::smd() const{ return _smd; }

void SchematicXMLHandler::_resetState(){
  if ( _el ){
    delete _el;
    _el = NULL;
  }
}
