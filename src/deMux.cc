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

#include "deMux.h"

Define_Module(DeMux);

void DeMux::initialize()
{
    mux = par("mux").boolValue();
}

void DeMux::handleMessage(cMessage *msg)
{
    // Convert incoming message into OpticalLayer format
    OpticalLayer *ol=  dynamic_cast<OpticalLayer *>(msg);

    // Do we behave as MUX - All in All out
    if( mux ){ send(msg,"out");return;}

    // We behave as DEMUX
    demuxing(ol);
}

void DeMux::demuxing(OpticalLayer *msg){
    if( msg->getWavelengthNo() == 0 ){
        // This is controlling packet .. CARBOS Header it will be directed to SOA Manager
        send(msg,"control");
    }else{
        // It is a Car so it is going to be send to SOA for switching
        send(msg,"soa");
    }
}
