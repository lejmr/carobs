//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef __CAROBS_DEMUX_H_
#define __CAROBS_DEMUX_H_

#include <omnetpp.h>
#include <messages/OpticalLayer_m.h>

/**
 *  All optical Array Waveguide Grating - AWG
 *  - All optical MUX
 *  - All optical DEMUX
 */
class DeMux : public cSimpleModule
{
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

  private:
    /**
     *  DeMux class implements both MUX and DEMUX since they are very similar.
     *  It behaves as a passive part of the network and only joins incoming ports
     *  into on output port or visa versa. It is designed to be used with only
     *  one port/fiber.
     */
    bool mux;

    /* IA section */
    // QoT
    double OSNR_drop_level;
    int64_t osnr_drop; // Statistics

    // Multiplex/Demultiplexing attenuation
    double att;

    // Amplifier parameters
    bool amp_enabled;
    double B_0, G_0, P_sat, A1, A2, NF;
    int G_N;

    // Simulation parameters
    double c_0, f_0; // Shortest wavelength

    /**
     *  Function muxing implements MUX behaviour
     *  @param msg: stands for incoming message
     */
    virtual void demuxing(OpticalLayer *msg);

    /**
     *  If amplification is turned on both levels of power and noise are changed
     *  i.e. the optical signal gets amplified
     */
    virtual void amplify(OpticalLayer *msg);

};
#endif
