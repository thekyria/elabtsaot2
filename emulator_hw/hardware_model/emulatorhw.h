/*!
\file emulatorhw.h
\brief Definition file for class EmulatorHw

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef EMULATORHW_H
#define EMULATORHW_H

#include "slice.h"

#include <vector>

namespace elabtsaot{

/*! EmulatorHw class
  The emulator is based on a hybrid architecture where grid equations are solved
  analogically and generator/load equations are solved digitally. Crossing from
  the analog to the digital domain and vice versa is performed through A/D and
  D/A converters respectively. The new digital computation scheme increases the
  speed of the digital computation part by a parallel, pipelined architecture,
  that is implemented on an Altera Cyclone EP3C80 FPGA, that is hosted on a
  specially created board. Currents are fed back to the grid (DAC) according to
  calculation performed based on measured voltages (ADC).

  Physically an emulator consists of a set of stacked slices consisting of a
  connected digital (FPGA) PCB and an analog PCB.

  The configuration of the FPGA is based around the softcore CPU Altera NIOS II,
  which is responsible for the initialization and configuration of its
  peripherals as well as of the resistor board. Pipeline stages are implemented
  as seperate NIOS peripherals. A Cypress CY7C68013a high speed USB (2.0)
  peripheral controller handles physically the communication with the PC. In
  order for the NIOS II to communicate with the PC a dual port shared memory
  scheme has been employed.

  On the analog grid side, a PCB structure that can accomodate up to 24 nodes
  has been created (4x6 lattice). The analog grid PCB is mounted on the FPGA PCB
  by using special connectors. The board consists of reconfigurable
  potentiometers and switches, so that different real-world power system
  topologies can be mapped on the reconfigurable topology of the emulator.

  In order to make the hardware emulator visible in software respective classes
  are created.

  \warning This class is part of the emulator classes that provide a software
  representation of the hardware emulator developed at ELAB. As such, changes in
  hardware are reflected in the software classes. For more information on the
  hardware of the project contact: guillaume dot lanz at epfl dot ch (hw team
  leader)

  \todo calculate_Y_matrix() for multi-slice

  \author thekyria
  \date Sep 2013 */
class EmulatorHw{

 public:

  int init(size_t sliceCount);
  int reset(bool complete);
  double getMinMaxAchievableR() const;
  size_t getEmbrCount() const;
  void set_got_gain(double val);
  int set_got_offset(double val);
  int set_real_voltage_ref_val(double val);
  int set_real_voltage_ref_tap(unsigned int tap);
  int set_imag_voltage_ref_val(double val);
  int set_imag_voltage_ref_tap(unsigned int tap);

  std::vector<Slice> sliceSet;
};

} // end of namespace elabtsaot

#endif // EMULATORHW_H
