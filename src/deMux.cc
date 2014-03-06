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
    att= par("att").doubleValue();

    // QoT
    OSNR_drop_level= par("OSNR_drop").doubleValue();
    osnr_drop = 0;

    // Amplifier parameters
    amp_enabled = par("amp").boolValue();
    B_0 = par("B_0").doubleValue();
    G_0 = par("G_0").doubleValue();
    P_sat = par("P_sat").doubleValue();
    A1 = par("A1").doubleValue();
    A2 = par("A2").doubleValue();
    c_0 = par("c_0").doubleValue();
    NF = par("NF").doubleValue();
    f_0 = 3e8 / c_0; // TODO: adjust for propagation in glass

    // Visualisation of enabled amplification
    if(amp_enabled){
        cDisplayString& dispStr = getDisplayString();
        getDisplayString().setTagArg("i2", 0, "status/lightning");
    }
}

void DeMux::handleMessage(cMessage *msg)
{
    // Convert incoming message into OpticalLayer format
    OpticalLayer *ol=  dynamic_cast<OpticalLayer *>(msg);

    // Do we behave as MUX - All in All out
    if( mux ){
        // SOA -- MUX -- Booster -- fiber ------->
        // Decrease power level of optical signal at MUX
        ol->setP(ol->getP() - att);
        ol->setN(ol->getN() - att);

        // Booster
        if(amp_enabled){
            EV << "OSNR MUX Input: "<< ol->getP()-ol->getN();
            amplify(ol);
            EV << "dB Output: "<< ol->getP()-ol->getN() <<"dB"<< endl;
        }

        // Send OpticalSignal onto optical fiber
        send(msg,"out");
        return;
    }

    // We behave as DEMUX
    demuxing(ol);
}

void DeMux::demuxing(OpticalLayer *msg){
    // fiber ----> Pre-amp -- DEMUX -- SOA/SOAm
    // Pre-amplifier
    if (amp_enabled) {
        EV << "OSNR DEMUX Input: " << msg->getP() - msg->getN();
        amplify(msg);
        EV << "dB Output: " << msg->getP() - msg->getN() << "dB" << endl;
    }

    // Decrease power level of optical signal at MUX
    msg->setP(msg->getP() - att);
    msg->setN(msg->getN() - att);

    // WDM demultiplex to SOAm or SOA
    // Verify OSNR of optical signal
    if (msg->getP() - msg->getN() >= OSNR_drop_level) {

        if (msg->getWavelengthNo() == 0)
            // This is controlling packet .. CARBOS Header it will be directed to SOA Manager
            send(msg, "control");
        else
            // It is a Car so it is going to be send to SOA for switching
            send(msg, "soa");

    } else {
        EV << "Optical signal got lost in noise having OSNR=" << msg->getP() - msg->getN() << "dB" << endl;
        osnr_drop++;
        delete msg;
    }
}

void DeMux::amplify(OpticalLayer *msg){
    // Input OSNR
    double input_osnr = pow(10,msg->getP()/10) / pow(10,msg->getN()/10);

    // Gain saturation and noise figure evaluation
    long double h = 6.626068e-34;  // Planck constant
    double f= f_0+msg->getWavelengthNo()*B_0;
    long double P_in_mw = pow(10, msg->getP()/10);
    long double P_sat_mw = pow(10, P_sat / 10);
    long double G_amp = pow(10, G_0 / 10) / (1 + P_in_mw / P_sat_mw);
    long double F_amp = pow(10, NF / 10) * (1 + A1 - A1 / (1 + P_in_mw / A2));

    // ASE noise added by this amplifier
    long double osnr_mx = P_in_mw / (h*f*F_amp*B_0);

    // Cumulative OSNR
    long double total_osnr= 1/input_osnr + 1/osnr_mx;
    total_osnr = 1/total_osnr;

    // P / N levels update of msg
    msg->setP( msg->getP() + 10*log10(G_amp) );
    msg->setN( msg->getP() - 10*log10(total_osnr) );
}

void DeMux::finish(){
    recordScalar("QoT bursts dropped", osnr_drop);
}
