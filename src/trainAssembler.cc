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

#include "trainAssembler.h"
#include <aggregationQueues.h>

Define_Module(TrainAssembler);

void TrainAssembler::initialize()
{

    // Loading the parameters
    poolTreshold = par("poolTreshold").longValue();


    /**
     *  obsoletes by cTopology logic, currently it prepares assigns AQs to APs
     */
    EV << "prirazuji" << endl;
    for(int i=0; i<16; i++){
        AP[(int)i/8].insert(i);
    }

    // Mutual AQs
    AP[0].insert(12);
    AP[0].insert(13);
    AP[1].insert(5);
    AP[1].insert(1);

    std::set<int>::iterator it;
    for( int i=0; i<2;i++){
        EV << "AP["<<i<<"]: ";
        for (it=AP[i].begin(); it!=AP[i].end(); it++){
            EV << " " << *it;
        }
        EV << endl;
    }
    // END



    //WATCH_MAP(AP);
}


void TrainAssembler::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
    EV << "Zpracovvama zpravu" << endl;
    EV << "Vyplivnu jednu zpravu" << endl;
    Car *car = dynamic_cast<Car *>(msg);
}


int64_t TrainAssembler::aggregationPoolSize(int poolId){
    //EV << "aggregationPoolSize 1/3" << endl;
    cModule *calleeModule = getParentModule()->getSubmodule("AQ");
    AggregationQueues *AQ = check_and_cast<AggregationQueues *>(calleeModule);

    //EV << "aggregationPoolSize 2/3" << endl;
    std::set<int>::iterator it;
    int64_t size=0;

    ////EV << "aggregationPoolSize 3/3" << endl;
    for ( it=AP[poolId].begin() ; it != AP[poolId].end(); it++ ){
        //EV << "Zjistuji velikost "<< *it << " = ";
        size += AQ->countAggregationQueueSize(*it);
        //EV << AQ->countAggregationQueueSize(*it) << endl;
    }

    //EV << "aggregationPoolSize size="<< size << endl;
    return size;
}


void TrainAssembler::aggregationQueuesNotificationInterface(int AQId){
    Enter_Method("aggregationQueuesNotificationInterface()");
    int poolSize=0;
    // Make a link for communication with AQ for releasing AQs from a AP
    cModule *calleeModule = getParentModule()->getSubmodule("AQ");
    AggregationQueues *AQ = check_and_cast<AggregationQueues *>(calleeModule);

    // Test all the pools managed by this module for its size
    for (int i=0; i<AP.size(); i++){
        // If AQId is not part of a path-pool, then is the pool skipped
        if( AP[i].find(AQId) == AP[i].end() ) continue;

        // AQId is part of the pool i, so lets count current size of the buffer
        // Test whether any pool containing AQId is full enough to be scheduled
        poolSize= aggregationPoolSize(i);
        if( poolSize >= poolTreshold ){
            EV << "AggregationPool "<< i << "is full" << endl;
            AQ->releaseAggregationQueues( AP[i], i );
        }
    }
}


void TrainAssembler::initialiseTimeBasedSending(int AQId){
    Enter_Method("initialiseTimeBasedSending()");
    EV << "QA chce vyprazdnovat nejlepsi pool pro AQId="<<AQId << endl;

    cModule *calleeModule = getParentModule()->getSubmodule("AQ");
    AggregationQueues *AQ = check_and_cast<AggregationQueues *>(calleeModule);

    int biggestPool, biggestSize=0, tmpSize=0;
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
        EV << "AP"<<i<<" je velke "<<tmpSize << " nejvetsi je AP"<<biggestPool<<" s "<< biggestSize << endl;
    }
    EV << " Biggest AP="<< biggestPool << " size="<<biggestSize << endl;
    AQ->releaseAggregationQueues( AP[biggestPool], biggestPool );
}
