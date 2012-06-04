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

    // Initiate Aggregation Pools
    cMessage *i = new cMessage("InitAPs");
    i->addPar("InitiateAggregationPools");
    scheduleAt(simTime(), i);

    bufferLengthT = par("bufferLengthT").doubleValue();
    poolTreshold = par("poolTreshold").doubleValue();

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
        initialiseTimeBasedSending(msg->par("initTBSDst").longValue());
        delete msg;
        return;
    }

    if (msg->isSelfMessage() and msg->hasPar("InitiateAggregationPools")) {
        initiateAggregationPools();
        delete msg;
        return;
    }

}

void AggregationQueues::handlePayload(cMessage *msg){
    Payload *pmsg = dynamic_cast<Payload *>(msg);

    int AQdst =  100 +  pmsg->getDst(); // TODO: Need to be fixed;
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
    aggregationQueueNotificationInterface(AQdst);
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
    double size = 0;
    for( cQueue::Iterator iter(queue,0); !iter.end(); iter++){
        Payload *pl = (Payload *) iter();
        //EV << pl->getSrc() << " --> " << pl->getDst() << ": " << pl->getByteLength() << endl;
        size += pl->getBitLength();
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
            car->setBuffered( 0 );
        }

        cMessage *msg= scheduled[*it];
        if ( msg != NULL and msg->isSelfMessage() and msg->isScheduled() ){
            delete cancelEvent(msg);
        }
        scheduled.erase(*it);

        // Do not show cQueue of leaving car
        car->getPayload().removeFromOwnershipTree();

        // Update AQSizeCache
        countAggregationQueueSize(*it);

        // Drop the queue and its scheduled initiation
        AQ.erase(*it);
        AQSizeCache.erase(*it);

        // Sends car of the queue to TA
        send(car,"out$o");
    }

    cMessage *snd = new cMessage();
    snd->addPar("allCarsHaveBeenSend");
    snd->par("allCarsHaveBeenSend").setLongValue(TSId);
    send(snd,"out$o");
}

void AggregationQueues::setAggregationQueueReleaseTime( int AQid, simtime_t release_time ){

}


void AggregationQueues::initiateAggregationPools() {
    cTopology EPtopo, topo;
    std::map< int, std::set<int> >::iterator it;
    EPtopo.extractByNedTypeName(cStringTokenizer("carobs.modules.PayloadInterface").asVector());
    topo.extractByNedTypeName(cStringTokenizer("carobs.modules.EdgeNode carobs.modules.CoreNode").asVector());
    for (int i = 0; i < EPtopo.getNumNodes(); i++) {
        cTopology::Node *endnode = EPtopo.getNode(i);

        // Skip finding pool for myself - AP is for different nodes not same ones
        if( endnode->getModule()->getParentModule() == getParentModule() ) continue;

        // Obtain Endnode address
        int ENaddress= endnode->getModule()->getParentModule()->par("address").longValue();
        // EV << "address="<<ENaddress << " " << endnode->getModule()->getParentModule()->getFullPath() << endl;

        // Calculate path from THIS -> Far Endnode and prepare path-pool[FarEndnode#]
        topo.calculateUnweightedSingleShortestPathsTo(topo.getNodeFor( endnode->getModule()->getParentModule() ));
        cTopology::Node *node = topo.getNodeFor(getParentModule());
        while (node != topo.getTargetNode()) {
            cTopology::LinkOut *path = node->getPath(0);
            node = path->getRemoteNode();
            AP[ENaddress].insert( node->getModule()->par("address").longValue() );
        }
    }

    // Print Path-pools
    std::set<int>::iterator it2;
    for( it = AP.begin(); it != AP.end(); it++ ){
        EV << "AP["<< (*it).first <<"]:";
        std::set<int> tmp= (*it).second;
        for ( it2=tmp.begin() ; it2 != tmp.end(); it2++ ){
            EV << " " << *it2;
        }
        EV << endl;
    }
}

int64_t AggregationQueues::aggregationPoolSize(int poolId) {
    std::set<int>::iterator it;
    int64_t size = 0;

    for (it = AP[poolId].begin(); it != AP[poolId].end(); it++) {
        size += getAggregationQueueSize(*it);
    }

    return size;
}


void AggregationQueues::initialiseTimeBasedSending(int AQId) {
    Enter_Method("initialiseTimeBasedSending()");
    EV << "AQ " << AQId << " has reached its time to be released thus";

    int biggestPool = -1, biggestSize = 0, tmpSize = 0;
    for (int i = 0; i < AP.size(); i++) {
        // If AQId is not part of a path-pool, then is the pool skipped
        if (AP[i].find(AQId) == AP[i].end())
            continue;

        // AQId is part of the pool i, so lets count current size of the buffer
        // Test whether any pool containing AQId is biggest and then schedule.
        tmpSize = aggregationPoolSize(i);
        if (tmpSize > biggestSize) {
            biggestSize = tmpSize;
            biggestPool = i;
        }
    }

    EV << "AP " << biggestPool << " is initiated" << endl;
    // Initialise sending
    if (biggestPool >= 0 and biggestPool <= AP.size())
        releaseAggregationQueues(AP[biggestPool], biggestPool);
}

void AggregationQueues::aggregationQueueNotificationInterface(int AQId) {
    Enter_Method("aggregationQueuesNotificationInterface()");
    int poolSize = 0;

    // Test all the pools managed by this module for its size
    for (int i = 0; i < AP.size(); i++) {
        // If AQId is not part of a path-pool, then is the pool skipped
        if (AP[i].find(AQId) == AP[i].end())
            continue;

        // AQId is part of the pool i, so lets count current size of the buffer
        // Test whether any pool containing AQId is full enough to be scheduled
        poolSize = aggregationPoolSize(i);
        if (poolSize >= poolTreshold) {
            EV << "AP " << i << " has reached its limit. Will be initiated." << endl;
            releaseAggregationQueues(AP[i], i);
        }
    }
}
