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


void AggregationPoolManager::initialize()
{
    // Loading the parameters
    poolTreshold = par("poolTreshold").longValue();

   /**
    *  obsoletes by cTopology logic based on Routing module, currently i make Artifical assignmenents AQs to APs
    */
    EV << "prirazuji" << endl;
    for (int i = 0; i < 16; i++) {
        AP[(int) i / 8].insert(i);
    }

    // Mutual AQs
    AP[0].insert(12);
    AP[0].insert(13);
    AP[1].insert(5);
    AP[1].insert(1);

    std::set<int>::iterator it;
    for (int i = 0; i < 2; i++) {
        EV << "AP[" << i << "]: ";
        for (it = AP[i].begin(); it != AP[i].end(); it++) {
            EV << " " << *it;
        }
        EV << endl;
    }
    // END


    // Making link with AQ
    cModule *calleeModule = getParentModule()->getSubmodule("AQ");
    AQ = check_and_cast<AggregationQueues *>(calleeModule);
}

void AggregationPoolManager::handleMessage(cMessage *msg)
{
    // This module does not handle any message, everything is
    // carried out through direct method calling of its public
    // interfaces: AQNotificationInterface, initTimeBasedSending
}

int64_t AggregationPoolManager::aggregationPoolSize(int poolId){
    cModule *calleeModule = getParentModule()->getSubmodule("AQ");
    AggregationQueues *AQ = check_and_cast<AggregationQueues *>(calleeModule);

    std::set<int>::iterator it;
    int64_t size=0;

    for ( it=AP[poolId].begin() ; it != AP[poolId].end(); it++ ){
        size += AQ->getAggregationQueueSize(*it);
    }

    return size;
}

void AggregationPoolManager::aggregationQueueNotificationInterface(int AQId){
    Enter_Method("aggregationQueuesNotificationInterface()");
    int poolSize=0;

    // Test all the pools managed by this module for its size
    for (int i=0; i<AP.size(); i++){
        // If AQId is not part of a path-pool, then is the pool skipped
        if( AP[i].find(AQId) == AP[i].end() ) continue;

        // AQId is part of the pool i, so lets count current size of the buffer
        // Test whether any pool containing AQId is full enough to be scheduled
        poolSize= aggregationPoolSize(i);
        if( poolSize >= poolTreshold ){
            EV << "AP "<< i << " has reached its limit. Will be initiated." << endl;
            AQ->releaseAggregationQueues( AP[i], i );
        }
    }
}

void AggregationPoolManager::initialiseTimeBasedSending(int AQId){
    Enter_Method("initialiseTimeBasedSending()");
    EV << "AQ "<<AQId << " has reached its time to be released thus";

    int biggestPool=-1, biggestSize=0, tmpSize=0;
    for (int i=0; i<AP.size(); i++){
        // If AQId is not part of a path-pool, then is the pool skipped
        if (AP[i].find(AQId) == AP[i].end())
            continue;

        // AQId is part of the pool i, so lets count current size of the buffer
        // Test whether any pool containing AQId is biggest and then schedule.
        tmpSize= aggregationPoolSize(i);
        if( tmpSize > biggestSize){
            biggestSize= tmpSize;
            biggestPool = i;
        }
    }

    EV << "AP " << biggestPool << " is initiated" << endl;
    // Initialise sending
    if( biggestPool >=0 and biggestPool <= AP.size() ) AQ->releaseAggregationQueues( AP[biggestPool], biggestPool );
}
