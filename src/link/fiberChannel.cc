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

    att = par("att").doubleValue();
    att_total = att * length;

    // Amplifier parameters
    B_0 = par("B_0").doubleValue();
    G_0 = par("G_0").doubleValue();
    P_sat = par("P_sat").doubleValue();
    A1 = par("A1").doubleValue();
    A2 = par("A2").doubleValue();
    c_0 = par("c_0").doubleValue();
    NF = par("NF").doubleValue();
    f_0 = 3e8 / c_0; // TODO: adjust for propagation in glass

    EV << "f0=" << f_0 << "Hz" << endl;

    // QoT
    OSNR_qot = par("OSNR_qot").doubleValue();
    OSNR_drop = par("OSNR_drop").doubleValue();

    // Obtain the number of optical amplifiers
    amp_len = G_0 / att; // Length supported by one Amplifier
    G_N = floor(length / amp_len);
    EV << "A number of reg needed for " << length << "km :" << G_N << endl;

    // Model correctness verifications
    osnr_drop = 0;

    // Bandwidth measurements
    thr_window = par("thr_window").doubleValue();

    // Link label
    cDisplayString& dispStr = getDisplayString();
    std::stringstream out;
    out << "Parameters:" << endl;
    out << " * length:\t" << length << "km" << endl;
    out << " * attenuation:\t" << att_total << "dB" << endl;
    out << " * delay:\t" << par("delay").doubleValue() << "s" << endl;
    out << " * amplifiers:\t" << G_N << "@" << G_0 << "dB" << endl;
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
    double f= f_0+WL*B_0;
    EV << "f="<<f<< "Hz"<<endl;

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
        int dst= ol->getEncapsulatedPacket()->par("dst").doubleValue()-100;
        int src= ol->getEncapsulatedPacket()->par("src").doubleValue()-100;
        int ident= src*10000+dst;

        if( thr_start.find(ident) == thr_start.end() ){
            thr_start[ident]= simTime();
            thr_usage[ident]= 0;

            std::stringstream out;
            out << "Throughput of " << src <<"-"<< dst << " [bps]";
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


    EV << "** Fiber start P=" << ol->getP() << " N=" << ol->getN() << " OSNR=" << ol->getP() - ol->getN() << endl;
    /**
     *  Apply all the impairments onto optical signal
     */
    long double h = 6.626068e-34; // Planck constant

    // Attenuation of optical signal
    // A ------ AMP1 ------- AMP2 ------ B
    // Length of one span in case of uniformly distributed AMPs along the path
    double len_tbused = length;

    std::vector<long double> OSNR;
    std::vector<long double>::iterator it;

    double input_osnr = pow(10, ol->getP() / 10) / pow(10, ol->getN() / 10);
    OSNR.push_back(input_osnr);

    // Power and Noise adjustments
    for (int i = 0; i < G_N; i++) {

        // Fiber attenuation effect
        ol->setP(ol->getP() - amp_len * att);
        ol->setN(ol->getN() - amp_len * att);
        len_tbused -= amp_len;

        // Power of optical signal at egress of AMP
        EV << " AMP(" << i + 1 << ") IN: P=" << ol->getP() << "dB N="
                  << ol->getN() << "dB OSNR=" << ol->getP() - ol->getN()
                  << "dB";

        long double P_in_mw = pow(10, ol->getP() / 10);
        long double P_sat_mw = pow(10, P_sat / 10);
        long double G_amp = pow(10, G_0 / 10) / (1 + P_in_mw / P_sat_mw);
        long double F_amp = pow(10, NF / 10)
                * (1 + A1 - A1 / (1 + P_in_mw / A2));

        long double osnr_stage = P_in_mw / (h * f * F_amp * B_0);
        long double total_osnr = 1 / osnr_stage;
        for (it = OSNR.begin(); it < OSNR.end(); it++) {
            total_osnr += 1 / (*it);
        }
        total_osnr = 1 / total_osnr;
        OSNR.push_back(total_osnr);

        // Amplifier addition to P and N of optical signal
        ol->setP(ol->getP() + 10 * log10(G_amp)); // P_out
        ol->setN(ol->getP() - 10 * log10(total_osnr));

        //  Output texts
        EV << " OUT: P=" << ol->getP() << "dB N=" << ol->getN() << "dB OSNR="
                  << ol->getP() - ol->getN() << "dB";
        EV << " @dist=" << length - len_tbused;
        EV << " [G_amp=" << G_amp << " (" << 10 * log10(G_amp) << "dB)";
        EV << " F_amp=" << F_amp << " (" << 10 * log10(F_amp) << "dB)]" << endl;
    }

    if (len_tbused > 0) {
        // Trail path
        ol->setP(ol->getP() - len_tbused * att);
        ol->setN(ol->getN() - len_tbused * att);
    }

    EV << "** Fiber end P=" << ol->getP() << " N=" << ol->getN() << " OSNR="
            << ol->getP() - ol->getN() << endl;
    if (ol->getP() - ol->getN() < OSNR_drop and ol->getWavelengthNo() > 0) {
        EV << "Optical signal got lost in noise with OSNR="
                << ol->getP() - ol->getN() << "dB" << endl;
        result.discard = true;
        osnr_drop++;
    } else {
        // Apply delay onto optical signal
        cDelayChannel::processMessage(msg, t, result);
    }


    // Inherit traditional DelayChannel behavior
    cDelayChannel::processMessage(msg,t,result);
}

// Conversion functions
double FiberChannel::dBm2W(double dBm){ return pow(10, dBm/10)/1000;}
double FiberChannel::dBm2mW(double dBm){ return pow(10, dBm/10); }
double FiberChannel::dBW2W(double dBW){ return pow(10, dBW/10); }
double FiberChannel::W2dBW(double W){   return 10*log10(W); }
double FiberChannel::W2dBm(double W){   return 10*log10(1000*W); }


void FiberChannel::finish(){
    if(scheduling) recordScalar(" ! Overlaping bursts", scheduling);
    if(overlap) recordScalar(" ! Burst spacing violated", overlap);
    recordScalar("Bursts transmitted", trans);
    recordScalar("QoT bursts dropped", osnr_drop);
}

