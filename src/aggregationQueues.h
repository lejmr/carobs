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

#ifndef __CAROBS_AGGREGATIONPOOL_H_
#define __CAROBS_AGGREGATIONPOOL_H_

#include <omnetpp.h>
#include <messages/car_m.h>
#include "messages/Payload_m.h"
#include <routing.h>



class AggregationQueues : public cSimpleModule
{
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

  private:
    /**
     *  Maximum time for a queue to be filled by incoming packets
     */
    simtime_t bufferLengthT;

    /**
     *  Extension of bufferLengthT for CoreNode where SOA manager can store
     *  information when an AQ need to be released in order to fit the empty
     *  time space in place of Car train head
     *    int - key value representing AQ id
     *    simtime_t - time when the AQ must be released
     */
    std::map<int, simtime_t> bufferlengthArray;

    /**
     *  Structure for incoming Payload packet storing, it is basically
     *  a array of arrays .. 2D array
     */
    std::map<int, cQueue> AQ;

    /**
     *  List of scheduled self-messages for time-based AQ release
     */
    std::map<int, cMessage*> scheduled;

    /**
     *  Cache for AQ size counting, it is updated by countAggregationQueueSize
     */
    std::map<int, double> AQSizeCache;


    /**
     *  Function handlePayload is for handling of new incoming payload
     *  packets, it creates aggregation queues AQs when needed and keeps
     *  an eye on time based scheduling of each AQ. When a new Payload
     *  packet comes and there are no other Payload packets in appropriate
     *  queue it schedules an event for AQ release when it is needed.
     *  Function is designed to be called by handleMessage.
     *
     *  @param msg - is cMessage of Payload packet
     */
    virtual void handlePayload(cMessage *msg);

    /**
     *  Function countAggregationQueueSize updates the cache of AQ sizes
     *  AQSizeCache to reduce computational demand with every request from
     *  AggregationPoolManager.
     */
    virtual void countAggregationQueueSize(int AQId);

    /**
     *  Pairing link for communication with Routing node
     */
    Routing *R;

  public:

    /**
     *  Function getAggregationQueueSize only return size of AQ in Bytes
     */
    virtual int64_t getAggregationQueueSize(int AQId);

    /**
     *  Function releaseAggregationQueues is remotely called by TA
     *  to demand AQ to release designated queues to be scheduled
     *  into burst-train (or car-train) by TA.
     *  Function converts all Payload packets from a queue into a Car
     *  packet which is send onto gate out.
     *
     *  @param queues - it is a std::set of queues numbers i.e. [1,2,3,..]
     *  @param tag - is used for tagging of cars to let know TA which AP
     *               are these cars assigned, the tag is removed at TA
     */
    virtual void releaseAggregationQueues( std::set<int> queues, int tag );

    /**
     *  Function setAggregationQueueReleaseTime fill bufferlengthArray structure
     *  - this function is called by SOA manager only
     *  @param AQid : Aggragation queue identifier
     *  @param release_time: time when the AQ must be released
     */
    virtual void setAggregationQueueReleaseTime( int AQid, simtime_t release_time );
};

#endif
