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
    avg_e2e=0;

    if( getParentModule()->hasPar("address"))
        address = getParentModule()->par("address").longValue();
    else{
        address = par("myMinID").longValue();
    }
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

    // Scalar E2E measturemtn
    if(avg_e2e==0) avg_e2e= delay;
    avg_e2e= (avg_e2e+delay)/2;

    // Misdelivered packets statistics
    int dst = pl->getDst();
    if( dst != address ){
        if( misdelivered.find(dst) == misdelivered.end( ))
            misdelivered[dst]=0;
        misdelivered[dst]++;

        EV << "MISDELIVERY from " << pl->getSrc() << " to " << pl->getDst() << endl;
    }

    delete msg;
}

void Sink::finish(){
    recordScalar("Simulation duration", simTime());

    recordScalar("Received packets", received);
    if( received != 0 ) recordScalar("Average delay", total_delay/received);
    else recordScalar("Average delay", 0 );

    int received=0;
    std::map<int,int>::iterator it;
    for(it=counts.begin();it!=counts.end();it++){
        std::stringstream out;
        out << "Received from " << (*it).first;
        //recordScalar(out.str().c_str(), (*it).second);
        received += (*it).second;
    }
    recordScalar("Received", received);
    recordScalar("Avererage End-to-End delay", avg_e2e);

    int64_t total=0;
    std::map<int, int64_t>::iterator it2;
    for(it2=misdelivered.begin();it2!=misdelivered.end();it2++){
            std::stringstream out;
            out << "Misdelivered for " << (*it2).first;
            recordScalar(out.str().c_str(), (*it2).second);
            total += (*it2).second;
    }
    recordScalar("Total misdelivered packets", total);


}
