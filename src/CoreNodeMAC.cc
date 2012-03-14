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

#include "CoreNodeMAC.h"

Define_Module(CoreNodeMAC);

void CoreNodeMAC::initialize() {
    cModule *calleeModule = getParentModule()->getSubmodule("soaManager");
    SM = check_and_cast<SOAManager *>(calleeModule);
}

void CoreNodeMAC::handleMessage(cMessage *msg) {

    // Lets make the sending done
    MACContainer *MAC = dynamic_cast<MACContainer *>(msg);

    // Dencapsulate CARBOSHeader
    CAROBSHeader *H = dynamic_cast<CAROBSHeader *>(MAC->decapsulate());

    // Sending parameters
    int dst = H->getDst();

    // Obtain waiting and wavelength
    int WL = 0;
    int outPort=0;
    simtime_t t0 = SM->getAggregationWaitingTime(dst, H->getOT() , WL, outPort);

    // Set wavelength of Car train to CAROBS header
    H->setWL(WL);

    /*   Sending ....   */
    EV << "CAROBS Train to " << dst << " will be send at " << t0;

    // CAROBS Header
    char buffer_name[50];
    sprintf(buffer_name, "Header %d", dst);
    OpticalLayer *OH = new OpticalLayer(buffer_name);
    OH->setWavelengthNo(0); //Signalisation is always on the first channel
    OH->encapsulate(H);
    sendDelayed(OH, t0, "control");

    // Cars to the SOA
    EV << " OT= " << H->getOT() << endl;
    // Schedule associate cars
    while (!MAC->getCars().empty()) {
        SchedulerUnit *su = (SchedulerUnit *) MAC->getCars().pop();
        Car *tcar = (Car *) su->decapsulate();

        // Encapsulate Car into OpticalLayer
        char buffer_name[50];
        sprintf(buffer_name, "Car %d", su->getDst());
        OpticalLayer *OC = new OpticalLayer(buffer_name);
        OC->setWavelengthNo(WL);
        OC->encapsulate(tcar);

        OC->setSchedulingPriority(10);
        EV << " + Sending car to " << su->getDst() << " at " << t0 + su->getStart() << " of length=" << su->getLength() << endl;
        // Send the car onto proper wl at proper time
        sendDelayed(OC, t0 + su->getStart(), "soa$o", outPort);
    }

}

