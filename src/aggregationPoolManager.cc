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

#include "aggregationPoolManager.h"
#include <aggregationQueues.h>

Define_Module(AggregationPoolManager);

void AggregationPoolManager::initialize() {
    // Loading the parameters
    poolTreshold = par("poolTreshold").longValue();

    // Initiate Aggregation Pools
    cMessage *i = new cMessage("InitAPs");
    i->addPar("InitiateAggregationPools");
    scheduleAt(simTime(), i);

    // Making link with AQ
    cModule *calleeModule = getParentModule()->getSubmodule("AQ");
    AQ = check_and_cast<AggregationQueues *>(calleeModule);
}

void AggregationPoolManager::handleMessage(cMessage *msg) {
    // This module does not handle any message, everything is
    // carried out through direct method calling of its public
    // interfaces: AQNotificationInterface, initTimeBasedSending
    if (msg->isSelfMessage() and msg->hasPar("InitiateAggregationPools")) {
        initiateAggregationPools();
        delete msg;
        return;
    }
}

void AggregationPoolManager::initiateAggregationPools() {
    cTopology EPtopo, topo;
    std::map< int, std::set<int> >::iterator it;
    EPtopo.extractByNedTypeName(cStringTokenizer("carobs.modules.Endnode").asVector());
    topo.extractByNedTypeName(cStringTokenizer("carobs.modules.Endnode carobs.modules.CoreNode").asVector());
    for (int i = 0; i < EPtopo.getNumNodes(); i++) {
        cTopology::Node *endnode = EPtopo.getNode(i);

        // Skip finding pool for myself - AP is for different nodes not same ones
        if( endnode->getModule() == getParentModule() ) continue;

        // Obtain Endnode address
        int ENaddress= endnode->getModule()->par("address").longValue();

        // Calculate path from THIS -> Far Endnode and prepare path-pool[FarEndnode#]
        topo.calculateUnweightedSingleShortestPathsTo(topo.getNodeFor( endnode->getModule() ));
        cTopology::Node *node = topo.getNodeFor(getParentModule());
        while (node != topo.getTargetNode()) {
            cTopology::LinkOut *path = node->getPath(0);
            node = path->getRemoteNode();
//            EV << node->getModule()->par("address").longValue() << " ";
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

int64_t AggregationPoolManager::aggregationPoolSize(int poolId) {
    cModule *calleeModule = getParentModule()->getSubmodule("AQ");
    AggregationQueues *AQ = check_and_cast<AggregationQueues *>(calleeModule);

    std::set<int>::iterator it;
    int64_t size = 0;

    for (it = AP[poolId].begin(); it != AP[poolId].end(); it++) {
        size += AQ->getAggregationQueueSize(*it);
    }

    return size;
}

void AggregationPoolManager::aggregationQueueNotificationInterface(int AQId) {
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
            AQ->releaseAggregationQueues(AP[i], i);
        }
    }
}

void AggregationPoolManager::initialiseTimeBasedSending(int AQId) {
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
        AQ->releaseAggregationQueues(AP[biggestPool], biggestPool);
}
