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

    // Communication link with soaManager
    cModule *calleeModule = getParentModule()->getSubmodule("soaManager");
    SM = check_and_cast<SOAManager *>(calleeModule);

    // Statistics and Watchers
    bufferSize.setName("Buffer Usage [B]"); // How much RAM do we need?
    WATCH(capacity);
}

void CoreNodeMAC::handleMessage(cMessage *msg) {
    /**
     *  This part of the code is executed when an OpticalLayer message approaches
     *  the MAC layer. This might happend only in case of buffering .. this is the
     *  code which buffers incoming Cars and creates scheduler to resend them back
     */
    if (dynamic_cast<OpticalLayer *>(msg) != NULL) {
        OpticalLayer *ol= dynamic_cast<OpticalLayer *>(msg);
        Car *car= (Car *) ol->decapsulate();    // OE conversion
        EV << "The car "<< car->getName() << "is to be stored in buffer" << endl;

        // Calculate buffer usage and create record for statistics
        capacity += car->getByteLength();
        bufferSize.record(capacity);

        // Determine description of incoming signal. Since it is optical only port and WL
        int inPort= msg->getArrivalGate()->getIndex();
        int inWL= ol->getWavelengthNo();
        delete ol;

        // Based on input port and WL find SOAEntry informing me when to send it back
        // Since we have car-train it is complicated and rescheduling time muset be
        // recalculated for each Car
        for ( int i=0;i<bufferedSOAe.size();i++){
            SOAEntry *tmpse= (SOAEntry *) bufferedSOAe[i];
            if( tmpse->getInPort() == inPort and tmpse->getInLambda() == inWL ){
                // Self-message informing this module about releasing Car from buffer
                cMessage *msg = new cMessage("ReleaseBuffer");
                msg->addPar("RelaseStoredCar");
                msg->addPar("RelaseStoredCar_CAR");
                msg->par("RelaseStoredCar").setPointerValue(tmpse);
                msg->par("RelaseStoredCar_CAR").setPointerValue(car);
                // Determine waiting time of cars from car-train which are not the first one
                usage[tmpse]++;
                scheduleAt(simTime()+waitings[tmpse], msg);
            }
        }
        return;
    }

    /** Continue of previous part - withdrawing the Car from waiting queue and send back to SOA **/
    if( msg->isSelfMessage() and msg->hasPar("RelaseStoredCar")){
        SOAEntry *se=(SOAEntry *) msg->par("RelaseStoredCar").pointerValue();
        Car *car= (Car *) msg->par("RelaseStoredCar_CAR").pointerValue();
        EV << "The car "<< car->getName() << "is to be released to SOA" << endl;

        // Convert Car->OpticalLayer == E/O conversion
        OpticalLayer *ol= new OpticalLayer( car->getName() );
        ol->setWavelengthNo( se->getOutLambda() );
        ol->encapsulate( car );

        // Calculate buffer usage and create record for statistics
        capacity -= car->getByteLength();
        bufferSize.record(capacity);

        // Send the OL to SOA
        send(ol, "soa$o", se->getOutPort() );

        // Clean waitings and bufferedSOAe - maintenance
        if( --usage[se] <= 0 ){
            EV << "Last car now clean up" << endl;
            waitings.erase(se);
            usage.erase(se);
            bufferedSOAe.remove(se);
        }

        // Drop the self-message since is not needed anymore
        delete msg; return;
    }



    /**
     *  This part of the code takes care of aggregation of Payload packet to
     *  SOA in case of mixed usage scenario when CoreNode can behave as an
     *  EdgeNode.
     */

    // Lets make the sending done
    MACContainer *MAC = dynamic_cast<MACContainer *>(msg);

    // Dencapsulate CARBOSHeader
    CAROBSHeader *H = dynamic_cast<CAROBSHeader *>(MAC->decapsulate());

    // Sending parameters
    int dst = H->getDst();

    // Obtain waiting and wavelength
    int WL = 0;
    int outPort=0;
    simtime_t t0 = SM->getAggregationWaitingTime(dst, H->getOT(), H->getLength(), WL, outPort);

    // Set wavelength of Car train to CAROBS header
    H->setWL(WL);

    /*   Sending ....   */
    EV << "CAROBS Train to " << dst << " will be send at " << simTime()+t0;

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

        EV << " + Sending car to " << su->getDst() << " at " << t0 + su->getStart() << " of length=" << su->getLength() << endl;
        // Send the car onto proper wl at proper time
        sendDelayed(OC, t0 + su->getStart(), "soa$o", outPort);
    }

}

void CoreNodeMAC::storeCar( SOAEntry *e, simtime_t wait ){
    Enter_Method("storeCar()");
    EV << "Pridavam scheduler " << simTime() << " release="<<e->getStart()<<endl;
    bufferedSOAe.add(e);
    waitings[e]= wait;
    usage[e]= 0;
}
