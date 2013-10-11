/*!
\file rawwritetodevicedialog.h

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef RAWWRITETODEVICEDIALOG_H
#define RAWWRITETODEVICEDIALOG_H

#include <QObject>
class QFormLayout;
class QSpinBox;
class QLayout;
class QDoubleSpinBox;

#include <vector>
#include <stdint.h>

namespace elabtsaot{

class RawWriteToDeviceDialog : public QObject {

  Q_OBJECT

 public:

  RawWriteToDeviceDialog( size_t deviceCount,
                          size_t& devId,
                          unsigned int& startAddress,
                          std::vector<uint32_t>& wrdata,
                          QWidget* parent = 0 );
  int exec();

 private slots:

  void wordCountChanged( int val );

 private:

  size_t const& _deviceCount;
  size_t& _devId;
  unsigned int& _startAddress;
  std::vector<uint32_t>& _wrdata;

  QSpinBox* startAddressForm;
  QFormLayout* wrdataLay;
  std::vector<QDoubleSpinBox*> wrdataForm;

};

} // end of namespace elabtsaot

#endif // RAWWRITETODEVICEDIALOG_H
