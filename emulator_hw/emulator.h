/*!
\file emulator.h

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef EMULATOR_H
#define EMULATOR_H

#include "emulatorhw.h"
#include "usbfpgainterface.h"
#include "pwsmappermodel.h"

#include <vector>
#include <map>

#define GEN_MECHSTARTTIME_THRESHOLD 100

namespace elabtsaot{

/* EMU_STATE_START
          \ /  initializeUSB()
   EMU_STATE_USBINIT
          \ /  validateSliceDeviceAssignement()
   EMU_STATE_SLCDEVASSIGNED
          \ /  validateMapping()
   EMU_STATE_MAPOK
          \ /  validateFitting()
   EMU_STATE_FITOK
          \ /  encodePowersystem()
   EMU_STATE_PWSENCODED

  Increasing ints in the enum denote increasing state of readiness for the
  Emulator */
enum EmulatorState{
  EMU_STATE_START = 100,
  EMU_STATE_USBINIT = 200,
  EMU_STATE_SLCDEVASSIGNED = 300,
  EMU_STATE_MAPOK = 400,
  EMU_STATE_FITOK = 500,
  EMU_STATE_PWSENCODED = 600
};

/* EMU_CALSTATE_YES
         \ /  endCalibrationMode() (!NOT endCalibrationMode(size_t&))
   EMU_CALSTATE_NO */
enum EmulatorCalState{
  EMU_CALSTATE_YES, // Emulator is in calibration mode
  EMU_CALSTATE_NO   // Emulator is not in calibration
};

class Emulator {

 public:

  Emulator( Powersystem const* pws );
  virtual ~Emulator();

  int init( Powersystem const* pws = NULL );
  int set_pws( Powersystem const* pws );
  void hardResetPressed();

  int preconditionEmulator();

  // EmulatorHw
  int setSliceCount(size_t val);
  int resetEmulator( bool complete );
  size_t getHwSliceCount() const;
  int node_set( size_t id_tab, size_t id_ver, size_t id_hor, Generator const& gen,
                double ratioZ, double ratioV, double ratioI, double maxIpu );
  int node_set( size_t id_tab, size_t id_ver, size_t id_hor, Load const& load,
                          double ratioI, double maxIpu );
  int embr_set(size_t id_tab, size_t id_ver, size_t id_hor, size_t pos,
               Branch const& br, double ratioZ, double distanceOfGndFromNearEnd);

  // USB
  int initializeUSB();
  int clearSliceDeviceMap();
  int assignSliceToDevice( size_t sliceId, int devId );
  int autoAssignSlicesToDevices();
  int validateSliceDeviceAssignement();
  int usbWrite( size_t devId,
                size_t starting_cypress_address,
                std::vector<uint32_t> data );
  int usbRead( size_t devId,
               size_t starting_cypress_address,
               size_t words_to_read,
               std::vector<uint32_t>& read_buffer );

  // Calibration
  int resetCalibration( size_t devId );
  int endCalibrationMode(); // ends calibration to all devices

  // Powersystem model - mapping & fitting
  void set_ratioZ(double val); //!< sets _ratioZ [Ohms/pu]; sets _ratioI=_ratioV/_ratioZ;
  void set_ratioV(double val); //!< sets _ratioZ [Volt/pu]; sets _ratioI=_ratioV/_ratioZ;
  void set_maxIpu(double val);
  int resetMapping();
  void cleaMappingHints();
  void hintComponent( int type, unsigned int extId);
  int mapComponent( int type, unsigned int extId,
                    int mapper_tab, int mapper_row, int mapper_col );
  int unmapComponent( int type, unsigned int extId);
  int autoMapping();
  int validateMapping();
  void autoRatioZ();
  void defaultRatios();
  int autoFitting(std::vector<std::string>* outputMsg = 0);
  int validateFitting();

  // Encoding powersystems and scenarios
  int encodePowersystem();
  int writeEncoding( bool verify );

  // Getters
  EmulatorHw* emuhw();
  Powersystem const* pws();
  PwsMapperModel* mmd();
  std::vector<USBDevice> getUSBDevices() const;
  size_t getUSBDevicesCount() const;
  double ratioZ() const;
  double ratioV() const;
  double ratioI() const;
  double maxIpu() const;
  std::map<size_t, int> sliceDeviceMap() const;
  int sliceDeviceMap(size_t index) const;
  int state() const;
  int state_calibration() const;

  // ---------- Variables ----------
  std::vector<std::vector<uint32_t> > encoding;

 private:

  // ---------- Function ----------
  // Calibration
  int _precalibrateSlice( size_t sliceId, bool toDefaultVoltage );
  std::vector<bool> _isAtCalibrationMode();
  int _endCalibrationMode( size_t devId );
  // Encoding functions
  int _verifyEncoding();

  // ---------- Variables----------
  EmulatorHw* _emuhw;
  Powersystem const* _pws;
  PwsMapperModel* _mmd;
  USBFPGAInterface* _usb;
  // Encoding
  std::map<std::string, std::pair<unsigned int, unsigned int> > _vRefsPrecalibrationMap;
  std::map<std::wstring, std::pair<unsigned int, unsigned int> > _vRefsPrecalibrationMapW;
  // Mapping variables
  double _ratioZ;   //!< emulator_Z / physical_Z [Ohm/pu]
  double _ratioV;   //!< emulator_V / physical_V [V/pu]
  double _ratioI;   //!< emulator_I/physical_I [A/pu]: calculated as _ratioV/Z
  double _maxIpu; //!< Maximum current [p.u.] output by a pipeline
  std::map<size_t, int> _sliceDeviceMap;
  // Internal state machine variables
  int _state;
  int _state_calibration;

};

} // end of namespace elabtsaot

#endif // EMULATOR_H
