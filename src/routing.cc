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

    cMessage *mp = new cMessage("PortAndOtDiscovery");
    mp->addPar("PortAndOtDiscovery");
    scheduleAt(simTime(), mp);

    // Read given parameter d_p - processing time of SOAmanager
    d_p = par("d_p").doubleValue();

    // Create watchers
    WATCH_MAP(OT);
    WATCH_MAP(DestMap);
    WATCH_MAP(outPort);
    WATCH_SET(terminatingIDs);
}

void Routing::handleMessage(cMessage *msg) {

    if (msg->isSelfMessage() and msg->hasPar("PortAndOtDiscovery")) {
        // Calculate output ports and OTS
        topo.extractByNedTypeName(cStringTokenizer("carobs.modules.CoreNode carobs.modules.EdgeNode").asVector());
        for (int i = 0; i < topo.getNumNodes(); i++) {
            cTopology::Node *dstnode = topo.getNode(i);

            if (dstnode->getModule() == getParentModule()) {
                continue;
            }

            topo.calculateUnweightedSingleShortestPathsTo(dstnode);
            cTopology::Node *node = topo.getNodeFor(getParentModule());

            int ad = dstnode->getModule()->par("address").longValue();
            OT[ad] = node->getDistanceToTarget() * d_p;

            cTopology::LinkOut *path = node->getPath(0);
            outPort[ad] = path->getLocalGate()->getIndex();
        }
        delete msg;
        return;
    }
}

simtime_t Routing::getOffsetTime(int destination) {
    // Check whether destination is in the network, otherwise return -1.0
    // which means drop the burst, cause such destination doesn't exist
    if (OT.find(destination) == OT.end())
        return (simtime_t) -1;

    return OT[destination];
}

simtime_t Routing::getNetworkOffsetTime(int dst) {
    if (DestMap.find(dst) == DestMap.end())
        return  (simtime_t) -1.0;

    return getOffsetTime(100+dst);
}

int Routing::getOutputPort(int destination) {
    // Check whether destination is in the DestMap, otherwise return -1.0
    // which means drop the burst, cause such destination doesn't exist
    if (outPort.find(destination) == outPort.end())
        return  -1;

    return outPort[destination];
}

std::set<int> Routing::getTerminatingIDs() {
    return terminatingIDs;
}


int Routing::getTerminationNodeAddress(int dst){
    Enter_Method("getTerminationNodeAddress()");
    if( DestMap.find(dst)==DestMap.end() )
        DestMap[dst]= dst+100;
    return dst + 100;
}

bool Routing::canForwardHeader(int destination){
    int port= getOutputPort(destination);

    cGate *g= getParentModule()->gate("gate$o", port);
    cObject *c = g->getPathEndGate()->getOwner()->getOwner();

    topo.extractByNedTypeName( cStringTokenizer("carobs.modules.EdgeNode").asVector());
        // Calculate translation of destination network vs terminating node
    for (int i = 0; i < topo.getNumNodes(); i++) {
        cTopology::Node *node = topo.getNode(i);
        if( node->getModule() == c ) return false;
    }

    return true;
}

void Routing::sourceAddressUpdate(int min, int max){
    Enter_Method("sourceAddressUpdate");
    for (int j = min; j <= max; j++) terminatingIDs.insert(j);
}
