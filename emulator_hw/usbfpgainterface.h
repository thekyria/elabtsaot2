/*!
\file usbfpgainterface.h
\brief Provides an application programming interface for communicating with the
emulator FPGA

This file requires CyUSB.lib and CyUSB.dll and respective
wrapping header file CyUSB.h

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef USBFPGAINTERFACE_H
#define USBFPGAINTERFACE_H

#ifndef USBFPGAMEMORYSIZE
//! Size [words] of the the shared RAM used for communication with the FPGA
#define USBFPGAMEMORYSIZE 32768
#endif // USBFPGAMEMORYSIZE

#include <vector>
#include <string>
#include <stdint.h>

class CyUSB;

namespace elabtsaot{

struct USBDevice;
struct USBEndpoint;
struct USBWrCommand;
struct USBRdCommand;

class USBFPGAInterface{

 public:

  USBFPGAInterface();
  virtual ~USBFPGAInterface();

  int init();
  int set_endpoint_timeout( size_t deviceId,
                            size_t endpointId,
                            unsigned long timeoutVal );
  int write( size_t devId,
             unsigned int starting_cypress_address,
             std::vector<uint32_t> data );
  int read( size_t devId,
            unsigned int starting_cypress_address,
            size_t words_to_read,
            std::vector<uint32_t>& read_buffer );

  std::vector<USBDevice> devices;

 private:

  //! Status of the USB communication
  CyUSB* _dev;                            // handle to the USB comm device
};

struct USBDevice{
  bool isHighSpeed;
  int USBVersion;
  int deviceClass;
  std::string deviceName;
  int deviceProtocol;
  std::string friendlyName;
  bool isOpen;
  std::wstring manufacturer;
  int maxPower;
  int powerState;
  std::wstring product;
  int productId;
  std::wstring serialNumber;
  int languageId;
  int USBAddress;
  unsigned long USBControlerDriver;
  int vendorId;
  std::vector<USBEndpoint> endpoints;
};

struct USBEndpoint{
  int address;
  int attributes;
  bool isIn;
  int maxPacketSize;
  unsigned long lastErrorCode;
  unsigned long timeout;
};

} // end of namespace elabtsaot

#endif // USBFPGAINTERFACE_H
