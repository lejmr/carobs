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

#include "fiberChannel.h"

Define_Channel(FiberChannel);

void FiberChannel::initialize()
{
    // Call the original function
    cDelayChannel::initialize();

    // Hard-coded datarte
    C = par("datarate").doubleValue();
    d_s = par("d_s").doubleValue();
    overlap=0;
    scheduling=0;
    trans=0;
}

void FiberChannel::processMessage(cMessage *msg, simtime_t t, result_t& result){


    OpticalLayer *ol = dynamic_cast<OpticalLayer *>(msg);
//    EV << "paket na WL="<< ol->getWavelengthNo();
//    EV << " simtime: " << simTime() << " - " << t ;
//    EV << " delka="<< (simtime_t) ol->getBitLength()/C;
//    EV << endl;


    int WL= ol->getWavelengthNo();
    simtime_t len= (simtime_t) ((double)ol->getBitLength()/C);

    // We take car of bursts not control packets
    if(WL > 0 ){
        if( free_time.find(WL) != free_time.end() ){
            if( t < free_time[WL] )overlap++;
        }
        free_time[WL]=t+len+d_s;


        if( stop_time.find(WL) != free_time.end() ){
            if( t < stop_time[WL] )scheduling++;
        }
        stop_time[WL]=t+len;

        trans++;
    }


    cDelayChannel::processMessage(msg,t,result);
}

void FiberChannel::finish(){
    recordScalar("Overlaping bursts", scheduling);
    recordScalar("Burst spacing violated", overlap);
    recordScalar("Bursts transmitted", trans);
}

