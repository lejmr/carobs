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

#ifndef __CAROBS_MAC_H_
#define __CAROBS_MAC_H_

#include <omnetpp.h>
#include <routing.h>
#include <messages/MACContainer_m.h>
#include <messages/CAROBSHeader_m.h>
#include <messages/schedulerUnit_m.h>
#include <messages/car_m.h>
#include <messages/OpticalLayer_m.h>
#include <MACEntry.h>

struct output_t{
    int WL;
    simtime_t t;
};
class MAC : public cSimpleModule
{
  protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
    //virtual void numberOfWavelengthsUsed();

  private:
    /**
     *  Pointer to Routing module for Offset time adjustment
     */
    Routing *R;

    /**
     *  Function getOutput resolves the best wavelength to be
     *  used for output port given by a parameter. The condition for best
     *  is lowest waiting time till burst is send onto fiber
     *
     *  @param port - refers to egress port of communication
     *  @return empty wavelength of outgoing port
     */
    virtual output_t getOutput(int port, simtime_t ot);

    /**
     *  Maximum number of wavelengths
     */
    int maxWL;

    /**
     *  Wavelength availability scheduler - it keeps information when an bust can be send
     *  onto outgoing fiber
     */
    cArray portScheduled;

    simtime_t guardTime;
    simtime_t avg_waitingtime, total_waitingtime;
    int64_t burst_send, nums;
};

#endif
