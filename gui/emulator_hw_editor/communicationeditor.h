/*!
\file communicationeditor.h
\brief Definition file for class CommunicationEditor

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef COMMUNICATIONEDITOR_H
#define COMMUNICATIONEDITOR_H

#include <QSplitter>
class QGroupBox;
class QGridLayout;
class QListWidget;
class QPushButton;

namespace elabtsaot{

class Emulator;
class USBFPGAInterface;
struct USBDevice;

class CommunicationEditor : public QSplitter{

  Q_OBJECT

 public:

  CommunicationEditor(Emulator* emu, QWidget* parent);

 public slots:

  void devicesUpdateSlot();
  void devicesViewSlot();
  void devicesTestSlot();
  void slicesSetSlot();
  void slicesAssignSlot();
  void slicesClearAssignSlot();
  void slicesValidateSlot();

  void updateDevicesAndSlicesSlot();
  void autoAssignSlicesToDevicesSlot();

 signals:

  void usbChanged();
  void emuChanged(bool);

 private:

  void _updateDevicesList();
  void _updateSlicesList();
  int _deviceViewDialog( USBDevice const& devId );
  int _slicesSetDialog( int currentSliceCount, int* sliceCount );
  int _slicesAssignDialog( int deviceCount, int* devId );

  Emulator* _emu;

  QGroupBox* devicesBox;
  QGridLayout* devicesLay;
  QListWidget* devicesList;

  QGroupBox* slicesBox;
  QGridLayout* slicesLay;
  QListWidget* slicesList;

};

} // end of namespace elabtsaot

#endif // COMMUNICATIONEDITOR_H
