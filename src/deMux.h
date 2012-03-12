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
  private:
    /**
     *  DeMux class implements both MUX and DEMUX since they are very similar.
     *  It behaves as a passive part of the network and only joins incoming ports
     *  into on output port or visa versa. It is designed to be used with only
     *  one port/fiber.
     */
    bool mux;

    /**
     *  Function muxing implements MUX behaviour
     *  @param msg: stands for incoming message
     */
    virtual void demuxing(OpticalLayer *msg);

};

#endif
