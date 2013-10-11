
#ifndef GUIAUXILIARY_H
#define GUIAUXILIARY_H

#include <QString>

namespace elabtsaot {
namespace guiauxiliary {

QString askFileName(QString ext , bool read);
int askDouble(QString const& title, double& v);

} // end of namespace guiauxiliary
} // end of namespace elabtsaot

#endif // GUIAUXILIARY_H
