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

#include "routing.h"

Define_Module(Routing);

void Routing::initialize() {
    /**
     *  LAN network discovery - I am finding addresses which this Node is terminating one
     *   - So I check for addresses of each Endpoint facing with Edge port
     */
    // Am I really and poing facing any endpoint with edge port?
    if (getParentModule()->hasGate("edge", 0)) {

        // Walk through all edge ports and discover destination addresses
        for (int i = 0; i < getParentModule()->gateSize("edge"); i++) {
            cGate *g = getParentModule()->gate("edge$o", i);
            int min =
                    g->getPathEndGate()->getOwnerModule()->getParentModule()->par(
                            "myMinID").longValue();
            int max =
                    g->getPathEndGate()->getOwnerModule()->getParentModule()->par(
                            "myMaxID").longValue();
            for (int j = min; j < max; j++)
                terminatingIDs.insert(j);
        }

        // Activate WATCH in case of Endpoint
        if (terminatingIDs.size() != 0)
            WATCH_SET(terminatingIDs);
    }

    /**
     *  Network translation discovery - I need a table which adds together destination address
     *  and terminating node address.
     *   - So I check all nodes having a Routing module to obtain terminatingIDs (previous step)
     *   - The exchange is done with selfMessage because all terminatingIDs are full filed after
     *     all Routing initialisation is done
     */
    cMessage *mp = new cMessage("NTD");
    mp->addPar("NetworkTranslationDiscovery");
    scheduleAt(simTime(), mp);

    /*
     cMessage *mp2 = new cMessage("OTs");
     mp2->addPar("CalculateNetworkOT");
     scheduleAt(simTime(), mp2);
     */

    d_p = par("d_p").doubleValue();
    // Obsoletes with cTopology routing decisions

    WATCH_MAP(OT);
    WATCH_MAP(DestMap);
    WATCH_MAP(outPort);
}

void Routing::handleMessage(cMessage *msg) {
    // Initiate Network translation discovery process
    if (msg->isSelfMessage() and msg->hasPar("NetworkTranslationDiscovery")) {
        doNetworkTranslationDiscovery();
        delete msg;
        return;
    }

    if (msg->isSelfMessage() and msg->hasPar("CalculateNetworkOT")) {
        calculateNetworkOT();
        delete msg;
        return;
    }

}

void Routing::calculateNetworkOT() {
    topo.extractByNedTypeName( cStringTokenizer("carobs.modules.CoreNode carobs.modules.Endnode").asVector());
    std::map<int, cTopology::Node *>::iterator it;
    for (it = NodeList.begin(); it != NodeList.end(); it++)
        EV << (*it).first << " => " << (*it).second << endl;
    topo.calculateUnweightedSingleShortestPathsTo((*it).second);

    cTopology::Node *node = topo.getNodeFor(getParentModule());

    EV << "numpaths= " << node->getNumPaths() << endl;

}

simtime_t Routing::getOffsetTime(int destination) {
    // Check whether destination is in the network, otherwise return -1.0
    // which means drop the burst, cause such destination doesn't exist
    if (NodeList.find(destination) == NodeList.end())
        return (simtime_t) -1;

    return OT[destination];
}

int Routing::getOutputPort(int destination) {
    // Check whether destination is in the DestMap, otherwise return -1.0
    // which means drop the burst, cause such destination doesn't exist
    if (outPort.find(destination) == outPort.end())
        return  -1;

    return outPort[destination];
}

simtime_t Routing::getNetworkOffsetTime(int dst) {
    if (DestMap.find(dst) == DestMap.end())
        return  (simtime_t) -1.0;

    return getOffsetTime(DestMap[dst]);
}

void Routing::doNetworkTranslationDiscovery() {
    topo.extractByNedTypeName( cStringTokenizer("carobs.modules.CoreNode carobs.modules.Endnode").asVector());
    for (int i = 0; i < topo.getNumNodes(); i++) {
        cTopology::Node *node = topo.getNode(i);

        // Skip this module to avoid mixing local and remote terminatingID
        if (node->getModule() == getParentModule()) {
            continue;
        }

        // Make a link with remote Routing module
        cModule *calleeModule = node->getModule()->getSubmodule("routing");

        // If is there a problem, just skip the module
        if (calleeModule == NULL)
            continue;

        // Prepare calle link and then we can ask for remote terminatingIDs
        Routing *tr = check_and_cast<Routing *>(calleeModule);

        // Get address of remote remote node to make REMOTEID <--> TerminatingID translation
        int address = node->getModule()->par("address").longValue();
        NodeList[address] = node;
        OT[address] = -1.0;

        // Make the pairing
        std::set<int>::iterator it;
        std::set<int> tmp = tr->getTerminatingIDs();
        for (it = tmp.begin(); it != tmp.end(); it++)
            DestMap[*it] = address;
    }

    EV << " ----------------------" << endl;
    for (int i = 0; i < topo.getNumNodes(); i++) {
        cTopology::Node *dstnode = topo.getNode(i);

        if (dstnode->getModule() == getParentModule()) {
            continue;
        }
        EV << "Node " << i << ": " << dstnode->getModule()->getFullPath() << " ";

        topo.calculateUnweightedSingleShortestPathsTo( dstnode );

        cTopology::Node *node = topo.getNodeFor(getParentModule());
        //EV << node->getModule()->getFullPath() << " - " << dstnode->getModule()->getFullPath() << " = " << node->getDistanceToTarget() << endl;

        EV << dstnode->getModule()->par("address").longValue() << " ("<<dstnode->getModule()->getFullPath()<<") = "<< node->getDistanceToTarget() ;

        int ad= dstnode->getModule()->par("address").longValue();
        OT[ad] = node->getDistanceToTarget()*d_p;

        cTopology::LinkOut *path = node->getPath(0);
        EV << " gate="<<path->getLocalGate()->getFullName();
        EV << " " << path->getLocalGate()->getIndex() << endl;

        outPort[ad]= path->getLocalGate()->getIndex();


    }

}

std::set<int> Routing::getTerminatingIDs() {
    return terminatingIDs;
}


int Routing::getTerminationNodeAddress(int dst){
    Enter_Method("getTerminationNodeAddress()");
    if( DestMap.find(dst) == DestMap.end() )
        return -1;

    return DestMap[dst];
}
