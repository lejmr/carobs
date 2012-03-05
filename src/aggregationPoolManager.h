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

#ifndef __CAROBS_AGGREGATIONPOOLMANAGER_H_
#define __CAROBS_AGGREGATIONPOOLMANAGER_H_

#include <omnetpp.h>
#include <aggregationQueues.h>

class AggregationPoolManager : public cSimpleModule
{

  private:
     /**
      *  Aggregation pool definition based on std library. It aggregates
      *  AQs into pools APs such that:
      *  Pool X is defined as AP[x] = [AQ#, AQ#, ... ] where AQ#, denotes
      *  Aggregation queue for a given destination #
      */
     std::map<int, std::set<int> > AP;

     /**
      *  Function aggregationPoolSize counts size of AggregationPool.
      *  Todoso it asks AggregationQueues for size of each Queue and
      *  sums these sizes together
      *
      *  @param poolId - ID of pool, pools are counted from 0
      *  @return int64_t - return total sum of all buffer sizes
      */
     virtual int64_t aggregationPoolSize(int poolId);

     /**
      *  Size of the pool when is to be released
      */
     int64_t poolTreshold;

     /**
      *  Communication link with AggregationQueues module AQ
      */
     AggregationQueues *AQ;

     /**
      *
      */
     virtual void initiateAggregationPools();

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

  public:
      /**
       *  Function AggregationQueueNotificationInterface is used as an interface
       *  for passing informations from AggregationQueues module about a queue
       *  change such that aggregationPoolSize gets changed
       *
       *  Function does not return anything but can initiate train assembly
       *
       *  @param AQId - stands for AQ#
       */
      virtual void aggregationQueueNotificationInterface(int AQId);

      /**
       *  Function initialiseTimeBasedSending serves as an interface for AQ module
       *  to inform AQM that a queue has reached its time to be send onto the network.
       *  So the function finds the most filled AggregationPool and initialise its sending
       *
       *  @param AQId - stands for AQ# which reached its time limit
       */
      virtual void initialiseTimeBasedSending(int AQId);
};

#endif
