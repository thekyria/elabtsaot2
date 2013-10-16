
#include "tabularpowersysteminterface.h"
using namespace elabtsaot;

#include "powersystem.h"
#include "pwsmappermodelelement.h"

// Implementation of methods of base class
TabularPowersystemInterface::TabularPowersystemInterface(int type,
                                                         Powersystem* pws,
                                                         QObject *parent) :
    QAbstractTableModel(parent), _type(type), _pws(pws){}

int TabularPowersystemInterface::rowCount(QModelIndex const& index) const{
  switch( _type ){
  case PWSMODELELEMENTTYPE_BUS: // Buses
    return _pws->getBusSet_size();
  case PWSMODELELEMENTTYPE_BR: // Branches
    return _pws->getBrSet_size();
  case PWSMODELELEMENTTYPE_GEN: // Generators
    return _pws->getGenSet_size();
  case PWSMODELELEMENTTYPE_LOAD: // Loads
    return _pws->getLoadSet_size();
  }

  return 0;
}

int TabularPowersystemInterface::columnCount(QModelIndex const& index) const{
  switch( _type ){
  case PWSMODELELEMENTTYPE_BUS: // Buses
    return 9;
  case PWSMODELELEMENTTYPE_BR: // Branches
    return 20;
  case PWSMODELELEMENTTYPE_GEN: // Generators
    return 31;
  case PWSMODELELEMENTTYPE_LOAD: // Loads
    return 12;
  }

  return 0;
}

QVariant TabularPowersystemInterface::data(QModelIndex const& index, int role) const{

  // Check that a valid index has been provided as an argument
  if( !index.isValid() || index.row()>=rowCount(QModelIndex()) )
    return QVariant();

  // Assert that the interface has a display role
  if( role!=Qt::DisplayRole ){
    return QVariant();
  }

  switch( _type ){

  case PWSMODELELEMENTTYPE_BUS:{ // Buses
    switch(index.column()){
    case 0: return _pws->getBus(index.row())->extId;
    case 1: return _pws->getBus(index.row())->name.c_str();
    case 2: return _pws->getBus(index.row())->Gsh;
    case 3: return _pws->getBus(index.row())->Bsh;
    case 4: return _pws->getBus(index.row())->baseKV;

    // Data resulting from the loadflow
    case 5: return _pws->getBus(index.row())->V;
    case 6: return _pws->getBus(index.row())->theta;
    case 7: return _pws->getBus(index.row())->P;
    case 8: return _pws->getBus(index.row())->Q;
    }
    break;
  }

  case PWSMODELELEMENTTYPE_BR:{ // Branches
    switch(index.column()){
    case  0: return _pws->getBranch(index.row())->extId;
    case  1: return _pws->getBranch(index.row())->status;
    case  2: return _pws->getBranch(index.row())->fromBusExtId;
    case  3: return _pws->getBranch(index.row())->toBusExtId;
    case  4: return _pws->getBranch(index.row())->R;
    case  5: return _pws->getBranch(index.row())->X;
    case  6: return _pws->getBranch(index.row())->Bfrom;
    case  7: return _pws->getBranch(index.row())->Bto;
    case  8: return _pws->getBranch(index.row())->Gfrom;
    case  9: return _pws->getBranch(index.row())->Gto;
    case 10: return _pws->getBranch(index.row())->Xratio;
    case 11: return _pws->getBranch(index.row())->Xshift;


    // Data resulting from the loadflow
    case 12: return _pws->getBranch(index.row())->Ifrom.real();
    case 13: return _pws->getBranch(index.row())->Ifrom.imag();
    case 14: return _pws->getBranch(index.row())->Ito.real();
    case 15: return _pws->getBranch(index.row())->Ito.imag();
    case 16: return _pws->getBranch(index.row())->Sfrom.real();
    case 17: return _pws->getBranch(index.row())->Sfrom.imag();
    case 18: return _pws->getBranch(index.row())->Sto.real();
    case 19: return _pws->getBranch(index.row())->Sto.imag();
    }
    break;
  }

  case PWSMODELELEMENTTYPE_GEN:{ // Generators
    switch(index.column()){
    case  0: return _pws->getGenerator(index.row())->extId();
    case  1: return _pws->getGenerator(index.row())->name().c_str();
    case  2: return _pws->getGenerator(index.row())->busExtId();
    case  3: return _pws->getGenerator(index.row())->avr();
    case  4: return _pws->getGenerator(index.row())->voltageSetpoint();
    case  5: return _pws->getGenerator(index.row())->status();

    case  6: return _pws->getGenerator(index.row())->pgen();
    case  7: return _pws->getGenerator(index.row())->qgen();
    case  8: return _pws->getGenerator(index.row())->pmin();
    case  9: return _pws->getGenerator(index.row())->pmax();
    case 10: return _pws->getGenerator(index.row())->qmin();
    case 11: return _pws->getGenerator(index.row())->qmax();

    case 12: return _pws->getGenerator(index.row())->model();
    case 13: return _pws->getGenerator(index.row())->xl();
    case 14: return _pws->getGenerator(index.row())->ra();
    case 15: return _pws->getGenerator(index.row())->xd();
    case 16: return _pws->getGenerator(index.row())->xd_1();
    case 17: return _pws->getGenerator(index.row())->xd_2();
    case 18: return _pws->getGenerator(index.row())->Td0_1();
    case 19: return _pws->getGenerator(index.row())->Td0_2();
    case 20: return _pws->getGenerator(index.row())->xq();
    case 21: return _pws->getGenerator(index.row())->xq_1();
    case 22: return _pws->getGenerator(index.row())->xq_2();
    case 23: return _pws->getGenerator(index.row())->Tq0_1();
    case 24: return _pws->getGenerator(index.row())->Tq0_2();
    case 25: return _pws->getGenerator(index.row())->M();
    case 26: return _pws->getGenerator(index.row())->D();

    // Data resulting from the loadflow
    case 27: return _pws->getGenerator(index.row())->Ess();
    case 28: return _pws->getGenerator(index.row())->deltass();
    case 29: return _pws->getGenerator(index.row())->Uss().real();
    case 30: return _pws->getGenerator(index.row())->Uss().imag();
    }
    break;
  }

  case PWSMODELELEMENTTYPE_LOAD:{ // Loads
    switch(index.column()){
    case  0: return _pws->getLoad(index.row())->extId();
    case  1: return _pws->getLoad(index.row())->name().c_str();
    case  2: return _pws->getLoad(index.row())->busExtId();
    case  3: return _pws->getLoad(index.row())->pdemand();
    case  4: return _pws->getLoad(index.row())->qdemand();
    case  5: return _pws->getLoad(index.row())->status();
    case  6: return _pws->getLoad(index.row())->v_exp_a();
    case  7: return _pws->getLoad(index.row())->v_exp_b();
    case  8: return _pws->getLoad(index.row())->k_p_f();
    case  9: return _pws->getLoad(index.row())->k_q_f();

    // Data resulting from the loadflow
    case 10: return _pws->getLoad(index.row())->Uss().real();
    case 11: return _pws->getLoad(index.row())->Uss().imag();
    }
    break;
  }
  default:{ // Any other
    return "";
    break;
  }
  } // end of switch( _type )

  return "";
}

bool TabularPowersystemInterface::setData(QModelIndex const& index,
                                          QVariant const& value,
                                          int role){

  // Assert that the index provided as an argument is valid and that the
  // interface has an edit role
  if( !index.isValid() || role!=Qt::EditRole )
    return false;

  QVariant v = value;
  switch( _type ){

  case PWSMODELELEMENTTYPE_BUS:{ // Buses
    Bus* bus = NULL;
    _pws->getBus( _pws->getBus(index.row())->extId, bus );
    if ( bus == NULL )
      return false;

    switch(index.column()){
    case 0: if(v.convert(QVariant::Int))
        bus->extId = v.toInt(); break;
    case 1: if(v.convert(QVariant::String))
        bus->name = v.toString().toStdString(); break;
    case 2: if(v.convert(QVariant::Double))
        bus->Gsh = v.toFloat(); break;
    case 3: if(v.convert(QVariant::Double))
        bus->Bsh = v.toFloat(); break;
    case 4: if(v.convert(QVariant::Double))
        bus->baseKV = v.toFloat(); break;
    }
    break;
  }

  case PWSMODELELEMENTTYPE_BR:{ // Branches
    Branch* branch = NULL;
    _pws->getBranch( _pws->getBranch(index.row())->extId, branch );
    if ( branch == NULL )
      return false;

    switch(index.column()){
    case  0: if(v.convert(QVariant::Int))
        branch->extId = v.toInt(); break;
    case  1: if(v.convert(QVariant::Int))
        branch->status = v.toBool(); break;
    case  2: if(v.convert(QVariant::Int))
        branch->fromBusExtId = v.toInt(); break;
    case  3: if(v.convert(QVariant::Int))
        branch->toBusExtId = v.toInt(); break;
    case  4: if(v.convert(QVariant::Double))
        branch->R = v.toFloat(); break;
    case  5: if(v.convert(QVariant::Double))
        branch->X = v.toFloat(); break;
    case  6: if(v.convert(QVariant::Double))
        branch->Bfrom = v.toFloat(); break;
    case  7: if(v.convert(QVariant::Double))
        branch->Bto = v.toFloat(); break;
    case  8: if(v.convert(QVariant::Double))
        branch->Gfrom = v.toFloat(); break;
    case  9: if(v.convert(QVariant::Double))
        branch->Gto = v.toFloat(); break;
    case 10: if(v.convert(QVariant::Double))
        branch->Xratio = v.toDouble(); break;
    case 11: if(v.convert(QVariant::Double))
        branch->Xshift = v.toDouble(); break;
    }
    break;
  }

  case PWSMODELELEMENTTYPE_GEN:{ // Generators
    Generator* gen = NULL;
    _pws->getGenerator( _pws->getGenerator(index.row())->extId(), gen );
    if ( gen == NULL )
      return false;

    switch(index.column()){
    case  0: if(v.convert(QVariant::Int))
        gen->set_extId( v.toInt() ); break;
    case  1: if(v.convert(QVariant::String))
        gen->set_name( v.toString().toStdString() ); break;
    case  2: if(v.convert(QVariant::Int))
        gen->set_busExtId( v.toInt() ); break;
    case  3: if(v.convert(QVariant::Bool))
        gen->set_avr( v.toBool() ); break;
    case  4: if(v.convert(QVariant::Double))
        gen->set_voltageSetpoint( v.toFloat() ); break;
    case  5: if(v.convert(QVariant::Int))
        gen->set_status( v.toBool() ); break;

    case  6: if(v.convert(QVariant::Double))
        gen->set_pgen( v.toFloat() ); break;
    case  7: if(v.convert(QVariant::Double))
        gen->set_qgen( v.toFloat() ); break;
    case  8: if(v.convert(QVariant::Double))
        gen->set_pmin( v.toFloat() ); break;
    case  9: if(v.convert(QVariant::Double))
        gen->set_pmax( v.toFloat() ); break;
    case 10: if(v.convert(QVariant::Double))
        gen->set_qmin( v.toFloat() ); break;
    case 11: if(v.convert(QVariant::Double))
        gen->set_qmax( v.toFloat() ); break;

    case 12: if(v.convert(QVariant::Int))
        gen->set_model( v.toInt() ); break;
    case 13: if(v.convert(QVariant::Double))
        gen->set_xl( v.toFloat() ); break;
    case 14: if(v.convert(QVariant::Double))
        gen->set_ra( v.toFloat() ); break;
    case 15: if(v.convert(QVariant::Double))
        gen->set_xd( v.toFloat() ); break;
    case 16: if(v.convert(QVariant::Double))
        gen->set_xd_1( v.toFloat() ); break;
    case 17: if(v.convert(QVariant::Double))
        gen->set_xd_2( v.toFloat() ); break;
    case 18: if(v.convert(QVariant::Double))
        gen->set_Td0_1( v.toFloat() ); break;
    case 19: if(v.convert(QVariant::Double))
        gen->set_Td0_2( v.toFloat() ); break;
    case 20: if(v.convert(QVariant::Double))
        gen->set_xq( v.toFloat() ); break;
    case 21: if(v.convert(QVariant::Double))
        gen->set_xq_1( v.toFloat() ); break;
    case 22: if(v.convert(QVariant::Double))
        gen->set_xq_2( v.toFloat() ); break;
    case 23: if(v.convert(QVariant::Double))
        gen->set_Tq0_1( v.toFloat() ); break;
    case 24: if(v.convert(QVariant::Double))
        gen->set_Tq0_2( v.toFloat() ); break;
    case 25: if(v.convert(QVariant::Double))
        gen->set_M( v.toFloat() ); break;
    case 26: if(v.convert(QVariant::Double))
        gen->set_D( v.toFloat() ); break;
    }
    break;
  }

  case PWSMODELELEMENTTYPE_LOAD:{ // Loads
    Load* load = NULL;
    _pws->getLoad( _pws->getLoad(index.row())->extId(), load );
    if ( load == NULL )
      return false;

    switch(index.column()){
    case  0: if(v.convert(QVariant::Int))
        load->set_extId( v.toInt() ); break;
    case  1: if(v.convert(QVariant::String))
        load->set_name( v.toString().toStdString() ); break;
    case  2: if(v.convert(QVariant::Int))
        load->set_busExtId( v.toInt() ); break;
    case  3: if(v.convert(QVariant::Double))
        load->set_pdemand( v.toFloat() ); break;
    case  4: if(v.convert(QVariant::Double))
        load->set_qdemand( v.toFloat() ); break;
    case  5: if(v.convert(QVariant::Int))
        load->set_status( v.toInt() ); break;
    case  6: if(v.convert(QVariant::Double))
        load->set_v_exp_a( v.toFloat() ); break;
    case  7: if(v.convert(QVariant::Double))
        load->set_v_exp_b( v.toFloat() ); break;
    case  8: if(v.convert(QVariant::Double))
        load->set_k_p_f( v.toFloat() ); break;
    case  9: if(v.convert(QVariant::Double))
        load->set_k_q_f( v.toFloat() ); break;
    }
    break;
  }

  default:{ // Any other
    break;
  }
  } // end of switch( _type )

  emit dataChanged(index,index);

  return true;
}

QVariant TabularPowersystemInterface::headerData(int section,
                                                 Qt::Orientation orientation,
                                                 int role) const{

  // Assert that the role of the interface is display and that the orientation
  // is horizontal
  if( role!=Qt::DisplayRole || orientation!=Qt::Horizontal )
    return QVariant();


  switch( _type ){

  case PWSMODELELEMENTTYPE_BUS:{ // Buses
    switch(section){
    case 0: return "ID";
    case 1: return "Name";
    case 2: return "Shunt Cond.";
    case 3: return "Shunt Susc.";
    case 4: return "BasekV";

    case 5: return "V";
    case 6: return "theta";
    case 7: return "P";
    case 8: return "Q";
    }
    break;
  }

  case PWSMODELELEMENTTYPE_BR:{ // Branches
    switch(section){
    case  0: return "extId";
    case  1: return "status";
    case  2: return "fromBusExtId";
    case  3: return "toBusExtId";
    case  4: return "R";
    case  5: return "X";
    case  6: return "Bfrom";
    case  7: return "Bto";
    case  8: return "Gfrom";
    case  9: return "Gto";
    case 10: return "Xratio";
    case 11: return "Xshift";

    case 12: return "re(If)";
    case 13: return "im(If)";
    case 14: return "re(It)";
    case 15: return "im(It)";
    case 16: return "re(Sf)";
    case 17: return "im(Sf)";
    case 18: return "re(St)";
    case 19: return "im(St)";
    }
    break;
  }

  case PWSMODELELEMENTTYPE_GEN:{ // Generators
    switch(section){
    case  0: return "ID";
    case  1: return "Name";
    case  2: return "Bus";
    case  3: return "AVR";
    case  4: return "Vset";
    case  5: return "Status";

    case  6: return "Pgen";
    case  7: return "Qgen";
    case  8: return "Pmin";
    case  9: return "Pmax";
    case 10: return "Qmin";
    case 11: return "Qmax";

    case 12: return "Model";
    case 13: return "x_l";
    case 14: return "r_a";
    case 15: return "x_d";
    case 16: return "x_d_1";
    case 17: return "x_d_2";
    case 18: return "Td0_1";
    case 19: return "Td0_2";
    case 20: return "x_q";
    case 21: return "x_q_1";
    case 22: return "x_q_2";
    case 23: return "Tq0_1";
    case 24: return "Tq0_2";
    case 25: return "M";
    case 26: return "D";

    case 27: return "Ess";
    case 28: return "Delta ss";
    case 29: return "re(Uss)";
    case 30: return "im(Uss)";
    }
    break;
  }

  case PWSMODELELEMENTTYPE_LOAD:{ // Loads
    switch(section){
    case  0: return "ID";
    case  1: return "Name";
    case  2: return "At Bus";
    case  3: return "Pdemand";
    case  4: return "Qdemand";
    case  5: return "Status";
    case  6: return "Vexp a";
    case  7: return "Vexp b";
    case  8: return "Kpf";
    case  9: return "Kqf";

    case 10: return "re(Uss)";
    case 11: return "im(Uss)";
    }
    break;
  }

  default:{ // Any other
    return QVariant();
    break;
  }

  } // end of switch( _type )

  return "";
}

Qt::ItemFlags TabularPowersystemInterface::flags(QModelIndex const& index) const {
  return QAbstractTableModel::flags(index)
          | Qt::ItemIsEditable
          | Qt::ItemIsSelectable;
}

void TabularPowersystemInterface::update(){
  emit this->layoutAboutToBeChanged();
  emit this->layoutChanged();
}
