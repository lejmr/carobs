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

    if (throughput.find(src) == throughput.end()){
        throughput[src]= 0;
    }

    // Throughput per src
    throughput[src] += (double)pl->getBitLength()/delay;

    vects[src]->record(delay);
    avg_delay.record(delay);
    received++;
    total_delay += delay;

    EV<< "Packet from "<<src<<" was sent at="<<pl->getT0()<<" it makes delay="<<delay<<"s ";
    // Scalar E2E measturemtn
    if(avg_e2e==0) avg_e2e= delay;
    avg_e2e= (avg_e2e+delay)/2;

    // Misdelivered packets statistics
    int dst = pl->getDst();
    if( dst != address ){
        if( misdelivered.find(dst) == misdelivered.end( ))
            misdelivered[dst]=0;
        misdelivered[dst]++;

        EV << "MISDELIVERY from " << pl->getSrc() << " to " << pl->getDst();
        opp_terminate("misdelivery");
    }

    EV  << endl;
    delete pl;
}

void Sink::finish(){
    recordScalar("Simulation duration", simTime());
    recordScalar("Packets received", received);

    if( received != 0 ) recordScalar("End-to-End delay (total)", total_delay/received);
    else recordScalar("Average delay", 0 );

    int received=0;
    std::map<int,int>::iterator it;
    for(it=counts.begin();it!=counts.end();it++){
        std::stringstream out;
        out << "Received from " << (*it).first;
        //recordScalar(out.str().c_str(), (*it).second);
        received += (*it).second;
    }
    recordScalar("Avererage End-to-End delay", avg_e2e);

    /* Monitoring variables.. shown only when models behave bad */
    int64_t total=0;
    std::map<int, int64_t>::iterator it2;
    for(it2=misdelivered.begin();it2!=misdelivered.end();it2++){
            std::stringstream out;
            out << " ! Misdelivered for " << (*it2).first;
            recordScalar(out.str().c_str(), (*it2).second);
            total += (*it2).second;
    }
    if(total>0) recordScalar(" ! Total misdelivered packets", total);

    long double thrpt_all= 0;
    long double thrpt_pp = 0;
    int srcs = 0;
    std::map<int, long double>::iterator it3;
    for(it3=throughput.begin();it3!=throughput.end();it3++){
        std::stringstream out;
        out << "Average throughput from " << (*it3).first;
        double thrpt= (*it3).second/(double)counts[(*it3).first];
        recordScalar(out.str().c_str(), thrpt );
        thrpt_all += (*it3).second;
        thrpt_pp += thrpt;
        srcs++;
    }
    recordScalar("Avererage throughput", thrpt_all/(double)received);
    recordScalar("Avererage throughput pp", thrpt_pp/(double)srcs);

}
