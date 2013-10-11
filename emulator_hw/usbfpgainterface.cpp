
#include "usbfpgainterface.h"
using namespace elabtsaot;

#include "cyusb.h"

//#include <vector>
using std::vector;
//#include <string>
using std::string;
using std::wstring;
#include <iostream>
using std::cout;
using std::endl;
#include <cmath>
using std::ceil;

//#include <boost/shared_ptr.hpp>
//using boost::shared_ptr;
#include <boost/bind.hpp>
using boost::bind;

//#define VERBOSE_USB

USBFPGAInterface::USBFPGAInterface(){

  _dev = ::GetCyUSB();
//  if(!_dev){
//    cout << "CyUSB* _dev = ::GetCyUSB() failed ... " << endl;
//  }
//  cout << "CyUSB* _dev = ::GetCyUSB() successful ... " << endl;
}

USBFPGAInterface::~USBFPGAInterface(){
  _dev->destroy();
}

int USBFPGAInterface::init(){

  int ans;
  // _dev->init() resets the status of the CyUSB devices and initializes them to
  // be ready for data transfer. For each device:
  // - endpoint[3] is set as the outEndpoint - for writing to the dev
  // - endpoint[4] is set as the inEndpoint - for reading from the dev
  ans = _dev->init();
  if ( ans )
    return 1;

  // Get device count
  int deviceCount;
  ans = _dev->deviceCount( &deviceCount );
  devices.resize( deviceCount );

  // --- Fill in all other values ---
  // Allocate char arrays
  char* deviceName;
  deviceName = new char[256];
  char* friendlyName;
  friendlyName = new char[256];
  wchar_t* manufacturer;
  manufacturer = new wchar_t[256];
  wchar_t* product;
  product = new wchar_t[256];
  wchar_t* serialNumber;
  serialNumber = new wchar_t[256];

  // Iterate through all devices
  for( size_t k = 0 ; k != static_cast<size_t>(deviceCount) ; ++k ){

    bool isHighSpeed = false;
    ans = _dev->isHighSpeed(k, &isHighSpeed);
    devices[k].isHighSpeed = isHighSpeed;

    int USBVersion = 0;
    ans = _dev->USBVersion(k, &USBVersion);
    devices[k].USBVersion = USBVersion;

    int deviceClass = 0;
    ans = _dev->deviceClass(k, &deviceClass);
    devices[k].deviceClass = deviceClass;

    ans = _dev->deviceName(k, deviceName);
    devices[k].deviceName = deviceName;

    int deviceProtocol = 0;
    ans = _dev->deviceProtocol(k, &deviceProtocol);
    devices[k].deviceProtocol = deviceProtocol;

    ans = _dev->friendlyName(k, friendlyName);
    devices[k].friendlyName = friendlyName;

    bool isOpen = false;
    ans = _dev->isOpen(k, &isOpen);
    devices[k].isOpen = isOpen;

    ans = _dev->manufacturer(k, manufacturer);
    devices[k].manufacturer = manufacturer;

    int maxPower = 0;
    ans = _dev->maxPower(k, &maxPower);
    devices[k].maxPower = maxPower;

    int powerState = 0;
    ans = _dev->powerState(k, &powerState);
    devices[k].powerState = powerState;

    ans = _dev->product(k, product);
    devices[k].product = product;

    int productId = 0;
    ans = _dev->productId(k, &productId);
    devices[k].productId = productId;

    ans = _dev->serialNumber(k, serialNumber);
    devices[k].serialNumber = serialNumber;

    int languageId = 0;
    ans = _dev->languageId(k, &languageId);
    devices[k].languageId = languageId;

    int USBAddress = 0;
    ans = _dev->USBAddress(k, &USBAddress);
    devices[k].USBAddress = USBAddress;

    unsigned long USBControlerDriver = 0;
    ans = _dev->USBControlerDriver(k, &USBControlerDriver);
    devices[k].USBControlerDriver = USBControlerDriver;

    int vendorId = 0;
    ans = _dev->vendorId(k, &vendorId);
    devices[k].USBAddress = USBAddress;

    int endpointCount = 0;
    ans = _dev->endpointCount(k, &endpointCount);
    devices[k].endpoints.resize(endpointCount);

    // Iterate through all endpoint of the device k
    for(size_t m = 0; m != static_cast<size_t>(endpointCount); ++m){

      int ep_address = 0;
      ans = _dev->endpoint_address(k,m, &ep_address);
      devices[k].endpoints[m].address = ep_address;

      int ep_attributes = 0;
      ans = _dev->endpoint_attributes(k,m, &ep_attributes);
      devices[k].endpoints[m].attributes = ep_attributes;

      bool ep_isIn = false;
      ans = _dev->endpoint_isIn(k,m, &ep_isIn);
      devices[k].endpoints[m].isIn = ep_isIn;

      int ep_maxPacketSize = 0;
      ans = _dev->endpoint_maxPacketSize(k,m, &ep_maxPacketSize);
      devices[k].endpoints[m].maxPacketSize = ep_maxPacketSize;

      unsigned long ep_lastErrorCode = 0;
      ans = _dev->endpoint_lastErrorCode(k,m, &ep_lastErrorCode);
      devices[k].endpoints[m].lastErrorCode = ep_lastErrorCode;

      unsigned long ep_timeout = 0;
      ans = _dev->endpoint_timeout(k,m, &ep_timeout);
      devices[k].endpoints[m].timeout = ep_timeout;
    }
  }

  delete [] deviceName;
  delete [] friendlyName;
  delete [] manufacturer;
  delete [] product;
  delete [] serialNumber;

  return 0;
}

int USBFPGAInterface::set_endpoint_timeout( size_t deviceId,
                                            size_t endpointId,
                                            unsigned long timeoutVal ){
  return _dev->set_endpoint_timeout( deviceId, endpointId, timeoutVal );
}

int USBFPGAInterface::write( size_t devId,
                             unsigned int starting_cypress_address,
                             vector<uint32_t> data ){

  if ( devId >= devices.size() )
    // Trying to read from a non-existent device
    return 1;

  int ans = 0;
  unsigned short __rddata[4096];          // Internal read buffer
  unsigned short __wrdata[4096];          // Internal write buffer

  // --- Limit checking ---
  if (starting_cypress_address + data.size() >= USBFPGAMEMORYSIZE)
    // Writing data starting from given address would result in memory overflow
    return 13;

  // --- Check whether the data has to be broken into smaller chunks ---
  // maxPacketSize is the maximum size of the data chunk that can be transferred
  // at once by the USB device out endpoint (endpoint[3])
  // The minus two ( - 2 ) comes from the analysis of Fabrizio regarding the
  // maximization of the effective bandwidth of the communication
  int maxPacketSize = devices[devId].endpoints[3].maxPacketSize - 2;
  // Alternative way to get maxPacketSize:
//  int maxPacketSize = 0;
//  int ans = _dev->endpoint_maxPacketSize( devId, 3, &maxPacketSize);
//  maxPacketSize -= 2;

  // Ceiling the quotient of the division
  int number_of_subpackets =  static_cast<int>( ceil(
                                  static_cast<double>(data.size())
                                / static_cast<double>(maxPacketSize/2) ));
  vector<unsigned int> add( number_of_subpackets );
  vector<vector<uint32_t> > dta( number_of_subpackets );
  for ( size_t k = 0 ; k != static_cast<size_t>(number_of_subpackets) ; ++k ){
    add[k] = starting_cypress_address + k*(maxPacketSize/2);
    dta[k].insert( dta[k].begin(),
                   data.begin()+k*(maxPacketSize/2),
                   (k+1)*(maxPacketSize/2) < data.size()
                      ? data.begin()+(k+1)*(maxPacketSize/2) : data.end() );

    // fill in write buffer
    // [      32b       ]
    // [  16b  ][  16b  ]
    // --------------
    // [ MSB:0 ][ LSB:1 ] 0
    // [ MSB:2 ][ LSB:3 ] 1
    uint32_t mask16 = static_cast<uint32_t> (0xFFFF);
    unsigned short LSB = 0;
    unsigned short MSB = 0;
    for ( size_t m = 0 ; m != dta[k].size() ; ++m ){
      MSB = static_cast<unsigned short>( (dta[k][m]>>16) & mask16 );
      LSB = static_cast<unsigned short>( dta[k][m] & mask16 );
      __wrdata[2*m + 0] = MSB;
      __wrdata[2*m + 1] = LSB;
    }

    // Build write packet
    unsigned short words_to_write = 2 *dta[k].size();
    unsigned short startAddress = static_cast<unsigned short>(2*add[k]);

    int addToWrBufResult;
    int sendDataResult;
    { // Atomic operation
      addToWrBufResult = _dev->addWriteCommand( devId, words_to_write,
                                                startAddress, __wrdata );
      // Send write packet
      sendDataResult = _dev->sendData( devId, __rddata, 4096 );
    }

    // Update result accordingly
    ans |= addToWrBufResult;
    ans |= (sendDataResult << 3);
  }

  return ans;
}

int USBFPGAInterface::read( size_t devId,
                            unsigned int starting_cypress_address,
                            size_t words_to_read,
                            vector<uint32_t>& read_buffer ){

  if ( devId >= devices.size() )
    // Trying to read from a non-existent device
    return 1;

  int ans = 0;
  unsigned short __rddata[4096];          // Internal read buffer

  // --- Limit checking ---
  if (starting_cypress_address + words_to_read >= USBFPGAMEMORYSIZE)
    // Reading data starting from given address would result in memory overflow
    return 13;

  read_buffer.clear();
  read_buffer.resize( words_to_read , 0 );

  // --- Check whether the data has to be broken into smaller chunks ---
  // maxPacketSize is the maximum size of the data chunk that can be transferred
  // at once by the USB device in endpoint (endpoint[4])
  // The minus two ( - 2 ) comes from the analysis of Fabrizio regarding the
  // maximization of the effective bandwidth of the communication
  int maxPacketSize = devices[devId].endpoints[4].maxPacketSize - 2;
  // Alternative way to get maxPacketSize:
//  int maxPacketSize = 0;
//  int ans = _dev->endpoint_maxPacketSize( devId, 4, &maxPacketSize);
//  maxPacketSize -= 2;

  // Ceiling the quotient of the division
  int number_of_subpackets =  static_cast<int>( ceil(
                                  static_cast<double>( words_to_read )
                                / static_cast<double>( maxPacketSize ) ));
  vector<unsigned int> rdcnt( number_of_subpackets );
  vector<unsigned int> rdadd( number_of_subpackets );
  for ( size_t k = 0 ; k != static_cast<size_t>(number_of_subpackets) ; ++k ){
    rdcnt[k] = (k+1)*maxPacketSize < words_to_read
                 ? maxPacketSize : words_to_read-k*maxPacketSize;
    rdadd[k] = starting_cypress_address + k*maxPacketSize;

    int addToRdBufResult;
    int sendDataResult;
    // --- low 16-bit nibbles ---
    { // Atomic operation
      // Build read packet (low 16-bit nibbles)
      addToRdBufResult = _dev->addReadCommand( devId,
                              static_cast<unsigned short>(rdcnt[k]),
                              static_cast<unsigned short>(rdadd[k]) );
      // Send read packet (low 16-bit nibbles)
      sendDataResult = _dev->sendData( devId, __rddata, 4096 );
    }
    // Update result accordingly
    ans |= addToRdBufResult;
    ans |= (sendDataResult << 3);
    // Fill output read_buffer with low 16-bit nibbles
    for ( size_t m = 0 ; m != rdcnt[k] ; ++m )
      read_buffer[k*maxPacketSize+m] = static_cast<uint32_t>( __rddata[m] );

    // --- High 16-bit nibbles ---
    { // Atomic operation
      // Build read packet (high 16-bit nibbles)
      addToRdBufResult = _dev->addReadCommand( devId,
                              static_cast<unsigned short>(rdcnt[k]),
                              static_cast<unsigned short>(rdadd[k] + (1<<15)) );
      // Send read packet (high 16-bit nibbles)
      sendDataResult = _dev->sendData( devId, __rddata, 4096 );
    }
    // Update result accordingly
    ans |= addToRdBufResult;
    ans |= (sendDataResult << 3);
    // Fill output read_buffer with high 16-bit nibbles
    for (size_t m = 0 ; m != rdcnt[k] ; ++m)
      read_buffer[k*maxPacketSize+m] |= static_cast<uint32_t>(__rddata[m]<<16);

  }

  return ans;
}
