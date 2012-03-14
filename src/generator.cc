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

#include "generator.h"
#include "messages/Payload_m.h"

Define_Module(Generator);

using namespace std;

void Generator::initialize()
{
    myMinID = par("myMinID").longValue();
    myMaxID = par("myMaxID").longValue();
    amount = par("n").longValue();
    dstPerm = par("dst").doubleValue();


    cModule *calleeModule = getParentModule()->getSubmodule("routingTable");
    RT = check_and_cast<RoutingTable *>(calleeModule);

    cMessage *init= new cMessage();
    if( par("send").boolValue() )
        scheduleAt(0, init);

}

void Generator::handleMessage(cMessage *msg)
{

    if ( dynamic_cast<Payload *>(msg) == NULL){

        int dst, src;
        int rnd;
        for (int i = 0; i < amount; i++) {
            // Generate random destination, but must take care of not generating for myself
            if( dstPerm >0 ){
                // Hardcoded destination
                dst = dstPerm;
            }else{
                // Random destination
                while (true) {
                    dst = RT->getNthRemoteAddress( uniform(0, RT->dimensionOfRemoteAddressSet()));
                    if (dst > myMaxID or dst < myMinID)
                        break;
                }
            }
            src = uniform(myMinID, myMaxID);
            simtime_t gap = par("l").doubleValue();
            int length= par("length").doubleValue();
            Payload *msg = new Payload();
            msg->setByteLength( length ); // testing of WC
            msg->setDst(dst);
            msg->setSrc(src);
            scheduleAt(simTime() + gap, msg);
            EV << src << "->" << dst << " ("<< length <<"B): " << simTime() + gap << endl;
        }

    }else{
        send(msg, "out");
    }

}
