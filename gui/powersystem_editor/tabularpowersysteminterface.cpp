
#include "tabularpowersysteminterface.h"
using namespace elabtsaot;

#include "powersystem.h"
#include "pwsmappermodelelement.h"

//#include <complex>
using std::complex;

// Implementation of methods of base class
TabularPowersystemInterface::TabularPowersystemInterface(int type,
                                                         Powersystem* pws,
                                                         QObject *parent) :
    QAbstractTableModel(parent), _type(type), _pws(pws){}

int TabularPowersystemInterface::rowCount(QModelIndex const& index) const{
  switch( _type ){
  case PWSMODELELEMENTTYPE_BUS: // Buses
    return _pws->getBusCount();
  case PWSMODELELEMENTTYPE_BR: // Branches
    return _pws->getBranchCount();
  case PWSMODELELEMENTTYPE_GEN: // Generators
    return _pws->getGenCount();
  case PWSMODELELEMENTTYPE_LOAD: // Loads
    return _pws->getLoadCount();
  }

  return 0;
}

int TabularPowersystemInterface::columnCount(QModelIndex const& index) const{
  switch( _type ){
  case PWSMODELELEMENTTYPE_BUS: // Buses
    return 10;
  case PWSMODELELEMENTTYPE_BR: // Branches
    return 12;
  case PWSMODELELEMENTTYPE_GEN: // Generators
    return 23;
  case PWSMODELELEMENTTYPE_LOAD: // Loads
    return 10;
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
    case 2: return _pws->getBus(index.row())->type;
    case 3: return _pws->getBus(index.row())->Gsh;
    case 4: return _pws->getBus(index.row())->Bsh;
    case 5: return _pws->getBus(index.row())->baseKV;
    case 6: return _pws->getBus(index.row())->P;
    case 7: return _pws->getBus(index.row())->Q;
    case 8: return _pws->getBus(index.row())->V;
    case 9: return _pws->getBus(index.row())->theta;
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
    }
    break;
  }

  case PWSMODELELEMENTTYPE_GEN:{ // Generators
    switch(index.column()){
    case  0: return _pws->getGenerator(index.row())->extId;
    case  1: return _pws->getGenerator(index.row())->status;
    case  2: return _pws->getGenerator(index.row())->name.c_str();
    case  3: return _pws->getGenerator(index.row())->busExtId;

    case  4: return _pws->getGenerator(index.row())->Pgen;
    case  5: return _pws->getGenerator(index.row())->Qgen;
    case  6: return _pws->getGenerator(index.row())->Vss.real();
    case  7: return _pws->getGenerator(index.row())->Vss.imag();

    case  8: return _pws->getGenerator(index.row())->model;
    case  9: return _pws->getGenerator(index.row())->xl;
    case 10: return _pws->getGenerator(index.row())->ra;
    case 11: return _pws->getGenerator(index.row())->xd;
    case 12: return _pws->getGenerator(index.row())->xd_1;
    case 13: return _pws->getGenerator(index.row())->xd_2;
    case 14: return _pws->getGenerator(index.row())->Td0_1;
    case 15: return _pws->getGenerator(index.row())->Td0_2;
    case 16: return _pws->getGenerator(index.row())->xq;
    case 17: return _pws->getGenerator(index.row())->xq_1;
    case 18: return _pws->getGenerator(index.row())->xq_2;
    case 19: return _pws->getGenerator(index.row())->Tq0_1;
    case 20: return _pws->getGenerator(index.row())->Tq0_2;
    case 21: return _pws->getGenerator(index.row())->M;
    case 22: return _pws->getGenerator(index.row())->D;
    }
    break;
  }

  case PWSMODELELEMENTTYPE_LOAD:{ // Loads
    switch(index.column()){
    case 0: return _pws->getLoad(index.row())->extId;
    case 1: return _pws->getLoad(index.row())->busExtId;

    case 2: return _pws->getLoad(index.row())->Pdemand;
    case 3: return _pws->getLoad(index.row())->Qdemand;
    case 4: return _pws->getLoad(index.row())->Vss.real();
    case 5: return _pws->getLoad(index.row())->Vss.imag();

    case 6: return _pws->getLoad(index.row())->Vexpa;
    case 7: return _pws->getLoad(index.row())->Vexpb;
    case 8: return _pws->getLoad(index.row())->kpf;
    case 9: return _pws->getLoad(index.row())->kqf;
    }
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
    case 2: if(v.convert(QVariant::Int))
        bus->type = v.toInt(); break;
    case 3: if(v.convert(QVariant::Double))
        bus->Gsh = v.toFloat(); break;
    case 4: if(v.convert(QVariant::Double))
        bus->Bsh = v.toFloat(); break;
    case 5: if(v.convert(QVariant::Double))
        bus->baseKV = v.toFloat(); break;
    case 6: if(v.convert(QVariant::Double))
        bus->P = v.toFloat(); break;
    case 7: if(v.convert(QVariant::Double))
        bus->Q = v.toFloat(); break;
    case 8: if(v.convert(QVariant::Double))
        bus->V = v.toFloat(); break;
    case 9: if(v.convert(QVariant::Double))
        bus->theta = v.toFloat(); break;
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
    _pws->getGenerator(_pws->getGenerator(index.row())->extId, gen);
    if ( gen == NULL )
      return false;

    switch(index.column()){
    case  0: if(v.convert(QVariant::Int))
        gen->extId=v.toInt(); break;
    case  1: if(v.convert(QVariant::Int))
        gen->status=v.toBool(); break;
    case  2: if(v.convert(QVariant::String))
        gen->name=v.toString().toStdString(); break;
    case  3: if(v.convert(QVariant::Int))
        gen->busExtId=v.toInt(); break;
//    case  4: if(v.convert(QVariant::Double))
//        gen->Pgen=v.toFloat(); break;
//    case  5: if(v.convert(QVariant::Double))
//        gen->Qgen=v.toFloat(); break;
//    case  6: if(v.convert(QVariant::Double))
//        gen->Vss=complex<double>(v.toFloat(),gen->Vss.imag()); break;
//    case  7: if(v.convert(QVariant::Double))
//        gen->Vss=complex<double>(gen->Vss.real(),v.toFloat()); break;
    case  8: if(v.convert(QVariant::Int))
        gen->model=v.toInt(); break;
    case  9: if(v.convert(QVariant::Double))
        gen->xl=v.toFloat(); break;
    case 10: if(v.convert(QVariant::Double))
        gen->ra=v.toFloat(); break;
    case 11: if(v.convert(QVariant::Double))
        gen->xd=v.toFloat(); break;
    case 12: if(v.convert(QVariant::Double))
        gen->xd_1=v.toFloat(); break;
    case 13: if(v.convert(QVariant::Double))
        gen->xd_2=v.toFloat(); break;
    case 14: if(v.convert(QVariant::Double))
        gen->Td0_1=v.toFloat(); break;
    case 15: if(v.convert(QVariant::Double))
        gen->Td0_2=v.toFloat(); break;
    case 16: if(v.convert(QVariant::Double))
        gen->xq=v.toFloat(); break;
    case 17: if(v.convert(QVariant::Double))
        gen->xq_1=v.toFloat(); break;
    case 18: if(v.convert(QVariant::Double))
        gen->xq_2=v.toFloat(); break;
    case 19: if(v.convert(QVariant::Double))
        gen->Tq0_1=v.toFloat(); break;
    case 20: if(v.convert(QVariant::Double))
        gen->Tq0_2=v.toFloat(); break;
    case 21: if(v.convert(QVariant::Double))
        gen->M=v.toFloat(); break;
    case 22: if(v.convert(QVariant::Double))
        gen->D=v.toFloat(); break;
    }
    break;
  }

  case PWSMODELELEMENTTYPE_LOAD:{ // Loads
    Load* load = NULL;
    _pws->getLoad( _pws->getLoad(index.row())->extId, load );
    if ( load == NULL )
      return false;

    switch(index.column()){
    case 0: if(v.convert(QVariant::Int))
        load->extId = v.toInt(); break;
    case 1: if(v.convert(QVariant::Int))
        load->busExtId = v.toInt(); break;
//    case 2: if(v.convert(QVariant::Double))
//        load->Pdemand = v.toFloat(); break;
//    case 3: if(v.convert(QVariant::Double))
//        load->Qdemand = v.toFloat(); break;
//    case 4: if(v.convert(QVariant::Double))
//        load->Vss=complex<double>(v.toFloat(),load->Vss.imag()); break;
//    case 5: if(v.convert(QVariant::Double))
//        load->Vss=complex<double>(load->Vss.real(),v.toFloat()); break;
    case 6: if(v.convert(QVariant::Double))
        load->Vexpa = v.toFloat(); break;
    case 7: if(v.convert(QVariant::Double))
        load->Vexpb = v.toFloat(); break;
    case 8: if(v.convert(QVariant::Double))
        load->kpf = v.toFloat(); break;
    case 9: if(v.convert(QVariant::Double))
        load->kqf = v.toFloat(); break;
    }
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
    case 0: return "extId";
    case 1: return "name";
    case 2: return "type";
    case 3: return "Gsh";
    case 4: return "Bsh";
    case 5: return "baseKV";
    case 6: return "P";
    case 7: return "Q";
    case 8: return "V";
    case 9: return "theta";
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
    }
    break;
  }

  case PWSMODELELEMENTTYPE_GEN:{ // Generators
    switch(section){
    case  0: return "extId";
    case  1: return "status";
    case  2: return "name";
    case  3: return "busExtId";
    case  4: return "Pgen";
    case  5: return "Qgen";
    case  6: return "Vss_real";
    case  7: return "Vss_imag";
    case  8: return "model";
    case  9: return "xl";
    case 10: return "ra";
    case 11: return "xd";
    case 12: return "xd_1";
    case 13: return "xd_2";
    case 14: return "Td0_1";
    case 15: return "Td0_2";
    case 16: return "xq";
    case 17: return "xq_1";
    case 18: return "xq_2";
    case 19: return "Tq0_1";
    case 20: return "Tq0_2";
    case 21: return "M";
    case 22: return "D";
    }
    break;
  }

  case PWSMODELELEMENTTYPE_LOAD:{ // Loads
    switch(section){
    case 0: return "extId";
    case 1: return "busExtId";
    case 2: return "Pdemand";
    case 3: return "Qdemand";
    case 4: return "Vss_real";
    case 5: return "Vss_imag";
    case 6: return "Vexpa";
    case 7: return "Vexpb";
    case 8: return "kpf";
    case 9: return "kqf";
    }
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
