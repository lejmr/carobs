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

#ifndef __CAROBS_ROUTING_H_
#define __CAROBS_ROUTING_H_

#include <omnetpp.h>
#include <CplexRouteEntry.h>
#include <iostream>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

/**
 *  Module which takes care of routing in network
 */
class Routing : public cSimpleModule
{
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

  public:
    /**
     * Function calculates minumum offset time needed for OBS system to work.
     * It sums all delays introduced by fibers, CoreNodes,.. along a flow
     * @param destination - refers to address of destination node
     * @return offset time ot^c - which stands for processing time of all intermediate nodes ot^c=l^c*d^H [8.2.3 - Coutelen DT]
     */
    virtual simtime_t getOffsetTime(int destination);

    /**
     *
     */
    virtual simtime_t getNetworkOffsetTime(int dst);

    /**
     * Function getOutputPort resolves best path for a given CAROBS train
     *
     * @param destination - refers to address of destination node
     * @return number of output port
     */
    virtual int getOutputPort(int destination);

    /**
     *  Function sourceAddressUpdate is usd by PayloadInterface to inform Routing
     *  about list of address which this CoreNode terminates
     */
    virtual void sourceAddressUpdate(int min, int max);

    /**
     *  Function getTerminatingIDs return set of IDs which this node terminates
     */
    virtual std::set<int> getTerminatingIDs();


    /**
     *
     */
    virtual int getTerminationNodeAddress(int dst);

    /**
     *
     */
    virtual bool canForwardHeader(int destination);

    /**
     *
     */
    virtual std::map<std::string, std::string> availableRoutingPaths() { Enter_Method("availableRoutingPaths"); return paths; };


  private:
    cQueue RoutingTable;
    std::map<int, int> inPort, outPort;

    /**
     *  DestMap maps dst network address and its last node address .. DestMap[dst] = #Endnode
     */
    std::map<int, int> DestMap;


    std::map<int, cTopology::Node *> NodeList;

    /**
     *  Network description variable
     */
    cTopology topo;

    /**
     *  Processing time of SOA manager - it assumes d_p same for all CoreNodes
     */
    simtime_t d_p, d_s;

    /**
     *  Set of IDs which are terminated by this Node
     */
    std::set<int> terminatingIDs;

    /**
     * Container to store information about paths in network
     */
    std::map<std::string, std::string> paths;

};

#endif
