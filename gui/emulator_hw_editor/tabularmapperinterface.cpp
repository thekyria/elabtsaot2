
#include "tabularmapperinterface.h"
using namespace elabtsaot;

#include "pwsmappermodel.h"

TabularMapperInterface::TabularMapperInterface(int type,
                                               PwsMapperModel const* mmd,
                                               QObject *parent) :
    QAbstractTableModel(parent),
    _type(type), _mmd(mmd) {}

int TabularMapperInterface::rowCount(QModelIndex const& index) const{
  switch( _type ){
  case PWSMODELELEMENTTYPE_BUS: // Buses
    return _mmd->busElements_size();
  case PWSMODELELEMENTTYPE_BR: // Branches
    return _mmd->branchElements_size();
  case PWSMODELELEMENTTYPE_GEN: // Generators
    return _mmd->genElements_size();
  case PWSMODELELEMENTTYPE_LOAD: // Loads
    return _mmd->loadElements_size();
  }

  return 0;
}

int TabularMapperInterface::columnCount(QModelIndex const& index) const{
  switch( _type ){
  case PWSMODELELEMENTTYPE_BUS: // Buses
    return 6;
  case PWSMODELELEMENTTYPE_BR: // Branches
    return 6;
  case PWSMODELELEMENTTYPE_GEN: // Generators
    return 6;
  case PWSMODELELEMENTTYPE_LOAD: // Loads
    return 6;
  }

  return 0;
}

QVariant TabularMapperInterface::data(QModelIndex const& index,
                                         int role) const{

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
    case  0: return _mmd->elementByIndex(PWSMODELELEMENTTYPE_BUS, index.row())->type;
    case  1: return _mmd->elementByIndex(PWSMODELELEMENTTYPE_BUS, index.row())->extId;
    case  2: return _mmd->elementByIndex(PWSMODELELEMENTTYPE_BUS, index.row())->mapped;
    case  3: return _mmd->elementByIndex(PWSMODELELEMENTTYPE_BUS, index.row())->tab;
    case  4: return _mmd->elementByIndex(PWSMODELELEMENTTYPE_BUS, index.row())->row;
    case  5: return _mmd->elementByIndex(PWSMODELELEMENTTYPE_BUS, index.row())->col;
    }
    break;
  }

  case PWSMODELELEMENTTYPE_BR:{ // Branches
    switch(index.column()){
    case  0: return _mmd->elementByIndex(PWSMODELELEMENTTYPE_BR, index.row())->type;
    case  1: return _mmd->elementByIndex(PWSMODELELEMENTTYPE_BR, index.row())->extId;
    case  2: return _mmd->elementByIndex(PWSMODELELEMENTTYPE_BR, index.row())->mapped;
    case  3: return _mmd->elementByIndex(PWSMODELELEMENTTYPE_BR, index.row())->tab;
    case  4: return _mmd->elementByIndex(PWSMODELELEMENTTYPE_BR, index.row())->row;
    case  5: return _mmd->elementByIndex(PWSMODELELEMENTTYPE_BR, index.row())->col;
    }
    break;
  }

  case PWSMODELELEMENTTYPE_GEN:{ // Generators
    switch(index.column()){
    case  0: return _mmd->elementByIndex(PWSMODELELEMENTTYPE_GEN, index.row())->type;
    case  1: return _mmd->elementByIndex(PWSMODELELEMENTTYPE_GEN, index.row())->extId;
    case  2: return _mmd->elementByIndex(PWSMODELELEMENTTYPE_GEN, index.row())->mapped;
    case  3: return _mmd->elementByIndex(PWSMODELELEMENTTYPE_GEN, index.row())->tab;
    case  4: return _mmd->elementByIndex(PWSMODELELEMENTTYPE_GEN, index.row())->row;
    case  5: return _mmd->elementByIndex(PWSMODELELEMENTTYPE_GEN, index.row())->col;
    }
    break;
  }

  case PWSMODELELEMENTTYPE_LOAD:{ // Loads
    switch(index.column()){
    case  0: return _mmd->elementByIndex(PWSMODELELEMENTTYPE_LOAD, index.row())->type;
    case  1: return _mmd->elementByIndex(PWSMODELELEMENTTYPE_LOAD, index.row())->extId;
    case  2: return _mmd->elementByIndex(PWSMODELELEMENTTYPE_LOAD, index.row())->mapped;
    case  3: return _mmd->elementByIndex(PWSMODELELEMENTTYPE_LOAD, index.row())->tab;
    case  4: return _mmd->elementByIndex(PWSMODELELEMENTTYPE_LOAD, index.row())->row;
    case  5: return _mmd->elementByIndex(PWSMODELELEMENTTYPE_LOAD, index.row())->col;
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

bool TabularMapperInterface::setData( QModelIndex const& index,
                                      QVariant const& value,
                                      int role){

  // Assert that the index provided as an argument is valid and that the
  // interface has an edit role
  if( !index.isValid() || role!=Qt::EditRole )
    return false;

  QVariant v = value;
  switch( _type ){

  case PWSMODELELEMENTTYPE_BUS:{ // Buses
    switch(index.column()){
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
      break;
    }
    break;
  }

  case PWSMODELELEMENTTYPE_BR:{ // Branches
    switch(index.column()){
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
      break;
    }
    break;
  }

  case PWSMODELELEMENTTYPE_GEN:{ // Generators
    switch(index.column()){
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
      break;
    }
    break;
  }

  case PWSMODELELEMENTTYPE_LOAD:{ // Loads
    switch(index.column()){
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
      break;
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

QVariant TabularMapperInterface::headerData(int section,
                                            Qt::Orientation orientation,
                                            int role) const{

  // Assert that the role of the interface is display and that the orientation
  // is horizontal
  if( role!=Qt::DisplayRole || orientation!=Qt::Horizontal )
    return QVariant();


  switch( _type ){

  case PWSMODELELEMENTTYPE_BUS:{ // Buses
    switch(section){
    case  0: return "type";
    case  1: return "extId";
    case  2: return "mapped";
    case  3: return "tab";
    case  4: return "row";
    case  5: return "col";
    }
    break;
  }

  case PWSMODELELEMENTTYPE_BR:{ // Branches
    switch(section){
    case  0: return "type";
    case  1: return "extId";
    case  2: return "mapped";
    case  3: return "tab";
    case  4: return "row";
    case  5: return "col";
    }
    break;
  }

  case PWSMODELELEMENTTYPE_GEN:{ // Generators
    switch(section){
    case  0: return "type";
    case  1: return "extId";
    case  2: return "mapped";
    case  3: return "tab";
    case  4: return "row";
    case  5: return "col";
    }
    break;
  }

  case PWSMODELELEMENTTYPE_LOAD:{ // Loads
    switch(section){
    case  0: return "type";
    case  1: return "extId";
    case  2: return "mapped";
    case  3: return "tab";
    case  4: return "row";
    case  5: return "col";
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

Qt::ItemFlags TabularMapperInterface::flags(QModelIndex const& index) const {
  return QAbstractTableModel::flags(index) | Qt::ItemIsSelectable;
}

void TabularMapperInterface::update(){
  emit this->layoutAboutToBeChanged();
  emit this->layoutChanged();
}
