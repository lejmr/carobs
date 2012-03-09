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

#include "MAC.h"

Define_Module(MAC);

void MAC::initialize()
{
    // reading parameters
    maxWL = par("maxWL").longValue();

    // Making link for communication with Routing module
    cModule *calleeModule = getParentModule()->getSubmodule("routing");
    R = check_and_cast<Routing *>(calleeModule);
}

void MAC::handleMessage(cMessage *msg)
{
    // MACContainer is incoming message
    if ( dynamic_cast<MACContainer *>(msg) != NULL){

        // Cast general incoming message cMessage into MACContainer
        MACContainer *MAC= dynamic_cast<MACContainer *>(msg);

        // Dencapsulate CARBOSHeader
        CAROBSHeader *H= dynamic_cast<CAROBSHeader *>(MAC->decapsulate());

        // Sending parameters
        int dst= H->getDst();
        int out= R->getOutputPort( dst );
        int wl = getOutputWavelength( out );
        simtime_t t0 = timeEgressIsReady(dst,out);

        // Set Wavelength to CARBOS Header once we know it
        H->setWL( wl );

        EV << "CAROBS Train to "<< dst << " will be send at " << t0;
        // Schedule CAROBS Header to send onto network desc. by port&wl
        // Such a CAROBS Header must be encapsulated into OpticalLayer 1st
        char buffer_name[50]; sprintf(buffer_name, "Header %d", dst);
        OpticalLayer *OH = new OpticalLayer(buffer_name);
        OH->setWavelengthNo(0);     //Signalisation is always on the first channel
        OH->encapsulate(H);
        sendDelayed(OH, t0, "out", out);

        EV << " OT= " << H->getOT() << endl;
        // Schedule associate cars
        while(!MAC->getCars().empty()){
            SchedulerUnit *su = (SchedulerUnit *)MAC->getCars().pop();
            Car *tcar= (Car *) su->decapsulate();

            // Encapsulate Car into OpticalLayer
            char buffer_name[50]; sprintf(buffer_name, "Car %d", su->getDst() );
            OpticalLayer *OC = new OpticalLayer(buffer_name);
            OC->setWavelengthNo( wl );
            OC->encapsulate(tcar);

            OC->setSchedulingPriority(10);
            EV<<" + Sending car to "<<su->getDst() <<" at "<< t0 + su->getStart()<<" of length="<<su->getLength()<<endl;
            // Send the car onto proper wl at proper time
            sendDelayed(OC, t0 + su->getStart(), "out", out);
            //EV << "car "<< su->getDst() <<" at"<< t0+su->getStart() << " length: " << su->getLength() << endl;
        }

    }
}

int MAC::getOutputWavelength(int port){
    // random approach
    return uniform(1,maxWL);
}

simtime_t MAC::timeEgressIsReady(int port, int wl){
    return 2.0;
}
