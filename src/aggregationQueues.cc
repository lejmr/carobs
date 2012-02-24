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


    WATCH_MAP( scheduled );
    WATCH_MAP( AQSizeCache );
}

void AggregationQueues::handleMessage(cMessage *msg)
{
bool deleted=false;

//EV << endl << "MSG 2";
    if( not deleted and msg->hasPar("initTBSDst") and msg->isSelfMessage() ){
//        EV << "entered";
        EV << "Initialising the time based sending procedure for AQ "<< msg->par("initTBSDst").longValue() << endl;
        cMessage *imsg = new cMessage();
        imsg->addPar("initialiseTimeBasedSending");
        imsg->par("initialiseTimeBasedSending") = msg->par("initTBSDst").longValue();
        send(imsg, "out");
        delete msg;deleted=true;
    }

//EV << endl << "MSG 3";
    if ( not deleted and dynamic_cast<Payload *>(msg) != NULL){
//        EV << "entered";
        handlePayload(msg);
    }
}

void AggregationQueues::handlePayload(cMessage *msg){
    Payload *pmsg = dynamic_cast<Payload *>(msg);
    //EV << pmsg->getSrc() << " --> " << pmsg->getDst() << endl;

    if (AQ.find(pmsg->getDst()) == AQ.end()) {
        // There is not a buffer for such destination must be created
        char buffer_name[50];
        sprintf(buffer_name, "Buffer %d", pmsg->getDst());
        AQ[pmsg->getDst()] = cQueue();
        AQ[pmsg->getDst()].setName(buffer_name);

        // Buffer release scheduling
        cMessage *snd = new cMessage();
        snd->addPar("initTBSDst");
        snd->par("initTBSDst").setLongValue(pmsg->getDst());
        scheduled[pmsg->getDst()] = snd;
        scheduleAt(simTime() + bufferLengthT, snd);

        EV << "**** Schdul " << snd->isScheduled() << "self " << snd->isSelfMessage() << endl;
    }

    // Add packet to its output Buffer #
    AQ[pmsg->getDst()].insert(pmsg);

    // Update AQSizeCache
    this->countAggregationQueueSize(pmsg->getDst());

    // Inform TA about AQ change
    cMessage *imsg = new cMessage("AQnotify");
    imsg->addPar("aggregationQueuesNotificationInterface");
    imsg->par("aggregationQueuesNotificationInterface")=pmsg->getDst();
    send(imsg, "out");
}

void AggregationQueues::releaseBuffer(int dst){
    EV << "Vyprazdnuji buffer "<< dst << endl;
    // Packets are send to different location, thus buffer can be erased
}


int64_t AggregationQueues::getAggregationQueueSize(int AQId){
    Enter_Method("getAggregationQueueSize()");
    if( AQSizeCache.find(AQId) == AQSizeCache.end() ) return 0;
    return AQSizeCache[AQId];
}

void AggregationQueues::countAggregationQueueSize(int AQId){
    // If such AQ has not been created, it returns 0
    if( AQ.find(AQId) == AQ.end() ) AQSizeCache[AQId]=0;

    // Such AQ exists thus I will count its size
    cQueue queue= AQ[ AQId ];
    int64_t size = 0;
    for( cQueue::Iterator iter(queue,0); !iter.end(); iter++){
        Payload *pl = (Payload *) iter();
        //EV << pl->getSrc() << " --> " << pl->getDst() << ": " << pl->getByteLength() << endl;
        size += pl->getByteLength();
    }

    AQSizeCache[AQId]=size;
}



void AggregationQueues::releaseAggregationQueues( std::set<int> queues, int tag ){
    Enter_Method("releaseAggregationQueues()");
    std::set<int>::iterator it;

    // Walk through all designated queues, convert them into a car and send them to TA
    for ( it=queues.begin() ; it != queues.end(); it++ ){
        EV << " "<< *it;
        // If designated queue is empty we can skip it
        if( AQ.find(*it) == AQ.end()) continue;

        // Create a car for queue and fill it by Payload packets from AQ#
        char buffer_name[50];
        sprintf(buffer_name, "AQ %d", *it);
        Car *car = new Car(buffer_name);
        while(!AQ[*it].empty()){
            Payload *pl = (Payload *) AQ[*it].pop();
            car->addByteLength(pl->getByteLength());
            car->getPayload().insert(pl);


            // Add tag to identify which pool demands this AQ, this is volatile parameter
            car->addPar("AP");
            car->par("AP").setLongValue(tag);
        }

        // Drop the queue and its scheduled initiation
        AQ.erase(*it);
        AQSizeCache.erase(*it);

        cMessage *msg= scheduled[*it];
        if ( msg != NULL and msg->isSelfMessage() and msg->isScheduled() ){
            EV << "Schdul " << msg->isScheduled() << "self " << msg->isSelfMessage() << endl;
            delete cancelEvent(msg);
        }
        scheduled.erase(*it);



        // Update AQSizeCache
        countAggregationQueueSize(*it);

        // Sends car of the queue to TA
        send(car,"out");
    }
}



void AggregationQueues::initAggregationQueuesRelease( std::set<int> queues, int tag ){
    Enter_Method("releaseAggregationQueues()");
        std::set<int>::iterator it;

        // Walk through all designated queues, convert them into a car and send them to TA
        for ( it=queues.begin() ; it != queues.end(); it++ ){
            EV << "hjgjh"<< *it << endl;
            cMessage *imsg = new cMessage();
            imsg->addPar("releaseAggregationQueue");
            imsg->addPar("queue");
            imsg->par("releaseAggregationQueue") = tag;
            imsg->par("queue")= *it;
            scheduleAt(simTime(), imsg);
        }
}
