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

void Routing::initialize()
{

    // Am I really and poing facing any endpoint with edge port?
    if( getParentModule()->hasGate("edge",0) ){

        // Walk through all edge ports and discover destination addresses
        for( int i=0; i<getParentModule()->gateSize("edge");i++ ){
            cGate *g = getParentModule()->gate("edge$o", i);
            int min= g->getPathEndGate()->getOwnerModule()->getParentModule()->par("myMinID").longValue();
            int max= g->getPathEndGate()->getOwnerModule()->getParentModule()->par("myMaxID").longValue();
            for(int j=min;j<max;j++) terminatingIDs.insert(j);
        }

        // Activate WATCH in case of Endpoint
        if( terminatingIDs.size() != 0 )
            WATCH_SET(terminatingIDs);
    }




    topo.extractByParameter( "address" );

//    for (int i=0; i<topo.getNumNodes(); i++)
//    {
//    cTopology::Node *node = topo.getNode(i);
//    ev << "Node i=" << i << " is " << node->getModule()->getFullPath() << endl;
//    ev << " It has " << node->getNumOutLinks() << " conns to other nodes\n";
//    ev << " and " << node->getNumInLinks() << " conns from other nodes\n";
//    ev << " Connections to other modules are:\n";
//    for (int j=0; j<node->getNumOutLinks(); j++)
//    {
//    cTopology::Node *neighbour = node->getLinkOut(j)->getRemoteNode();
//    cGate *gate = node->getLinkOut(j)->getLocalGate();
//    ev << " " << neighbour->getModule()->getFullPath()
//    << " through gate " << gate->getFullName() << endl;
//    }
//    }


    d_p= par("d_p").doubleValue();
    // Obsoletes with cTopology routing decisions
    for(int i=0; i<=100;i++){
//        OT[i]= intuniform(1, par("maxrand").longValue() )*d_p ;
        OT[i]= 3*d_p;
    }
    WATCH_MAP(OT);
}

void Routing::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
}

simtime_t Routing::getOffsetTime(int destination)
{
    // Check whether destination is in the network, otherwise return -1.0
    // which means drop the burst, cause such destination doesn't exist
    if( OT.find(destination) == OT.end() )
        return (simtime_t) -1;

    return OT[destination];
}

int Routing::getOutputPort(int destination){
    return 0;
}
