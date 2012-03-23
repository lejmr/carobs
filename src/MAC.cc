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
    guardTime= par("guardTime").doubleValue();

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
        output_t wlwt = getOutput( out, H->getOT() );
        simtime_t t0 = wlwt.t;

        // Set Wavelength to CARBOS Header once we know it
        H->setWL( wlwt.WL );

        EV << "CAROBS Train to "<< dst << " will be send at " << t0+simTime();
        EV << " to port "<<out<<"#"<<wlwt.WL;
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
            OC->setWavelengthNo( wlwt.WL );
            OC->encapsulate(tcar);

            OC->setSchedulingPriority(10);
            EV<<" + Sending car to "<<su->getDst() <<" at "<< t0 + su->getStart()+simTime()<<" of length="<<su->getLength()<<endl;
            // Send the car onto proper wl at proper time
            sendDelayed(OC, t0 + su->getStart(), "out", out);
            //EV << "car "<< su->getDst() <<" at"<< t0+su->getStart() << " length: " << su->getLength() << endl;

            // Drop the SchedulerUnit - it is empty now
            delete su;
        }

        // Schedule portScheduled entry to be set up and torn down
        simtime_t from_time = simTime() + wlwt.t + H->getOT() ;
        simtime_t to_time = from_time + H->getLength();
        MACEntry *me = new MACEntry(out, wlwt.WL, from_time, to_time+guardTime);
        portScheduled.add(me);

        cMessage *down = new cMessage("DOWN");
        down->addPar("TeardownOutputPortWL");
        down->par("TeardownOutputPortWL").setPointerValue(me);
        scheduleAt(to_time+guardTime, down);

        // Drop the MACContainer - it is empty now and not needed.
        delete MAC;
    }

    if( msg->isSelfMessage() and msg->hasPar("TeardownOutputPortWL") ){
        MACEntry *tmpe = (MACEntry *) msg->par("TeardownOutputPortWL").pointerValue();
        delete portScheduled.remove(tmpe);
        delete msg;
        return;
    }

}

output_t MAC::getOutput(int port, simtime_t ot){

    // If there are no scheduling fire it up right now
    if (portScheduled.size() == 0) {
        output_t t;
        t.WL = 1;   // FIFO, Might be changed to something different
        t.t = 0.0;
        return t;
    }

    std::map<int, simtime_t>::iterator it;
    std::map<int, simtime_t> timings;
    for (int j = 0; j < portScheduled.size(); j++) {
        if( portScheduled[j] == NULL ) continue;
        MACEntry *m = (MACEntry *) portScheduled[j];
        if (m->getOutPort() == port){
            if( timings.find(m->getOutLambda()) == timings.end() ){
                timings[m->getOutLambda()] = m->getTo()-simTime();
                continue;
            }

            // Update in case of worse waiting .. lesser reservation
            if( timings[m->getOutLambda()] < m->getTo() ){
                timings[m->getOutLambda()] = m->getTo()-simTime();
            }
        }
    }

    EV << " Waiting for WL: "<< endl;
    for (int i=1;i<=maxWL;i++){
        EV << i << "= ";
            if( timings.find(i) == timings.end() ){
                // We found an empty WL so lets use it
                EV << 0 << endl;
                continue;
            }
            EV << timings[i] << endl;
    }

    simtime_t waiting= 1e6;
    int WL=1;
    for (int i=1;i<=maxWL;i++){
        if( timings.find(i) == timings.end() ){
            // We found an empty WL so lets use it
            waiting = ot;
            WL=i;
            break;
        }

        if( timings[i] < waiting ){
            waiting = timings[i];
            WL=i;
        }
    }

    // Concate after a scheduled burst .. we do not care about BLP of Headers
    waiting= waiting-ot;
    if( waiting < 0 ) waiting=0;

    // Formulate output
    output_t t;
    t.WL = WL;   // FIFO, Might be changed to something different
    t.t = waiting;
    return t;
}
