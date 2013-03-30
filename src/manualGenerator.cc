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

#include "manualGenerator.h"
#include "messages/Payload_m.h"

Define_Module(ManualGenerator);

using namespace std;

void ManualGenerator::initialize()
{
    address = par("address").longValue();
    dst = par("dst").longValue();
    n = par("n").doubleValue();
    bandwidth = par("bandwidth").doubleValue();
    length = par("length").doubleValue();
    n_done= 0;
    last_send= 0;
    lambda= bandwidth / length / 8;
    blast = par("blast").doubleValue();

    EV << "Preparing "<<n<<" payload packets to " << dst << " with bandwidth "<<bandwidth/1e9;
    EV << "Gbps => lambda "<<lambda<< endl;

    if( par("send").boolValue() ){
        cMessage *t = new cMessage();
        t->addPar("initiateSending");
        scheduleAt(0, t);
    }
}

void ManualGenerator::handleMessage(cMessage *msg)
{
    if ( msg->isSelfMessage() and msg->hasPar("initiateSending") ){
        EV << "Posilam " << endl;

        int to_send= blast;
        if (n-n_done < blast)
            to_send=n-n_done;

        // Send some messages
        sendMessages(to_send);

        // Next sending
        if( n-n_done > 0 ){
            cMessage *t = new cMessage();
            t->addPar("initiateSending");
            scheduleAt(last_send, t);
        }

        // Finish processing of this msg
        delete msg; return;
    }


    // Send prepared Payload packet
    if (dynamic_cast<Payload *>(msg) != NULL) {
        send(msg, "out");
    }
}

void ManualGenerator::sendMessages(int amount){
    for(int i=0; i < amount;i++){
        Payload *msg = new Payload();
        msg->setByteLength( length ); // testing of WC
        msg->setDst(dst);
        msg->setSrc(address);
        msg->setSchedulingPriority(10);
        msg->setT0(last_send);

        scheduleAt(last_send, msg);
        EV << " + " << address << "->" << dst << " (" << length << "B): " << last_send << endl;
        last_send += exponential(1/lambda);
        n_done++;
    }
    EV << "Next sending is planned at " << last_send << endl;
}
