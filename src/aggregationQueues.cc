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

    // Connect with routing module
    cModule *calleeModule = getParentModule()->getSubmodule("routing");
    R = check_and_cast<Routing *>(calleeModule);

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

    int AQdst =  pmsg->getDst(); // TODO: Need to be fixed;
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

void AggregationQueues::releaseAggregationQueues( std::set<int> queues, std::string tag ){
    Enter_Method("releaseAggregationQueues()");
    std::set<int>::iterator it;
    int TSId= uniform(1e3, 1e4);

    EV << "Releasing AQ: ";

    // Walk through all designated queues, convert them into a car and send them to TA
    for ( it=queues.begin() ; it != queues.end(); it++ ){

        EV << *it << " ";
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
            car->setBypass( 0 );
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
    EV << endl;

    cMessage *snd = new cMessage();
    snd->addPar("allCarsHaveBeenSend");
    snd->par("allCarsHaveBeenSend").setLongValue(TSId);
    send(snd,"out$o");
}

void AggregationQueues::setAggregationQueueReleaseTime( int AQid, simtime_t release_time ){

}


void AggregationQueues::initiateAggregationPools() {

    EV << "** Getting information about paths in network from Routing module" << endl;
    std::map<std::string, std::string> opaths= R->availableRoutingPaths();
    std::map<std::string, std::vector<int> > paths;

    for (std::map<std::string, std::string>::iterator iter = opaths.begin(); iter != opaths.end(); iter++) {
        paths[ iter->first ] = cStringTokenizer( iter->second.c_str() , " ").asIntVector();
    }

    // Match paths that constitute the Aggreagion pool .. streamlined
    for (std::map<std::string, std::vector<int> >::iterator oiter = paths.begin(); oiter != paths.end(); oiter++) {
        std::vector<int> ost= cStringTokenizer( oiter->first.c_str(), "-").asIntVector();
        //EV << oiter->first << ": ";

        for (std::map<std::string, std::vector<int> >::iterator iter = paths.begin(); iter != paths.end(); iter++) {
            std::vector<int> ist= cStringTokenizer( iter->first.c_str(), "-").asIntVector();

            // Match paths only on the same wavelength
            if( ost[0] != ist[0] ) continue;

            // Do not compare the same paths
            if( oiter->first == iter->first ) continue;

            // iter can not be longer ten oiter
            if( paths[oiter->first].size() <= paths[iter->first].size() ) continue;

            bool match=true;
            for(int i=0;i<paths[iter->first].size();i++){
                if( paths[oiter->first][i] != paths[iter->first][i] ) { match=false; }
            }

            if( match) {
                //EV << iter->first << " ";
                if( AP_ON ) AP[ oiter->first ].insert( ist[1] );
            }

        }
        //EV << endl;

        AP[ oiter->first ].insert( ost[1] );

    }



/*
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
*/
    // Print Path-pools

    for( std::map<std::string, std::set<int> >::iterator it = AP.begin(); it != AP.end(); it++ ){
        EV << "AP["<< (*it).first <<"]:";
        std::set<int> tmp= (*it).second;
        for ( std::set<int>::iterator it2=tmp.begin() ; it2 != tmp.end(); it2++ ){
            EV << " " << *it2;
        }
        EV << endl;
    }

}

int64_t AggregationQueues::aggregationPoolSize(std::string poolId) {
    std::set<int>::iterator it;
    int64_t size = 0;

    for (it = AP[poolId].begin(); it != AP[poolId].end(); it++) {
        size += getAggregationQueueSize(*it);
    }

    return size;
}


void AggregationQueues::initialiseTimeBasedSending(int AQId) {
    Enter_Method("initialiseTimeBasedSending()");
    EV << "AQ " << AQId << " has reached its time to be released thus ";

    std::string biggestPool = "";
    int biggestSize = 0, tmpSize = 0;
    for (std::map<std::string, std::set<int> >::iterator i = AP.begin(); i != AP.end(); i++) {
        // Pick only the AP where AQ constructs highest load for the outgoing wavelength
        if( i->second.find(AQId) == i->second.end() ) continue;

        // AQId is part of the pool i, so lets count current size of the buffer
        // Test whether any pool containing AQId is biggest and then schedule.
        tmpSize = aggregationPoolSize(i->first);
        if (tmpSize > biggestSize) {
            biggestSize = tmpSize;
            biggestPool = i->first;
        }
    }
    EV << "AP " << biggestPool.c_str() << " is initiated" << endl;

    // Initialise sending
    if (biggestPool != "" and AP.find(biggestPool) != AP.end() )
        releaseAggregationQueues(AP[biggestPool], biggestPool);

}

void AggregationQueues::aggregationQueueNotificationInterface(int AQId) {
    Enter_Method("aggregationQueuesNotificationInterface()");
    int poolSize = 0;

    // Test all the pools managed by this module for its size
    for (std::map<std::string, std::set<int> >::iterator i = AP.begin(); i != AP.end(); i++) {
        // If AQId is not part of a path-pool, then is the pool skipped
        if (AP[i->first].find(AQId) == AP[i->first].end())
            continue;

        // AQId is part of the pool i, so lets count current size of the buffer
        // Test whether any pool containing AQId is full enough to be scheduled
        poolSize = aggregationPoolSize(i->first);
        if (poolSize >= poolTreshold) {
            EV << "AP " << i->first << " has reached its limit. Will be initiated." << endl;
            releaseAggregationQueues(AP[i->first], i->first);
        }
    }
}
