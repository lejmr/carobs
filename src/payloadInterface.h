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

#ifndef __CAROBS_PAYLOADINTERFACE_H_
#define __CAROBS_PAYLOADINTERFACE_H_

#include <omnetpp.h>
#include <messages/Payload_m.h>


/**
 *   PayloadInterface is transient module helping with first/last step
 *   switching of Payload packets. It is tightly bounded with PortManger
 *   which table updates.
 */
class PayloadInterface : public cSimpleModule
{
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

  private:
    /**
     *  Pairing information of port - src/dst .. structure is as follows
     *  pairing[ src/dst ] = port
     */
    std::map<int, int> pairing;

    /**
     *    Function updatePairing is remotely called by PayloadInterface to
     *    update informations about src address to port pairing
     *
     *    @param port: #of input port obtained from incoming message
     *    @param src: source address of incoming port
     */
    virtual void updatePairing(int port, int src);

    /**
     *    Function getOutputPort resolves outgoing port for given destination
     *
     *    @param dst: destination of de-cared Payload packet
     *    @return: index of outgoing port
     */
    virtual int getOutputPort(int dst);


};

#endif
