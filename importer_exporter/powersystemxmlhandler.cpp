
#include "powersystemxmlhandler.h"
using namespace elabtsaot;

#include "powersystem.h"

#include <QString>

#include <iostream>
using std::cout;
using std::endl;

PowersystemXMLHandler::PowersystemXMLHandler( Powersystem* p_ps ) :
    _p_ps(p_ps), _p_bus(0), _p_br(0), _p_gen(0), _p_load(0){

  _p_ps->clear();
}

// Implementation of virtual functions
bool PowersystemXMLHandler::characters(QString const& ch){

  QString value = ch.trimmed();

  if( value.length()==0 )
    return true;

  // Fill in info
  if( _tags.contains("info") ){
    if( _tags.last()=="name" )
      _p_ps->set_name( value.toStdString() );
    if( _tags.last()=="description" )
      _p_ps->set_description( value.toStdString() );
    if( _tags.last()=="baseMVA" )
      _p_ps->set_baseS( value.toDouble() );
    if( _tags.last()=="baseFreq" )
      _p_ps->set_baseF( value.toDouble() );
    if( _tags.last()=="slackBus" )
      _p_ps->set_slackBusExtId( value.toInt() );
    if( _tags.last()=="slackGen" )
      _p_ps->set_slackGenExtId( value.toInt() );
  }
  // Find which attribute is set
  // --------------- Bus ---------------
  if ( _p_bus ){
    if( _tags.last()=="id" )
      _p_bus->set_extId( value.toInt() );
    if( _tags.last()=="name" )
      _p_bus->set_name( value.toStdString() );
    if( _tags.last()=="shconductance" )
      _p_bus->set_gsh( value.toDouble() );
    if( _tags.last()=="shsusceptance" )
      _p_bus->set_bsh( value.toDouble() );
    if( _tags.last()=="baseKV" )
      _p_bus->set_baseKV( value.toDouble() );
    if( _tags.last()=="vmax" )
      _p_bus->set_Vmax( value.toDouble() );
    if( _tags.last()=="vmin" )
      _p_bus->set_Vmin( value.toDouble() );
    if( _tags.last()=="v_steadystate" )
      _p_bus->set_Vss( value.toDouble() );
    if( _tags.last()=="th_steadystate" )
      _p_bus->set_thss( value.toDouble() );

  // --------------- Branch ---------------
  } else if( _p_br ){
    if( _tags.last()=="id" )
      _p_br->set_extId( value.toDouble() );
    if( _tags.last()=="name" )
      _p_br->set_name( value.toStdString() );
    if( _tags.last()=="frombus" )
      _p_br->set_fromBusExtId( value.toInt() );
    if( _tags.last()=="tobus" )
      _p_br->set_toBusExtId( value.toInt() );
    if( _tags.last()=="resistance" )
      _p_br->set_r( value.toDouble() );
    if( _tags.last()=="reactance" )
      _p_br->set_x( value.toDouble() );
    if( _tags.last()=="susceptance" )
      _p_br->set_b( value.toDouble() );
    if( _tags.last()=="series_capacitance" )
      _p_br->set_c_series_x( value.toDouble() );
    if (_tags.last() == "xratio" )
      _p_br->set_Xratio( value.toDouble() );
    if (_tags.last() == "xratio_tap" )
      _p_br->set_Xratio_tap( value.toInt() );
    if (_tags.last() == "xratio_base" )
      _p_br->set_Xratio_base( value.toDouble() );
    if (_tags.last() == "xratio_tap_min" )
      _p_br->set_Xratio_tap_min( value.toInt() );
    if (_tags.last() == "xratio_tap_max" )
      _p_br->set_Xratio_tap_max( value.toInt() );
    if (_tags.last() == "xratio_tap_step" )
      _p_br->set_Xratio_tap_step( value.toDouble() );
    if (_tags.last() == "xshift" )
      _p_br->set_Xshift( value.toDouble() );
    if (_tags.last() == "xshift_tap" )
      _p_br->set_Xshift_tap( value.toInt() );
    if (_tags.last() == "xshift_base" )
      _p_br->set_Xshift_base( value.toDouble() );
    if (_tags.last() == "xshift_tap_min" )
      _p_br->set_Xshift_tap_min( value.toInt() );
    if (_tags.last() == "xshift_tap_max" )
      _p_br->set_Xshift_tap_max( value.toInt() );
    if (_tags.last() == "xshift_tap_step" )
      _p_br->set_Xshift_tap_step( value.toDouble() );
    if( _tags.last()=="status" )
      _p_br->set_status( static_cast<bool>( value.toInt() ) );

  // --------------- Generator ---------------
  } else if( _p_gen ){
    if( _tags.last()=="id" )
      _p_gen->set_extId( value.toInt() );
    if( _tags.last()=="name" )
      _p_gen->set_name( value.toStdString() );
    if( _tags.last()=="atbus" )
      _p_gen->set_busExtId( value.toInt() );
    if ( _tags.last() == "avr" )
      _p_gen->set_avr( static_cast<bool>( value.toInt() ) );
    if( _tags.last()=="vsetpoint" )
      _p_gen->set_voltageSetpoint( value.toDouble() );
    if( _tags.last()=="status" )
      _p_gen->set_status( static_cast<bool>( value.toInt() ) );
    if( _tags.last()=="pgen" )
      _p_gen->set_pgen( value.toDouble() );
    if( _tags.last()=="qgen" )
      _p_gen->set_qgen( value.toDouble() );
    if( _tags.last()=="pmin" )
      _p_gen->set_pmin( value.toDouble() );
    if( _tags.last()=="qmin" )
      _p_gen->set_qmin( value.toDouble() );
    if( _tags.last()=="pmax" )
      _p_gen->set_pmax( value.toDouble() );
    if( _tags.last()=="qmax" )
      _p_gen->set_qmax( value.toDouble() );
    if( _tags.last()=="model" )
      _p_gen->set_model( value.toInt() );
    if( _tags.last()=="leakage_reactance" )
      _p_gen->set_xl( value.toDouble() );
    if( _tags.last()=="armature_resistance" )
      _p_gen->set_ra( value.toDouble() );
    if( _tags.last()=="synchronous_direct_reactance" )
      _p_gen->set_xd( value.toDouble() );
    if( _tags.last()=="transient_direct_reactance" )
      _p_gen->set_xd_1( value.toDouble() );
    if( _tags.last()=="subtransient_direct_reactance" )
      _p_gen->set_xd_2( value.toDouble() );
    if( _tags.last()=="transient_direct_time_const" )
      _p_gen->set_Td0_1( value.toDouble() );
    if( _tags.last()=="subtransient_direct_time_const" )
      _p_gen->set_Td0_2( value.toDouble() );
    if( _tags.last()=="synchronous_quadrature_reactance" )
      _p_gen->set_xq( value.toDouble() );
    if( _tags.last()=="transient_quadrature_reactance" )
      _p_gen->set_xq_1( value.toDouble() );
    if( _tags.last()=="subtransient_quadrature_reactance" )
      _p_gen->set_xq_2( value.toDouble() );
    if( _tags.last()=="transient_quadrature_time_const" )
      _p_gen->set_Tq0_1( value.toDouble() );
    if( _tags.last()=="subtransient_quadrature_time_const" )
      _p_gen->set_Tq0_2( value.toDouble() );
    if( _tags.last()=="mechanical_starting_time" )
      _p_gen->set_M( value.toDouble() );
    if( _tags.last()=="damping_coefficient" )
      _p_gen->set_D( value.toDouble() );

  // --------------- Load ---------------
  } else if( _p_load ){
    if( _tags.last()=="id" )
      _p_load->set_extId( value.toInt() );
    if( _tags.last()=="name" )
      _p_load->set_name( value.toStdString() );
    if( _tags.last()=="atbus" )
      _p_load->set_busExtId( value.toInt() );
    if( _tags.last()=="pdemand" )
      _p_load->set_pdemand( value.toDouble() );
    if( _tags.last()=="qdemand" )
      _p_load->set_qdemand( value.toDouble() );
    if( _tags.last()=="status" )
      _p_load->set_status( static_cast<bool>( value.toInt() ) );
    if( _tags.last()=="v_exp_a" )
      _p_load->set_v_exp_a( value.toDouble() );
    if( _tags.last()=="v_exp_b" )
      _p_load->set_v_exp_b( value.toDouble() );
    if( _tags.last()=="k_p_f" )
      _p_load->set_k_p_f( value.toDouble() );
    if( _tags.last()=="k_q_f" )
      _p_load->set_k_q_f( value.toDouble() );
  }

  return true;
}

bool PowersystemXMLHandler::startElement(QString const& namespaceURI,
                                         QString const& localName,
                                         QString const& qName,
                                         QXmlAttributes const& atts){
  _tags.append(qName);
  // Parse whether a new net component is instantiated
  if( qName=="bus" ){
    _resetState();
    _p_bus = new Bus();
  }else if( qName=="generator" ){
    _resetState();
    _p_gen = new Generator();
  }else if( qName=="load" ){
    _resetState();
    _p_load = new Load();
  }else if( qName=="branch" ){
    _resetState();
    _p_br = new Branch();
  }

  return true;
}

bool PowersystemXMLHandler::endElement(QString const& namespaceURI,
                                       QString const& localName,
                                       QString const& qName){
  _tags.remove(_tags.size()-1);
  int ans;

  // Check if component instantiation has been finished
  if( qName=="bus" ){
    ans = _p_ps->addBus( *_p_bus );
    if( ans )
      cout << "Failed adding bus with code " << ans << "." << endl;
    _resetState();

  }else if( qName=="generator" ){
    ans = _p_ps->addGen( *_p_gen );
    if( ans )
      cout << "Failed adding generator with code " << ans << "." << endl;
    _resetState();

  }else if( qName=="load" ){
    ans = _p_ps->addLoad( *_p_load );
    if( ans )
      cout << "Failed adding load with code " << ans << "." << endl;
    _resetState();

  }else if( qName=="branch" ){
    ans = _p_ps->addBranch( *_p_br );
    if( ans )
      cout << "Failed adding branch with code " << ans << "." << endl;
    _resetState();
  }

  return true;
}

int PowersystemXMLHandler::set_p_ps( Powersystem* p_ps ){
  if ( p_ps ){
    _p_ps = p_ps;
    return 0;
  } else {
    // Non-valid (NULL) pointer provided as argument!
    return 1;
  }
}
Powersystem* PowersystemXMLHandler::p_ps() const{  return _p_ps; }

void PowersystemXMLHandler::_resetState(){ // Reset handler to its first stage
  if( _p_bus ){
    delete _p_bus;
    _p_bus = 0;
  }
  if( _p_gen ){
    delete _p_gen;
    _p_gen = 0;
  }
  if( _p_load ){
    delete _p_load;
    _p_load = 0;
  }
  if( _p_br ){
    delete _p_br;
    _p_br = 0;
  }
  return;
}
