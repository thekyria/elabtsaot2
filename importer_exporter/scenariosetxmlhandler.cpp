
#include "scenariosetxmlhandler.h"
using namespace elabtsaot;

#include "scenarioset.h"

#include <QString>

#include <iostream>
using std::cout;
using std::endl;

ScenarioSetXMLHandler::ScenarioSetXMLHandler( ScenarioSet* scs ) :
    _sce(NULL), _evn(NULL), _scs(scs){

  _scs->reset();
}

bool ScenarioSetXMLHandler::characters(QString const& ch){

  QString value = ch.trimmed();

  if( value.length()==0 )
    return true;

  int ans = 0;
  bool ok = true;
  // General scenario set info
  if ( _tags.contains("id") && !_tags.contains("scenario") ){
    _scs->set_id( value.toUInt(&ok) );
    if ( !ok ) cout << "'id' tag parsing error!" << endl;
  }

  ok = true;
  if ( _tags.contains("name") && !_tags.contains("scenario") ){
    _scs->set_name( value.toStdString() );
    if ( !ok ) cout << "'name' tag parsing error!" << endl;
  }

  ok = true;
  if ( _tags.contains("description") && !_tags.contains("scenario") ){
    _scs->set_description( value.toStdString() );
    if ( !ok ) cout << "'description' tag parsing error!" << endl;
  }

  // Scenario data
  if ( _sce ){

    ok = true;
    if ( _tags.last() == "extId" && !_tags.contains("event") ){
      _sce->set_extId( value.toUInt(&ok) );
      if ( !ok ) cout << "'extId' tag parsing error!" << endl;
    }

    ok = true;
    if ( _tags.last() == "name" && !_tags.contains("event") ){
      _sce->set_name( value.toStdString() );
      if ( !ok ) cout << "'name' tag parsing error!" << endl;
    }

    ok = true;
    if ( _tags.last() == "description" && !_tags.contains("event") ){
      _sce->set_description( value.toStdString() );
      if ( !ok ) cout << "'description' tag parsing error!" << endl;
    }

    ok = true;
    if ( _tags.last() == "startTime" && !_tags.contains("event") ){
      ans = _sce->set_startTime( value.toDouble(&ok) );
      if ( ans || !ok ) cout << "'startTime' tag parsing error!" << endl;
    }

    ok = true;
    if ( _tags.last() == "stopTime" && !_tags.contains("event")  ){
      ans = _sce->set_stopTime( value.toDouble(&ok) );
      if ( ans || !ok ) cout << "'stopTime' tag parsing error!" << endl;
    }

    ok = true;
    if ( _tags.last() == "status" && !_tags.contains("event") ){
      _sce->set_status( static_cast<bool>(value.toInt(&ok)) );
      if ( !ok ) cout << "'status' tag parsing error!" << endl;
    }

    // Event data
    if ( _evn ){

      ok = true;
      if ( _tags.last() == "name" ){
        _evn->set_name( value.toStdString() );
        if ( !ok ) cout << "'name' tag parsing error!" << endl;
      }

      ok = true;
      if ( _tags.last() == "time" ){
        ans = _evn->set_time( value.toDouble(&ok) );
        if ( ans || !ok ) cout << "'time' tag parsing error!" << endl;
      }

      ok = true;
      if ( _tags.last() == "description" ){
        _evn->set_description( value.toStdString() );
        if ( !ok ) cout << "'description' tag parsing error!" << endl;
      }

      ok = true;
      if ( _tags.last() == "status" ){
        _evn->set_status( static_cast<bool>(value.toInt(&ok)) );
        if ( !ok ) cout << "'nastatusme' tag parsing error!" << endl;
      }

      ok = true;
      if ( _tags.last() == "element_type" ){
        _evn->set_element_type( value.toUInt(&ok) );
        if ( !ok ) cout << "'element_type' tag parsing error!" << endl;
      }

      ok = true;
      if ( _tags.last() == "element_extId" ){
        _evn->set_element_extId( value.toUInt(&ok) );
        if ( !ok ) cout << "'element_extId' tag parsing error!" << endl;
      }

      ok = true;
      if ( _tags.last() == "event_type" ){
        _evn->set_event_type( value.toUInt(&ok) );
        if ( !ok ) cout << "'event_type' tag parsing error!" << endl;
      }

      ok = true;
      if ( _tags.last() == "bool_arg" ){
        _evn->set_bool_arg( static_cast<bool>(value.toInt(&ok)) );
        if ( !ok ) cout << "'bool_arg' tag parsing error!" << endl;
      }

      ok = true;
      if ( _tags.last() == "double_arg" ){
        _evn->set_double_arg( value.toDouble(&ok) );
        if ( !ok ) cout << "'double_arg' tag parsing error!" << endl;
      }

      ok = true;
      if ( _tags.last() == "double_arg_1" ){
        _evn->set_double_arg_1( value.toDouble(&ok) );
        if ( !ok ) cout << "'double_arg_1' tag parsing error!" << endl;
      }

      ok = true;
      if ( _tags.last() == "double_arg_2" ){
        _evn->set_double_arg_2( value.toDouble(&ok) );
        if ( !ok ) cout << "'double_arg_2' tag parsing error!" << endl;
      }

      ok = true;
      if ( _tags.last() == "uint_arg" ){
        _evn->set_uint_arg( value.toUInt(&ok) );
        if ( !ok ) cout << "'uint_arg' tag parsing error!" << endl;
      }
    }
  }

  return true;
}

bool ScenarioSetXMLHandler::startElement(QString const& namespaceURI,
                                         QString const& localName,
                                         QString const& qName,
                                         QXmlAttributes const& atts){

  _tags.append(qName);
  if ( qName == "scenario" ){
    _resetScenario();
    _sce = new Scenario();
  } else if ( qName == "event" ){
    _resetEvent();
    _evn = new Event();
  }

  return true;
}

bool ScenarioSetXMLHandler::endElement( QString const& namespaceURI,
                                        QString const& localName,
                                        QString const& qName ){
  _tags.remove( _tags.size()-1 );
  int ans;

  if ( qName == "scenario" ){
    _scs->scenarios.push_back( *_sce );
    _resetScenario();

  } else if ( qName == "event" ){
    ans = _sce->insertEvent( *_evn );
    if ( ans )
      cout << "Failed adding event with code " << ans << endl;
    _resetEvent();
  }

  return true;
}

int ScenarioSetXMLHandler::set_scs( ScenarioSet* scs ){

  if ( scs ){
    _scs = scs;
    return 0;
  } else {
    // Non-valid (NULL) argument provided as argument!
    return 1;
  }
}

ScenarioSet* ScenarioSetXMLHandler::scs() const{ return _scs; }

void ScenarioSetXMLHandler::_resetScenario(){

  if ( _sce ){
    delete _sce;
    _sce = NULL;
  }

  return;
}

void ScenarioSetXMLHandler::_resetEvent(){

  if ( _evn ){
    delete _evn;
    _evn = NULL;
  }

  return;
}
