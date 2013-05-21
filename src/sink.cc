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
    thr_window = par("thr_window").doubleValue();

    if( getParentModule()->hasPar("address"))
        address = getParentModule()->par("address").longValue();
    else{
        address = par("myMinID").longValue();
    }
}

void Sink::handleMessage(cMessage *msg)
{
    // Calculations
    Payload *pl = dynamic_cast<Payload *>(msg);
    simtime_t delay = simTime()-pl->getT0();
    int src = pl->getSrc();
    EV<< "Packet from "<<src<<" was sent at="<<pl->getT0()<<" it makes delay="<<delay<<"s ";

    // Misdelivered packets statistics - verification of proper behaviour
    int dst = pl->getDst();
    if( dst != address ){
        if( misdelivered.find(dst) == misdelivered.end( ))
            misdelivered[dst]=0;
        misdelivered[dst]++;

        EV << "MISDELIVERY from " << pl->getSrc() << " to " << pl->getDst();
        opp_terminate("misdelivery");
    }


    /**
     * Statistics
     */

    // Packet measurement - how many has been received
    if (counts.find(src) == counts.end())
        counts[src] = 0;
    counts[src]++;

    // Vectors initialisation
    if (throughput.find(src) == throughput.end()) {
        // Throughput
        throughput[src] = new cOutVector();
        std::stringstream out;
        out << "Throughput from " << src << " [bps]";
        throughput[src]->setName(out.str().c_str());
        thr_t0[src] = simTime();
        thr_bites[src]=0;

        //  E2E
        e2e[src] = new cOutVector();
        std::stringstream out1;
        out1 << "End-to-End Delay from " << src << " [s]";
        e2e[src]->setName(out1.str().c_str());
    }

    // End-to-End delay measurement
    e2e[src]->record(delay);

    // Throughput measurements
    if( simTime() - thr_t0[src] >= thr_window){
        double thr= thr_bites[src] / (simTime()-thr_t0[src] );
        EV << endl << " ** data> " << thr_bites[src] << " cas "<< simTime()-thr_t0[src] << endl;
        thr_t0[src] = simTime();
        thr_bites[src] = 0;
        throughput[src]->record( thr );
    }
    thr_bites[src] += pl->getBitLength();

    received++;
    EV  << endl;
    delete pl;
}

void Sink::finish(){
    recordScalar("Simulation duration", simTime());
    recordScalar("Packets received", received);

    /* Monitoring variables.. shown only when models behave badly */
    int64_t total=0;
    std::map<int, int64_t>::iterator it2;
    for(it2=misdelivered.begin();it2!=misdelivered.end();it2++){
            std::stringstream out;
            out << " ! Misdelivered for " << (*it2).first;
            recordScalar(out.str().c_str(), (*it2).second);
            total += (*it2).second;
    }
    if(total>0) recordScalar(" ! Total misdelivered packets", total);

    // Statistics of the number of received packets
    std::map<int,int>::iterator it3;
    for(it3=counts.begin();it3!=counts.end();it3++){
        std::stringstream out;
        out << "Packets from " << (*it3).first;
        recordScalar(out.str().c_str(), counts[(*it3).first] );
    }

}
