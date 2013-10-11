
#ifndef PROPERTYDIALOG_H
#define PROPERTYDIALOG_H

#include "property.h"
#include "property_t.h"

#include <QObject>

#include <list>
#include <vector>

namespace elabtsaot{

class PropertyDialog : public QObject {

  Q_OBJECT

 public:

  PropertyDialog( std::vector<elabtsaot::property_type> const& propertyTypes,
                  std::list<elabtsaot::property>& propertyList );
  int exec();

 private:

  std::vector<elabtsaot::property_type> const& _propertyTypes;
  std::list<elabtsaot::property>& _propertyList;
  std::vector<QWidget*> _formWidgets;

};

} // end of namespace elabtsaot

#endif // PROPERTYDIALOG_H
