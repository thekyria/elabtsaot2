
#include "emulator.h"
using namespace elabtsaot;

#include "powersystem.h"
#include "encoder.h"
#include "auxiliary.h"
#include "precisiontimer.h"

//#include <string>
using std::string;
using std::wstring;
//#include <vector>
using std::vector;
//#include <complex>
using std::complex;
//#include <map>
using std::map;
using std::pair;
#include <iostream>
using std::cout;
using std::endl;
#include <limits>
using std::numeric_limits;
#include <utility>
using std::make_pair;
#include <cmath>
#include <algorithm>
using std::min;

#define REAL_VOLTAGE_REF_MEASURED_VAL 2.5
#define IMAG_VOLTAGE_REF_MEASURED_VAL 2.5
#define DEVICE_CALIBRATION_SIGNATURE 36864
//#define DEVICE_PIPELINES_SIGNARTURE 39320
#define DEFRATIOZ 20000
#define DEFRATIOV 1
#define DEFMAXIPU 16

Emulator::Emulator( Powersystem const* pws ) :
    _emuhw(new EmulatorHw()),
    _pws(pws),
    _mmd(new PwsMapperModel(_pws,_emuhw)),
    _usb(new USBFPGAInterface()),
    _ratioZ(DEFRATIOZ),
    _ratioV(DEFRATIOV),
    _ratioI(_ratioV/_ratioZ),
    _maxIpu(DEFMAXIPU),
    _state(EMU_STATE_START),
    _state_calibration(EMU_CALSTATE_YES) {

  // Initialize voltage references precalibration measurements map
  // This map corresponds to (real, imag) tap settings required for emulator
  // board with name 'key', in order to get REAL_VOLTAGE_REF_MEASURED_VAL and
  // IMAG_VOLTAGE_REF_MEASURED_VAL at its output

  // std::string
  _vRefsPrecalibrationMap[ "Emulator 1" ] = make_pair( 32848, 32784 ); // old
  _vRefsPrecalibrationMap[ "Emulator 2" ] = make_pair( 32786, 32749 ); // old
  _vRefsPrecalibrationMap[ "Emulator 3" ] = make_pair( 32816, 32672 ); // old
  _vRefsPrecalibrationMap[ "Emulator 4" ] = make_pair( 32896, 32672 ); // old
  _vRefsPrecalibrationMap[ "Emulator 5" ] = make_pair( 32812, 32777 ); // new
  _vRefsPrecalibrationMap[ "Emulator 6" ] = make_pair( 32820, 32758 ); // new
  _vRefsPrecalibrationMap[ "Emulator 7" ] = make_pair( 32781, 32706 ); // new
  _vRefsPrecalibrationMap[ "Emulator 8" ] = make_pair( 32828, 32865 ); // new

  // std::wstring
  _vRefsPrecalibrationMapW[ L"Emulator 1" ] = make_pair( 32848, 32784 ); // old
  _vRefsPrecalibrationMapW[ L"Emulator 2" ] = make_pair( 32786, 32749 ); // old
  _vRefsPrecalibrationMapW[ L"Emulator 3" ] = make_pair( 32816, 32672 ); // old
  _vRefsPrecalibrationMapW[ L"Emulator 4" ] = make_pair( 32896, 32672 ); // old
  _vRefsPrecalibrationMapW[ L"Emulator 5" ] = make_pair( 32812, 32777 ); // new
  _vRefsPrecalibrationMapW[ L"Emulator 6" ] = make_pair( 32820, 32758 ); // new
  _vRefsPrecalibrationMapW[ L"Emulator 7" ] = make_pair( 32781, 32706 ); // new
  _vRefsPrecalibrationMapW[ L"Emulator 8" ] = make_pair( 32828, 32865 ); // new
}

Emulator::~Emulator(){
  delete _emuhw;
  delete _usb;
  delete _mmd;
}

int Emulator::init( Powersystem const* pws ){

  // Regardless of the previous state init() resets the state of the Emulator
  // to the initial state
  _state = EMU_STATE_START;

  int ans = 0;
  if ( pws != NULL && _pws != pws ){
    // A new powersystem has been supplied as argument to the Emulator
    _pws = pws;
    delete _mmd;
    _mmd = new PwsMapperModel( _pws, _emuhw );
  } else {
    // The powersystem pointer remains the same (or no arg has been provided)
    ans |= _mmd->init(); // also initializes _mmd
  }
  // Anyway reinitialize the usb connection
  ans |= _usb->init();

  // init private members
  _ratioZ = static_cast<double>(DEFRATIOZ);
  _ratioV = static_cast<double>(DEFRATIOV);
  _ratioI = static_cast<double>(_ratioV/_ratioZ);
  _maxIpu = static_cast<double>(DEFMAXIPU);

  return ans;
}

int Emulator::set_pws( Powersystem const* pws ){

  // Calling set_pws() downsets the state of the Emulator to
  // EMU_STATE_SLCDEVASSIGNED
  if ( _state > EMU_STATE_SLCDEVASSIGNED )
    _state = EMU_STATE_SLCDEVASSIGNED;

  int ans = 0;
  if ( pws != NULL && _pws != pws ){
    // A new powersystem has been supplied as argument to the Emulator
    _pws = pws;
    delete _mmd;
    _mmd = new PwsMapperModel( _pws, _emuhw );
  } else {
    // The powersystem pointer remains the same (or no arg has been provided)
    ans |= _mmd->init();
  }

  return ans;
}

void Emulator::hardResetPressed(){

  // Downset the state of the Emulator to EMU_STATE_SLCDEVASSIGNED
  if ( _state > EMU_STATE_SLCDEVASSIGNED )
    _state = EMU_STATE_SLCDEVASSIGNED;

  // Set the calibration state of the Emulator to EMU_CALSTATE_YES
  _state_calibration = EMU_CALSTATE_YES;

  return;
}

int Emulator::preconditionEmulator(){

  PrecisionTimer timer; // counts in seconds
  timer.Start();

  int ans;
  // --------------- Map & fit & encode pws ---------------
  switch ( _state ){
  case EMU_STATE_START:
    ans = initializeUSB(); // would change _state to EMU_STATE_USBINIT
    if ( ans ) return 10;
    cout << "Emulator::_preconditionEmulator(): ";
    cout << "initialize usb " << timer.Stop() << " s" << endl;
    timer.Start();
  case EMU_STATE_USBINIT:
    ans = autoAssignSlicesToDevices(); // would change _state to EMU_STATE_SLCDEVASSIGNED
    if ( ans ) return 11;
    cout << "Emulator::_preconditionEmulator(): ";
    cout << "auto assign slices to devices " << timer.Stop() << " s" << endl;
    timer.Start();
  case EMU_STATE_SLCDEVASSIGNED:
    ans = autoMapping();
    if ( ans ) return 12;
    cout << "Emulator::_preconditionEmulator(): ";
    cout << "auto mapping " << timer.Stop() << " s" << endl;
    timer.Start();
    ans = validateMapping(); // would change _state to EMU_STATE_MAPOK
    if ( ans ) return 13;
    cout << "Emulator::_preconditionEmulator(): ";
    cout << "validate mapping " << timer.Stop() << " s" << endl;
    timer.Start();
  case EMU_STATE_MAPOK:
    ans = autoFitting();
    if ( ans ) return 14;
    cout << "Emulator::_preconditionEmulator(): ";
    cout << "auto fitting " << timer.Stop() << " s" << endl;
    timer.Start();
    ans = validateFitting(); // would change _state to EMU_STATE_FITOK
    if ( ans ) return 15;
    cout << "Emulator::_preconditionEmulator(): ";
    cout << "validate fitting " << timer.Stop() << " s" << endl;
    timer.Start();
  case EMU_STATE_FITOK:
    ans = encodePowersystem();
    if ( ans ) return 16; // would change _state to EMU_STATE_PWSENCODED
    cout << "Emulator::_preconditionEmulator(): ";
    cout << "encode powersystem " << timer.Stop() << " s" << endl;
    timer.Start();
  case EMU_STATE_PWSENCODED:
    // Emulator is ready to simulate
    break;
  default:
    return 17; // weird _state
  }

  // --------------- End calibration mode ---------------
  if ( _state_calibration == EMU_CALSTATE_YES ){
    cout << "Emulator::_preconditionEmulator(): ";
    cout << "end calibration mode " << timer.Stop() << " s" << endl;
    endCalibrationMode();
  }

  return 0;
}

int Emulator::setSliceCount(size_t val){

//  // Regardless of the previous state setSliceCount() resets the state of the
//  // Emulator to the initial state
//  _state = EMU_STATE_START;
  // Downset the state of the Emulator to EMU_STATE_USBINIT
  if ( _state > EMU_STATE_USBINIT )
    _state = EMU_STATE_USBINIT;

  // init private members
  _ratioZ = static_cast<double>(DEFRATIOZ);
  _ratioV = static_cast<double>(DEFRATIOV);
  _ratioI = static_cast<double>( _ratioV / _ratioZ );
  _maxIpu = static_cast<double>(DEFMAXIPU);

  // Update emulator
  int ans = _emuhw->init( val );
  if ( ans ) return 1;

  // Initialize mmd
  ans = _mmd->init();
  if ( ans ) return 3;

  // Update _sliceDeviceMap
  _sliceDeviceMap.clear();
  for ( size_t k = 0; k != val; ++k )
    _sliceDeviceMap.insert( pair<size_t,int>(k,-1) );

  return 0;
}

int Emulator::resetEmulator( bool complete ){

  // Reset private members
  _ratioZ = static_cast<double>(DEFRATIOZ);
  _ratioV = static_cast<double>(DEFRATIOV);
  _ratioI = static_cast<double>( _ratioV / _ratioZ );
  _maxIpu = static_cast<double>(DEFMAXIPU);

  // Reset emulator hardware
  int ans = _emuhw->reset( complete );
  if ( ans ) return ans;

  // If a complete reset is asked for do not precalibrate slices
  if ( !complete ){
    // Precalibrate the slices according to premade measurements
    for(size_t k = 0; k != _emuhw->sliceSet.size(); ++k){
      _precalibrateSlice(k , true );
    }
  }

  return 0;
}

size_t Emulator::getHwSliceCount() const{ return _emuhw->sliceSet.size(); }

int Emulator::node_set( size_t id_tab, size_t id_ver, size_t id_hor, Generator const& gen,
                        double ratioZ, double ratioV, double ratioI, double max_I_pu ){
  int ans = 0;
  Slice* slc = &_emuhw->sliceSet[id_tab];

  // Disconnect everything from the node
  ans |= slc->dig.remove(id_ver,id_hor);
  ans |= slc->ana.nodeDisconnect(id_ver,id_hor);

  double Vmaxreal = slc->ana.real_voltage_ref_val_max()- slc->ana.real_voltage_ref_val();
  double Vmaximag = slc->ana.imag_voltage_ref_val_max()- slc->ana.imag_voltage_ref_val();
  if ( gen.M < GEN_MECHSTARTTIME_THRESHOLD ){
    // Normal (non-slack) generator inserted into generator pipeline
    // Configure dig.pipe_gen
    if ( slc->dig.pipe_gen.insert_element(id_ver, id_hor, gen, true) )
      return 25;

    double shuntR = gen.xd_1 * ratioZ;
    double Vmax = std::min(Vmaxreal,Vmaximag);
    double seriesR = Vmax/(max_I_pu*ratioI);
    if ( slc->ana.nodeCurrentSource(id_ver,id_hor, seriesR, shuntR) )
      return 27;
  } else { // ( gen.M() >= GEN_MECHSTARTTIME_THRESHOLD )
    // Slack generator (for dynamic analysis) inserted into const I load
    // pipeline. Convert gen to const I load as needed so as to have the voltage
    // DAC (output of the const I pipeline) to output constant voltage to the
    // node (by closing the node::_real/imag_sw_voltage)
    // We need emulator hw voltage of = 1+j0 pu
    // On the emulator topology the real current is related to the imag voltage
    // and the imag current to the real voltage
    // Current corresponding to real( gen.Uss() ) has to be output to imag I and
    // current corresponding to imag( gen.Uss() ) has to be output to real I
    // V [pu]*ratioV > V [V] > DACcode = V/maxV > I [pu] = DACcode * maxI[pu]
    // e.g. for
    // ratioV = 2 Volt/pu, maxV = 2.5 (DAC range [-2.5,+2.5] V), maxI = 16 [pu]
    // for desired Voltage = 1 + j0 [u] -> 2 + j0 [V]
    // the resulting DAC code would be 2/maxV = 2/2.5 = 0.8
    // and the resulting currents = maxI(0*0.8 + j 1*0.8) = 0 + j12.8 pu
    double imagI = max_I_pu *  gen.Vss.real()*ratioV / Vmaxreal;
    double realI = max_I_pu *  gen.Vss.real()*ratioV / Vmaximag;
    // So into the const I load pipeline we have to insert a fake "load" that
    // results in the above currents in [pu]
    complex<double> I ( realI, imagI );
    complex<double> S = gen.Vss * conj(I);
    Load temp_iload = Load();
    temp_iload.Pdemand = real(S);
    temp_iload.Qdemand = imag(S);
    temp_iload.setType(LOADTYPE_CONSTI);
    // Configure dig.pipe_iload
    if ( slc->dig.pipe_iload.insert_element(id_ver,id_hor,temp_iload,true) )
      return 26;
    // Notice: in ConstILoadPipeline::insert_element, temp_iload.pdemand/qdemand
    // are translated (again) to I = realI + j*imagI that was determined above.
    // That is the function is "fooled" with the "fake" load temp_iload :)

    // Slack is represented as a voltage source directly connected to the node
    if ( slc->ana.nodeVoltageSource(id_ver, id_hor, -1) )
      return 27;
  }

  return 0;
}

int Emulator::node_set( size_t id_tab, size_t id_ver, size_t id_hor, Load const& load,
                        double ratioI, double max_I_pu ){
  int ans = 0;
  Slice* slc = &_emuhw->sliceSet[id_tab];

  // Disconnect everything from the node
  ans |= slc->dig.remove(id_ver,id_hor);
  ans |= slc->ana.nodeDisconnect(id_ver,id_hor);

  if ( (load.Vexpa==2) && (load.Vexpb==2) ){
    // ------- Const Z load -------
    // Configure dig.pipe_zload
    if ( slc->dig.pipe_zload.insert_element(id_ver, id_hor, load, true) )
      return 24;

  } else if ( (load.Vexpa==1) && (load.Vexpb==0) ){
    // ------- Const P load -------
    // Configure dig.pipe_pload
    if ( slc->dig.pipe_pload.insert_element(id_ver, id_hor, load, true) )
      return 25;

  } else { // ( (load.Vexpa==1) && (load.Vexpb==1) )
    // ------- Const I load -------
    // ------- OR Unknown load type (assume const I load) -------
    // Configure dig.pipe_iload
    if ( slc->dig.pipe_iload.insert_element(id_ver, id_hor, load, true) )
      return 26;
  }

  // Configure atom as load
  double Vmaxreal = slc->ana.real_voltage_ref_val_max()- slc->ana.real_voltage_ref_val();
  double Vmaximag = slc->ana.imag_voltage_ref_val_max()- slc->ana.imag_voltage_ref_val();
  double Vmax = std::min(Vmaxreal,Vmaximag);
  double seriesR = Vmax/(max_I_pu*ratioI);
  ans |= slc->ana.nodeCurrentSource(id_ver,id_hor, seriesR, -1);

  return ans;
}

/*! Maps a real world branch to an emulator branch
  from bus                    to bus
  |<------ x ------>|<- 1-x ->|
  |---------------------------|
  |                 |         |
                   ---
                    -  grounding  */
int Emulator::embr_set(size_t id_tab, size_t id_ver, size_t id_hor, size_t pos,
                       Branch const& br, double ratioZ, double distanceOfGndFromNearEnd){
  Slice* slc = &_emuhw->sliceSet[id_tab];
  double r_near = br.X * ratioZ * distanceOfGndFromNearEnd;
  double r_far  = br.X * ratioZ * (1 - distanceOfGndFromNearEnd);
  if (br.status)
    return slc->ana.embrConnect(id_ver,id_hor,pos,r_near,r_far);
  else
    return slc->ana.embrDisconnect(id_ver, id_hor, pos);
}

int Emulator::initializeUSB(){

  // Regardless of the previous state, calling initializeUSB() resets the state
  // of the Emulator to the initial state
  _state = EMU_STATE_START;
  // Calling initializeUSB() also resets the calibration state of Emulator
  _state_calibration = EMU_CALSTATE_YES;

  int ans = _usb->init();
  if ( ans ) return ans;

  // If the function executed properly, the state of Emulator changes
  // implementing the EMU_STATE_START -> EMU_STATE_USBINIT transition
  _state = EMU_STATE_USBINIT;
  return 0;
}

int Emulator::clearSliceDeviceMap(){

  // Calling clearSliceDeviceMap() downsets the state of the Emulator to
  // EMU_STATE_USBINIT
  if ( _state > EMU_STATE_USBINIT )
    _state = EMU_STATE_USBINIT;

  _sliceDeviceMap.clear();
  for ( size_t k = 0; k != _emuhw->sliceSet.size(); ++k )
    _sliceDeviceMap.insert( pair<size_t,int>(k,-1) );

  return 0;
}

int Emulator::assignSliceToDevice( size_t sliceId, int devId ){

  // Calling assignSliceToDevice() downsets the state of the Emulator to
  // EMU_STATE_USBINIT
  if ( _state > EMU_STATE_USBINIT )
    _state = EMU_STATE_USBINIT;

  // De-assign the selected device from the slice it was assigned to
  size_t oldSliceId;
  bool deviceAssigned = false; // whether the device was assigned to another slice
  for ( map<size_t,int>::iterator it = _sliceDeviceMap.begin() ;
        it != _sliceDeviceMap.end() ; ++it ){
    if ( (*it).first == sliceId )
      // Skip if the device was assigned to the slice currently in question
      continue;

    if ( (*it).second == devId ){
      deviceAssigned = true;
      oldSliceId = (*it).first;
      break;
    }
  }
  if ( deviceAssigned ){
    _sliceDeviceMap.erase( oldSliceId );
    _sliceDeviceMap.insert( std::pair<size_t,int>( oldSliceId, -1 ) );
  }

  // Assign slice to the selected device
  _sliceDeviceMap.erase( sliceId );
  _sliceDeviceMap.insert( std::pair<size_t,int>( sliceId, devId ) );

  return 0;
}

// Automation function; calls initializeUSB(), clearSliceDeviceMap(),
// setSliceCount(), assignSliceToDevice(), validateSliceDeviceAssignement()
int Emulator::autoAssignSlicesToDevices(){

  // The default number of slices and default names for them
  size_t const MAX_SLICE_COUNT = 4;
  string const SLICE_NAMES_OLD[MAX_SLICE_COUNT] = {
                                                    "Emulator 1",
                                                    "Emulator 2",
                                                    "Emulator 3",
                                                    "Emulator 4"
                                                  };
  string const SLICE_NAMES_NEW[MAX_SLICE_COUNT] = {
                                                    "Emulator 5",
                                                    "Emulator 6",
                                                    "Emulator 7",
                                                    "Emulator 8"
                                                  };
  wstring const SLICE_PRODUCT_OLD[MAX_SLICE_COUNT] = {
                                                    L"Emulator 1",
                                                    L"Emulator 2",
                                                    L"Emulator 3",
                                                    L"Emulator 4"
                                                     };
  wstring const SLICE_PRODUCT_NEW[MAX_SLICE_COUNT] = {
                                                    L"Emulator 5",
                                                    L"Emulator 6",
                                                    L"Emulator 7",
                                                    L"Emulator 8"
                                                     };

  // Initializes devices
  int ans = initializeUSB();
  if (ans) return 1;

  // Clear current assignemnt
  ans = clearSliceDeviceMap();

  // Sets number of slices
  size_t deviceCount = _usb->devices.size();
  if (deviceCount > 2*MAX_SLICE_COUNT)
    return 2;
  size_t sliceCount = deviceCount;
  ans = setSliceCount(sliceCount);
  if (ans) return 3;

  // Assign slices to devices according to expected device names
  for ( size_t k = 0 ; k != sliceCount ; ++k ){
    bool sliceAssigned = false;
    for ( size_t m = 0 ; m != deviceCount ; ++m ){
      // First 4 slices
      if ( k < MAX_SLICE_COUNT ){
        // OLD names/product labels ("Emulator 1-4")
        if ( !_usb->devices[m].deviceName.compare( SLICE_NAMES_OLD[k] ) ){
          // name of device m matches the expected name (OLD) for slice k
          assignSliceToDevice( k, m );
          sliceAssigned = true;
          break;
        } else if ( !_usb->devices[m].product.compare( SLICE_PRODUCT_OLD[k] ) ){
          // product label of device m matches the expected product label (OLD)
          // for slice k
          assignSliceToDevice( k, m );
          sliceAssigned = true;
          break;
        }

        // NEW names/product labels ("Emulator 5-8")
        if ( !_usb->devices[m].deviceName.compare( SLICE_NAMES_NEW[k] ) ){
          // name of device m matches the expected name (NEW) for slice k
          assignSliceToDevice( k, m );
          sliceAssigned = true;
          break;
        } else if ( !_usb->devices[m].product.compare( SLICE_PRODUCT_NEW[k] ) ){
          // product label of device m matches the expected product label (NEW)
          // for slice k
          assignSliceToDevice( k, m );
          sliceAssigned = true;
          break;
        }
      }

      // Following 4 slices (can only be matched against NEW names/product labels)
      else if ( MAX_SLICE_COUNT <= k && k < 2*MAX_SLICE_COUNT ){
        if ( !_usb->devices[m].deviceName.compare( SLICE_NAMES_NEW[k-MAX_SLICE_COUNT] ) ){
          // name of device m matches the expected name (NEW) for slice k
          assignSliceToDevice( k, m );
          sliceAssigned = true;
          break;
        } else if ( !_usb->devices[m].product.compare( SLICE_PRODUCT_NEW[k-MAX_SLICE_COUNT] ) ){
          // product label of device m matches the expected product label (NEW)
          // for slice k
          assignSliceToDevice( k, m );
          sliceAssigned = true;
          break;
        }
      }
    }

    // If the slice is still unassigned then auto-assignement has failed!
    if ( !sliceAssigned )
      return 4;
  }

  // Validate assignemnt
  ans = validateSliceDeviceAssignement();
  if ( ans ) return 5;

  return 0;
}

int Emulator::validateSliceDeviceAssignement(){

  // Calling validateSliceDeviceAssignement() downsets the state of the
  // Emulator to EMU_STATE_USBINIT
  if ( _state > EMU_STATE_USBINIT )
    _state = EMU_STATE_USBINIT;
  // Else if the state of the Emulator is not at least EMU_STATE_USBINIT
  // the function is not executed (returns non-zero)
  else if ( _state < EMU_STATE_USBINIT )
    return -1;

  map<size_t,int>::iterator it;
  for ( size_t k  = 0 ; k != _emuhw->sliceSet.size() ; ++k ){
    if ( (it = _sliceDeviceMap.find(k)) == _sliceDeviceMap.end() )
      // Slice k is unmapped
      return 1;
    if ( (*it).second < 0 || (*it).second >= static_cast<int>(_usb->devices.size()) )
      // Slice mapped to invalid device
      return 2;
  }

  // Precalibrate the slices according to premade measurements
  for ( size_t k = 0 ; k != _emuhw->sliceSet.size() ; ++k ){
    _precalibrateSlice(k, true);
  }

  // If the function executed properly, the state of Emulator changes
  // implementing the EMU_STATE_USBINIT -> EMU_STATE_SLCDEVASSIGNED transtn.
  _state = EMU_STATE_SLCDEVASSIGNED;

  return 0;
}

int Emulator::usbWrite( size_t devId,
                        size_t starting_cypress_address,
                        vector<uint32_t> data ){
  return _usb->write( devId, starting_cypress_address, data );
}

int Emulator::usbRead( size_t devId,
                       size_t starting_cypress_address,
                       size_t words_to_read,
                       std::vector<uint32_t>& read_buffer ){
  return _usb->read( devId, starting_cypress_address, words_to_read, read_buffer );
}

int Emulator::resetCalibration( size_t devId ){
  vector<uint32_t> data(1, 2222);
  return _usb->write( devId, 286, data );
}

int Emulator::endCalibrationMode(){

  // Regardless of the previous state init() resets the calibration state of the
  // Emulator to the initial state (YES: in calibration mode)
  _state_calibration = EMU_CALSTATE_YES;

  vector<bool> _isAtCalibrationMode_ = _isAtCalibrationMode();
  // Require all devices not to be at calibration mode
  bool calibrationModeChanged = false;
  for ( size_t k = 0 ; k != _usb->devices.size() ; ++k ){
    if ( _isAtCalibrationMode_[k] ){
      _endCalibrationMode( k );
      calibrationModeChanged = true;
    }
  }
  if ( calibrationModeChanged ){
    // Wait for mode change to be enacted
    auxiliary::sleep(500000);

    // Verify that change to the pipeline mode was successful
    _isAtCalibrationMode_ = _isAtCalibrationMode();
    for ( size_t k = 0 ; k != _usb->devices.size() ; ++k )
      if ( _isAtCalibrationMode_[k] != 0 )
        return 31;
  }

  // If the function executed properly, the calibration state of Emulator
  // changes implementing the EMU_CALSTATE_YES -> EMU_CALSTATE_NO transition
  _state_calibration = EMU_CALSTATE_NO;

  return 0;
}

void Emulator::set_ratioZ(double val){
  _ratioZ = val;
  _ratioI = _ratioV/_ratioZ;
}
void Emulator::set_ratioV(double val){
  _ratioV = val;
  _ratioI = _ratioV/_ratioZ;
}
void Emulator::set_maxIpu(double val){ _maxIpu = val; }

int Emulator::resetMapping(){

  // Calling resetMapping() downsets the state of the Emulator to
  // EMU_STATE_SLCDEVASSIGNED
  if ( _state > EMU_STATE_SLCDEVASSIGNED )
    _state = EMU_STATE_SLCDEVASSIGNED;

  return _mmd->resetMapping();
}

void Emulator::cleaMappingHints(){
  _mmd->clearHints();
  return;
}

void Emulator::hintComponent( int type, unsigned int extId){
  int mdlId = _mmd->getElement_mdlId( type, extId );
  _mmd->hintComponent(type, mdlId);
  return;
}

int Emulator::mapComponent( int type, unsigned int extId,
                            int mapper_tab, int mapper_row, int mapper_col ){

  // Calling mapComponent() downsets the state of the Emulator to
  // EMU_STATE_SLCDEVASSIGNED
  if ( _state > EMU_STATE_SLCDEVASSIGNED )
    _state = EMU_STATE_SLCDEVASSIGNED;

  int ans = _mmd->mapComponent( type, extId,
                                mapper_tab, mapper_row, mapper_col );
  if ( !ans )
    hintComponent( type, extId );
  return ans;
}

int Emulator::unmapComponent( int type, unsigned int extId){

  // Calling unmapComponent() downsets the state of the Emulator to
  // EMU_STATE_SLCDEVASSIGNED
  if ( _state > EMU_STATE_SLCDEVASSIGNED )
    _state = EMU_STATE_SLCDEVASSIGNED;

  return _mmd->unmapComponent( type, extId );
}

int Emulator::autoMapping(){

  // Calling autoMapping() downsets the state of the Emulator to
  // EMU_STATE_SLCDEVASSIGNED
  if ( _state > EMU_STATE_SLCDEVASSIGNED )
    _state = EMU_STATE_SLCDEVASSIGNED;

  return _mmd->autoMapping();
}

int Emulator::validateMapping(){

  // Calling validateMapping() downsets the state of the Emulator to
  // EMU_STATE_SLCDEVASSIGNED
  if ( _state > EMU_STATE_SLCDEVASSIGNED )
    _state = EMU_STATE_SLCDEVASSIGNED;
  // Else if the state of the Emulator is not at least
  // EMU_STATE_SLCDEVASSIGNED the function is not executed (returns non-zero)
  else if ( _state < EMU_STATE_SLCDEVASSIGNED )
    return -1;

  int ans = _mmd->validate();
  if ( ans ) return ans;

  // If the function executed properly, the state of Emulator changes
  // implementing the EMU_STATE_SLCDEVASSIGNED -> EMU_STATE_MAPOK transition
  _state = EMU_STATE_MAPOK;

  return 0;
}

void Emulator::autoRatioZ(){
  double maxX = _pws->getMaxX();
  // the following cannot be 2 * POTENTIOMETER_RAB, as generator xd_1's have to
  // be taken into account
  double maxAchievableR = _emuhw->getMinMaxAchievableR();
  double newRatioZ = maxAchievableR / maxX;
  set_ratioZ( newRatioZ );
}

void Emulator::defaultRatios(){
  _ratioZ = static_cast<double>(DEFRATIOZ);
  _ratioV = static_cast<double>(DEFRATIOV);
  _ratioI = static_cast<double>(_ratioV/_ratioZ);
  _maxIpu = static_cast<double>(DEFMAXIPU);
}

double Emulator::getMaxR() const{ return _emuhw->getMinMaxAchievableR(); }

int Emulator::autoFitting(vector<string>* outputMsg){

  // Calling autoFitting() downsets the state of the Emulator to
  // EMU_STATE_MAPOK
  if ( _state > EMU_STATE_MAPOK )
    _state = EMU_STATE_MAPOK;
  // Else if the state of the Emulator is not at least
  // EMU_STATE_MAPOK the function is not executed (returns non-zero)
  else if ( _state < EMU_STATE_MAPOK )
    return -1;

//  if ( _pws->status() != PWSSTATUS_LF ){
//    // Power flow must be solved before fitting onto the emulator!
//    return 10;
//  }
//  if ( _mmd->status() != PWSMODELSTATUS_VALID )
//    // Mapping must be validated before fitting onto the emulator!
//    return 11;
//  if ( _emuhw->sliceSet.size() <= 0 )
//    // Cannot perform auto-fitting on an emulator with no slices!
//    return 12;

  size_t k;           // counter
  int ans;            // result
  size_t fit_tab;     // tab where the component is to be fitted to
  size_t fit_row;     // row where the component is to be fitted to
  size_t fit_col;     // column where the component is to be fitted to

//  // ----- Reset fitting before autoFitting again -----
//  ans =_emuhw->reset(false);
//  if ( ans ) return 20;

//  // ----- Fit buses (maybe redundant) -----
//  PwsMapperModelElement const* cdBus;
//  for ( k = 0 ; k != _mmd->busElements_size() ; ++k ){
//    cdBus = _mmd->elementByIndex( PWSMODELELEMENTTYPE_BUS, k );
//    if (!cdBus->mapped)
//      // Normally should never be reached!
//      // Bus with ext id cdBus->extId is unmapped!
//      return 30;
//
//    // Fitting position
//    fit_tab = static_cast<size_t>(cdBus->tab) / 2;
//    fit_row = static_cast<size_t>(cdBus->row) / 2;
//    fit_col = static_cast<size_t>(cdBus->col) / 2;
//
//    // Fit
//    ans = _emuhw->sliceSet.at(fit_tab).node_set(fit_row,fit_col);
//    if ( ans ){
//      string msg( "Fitting bus with ext id " + auxiliary::to_string(cdBus->extId)
//                  + " failed with code " + auxiliary::to_string(ans) );
//      if ( outputMsg )
//        outputMsg->push_back(msg);
//    }
//  }

  // ----- Fit branches -----
  PwsMapperModelElement const* cdBr;
  Branch const* pBr;
  size_t fit_embrpos;
  size_t rows, cols;
  for ( k = 0 ; k != _mmd->branchElements_size() ; ++k ){
    cdBr = _mmd->elementByIndex( PWSMODELELEMENTTYPE_BR, k );
    if (!cdBr->mapped )
      // Normally should never be reached!
      // Branch with ext id cdBr->extId is unmapped!
      return 40;

    // Determine fitting position
    bool onTab = (cdBr->tab%2 == 0);
    bool onRow = (cdBr->row%2 == 0);
    bool onCol = (cdBr->col%2 == 0);
    _emuhw->sliceSet[0].ana.size( rows, cols);

    if ( onTab & onRow & !onCol ){
      // Intratab - horizontal branch
      fit_tab = static_cast<size_t>(cdBr->tab)/2;
      fit_row = static_cast<size_t>(cdBr->row)/2;
      fit_col = (static_cast<size_t>(cdBr->col)-1)/2;
      fit_embrpos = EMBRPOS_R;

    } else if ( onTab & !onRow & onCol ){
      // Intratab - vertical branch
      fit_tab = static_cast<size_t>(cdBr->tab)/2;
      fit_row = (static_cast<size_t>(cdBr->row)-1)/2;
      fit_col = static_cast<size_t>(cdBr->col)/2;
      fit_embrpos = EMBRPOS_U;

    } else if ( onTab & !onRow & !onCol ){
      // Intratab - diagonal branch
      // Intratab diagonal branches can only be in the following sense:
      // bottom-left -> top-right
      fit_tab = static_cast<size_t>(cdBr->tab)/2;
      fit_row = (static_cast<size_t>(cdBr->row)-1)/2;
      fit_col = (static_cast<size_t>(cdBr->col)-1)/2;
      fit_embrpos = EMBRPOS_UR;

    } else if ( !onTab & onRow & onCol ){
      // Intertab branch - exactly vertical
      fit_tab = (static_cast<size_t>(cdBr->tab)-1)/2;
      fit_row = static_cast<size_t>(cdBr->row)/2;
      fit_col = static_cast<size_t>(cdBr->col)/2;
      if ( fit_row == (rows-1) ){
        // Top-row vertical intertab branches correspond to U embr pos
        fit_embrpos = EMBRPOS_U;
      } else if ( fit_col == (cols-1) ){
        // Last-column vertical intertab branches correspond to R embr pos
        fit_embrpos = EMBRPOS_R;
      } else if ( fit_col == 0 ){ // && fit_row != rows
        // First-column vertical intertab branches correspond to L embr pos
        fit_embrpos = EMBRPOS_L;
      } else if ( fit_row == 0 ){ // && fit_col != 0 && fit_col != cols
        // Bottom-row vertical intertab branches correspond to D embr pos
        fit_embrpos = EMBRPOS_D;
      } else {
        // Weird mapping!
        // Branch with ext id cdBr->extId mapped on invalid position:
        // [cdBr->tab,cdBr->row,cdBr->col] = [mapper_tab,row,col]
        return 41;
      }

    } else if ( !onTab & onRow & !onCol ){
      // Intertab branch - diagonal in the column sense
      // Corresponds to the diagonal intertab branches on the top of the
      // emulator grid
      fit_tab = (static_cast<size_t>(cdBr->tab)-1)/2;
      fit_row = static_cast<size_t>(cdBr->row)/2;
      fit_col = (static_cast<size_t>(cdBr->col)-1)/2;
      if ( fit_row == (rows-1) ){
        // Top-row diagonal (col. sense) intertab branches correspond to UR embr
        // pos
        fit_embrpos = EMBRPOS_UR;
      } else {
        // Weird mapping!
        // Branch with ext id cdBr->extId mapped on invalid position:
        // [cdBr->tab,cdBr->row,cdBr->col] = [mapper_tab,row,col]
        return 42;
      }

    } else if ( !onTab & !onRow & onCol ){
      // Intertab branch - diagonal in the row sense
      // Corresponds to the diagonal intertab branches on the right side of
      // the emulator grid
      fit_tab = (static_cast<size_t>(cdBr->tab)-1)/2;
      fit_row = (static_cast<size_t>(cdBr->row)-1)/2;
      fit_col = static_cast<size_t>(cdBr->col)/2;
      if ( fit_col == (cols-1) ){
        // Last-column diagonal (row sense) intertab branches correspond to UR
        // embr pos
        fit_embrpos = EMBRPOS_UR;
      } else {
        // Weird mapping!
        // Branch with ext id cdBr->extId mapped on invalid position:
        // [cdBr->tab,cdBr->row,cdBr->col] = [mapper_tab,row,col]
        return 43;
      }


    } else{ // ( !onTab & !onRow & !onCol )
      // Intertab branch
      // Warning! No such intertab branch is possible on the current version
      // of the emulator hardware! Nov 2011
      // Branch with ext id cdBr->extId mapped on invalid position:
      // [cdBr->tab,cdBr->row,cdBr->col] = [mapper_tab,row,col]
      return 44;
    }

    // Retrieve branch to be fitted
    ans = _pws->getBranch( cdBr->extId, pBr );
    if ( ans )
      // Branch with ext id cdBr->extId cannot be retrieved!
      return 45;

    // Fit
    ans = embr_set(fit_tab, fit_row, fit_col, fit_embrpos, *pBr, _ratioZ, 0.5);
    if ( ans ){
      string msg( "Fitting branch with ext id " + auxiliary::to_string(cdBr->extId)
                  + " failed with code " + auxiliary::to_string(ans) );
      if ( outputMsg )
        outputMsg->push_back(msg);
    }
  }

  // ----- Fit generators -----
  PwsMapperModelElement const* cdGen;
  Generator const* pGen;
  for ( k = 0 ; k != _mmd->genElements_size() ; ++k ){
    cdGen = _mmd->elementByIndex( PWSMODELELEMENTTYPE_GEN, k );
    if (!cdGen->mapped )
      // Normally should never be reached!
      // Gen with ext id cdGen->extId is unmapped!
      return 50;

    // Determine fitting position
    fit_tab = static_cast<size_t>(cdGen->tab) / 2;
    fit_row = static_cast<size_t>(cdGen->row) / 2;
    fit_col = static_cast<size_t>(cdGen->col) / 2;

    // Retrieve generator to be fitted
    ans = _pws->getGenerator( cdGen->extId, pGen );
    if ( ans )
      // Generator with ext id cdGen->extId cannot be retrieved!
      return 51;

    // Fit
    ans = node_set(fit_tab,fit_row,fit_col, *pGen, _ratioZ,_ratioV,_ratioI,_maxIpu);
    if ( ans ){
      string msg( "Fitting generator with ext id " + auxiliary::to_string(cdGen->extId)
                  + " failed with code " + auxiliary::to_string(ans) );
      if ( outputMsg )
        outputMsg->push_back(msg);
    }
  }

  // ----- Fit loads -----
  PwsMapperModelElement const* cdLoad;
  Load const* pLoad;
  for ( k = 0 ; k != _mmd->loadElements_size() ; ++k ){
    cdLoad = _mmd->elementByIndex( PWSMODELELEMENTTYPE_LOAD, k );
    if (!cdLoad->mapped )
      // Normally should never be reached!
      // Load with ext id cdLoad->extId is unmapped!
      return 52;

    // Determine fitting position
    fit_tab = static_cast<size_t>(cdLoad->tab) / 2;
    fit_row = static_cast<size_t>(cdLoad->row) / 2;
    fit_col = static_cast<size_t>(cdLoad->col) / 2;

    // Retrieve load to be fitted
    ans = _pws->getLoad( cdLoad->extId, pLoad );
    if (ans)
      // Load with ext id cdLoad->extId cannot be retrieved!
      return 53;

    // Fit
    ans = node_set(fit_tab,fit_row,fit_col, *pLoad, _ratioI,_maxIpu);
    if ( ans ){
      string msg( "Fitting load with ext id "
                  + auxiliary::to_string(cdLoad->extId)
                  + " failed with code " + auxiliary::to_string(ans) );
      if ( outputMsg )
        outputMsg->push_back(msg);
    }
  }

  return 0;
}

int Emulator::validateFitting(){

  // Calling autoFitting() downsets the state of the Emulator to
  // EMU_STATE_MAPOK
  if ( _state > EMU_STATE_MAPOK )
    _state = EMU_STATE_MAPOK;
  // Else if the state of the Emulator is not at least
  // EMU_STATE_MAPOK the function is not executed (returns non-zero)
  else if ( _state < EMU_STATE_MAPOK )
    return -1;

//  if ( _pws->status() != PWSSTATUS_LF ){
//    // Power flow must be solved before fitting onto the emulator!
//    return 10;
//  }

  // validateFitting() does really nothing. It simply updates the state machine
  // to EMU_STATE_FITOK
  // TODO REALLY IMPLEMENT validateFitting()

  // If the function executed properly, the state of Emulator changes
  // implementing the EMU_STATE_MAPOK -> EMU_STATE_FITOK transition
  _state = EMU_STATE_FITOK;

  return 0;
}

int Emulator::encodePowersystem(){

  // Calling autoFitting() downsets the state of the Emulator to
  // EMU_STATE_FITOK
  if ( _state > EMU_STATE_FITOK )
    _state = EMU_STATE_FITOK;
  // Else if the state of the Emulator is not at least
  // EMU_STATE_FITOK the function is not executed (returns non-zero)
  else if ( _state < EMU_STATE_FITOK )
    return -1;

  encoding.clear();
  encoding.resize( _emuhw->sliceSet.size() );

  for( size_t k = 0 ; k != _emuhw->sliceSet.size(); ++k ){
    if ( encoder::encodeSliceTD( _emuhw->sliceSet[k], encoding[k] ) )
      return k;
  }

  // If the function executed properly, the state of Emulator changes
  // implementing the EMU_STATE_FITOK -> EMU_STATE_PWSENCODED transition
  _state = EMU_STATE_PWSENCODED;

  return 0;
}

// Simply writes encoding to slices
int Emulator::writeEncoding( bool verify ){

  PrecisionTimer timer; // counts in seconds
  timer.Start();

  // If the state of the Emulator is not at least EMU_STATE_PWSENCODED
  // then the function is not executed (returns non-zero)
  if ( _state < EMU_STATE_PWSENCODED )
    return -1;

  int ans;
  for( size_t k = 0 ; k != _emuhw->sliceSet.size(); ++k ){
    // Limit checking
    int devId = _sliceDeviceMap.at(k);
    if ( devId < 0 || devId >= static_cast<int>(_usb->devices.size()) )
      return 1;

    // Write bitstream to board
    ans = _usb->write( devId,  1 , encoding[k] );
    if ( ans ) return 2;
    cout << "Emulator::_writeEncoding(): ";
    cout << "write dev" << devId << " " << timer.Stop() << " s" << endl;
    timer.Start();
  }

  // Verify encoding after writing was requested
  if ( verify ){
    ans = _verifyEncoding();
    if ( ans ) return 3;
    cout << "Emulator::_writeEncoding(): ";
    cout << "verify " << timer.Stop() << " s" << endl;
  }

  return 0;
}

EmulatorHw* Emulator::emuhw(){ return _emuhw; }
Powersystem const* Emulator::pws(){ return _pws; }
PwsMapperModel* Emulator::mmd(){ return _mmd; }
vector<USBDevice> Emulator::getUSBDevices() const{ return _usb->devices; }
size_t Emulator::getUSBDevicesCount() const{ return _usb->devices.size(); }
double Emulator::ratioZ() const{ return _ratioZ; }
double Emulator::ratioV() const{ return _ratioV; }
double Emulator::ratioI() const{ return _ratioI; }
double Emulator::maxIpu() const{ return _maxIpu; }
map<size_t, int> Emulator::sliceDeviceMap() const{ return _sliceDeviceMap; }
int Emulator::sliceDeviceMap(size_t index) const{ return _sliceDeviceMap.at(index); }
int Emulator::state() const{ return _state; }
int Emulator::state_calibration() const{ return _state_calibration; }


int Emulator::_precalibrateSlice( size_t sliceId, bool toDefaultVoltage ){

  // Validate input argument
  if ( sliceId >=  _emuhw->sliceSet.size() )
    return 1;

  // Validate slice mapping
  int devId = _sliceDeviceMap.at( sliceId );
  if ( devId < 0 || devId >= static_cast<int>(_usb->devices.size()) )
    return 2;

  // Retrieve precalibration measurements for the device corresponding to the
  // slice in question
  Slice* sl = &_emuhw->sliceSet[sliceId];
  // by using the device name
  string devName = _usb->devices[devId].deviceName;
  map<string, pair<unsigned int, unsigned int> >::const_iterator itName;
  itName = _vRefsPrecalibrationMap.find( devName );
  bool foundName = (itName != _vRefsPrecalibrationMap.end());
  // by using the product label
  wstring devProduct = _usb->devices[devId].product;
  map<wstring, pair<unsigned int, unsigned int> >::const_iterator itProduct;
  itProduct = _vRefsPrecalibrationMapW.find( devProduct );
  bool foundProduct = (itProduct!= _vRefsPrecalibrationMapW.end());

  // Get the actual measurements
  pair<unsigned int, unsigned int> tapMeasurements;
  if ( foundName ){
    tapMeasurements = itName->second;
  } else if ( foundProduct ){
    tapMeasurements = itProduct->second;
  } else {
    // No measurement data for device with name devName
    return 3;
  }
  unsigned int tapRealMeaserement = tapMeasurements.first;
  unsigned int tapImagMeasurement = tapMeasurements.second;

  // Real voltage reference precalibration
  double newOutMax = sl->ana.real_voltage_ref_val_min()
      + (REAL_VOLTAGE_REF_MEASURED_VAL - sl->ana.real_voltage_ref_val_min())
      * ( (static_cast<double>(sl->ana.real_voltage_ref_tap_max()) + 1)
           / tapRealMeaserement );
  int ans = sl->ana.set_real_voltage_ref_out_max( newOutMax, true );
  if ( ans ) return 4;

  // Imag voltage reference precalibration
  newOutMax = sl->ana.imag_voltage_ref_val_min()
      + (IMAG_VOLTAGE_REF_MEASURED_VAL - sl->ana.imag_voltage_ref_val_min())
      * ( (static_cast<double>(sl->ana.imag_voltage_ref_tap_max()) + 1)
           / tapImagMeasurement );
  ans = sl->ana.set_imag_voltage_ref_out_max( newOutMax, true );
  if ( ans ) return 5;

  // Set voltage to default value if asked so
  if ( toDefaultVoltage ){
    ans = sl->ana.set_real_voltage_ref_val( REAL_VOLTAGE_REF_MEASURED_VAL );
    if ( ans ) return 6;
    ans = sl->ana.set_imag_voltage_ref_val( REAL_VOLTAGE_REF_MEASURED_VAL );
    if ( ans ) return 7;
  }

  return 0;
}

vector<bool> Emulator::_isAtCalibrationMode(){
  vector<bool> ans;
  vector<uint32_t> tempvector;
  for ( size_t k = 0 ; k != _usb->devices.size() ; ++k ){
    _usb->read(k, 0, 1, tempvector );
    if ( tempvector[0] == DEVICE_CALIBRATION_SIGNATURE )
      ans.push_back( true );
    else
      ans.push_back( false );
  }
  return ans;
}

int Emulator::_endCalibrationMode( size_t devId ){

  vector<uint32_t> tempvector;
  tempvector.push_back(5555);
  int ans = _usb->write( devId, 286, tempvector);
  if ( ans )
    return 1;

  return 0;
}

int Emulator::_verifyEncoding(){

  vector<uint32_t> temp;

  for( size_t k = 0 ; k != _emuhw->sliceSet.size(); ++k ){
    // Limit checking
    int devId = _sliceDeviceMap.at(k);
    if ( devId < 0 || devId >= static_cast<int>( _usb->devices.size()) )
      return -devId;

    // Verify data written
    int ans = _usb->read( devId,  1 , encoding[k].size(), temp );
    if ( ans )
      return k;
    if ( temp.size() != encoding[k].size() )
      return (k+10);
    for ( size_t m = 0; m != temp.size(); ++m ){

      // Prevision for stop code 'hole' in encoding
      if ( m == 556 )
        continue;

      // Verify written conf agains theoretically calculated one
      if ( temp[m] != encoding[k][m] )
        return (k+100);
    }
  }

  return 0;
}
