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
#include <aggregationPoolManager.h>


Define_Module(AggregationQueues);

void AggregationQueues::initialize()
{

    cModule *calleeModule = getParentModule()->getSubmodule("routing");
    R = check_and_cast<Routing *>(calleeModule);

    bufferLengthT = par("bufferLengthT").doubleValue();
    WATCH_MAP( scheduled );
    WATCH_MAP( AQSizeCache );
}

void AggregationQueues::handleMessage(cMessage *msg)
{

    if ( dynamic_cast<Payload *>(msg) != NULL){
        handlePayload(msg);
        return;
    }

    if( msg->hasPar("initTBSDst") and msg->isSelfMessage() ){
        EV << "Initialising the time based sending procedure for AQ "<< msg->par("initTBSDst").longValue() << endl;
        cModule *calleeModule = getParentModule()->getSubmodule("APm");
        AggregationPoolManager *APm = check_and_cast<AggregationPoolManager *>(calleeModule);
        APm->initialiseTimeBasedSending(msg->par("initTBSDst").longValue());
        delete msg;
    }

}

void AggregationQueues::handlePayload(cMessage *msg){
    Payload *pmsg = dynamic_cast<Payload *>(msg);

    int AQdst =  R->getTerminationNodeAddress( pmsg->getDst() );
    if( AQdst == -1 ){
        EV << "Wrong paring for dst address " << pmsg->getDst() << " !!!"<<endl;
        return;
    }

    if (AQ.find(AQdst) == AQ.end()) {
        // There is not a buffer for such destination thus must be created
        // And once it is created there is scheduled releasing process later
        char buffer_name[50];
        sprintf(buffer_name, "AQ %d", AQdst);
        AQ[AQdst] = cQueue();
        AQ[AQdst].setName(buffer_name);

        // Buffer release scheduling
        cMessage *snd = new cMessage();
        snd->addPar("initTBSDst");
        snd->par("initTBSDst").setLongValue(AQdst);
        scheduled[AQdst] = snd;
        scheduleAt(simTime() + bufferLengthT, snd);
    }

    // Add packet to its output AQueue#
    AQ[AQdst].insert(pmsg);

    // AQSizeCache Update
    this->countAggregationQueueSize(AQdst);

    // Inform TA about AQ change
    cModule *calleeModule = getParentModule()->getSubmodule("APm");
    AggregationPoolManager *TA = check_and_cast<AggregationPoolManager *>(calleeModule);
    TA->aggregationQueueNotificationInterface(AQdst);
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

    // set the current value
    AQSizeCache[AQId]=size;
}

void AggregationQueues::releaseAggregationQueues( std::set<int> queues, int tag ){
    Enter_Method("releaseAggregationQueues()");
    std::set<int>::iterator it;
    int TSId= uniform(1e3, 1e4);

    // Walk through all designated queues, convert them into a car and send them to TA
    for ( it=queues.begin() ; it != queues.end(); it++ ){
        // If designated queue is empty we can skip it
        if( AQ.find(*it) == AQ.end()) continue;
        if( AQ[*it].empty() ) continue;

        // Create a car for queue and fill it by Payload packets from AQ#
        char buffer_name[50];
        sprintf(buffer_name, "AQ %d", *it);
        Car *car = new Car(buffer_name);
        while(!AQ[*it].empty()){
            Payload *pl = (Payload *) AQ[*it].pop();
            car->addByteLength(pl->getByteLength());
            car->getPayload().insert(pl);

            // Add tag to identify which pool demands this AQ, this is volatile parameter
            car->addPar("AQ");
            car->par("AQ").setLongValue(*it);

            car->addPar("TSId");
            car->par("TSId").setLongValue(TSId);
        }

        // Drop the queue and its scheduled initiation
        AQ.erase(*it);
        AQSizeCache.erase(*it);

        cMessage *msg= scheduled[*it];
        if ( msg != NULL and msg->isSelfMessage() and msg->isScheduled() ){
            delete cancelEvent(msg);
        }
        scheduled.erase(*it);

        // Update AQSizeCache
        countAggregationQueueSize(*it);

        // Sends car of the queue to TA
        send(car,"out");
    }

    cMessage *snd = new cMessage();
    snd->addPar("allCarsHaveBeenSend");
    snd->par("allCarsHaveBeenSend").setLongValue(TSId);
    send(snd,"out");
}

void AggregationQueues::setAggregationQueueReleaseTime( int AQid, simtime_t release_time ){}
