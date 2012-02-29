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

#include "payloadInterface.h"

Define_Module(PayloadInterface);

void PayloadInterface::initialize()
{
    // Making link with AQ
    cModule *calleeModule = getParentModule()->getSubmodule("PM");
    PM = check_and_cast<PortManager *>(calleeModule);
}

void PayloadInterface::handleMessage(cMessage *msg)
{
    // cMessage to Payload conversion
    Payload *pl = dynamic_cast<Payload *>(msg);

    // Payload packet disassembled from a car
    if ( msg->getArrivalGate()->getName() == "incoming" ) {
        int port = PM->getOutputPort( pl->getDst() );
        send(msg, "in", port);
        return;
    }

    // Update pairing informations
    PM->updatePairing( msg->getArrivalGate()->getIndex(), pl->getSrc() );

    // Forward the message to AQ module get processed
    send( msg, "outgoing" );
}
