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

#include "aggregationQueues.h"


Define_Module(AggregationQueues);

void AggregationQueues::initialize()
{
    bufferLengthT = par("bufferLengthT").doubleValue();
    bufferLengthB = par("bufferLengthB").longValue();

    cModule *calleeModule = getParentModule()->getSubmodule("TA");
    TA = check_and_cast<TrainAssembler *>(calleeModule);

    WATCH_MAP( scheduled );
}

void AggregationQueues::handleMessage(cMessage *msg)
{
    /*
     *  Self-messages
     *  Self-messages are used for AggregationPool controlling
     *
     */
    if (dynamic_cast<Payload *>(msg) == NULL and msg->isSelfMessage() ){
        EV << "Initialising the time based sending procedure for AQ "<< msg->par("dst").longValue() << endl;
        TA->initialiseTimeBasedSending(msg->par("dst").longValue());
        delete msg;
    }
    else{
        /*  Payload packets controlling  */
        handlePayload(msg);
    }
}

void AggregationQueues::handlePayload(cMessage *msg){
    Payload *pmsg = dynamic_cast<Payload *>(msg);
    //EV << pmsg->getSrc() << " --> " << pmsg->getDst() << endl;

    if (AQ.find(pmsg->getDst()) == AQ.end()) {
        // There is not a buffer for such destination must be created
        //EV << "pro " << pmsg->getDst() << " nutno zalozit" << endl;
        char buffer_name[50];
        sprintf(buffer_name, "Buffer %d", pmsg->getDst());
        AQ[pmsg->getDst()] = cQueue();
        AQ[pmsg->getDst()].setName(buffer_name);

        // Buffer release scheduling
        cMessage *snd = new cMessage();
        snd->addPar("dst");
        snd->par("dst").setLongValue(pmsg->getDst());
        scheduleAt(simTime() + bufferLengthT, snd);
        scheduled[pmsg->getDst()] = snd;
    }

    // Add packet to its output Buffer #
    AQ[pmsg->getDst()].insert(pmsg);

    // Inform TA about a new incoming packet
    TA->aggregationQueuesNotificationInterface(pmsg->getDst());
}

void AggregationQueues::releaseBuffer(int dst){
    EV << "Vyprazdnuji buffer "<< dst << endl;
    // Packets are send to different location, thus buffer can be erased
}


int64_t AggregationQueues::countAggregationQueueSize(int AQId){
    Enter_Method("countAggregationQueueSize()");

    // If such AQ has not been created, it returns 0
    if( AQ.find(AQId) == AQ.end() ) return 0;

    // Such AQ exists thus I will count its size
    cQueue queue= AQ[ AQId ];
    int64_t size = 0;
    for( cQueue::Iterator iter(queue,0); !iter.end(); iter++){
        Payload *pl = (Payload *) iter();
        //EV << pl->getSrc() << " --> " << pl->getDst() << ": " << pl->getByteLength() << endl;
        size += pl->getByteLength();
    }
    return size;
}

void AggregationQueues::releaseAggregationQueues( std::set<int> queues, int tag ){
    Enter_Method("releaseAggregationQueues()");
    std::set<int>::iterator it;
    EV << "taguji pro AP="<<tag <<endl;

    // Walk through all designated queues, convert them into a car and send them to TA
    for ( it=queues.begin() ; it != queues.end(); it++ ){

        // If designated queue is empty we can skip it
        if( AQ.find(*it) == AQ.end()) continue;

        // Create a car for queue and fill it by Payload packets from AQ#
        Car *car = new Car();
        while(!AQ[*it].empty()){
            Payload *pl = (Payload *) AQ[*it].pop();
            car->addByteLength(pl->getByteLength());
            car->getPayload().insert(pl);
        }

        // Drop the queue and its scheduled initiation
        AQ.erase(*it);
        scheduled.erase(*it);

        // Sends car of the queue to TA
        send(car,"out");
    }
    */
}
