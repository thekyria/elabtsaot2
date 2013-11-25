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

enum EmulatorOpType{
  EMU_OPTYPE_GPF,  // Operation concerning Guillaume's Power Flow
  EMU_OPTYPE_DCPF, // Operation concerning DC Power Flow
  EMU_OPTYPE_TD    // Operation concerning TD simulation
};

class Emulator {

 public:

  Emulator(Powersystem const* pws);
  virtual ~Emulator();

  int init(EmulatorOpType opType, Powersystem const* pws = NULL);
  int set_pws(Powersystem const* pws);
  void hardResetPressed();
  int preconditionEmulator(EmulatorOpType opType);

  // EmulatorHw
  int setSliceCount(size_t val, EmulatorOpType opType);
  int resetEmulator(bool complete, EmulatorOpType opType);
  size_t getHwSliceCount() const;
  // EmulatorHw-Mapping related
  int nodeSetGPF(size_t id_tab, size_t id_ver, size_t id_hor, Bus bus);
  int nodeSetDCPF(size_t id_tab, size_t id_ver, size_t id_hor, Bus bus);
  int nodeSetTD(size_t id_tab, size_t id_ver, size_t id_hor, Generator const& gen);
  int nodeSetTD(size_t id_tab, size_t id_ver, size_t id_hor, Load const& load);
  int embrSet(size_t id_tab, size_t id_ver, size_t id_hor, size_t pos, Branch const& br,
              double distanceOfGndFromNearEnd);
  int embrSetXtimesTap(size_t id_tab, size_t id_ver, size_t id_hor, size_t pos, Branch const& br);

  // USB
  int initializeUSB();
  int clearSliceDeviceMap();
  int assignSliceToDevice( size_t sliceId, int devId );
  int autoAssignSlicesToDevices();
  int validateSliceDeviceAssignement();
  int usbWrite(size_t devId, size_t starting_cypress_address, std::vector<uint32_t> data);
  int usbRead(size_t devId, size_t starting_cypress_address, size_t words_to_read,
              std::vector<uint32_t>& read_buffer);

  // Calibration
  int resetCalibration(size_t devId);
  int endCalibrationMode(); // ends calibration to all devices

  // Powersystem model - mapping
  void set_ratioZ(double val); //!< sets _ratioZ [Ohms/pu]; sets _ratioI=_ratioV/_ratioZ;
  void set_ratioV(double val); //!< sets _ratioZ [Volt/pu]; sets _ratioI=_ratioV/_ratioZ;
  void set_maxIpu(double val);
  void autoRatioZ(EmulatorOpType opType);
  double getMaxR() const;
  void defaultRatios(EmulatorOpType opType);
  int resetMapping();
  void cleaMappingHints();
  void hintComponent(int type, unsigned int extId);
  int mapComponent(int type, unsigned int extId, int map_tab,int map_row,int map_col);
  int unmapComponent(int type, unsigned int extId);
  int autoMapping();
  int validateMapping();

  // Powersystem model - fitting TD
  int autoFitting(EmulatorOpType opType, std::vector<std::string>* outputMsg = 0);
  int validateFitting(EmulatorOpType opType);

  // Encoding powersystems and scenarios
  int encodePowersystem(EmulatorOpType opType);
  int writeEncoding(bool verify, bool force=false);

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

  // ---------- Functions ----------
  // Calibration
  int _precalibrateSlice(size_t sliceId, bool toDefaultVoltage);
  std::vector<bool> _isAtCalibrationMode();
  int _endCalibrationMode( size_t devId );
  // Fitting
  int _fitBranches(EmulatorOpType opType, std::vector<std::string>* outputMsg = 0);
  int _fitBusesGPF(std::vector<std::string>* outputMsg = 0);
  int _fitBusesDCPF(std::vector<std::string>* outputMsg = 0);
  int _fitGeneratorsTD(std::vector<std::string>* outputMsg = 0);
  int _fitLoadsTD(std::vector<std::string>* outputMsg =0);
  // Encoding functions
  int _verifyEncoding();

  // ---------- Variables ----------
  EmulatorHw* _emuhw;
  Powersystem const* _pws;
  PwsMapperModel* _mmd;
  USBFPGAInterface* _usb;
  // Encoding
  std::map<std::string, std::pair<unsigned int, unsigned int> > _vRefsPrecalibrationMap;
  std::map<std::wstring, std::pair<unsigned int, unsigned int> > _vRefsPrecalibrationMapW;
  /*   Mapping variables
   * emulator_Z: is the real resistance of an emulator branch; it is
   *             determined by the reconfigurable potentiometers and is measured
   *             in Ohms
   * emulator_V: is the real voltage of an emulator node; it is measured in
   *             Volts by the ADC
   * emulator_I: is the real current injected into an emulator node; it is what
   *             injected by the dac through a voltage-to-current conversion
   *             resistance, plus all current flows into/outof the node through
   *             connecting emulator branches; it is measured in Amperes
   * flownet_Z:  is the real resistance of the flownet; it is measured in pu; eg.
   *             in the case of the simplified power systems (R neglected)
   *             it is the X of the branch and is measured in [pu] (in the
   *             impedance base)
   * flownet_V:  is the real potential of the flownet; it is measured in pu; eg.
   *             in the case of the simplified power systems (R neglected)
   *             it is the V(_real or _imag) of the node and is measured in [pu]
   *             (in the voltage base); eg. in the case of the DC simplified
   *             power systems it is the theta (angle) of the node and is
   *             measured in [rad]
   * flownet_I:  is the real flow of the flownet; it is measured in pu; eg. in
   *             the case of the simplified power systems (R neglected) it is the
   *             (real or imag) current injected to the node and is measured in
   *             [pu] (in the curent base); eg. in the case of the DC simplified
   *             power system it is the active power injected into the node and
   *             is measured in [pu] (in the power base)
   */
  double _ratioZ;   //!< emulator_Z/flownet_Z [Ohm / pu of flownet_Z]
  double _ratioV;   //!< emulator_V/flownet_V [V / pu of flownet_V]
  double _ratioI;   //!< emulator_I/flownet_I [A / pu of flownet_I]: calculated as _ratioV/_ratioZ
  double _maxIpu;   //!< Maximum current [pu] output by a pipeline
  std::map<size_t, int> _sliceDeviceMap;
  // Internal state machine variables
  int _state;
  int _state_calibration;

};

} // end of namespace elabtsaot

#endif // EMULATOR_H
