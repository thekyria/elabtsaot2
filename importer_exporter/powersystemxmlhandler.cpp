
#include "powersystemxmlhandler.h"
using namespace elabtsaot;

#include "powersystem.h"

#include <QString>

//#include <complex>
using std::complex;

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
  if (_tags.contains("info")){
    if (_tags.last()=="name")
      _p_ps->name = value.toStdString();
    if (_tags.last()=="baseMVA")
      _p_ps->baseS = value.toDouble();
    if (_tags.last()=="baseFreq")
      _p_ps->baseF = value.toDouble();
  }
  // Find which attribute is set
  // --------------- Bus ---------------
  if (_p_bus){
    if (_tags.last()=="extId")
      _p_bus->extId = value.toInt();
    if (_tags.last()=="name")
      _p_bus->name = value.toStdString();
    if (_tags.last()=="type")
      _p_bus->type = value.toInt();
    if (_tags.last()=="Gsh")
      _p_bus->Gsh = value.toDouble();
    if (_tags.last()=="Bsh")
      _p_bus->Bsh = value.toDouble();
    if (_tags.last()=="baseKV")
      _p_bus->baseKV = value.toDouble();
    if (_tags.last()=="P")
      _p_bus->baseKV = value.toDouble();
    if (_tags.last()=="Q")
      _p_bus->baseKV = value.toDouble();
    if (_tags.last()=="V")
      _p_bus->V = value.toDouble();
    if (_tags.last()=="theta")
      _p_bus->theta = value.toDouble();

  // --------------- Branch ---------------
  } else if(_p_br){
    if (_tags.last()=="extId")
      _p_br->extId = value.toDouble();
    if (_tags.last()=="status")
      _p_br->status = static_cast<bool>(value.toInt());
    if (_tags.last()=="fromBusExtId")
      _p_br->fromBusExtId = value.toInt();
    if (_tags.last()=="toBusExtId")
      _p_br->toBusExtId = value.toInt();
    if (_tags.last()=="R")
      _p_br->R = value.toDouble();
    if (_tags.last()=="X")
      _p_br->X = value.toDouble();
    if (_tags.last()=="Bfrom")
      _p_br->Bfrom = value.toDouble();
    if (_tags.last()=="Bto")
      _p_br->Bto = value.toDouble();
    if (_tags.last()=="Gfrom")
      _p_br->Gfrom = value.toDouble();
    if (_tags.last()=="Gto")
      _p_br->Gto = value.toDouble();
    if (_tags.last() == "Xratio" )
      _p_br->Xratio = value.toDouble();
    if (_tags.last() == "Xshift" )
      _p_br->Xshift = value.toDouble();

  // --------------- Generator ---------------
  } else if( _p_gen ){
    if( _tags.last()=="extId" )
      _p_gen->extId = value.toInt();
    if( _tags.last()=="status" )
      _p_gen->status = static_cast<bool>(value.toInt());
    if( _tags.last()=="name" )
      _p_gen->name = value.toStdString();
    if( _tags.last()=="busExtId" )
      _p_gen->busExtId = value.toInt();

    if( _tags.last()=="Pgen" )
      _p_gen->Pgen = value.toDouble();
    if( _tags.last()=="Qgen" )
      _p_gen->Qgen = value.toDouble();
    if( _tags.last()=="Vss_real" )
      _p_gen->Vss = complex<double>(value.toDouble(),_p_gen->Vss.imag());
    if( _tags.last()=="Vss_imag" )
      _p_gen->Vss = complex<double>(_p_gen->Vss.real(),value.toDouble());

    if( _tags.last()=="model" )
      _p_gen->model = value.toInt();
    if( _tags.last()=="xl" )
      _p_gen->xl = value.toDouble();
    if( _tags.last()=="ra" )
      _p_gen->ra = value.toDouble();
    if( _tags.last()=="xd" )
      _p_gen->xd = value.toDouble();
    if( _tags.last()=="xd_1" )
      _p_gen->xd_1 = value.toDouble();
    if( _tags.last()=="xd_2" )
      _p_gen->xd_2 = value.toDouble();
    if( _tags.last()=="Td0_1" )
      _p_gen->Td0_1 = value.toDouble();
    if( _tags.last()=="Td0_2" )
      _p_gen->Td0_2 = value.toDouble();
    if( _tags.last()=="xq" )
      _p_gen->xq = value.toDouble();
    if( _tags.last()=="xq_1" )
      _p_gen->xq_1 = value.toDouble();
    if( _tags.last()=="xq_2" )
      _p_gen->xq_2 = value.toDouble();
    if( _tags.last()=="Tq0_1" )
      _p_gen->Tq0_1 = value.toDouble();
    if( _tags.last()=="Tq0_2" )
      _p_gen->Tq0_2 = value.toDouble();
    if( _tags.last()=="M" )
      _p_gen->M = value.toDouble();
    if( _tags.last()=="D" )
      _p_gen->D = value.toDouble();

  // --------------- Load ---------------
  } else if( _p_load ){
    if( _tags.last()=="extId" )
      _p_load->extId = value.toInt();
    if( _tags.last()=="busExtId" )
      _p_load->busExtId = value.toInt();
    if( _tags.last()=="Pdemand" )
      _p_load->Pdemand = value.toDouble();
    if( _tags.last()=="Qdemand" )
      _p_load->Qdemand = value.toDouble();
    if( _tags.last()=="Vss_real" )
      _p_load->Vss = complex<double>(value.toDouble(),_p_load->Vss.imag());
    if( _tags.last()=="Vss_imag" )
      _p_load->Vss = complex<double>(_p_load->Vss.real(),value.toDouble());
    if( _tags.last()=="Vexpa" )
      _p_load->Vexpa = value.toDouble();
    if( _tags.last()=="Vexpb" )
      _p_load->Vexpb = value.toDouble();
    if( _tags.last()=="kpf" )
      _p_load->kpf = value.toDouble();
    if( _tags.last()=="kqf" )
      _p_load->kqf = value.toDouble();
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

  // Check if component instantiation has been finished
  if( qName=="bus" ){
    _p_ps->addBus( *_p_bus );
    _resetState();

  }else if( qName=="generator" ){
    _p_ps->addGen( *_p_gen );
    _resetState();

  }else if( qName=="load" ){
    _p_ps->addLoad( *_p_load );
    _resetState();

  }else if( qName=="branch" ){
    _p_ps->addBranch( *_p_br );
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
