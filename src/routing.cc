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
    // Obsoletes with cTopology routing decisions
    for(int i=0; i<=100;i++){
        OT[i]=(simtime_t) intuniform(1, 10);
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
