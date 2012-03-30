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

#include "sink.h"

Define_Module(Sink);

void Sink::initialize()
{
    received=0;
    total_delay = 0;
    avg_delay.setName("End-to-End delay");
}

void Sink::handleMessage(cMessage *msg)
{
    Payload *pl = dynamic_cast<Payload *>(msg);
    simtime_t delay = simTime()-pl->getT0();
    int src = pl->getSrc();

    if (counts.find(src) == counts.end())
        counts[src] = 0;
    counts[src]++;

    if (vects.find(src) == vects.end()){
        vects[src]= new cOutVector();
        std::stringstream out;
        out << "End-to-End Delay from " << src;
        vects[src]->setName( out.str().c_str() );
    }

    vects[src]->record(delay);
    avg_delay.record(delay);
    received++;
    total_delay += delay;

    delete msg;
}

void Sink::finish(){
    recordScalar("Simulation duration", simTime());

    recordScalar("Received packets", received);
    if( received != 0 ) recordScalar("Average delay", total_delay/received);
    else recordScalar("Average delay", 0 );

    std::map<int,int>::iterator it;
    for(it=counts.begin();it!=counts.end();it++){
        std::stringstream out;
        out << "Received from " << (*it).first;
        recordScalar(out.str().c_str(), (*it).second);
    }

}
