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

    // TODO: fix and make in more scalable from d_s point of view .. this constant assumes 10e-9
    C = par("datarate").doubleValue() * par("datarate_correction").doubleValue();

    d_s = par("d_s").doubleValue();
    length = par("length").doubleValue();
    overlap=0;
    scheduling=0;
    trans=0;

    // Bandwidth measurements
    thr_window = par("thr_window").doubleValue();

    cDisplayString& dispStr = getDisplayString();
    std::stringstream out;
    out << "Parameters:"<<endl;
    out<<" * length:\t" << length<<"km"<< endl;
    out<<" * delay:\t"<< par("delay").doubleValue() <<"s" << endl;
    getDisplayString().setTagArg("tt", 0, out.str().c_str());
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

        // Inter-arrival time measurements
        simtime_t inter_arrival_t= t - stop_time[WL];
        if (inter_arrival.find(WL) == inter_arrival.end()) {
            std::stringstream out_inter;
            out_inter << "Inter-arrival time of wl#" << WL << " [s]";
            inter_arrival[WL] = new cOutVector(out_inter.str().c_str());
        }
        inter_arrival[WL]->record(inter_arrival_t);

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


    /**
     * Performance analysis
     */
    if( WL  >= 0 ){
    /**
     * Bandwidth usage measurements - bit count method
     * Counting incoming bits which are averaged in a window thr_window
     */
        if( bw_start.find(WL) == bw_start.end() ){
            bw_start[WL]= simTime();
            bw_usage[WL]= 0;

            std::stringstream out;
            out << "Bandwidth of wl#" << WL<<" [bps]";
            bandwidth[WL] = new cOutVector(out.str().c_str());
        }

        // Averaging window full filed so lets make math and graphs
        if( simTime() - bw_start[WL] >= thr_window ){
            simtime_t dur= simTime() - bw_start[WL];
            bandwidth[WL]->record( bw_usage[WL]/dur );
            bw_start[WL]= simTime();
            bw_usage[WL]= 0;
        }

        // Bit count
        bw_usage[WL]+= ol->getBitLength();
    }

    if( WL  > 0 ){
    /**
     * Throughput estimation - per destination
     * Countin
     */
        int dst= ol->getEncapsulatedPacket()->par("dst").doubleValue();
        int src= ol->getEncapsulatedPacket()->par("src").doubleValue()-100;
        int label= ol->getEncapsulatedPacket()->par("AQ").doubleValue();
        int ident= src*10000+dst;

        if( thr_start.find(ident) == thr_start.end() ){
            thr_start[ident]= simTime();
            thr_usage[ident]= 0;

            std::stringstream out;
            //out << "Throughput of " << src <<"-"<< dst << " [bps]";
            out << "Throughput of " << label << " [bps]";
            throughput[ident] = new cOutVector(out.str().c_str());
        }

        // Averaging window full filed so lets make math and graphs
        if( simTime() - thr_start[ident] >= thr_window ){
            simtime_t dur= simTime() - thr_start[ident];
            throughput[ident]->record( thr_usage[ident]/dur );
            thr_start[ident]= simTime();
            thr_usage[ident]= 0;
        }

        // Bit count
        thr_usage[ident]+= ol->getBitLength();
    }

    cDelayChannel::processMessage(msg,t,result);
}

void FiberChannel::finish(){
    if(scheduling) recordScalar(" ! Overlaping bursts", scheduling);
    if(overlap) recordScalar(" ! Burst spacing violated", overlap);
    recordScalar("Bursts transmitted", trans);
}

