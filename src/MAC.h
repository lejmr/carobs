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

/**
 * TODO - Generated class
 */
class MAC : public cSimpleModule
{
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

  private:
    /**
     *  Pointer to Routing module for Offset time adjustment
     */
    Routing *R;

    /**
     *  Function getOutputWavelength resolves the best wavelength to be
     *  used for output port given by a parameter
     *
     *  @param port - refers to egress port of communication
     *  @return empty wavelength of outgoing port
     */
    virtual int getOutputWavelength(int port);

    /**
     *  Function timeEgressIsReady counts when the output port and wavelength
     *  are going to be free for another CARBOS train initialisation
     *
     *  @param port - refers to number of outgoing port
     *  @param wl   - refers to wavelength of associate port
     *  @return     - time when combination of port&wl is ready to be used for sending
     */
    virtual simtime_t timeEgressIsReady(int port, int wl);

};

#endif
