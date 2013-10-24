
#ifndef GUIAUXILIARY_H
#define GUIAUXILIARY_H

#include <string>
#include <QString>

namespace elabtsaot {
namespace guiauxiliary {

QString askFileName(QString ext , bool read);
int askInt(QString const& title, int& val, int const& min, int const& max);
int askDouble(QString const& title, double& v);
int askString(QString const& title, std::string& v);

} // end of namespace guiauxiliary
} // end of namespace elabtsaot

#endif // GUIAUXILIARY_H
