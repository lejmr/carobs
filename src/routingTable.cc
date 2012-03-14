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

#include "routingTable.h"

Define_Module(RoutingTable);

void RoutingTable::initialize()
{

    min= par("myMinID").longValue();
    max= par("myMaxID").longValue();

    // Full local IDs cache
    for(int i=min; i<=max; i++ )
        localIDs.insert(i);

    WATCH_SET(localIDs);
    WATCH_SET(remoteIDs);


    cTopology topo;
    topo.extractByNedTypeName( cStringTokenizer("carobs.modules.EdgeNode carobs.modules.Endpoint").asVector() );
    for (int i = 0; i < topo.getNumNodes(); i++) {
        cTopology::Node *node = topo.getNode(i);
        cModule *mod= node->getModule();
        if( !strcmp( mod->getNedTypeName(), "carobs.modules.Endpoint") ){
            cModule *calleeModule = mod->getSubmodule("routingTable");
            RoutingTable *RT = check_and_cast<RoutingTable *>(calleeModule);

            EV << mod->getFullPath() << endl;
            std::set<int>::iterator it;
            std::set<int> myset = RT->getLocalIDs();
            for (it = myset.begin(); it != myset.end(); it++) {
                if( *it < min or *it >max ) remoteIDs.insert(*it);
            }

            RT->giveMyLocalIds(localIDs);
        }
    }
}

std::set<int> RoutingTable::getLocalIDs(){
    return localIDs;
}

void RoutingTable::giveMyLocalIds(std::set<int> localIDs){
    std::set<int>::iterator it;
    for (it = localIDs.begin(); it != localIDs.end(); it++) {
        if (*it < min or *it > max)
            remoteIDs.insert(*it);
    }
}

int RoutingTable::getNthRemoteAddress( int n){
    std::set<int>::iterator it= remoteIDs.begin();
    //for(int i=0;i<n;i++)it++;
    std::advance( it, n );
    return *it;
}

int RoutingTable::dimensionOfRemoteAddressSet(){
    return remoteIDs.size();
}
