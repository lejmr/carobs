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

#include "aggregationPool.h"
#include "messages/Payload_m.h"

Define_Module(AggregationPool);

void AggregationPool::initialize()
{
    bufferLengthT = par("bufferLengthT").doubleValue();
    bufferLengthB = par("bufferLengthB").longValue();
}

void AggregationPool::handleMessage(cMessage *msg)
{
    /*
     *  Self-messages
     *  Self-messages are used for AggregationPool controlling
     *
     */
    if (dynamic_cast<Payload *>(msg) == NULL and msg->isSelfMessage() ){
        EV << "Scheduling the buffer to be send further." << endl;
        releaseBuffer( msg->par("dst").longValue() );
    }
    else{
        /*
         *  Payload packets controlling
         */
        Payload *pmsg = dynamic_cast<Payload *>(msg);
        //EV << pmsg->getSrc() << " --> " << pmsg->getDst() << endl;

        if( queues.find( pmsg->getDst() ) == queues.end() ){
            // There is not a buffer for such destinationm must be created
            //EV << "pro " << pmsg->getDst() << " nutno zalozit" << endl;
            char buffer_name[50];
            sprintf(buffer_name, "Buffer %d", pmsg->getDst());
            queues[ pmsg->getDst() ] = cQueue();
            queues[ pmsg->getDst() ].setName(buffer_name);

            // Buffer releas scheduling
            cMessage *snd = new cMessage();
            snd->addPar("dst");
            snd->par("dst").setLongValue(pmsg->getDst());
            scheduleAt(simTime()+bufferLengthT, snd);
            scheduled[pmsg->getDst()] = snd;
        }

        // Add packet to its output Buffer #
        queues[ pmsg->getDst() ].insert( pmsg );

        // Count the amount of buffered data
        cQueue queue= queues[ pmsg->getDst() ];
        int64_t size = 0;
        for( cQueue::Iterator iter(queue,0); !iter.end(); iter++){
            Payload *pl = (Payload *) iter();
            //EV << pl->getSrc() << " --> " << pl->getDst() << ": " << pl->getByteLength() << endl;
            size += pl->getByteLength();
        }
        //EV << "Velikost " << queue.getName() << " je " << size << endl;

        if( size >= bufferLengthB ){
            // Size of the buffer is over the limit, buffer will be send onto network as a burst
            EV << queue.getName() << " is full, packets are send onto network." << endl;

            // Self-message (signaling) to release the buffer
            cMessage *snd = new cMessage();
            snd->addPar("dst");
            snd->par("dst").setLongValue(pmsg->getDst());
            scheduleAt(simTime(), snd);
            // Descheduling the time-based Self-message
            EV << "Descheduling time-based trigger for buffer " << pmsg->getDst() << endl;
            cancelEvent(scheduled[pmsg->getDst()]);
            scheduled.erase(pmsg->getDst());
        }
    }
}


void AggregationPool::releaseBuffer(int dst){

    EV << "Vyprazdnuji buffer "<< dst << endl;
    // Packets are send to different location, thus buffer can be erased
    queues.erase (dst);
}
