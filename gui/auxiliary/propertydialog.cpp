
#include "propertydialog.h"
using namespace elabtsaot;
//using elabtsaot::property;

#include "auxiliary.h"

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>

//#include <vector>
using std::vector;
//#include <list>
using std::list;
//#include <iostream>

PropertyDialog::PropertyDialog( vector<property_type> const& propertyTypes,
                                list<elabtsaot::property>& propertyList ) :
    _propertyTypes(propertyTypes),
    _propertyList(propertyList) {}

int PropertyDialog::exec(){

  QDialog* dialog = new QDialog();
  dialog->setWindowTitle( "Properties" );
  QVBoxLayout* mainLay = new QVBoxLayout();
  dialog->setLayout( mainLay );

  for ( size_t k = 0 ; k != _propertyTypes.size() ; ++k ){

    // Get property type (from available property types)
    property_type prop_type = _propertyTypes[k];

    // Get respective property value (if any) from the input list
    boost::any propVal;
    list<elabtsaot::property>::const_iterator it;
    for ( it = _propertyList.begin() ; it != _propertyList.end() ; ++it )
      if ( it->key == prop_type.key )
        break;
    if ( it != _propertyList.end() )
      // If property was found, then take its value
      propVal = it->value;
    else
      // If not, take the default value of the property type
      propVal = prop_type.defaultValue;

    // Create widgets common to all prop types
    QHBoxLayout* tempLay = new QHBoxLayout();
    mainLay->addLayout(tempLay);
    QLabel* tempLabel = new QLabel( QString::fromStdString(prop_type.name), dialog);
    tempLay->addWidget(tempLabel);
    tempLabel->setToolTip(QString::fromStdString(prop_type.description));

    // Create prop-type-specific widgets
    switch(prop_type.dataType){
    case PROPERTYT_DTYPE_INT:{
      QSpinBox* tempForm = new QSpinBox(dialog);
      tempLay->addWidget(tempForm);
      int tempMin = boost::any_cast<int>(prop_type.minValue);
      int tempMax = boost::any_cast<int>(prop_type.maxValue);
      int tempVal = boost::any_cast<int>(propVal);
      tempForm->setMinimum( tempMin );
      tempForm->setMaximum( tempMax );
      tempForm->setValue( tempVal );
      _formWidgets.push_back( tempForm );
      break;}

    case PROPERTYT_DTYPE_DOUBLE:{
      QDoubleSpinBox* tempForm = new QDoubleSpinBox(dialog);
      tempLay->addWidget(tempForm);
      double tempMin = boost::any_cast<double>(prop_type.minValue);
      double tempMax = boost::any_cast<double>(prop_type.maxValue);
      double tempVal = boost::any_cast<double>(propVal);
      int tempDecimals = auxiliary::countToFirstDecimal(tempMin);
//      std::cout << "DEBUG: tempMin = " << tempMin << std::endl;
//      std::cout << "DEBUG: tempDecimals = " << tempDecimals << std::endl;
      tempForm->setDecimals( tempDecimals );
      tempForm->setSingleStep( pow(10.0,-static_cast<double>(tempDecimals)) );
      tempForm->setMinimum( tempMin );
      tempForm->setMaximum( tempMax  );
      tempForm->setValue( tempVal );
      _formWidgets.push_back( tempForm );
      break;}

    case PROPERTYT_DTYPE_BOOL:{
      QCheckBox* tempForm = new QCheckBox(dialog);
      tempLay->addWidget(tempForm);
      bool tempVal = boost::any_cast<bool>(propVal);
      tempForm->setChecked( tempVal );
      _formWidgets.push_back( tempForm );
      break;}

    default:
      continue;
    }
  }

  QHBoxLayout* buttonLay = new QHBoxLayout();
  mainLay->addLayout( buttonLay );
  QPushButton* okb = new QPushButton("OK");
  buttonLay->addWidget( okb );
  dialog->connect( okb, SIGNAL(clicked()), dialog, SLOT(accept()) );
  QPushButton* cancelb = new QPushButton("Cancel");
  buttonLay->addWidget( cancelb );
  dialog->connect( cancelb, SIGNAL(clicked()), dialog, SLOT(reject()) );

  // Execute dialog
  if ( dialog->exec() ){
    // Clear the property list
    _propertyList.clear();
    // Dialog executed properly
    for ( size_t k = 0 ; k != _propertyTypes.size() ; ++k ){
      elabtsaot::property prop;
      prop.key = _propertyTypes[k].key;
      switch ( _propertyTypes[k].dataType ){
      case PROPERTYT_DTYPE_INT:
        prop.value = dynamic_cast<QSpinBox*>(_formWidgets[k])->value();
        break;
      case PROPERTYT_DTYPE_DOUBLE:
        prop.value = dynamic_cast<QDoubleSpinBox*>(_formWidgets[k])->value();
        break;
      case PROPERTYT_DTYPE_BOOL:
        prop.value = dynamic_cast<QCheckBox*>(_formWidgets[k])->isChecked();
        break;
      default:
        continue;
      }
      _propertyList.push_back(prop);
    }
    return 0;

  } else {
    // Dialog failed
    return 1;
  }
  return 0;
}
