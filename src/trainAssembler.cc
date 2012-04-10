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

#include "trainAssembler.h"
#include <iostream>
#include <algorithm>
#include <vector>

Define_Module(TrainAssembler);

void TrainAssembler::initialize() {

    // Switching and processing times
    d_p = (simtime_t) par("d_p").doubleValue(); // set d_p
    d_s = (simtime_t) par("d_s").doubleValue(); // set d_s
    ASSERT(d_p>d_s);

    // Hard-coded datarte
    C = par("datarate").doubleValue();
    CTA = par("CTA").boolValue();

    // Making link for communication with Routing module
    cModule *calleeModule = getParentModule()->getSubmodule("routing");
    R = check_and_cast<Routing *>(calleeModule);

    // Structure watchers in TK
    WATCH_MAP(schedulerCAR);
}

void TrainAssembler::handleMessage(cMessage *msg) {

    if (dynamic_cast<Car *>(msg) != NULL) {
        Car *tcar = dynamic_cast<Car *>(msg);
        int TSId = msg->par("TSId").longValue();
        int AQ = msg->par("AQ").longValue();

        EV << "A new car has arrived" << " with random ID=" << TSId << " and AQ=" << AQ;

        simtime_t ot = R->getOffsetTime(AQ);
        EV << " with distance ot=" << ot;

        /**
         *  Preparion of artificial unit having accurate parameters for scheduling purpose
         *  It is destination, start, end, offset time and length
         */
        SchedulerUnit *su = new SchedulerUnit();
        su->setDst(AQ);
        su->setOt(ot);

        // In the first step (definition) Start, End, Length are
        simtime_t tlength = (simtime_t) tcar->getBitLength() / C;
        su->setStart(ot);
        su->setLength(tlength);
        su->setLengthB(tcar->getByteLength());
        su->setEnd(ot + tlength);
        su->setEOT(0);

        su->encapsulate(tcar);
        schedulerCAR[TSId].insert(su);

        char buffer_name[50];
        sprintf(buffer_name, "Queue %d", TSId);
        schedulerCAR[TSId].setName(buffer_name);

        EV << endl;
    }

    if (msg->hasPar("allCarsHaveBeenSend")) {
        // Initialise process of ordering and sending
        int TSId = msg->par("allCarsHaveBeenSend").longValue();
        EV << "Release packet has arrived for AP with rand ID=" << TSId << endl;
        prepareTrain(TSId);
        delete msg;
        // Drop information about released AP
        schedulerCAR.erase(TSId);
    }
}

void TrainAssembler::prepareTrain(int TSId) {
    EV << "prepareTrain" << endl;
    std::vector<SchedulerUnit *> dst;
    std::vector<SchedulerUnit *>::iterator it;

    // conversion for cQueue into std::vector because of sorting
    for (cQueue::Iterator iter(schedulerCAR[TSId], 0); !iter.end(); iter++) {
        SchedulerUnit *msg = (SchedulerUnit *) iter();
        dst.push_back(msg);
    }

    // Sort cars according the OT for further car assignement into a train
    bubbleSort(dst);

    // Apply the CTA if set on
    if( CTA ) ctaTrainTruncate(dst);

    // Make the train dense
    smoothTheTrain(dst);

    /**
     *  Prepare CAROBS header
     */
    int N = dst.size();
    if( N == 0) return;

    simtime_t OT = dst[0]->getStart();
    int dt = dst[dst.size() - 1]->getDst();
    simtime_t len= dst[N-1]->getEnd() - dst[0]->getStart();

    EV << "Burst train length="<<len << endl;

    // Train Section
    CAROBSHeader *H = new CAROBSHeader();
    H->setDst(dt);
    H->setOT(OT);
    H->setN(N);
    H->setLength(len);

    // Car section
    for (int i = 0; i < dst.size(); i++) {
        CAROBSCarHeader *ch = new CAROBSCarHeader();
        ch->setDst(dst[i]->getDst());
        ch->setSize(dst[i]->getLengthB());
        ch->setD_c(dst[i]->getStart() - OT);
        H->getCars().insert(ch);
    }

    /**
     *  Put CAROBS Header and cars into storage container MACContainer which
     *  is send onto output interface waiting there until outgoing port is empty
     */
    MACContainer *MAC = new MACContainer();
    MAC->setDst(dt);
    MAC->encapsulate(H); //  Storing CAROBS header into MAC packet

    // Storing scheduled units with encapsulated cars into MAC->cars
    EV << " Scheduler verification: " << endl;
    while (!dst.empty()) {
        SchedulerUnit *tmp = dst.back();
        dst.pop_back();
        EV << " + " <<tmp->getDst()<<"OT="<<tmp->getOt();
        EV << "..EOT="<<tmp->getEOT();
        EV <<" starts: "<<tmp->getStart()<<"-"<<tmp->getEnd()<<"="<<tmp->getLength()<<"("<<tmp->getLengthB()<<"B)"<<endl;
        schedulerCAR[TSId].remove(tmp);
        MAC->getCars().insert(tmp);
    }

    // Do not show in inspector TKenv
    MAC->getCars().removeFromOwnershipTree();
    H->getCars().removeFromOwnershipTree();

    // Sending MAC Packet to MAC controller which will take care of sending
    send(MAC, "out");
}

// Think about std::sort instead of this hand-made function
void TrainAssembler::bubbleSort(std::vector<SchedulerUnit *> &num) {
    int i, j, flag = 1; // set flag to 1 to start first pass
    SchedulerUnit *temp; // holding variable
    int numLength = num.size();
    for (i = 1; (i <= numLength) && flag; i++) {
        flag = 0;
        for (j = 0; j < (numLength - 1); j++) {
            if (num[j + 1]->getOt() < num[j]->getOt()) // ascending order simply changes to <
                    {
                temp = num[j]; // swap elements
                num[j] = num[j + 1];
                num[j + 1] = temp;
                flag = 1; // indicates that a swap occurred.
            }
        }
    }
    return; //arrays are passed to functions by address; nothing is returned
}

void TrainAssembler::smoothTheTrain(std::vector<SchedulerUnit *> &dst) {
    // add extra d_s for the first car in order to give time for switching
    //dst[0]->setStart(dst[0]->getStart() + d_s);

    // Smoothening process
    for (int i = 0; i < dst.size() - 1; i++) {
        if (dst[i]->getEnd() < dst[i + 1]->getStart() - d_s) {
            // Big gap between c_1 <----> c_2 -- c_1 must be given by EOT
            dst[i]->setEnd(dst[i + 1]->getStart() - d_s);
            dst[i]->setStart(dst[i + 1]->getStart() - d_s - dst[i]->getLength());
            dst[i]->setEOT(dst[i]->getStart() - dst[i]->getOt());
            if (i > 0) {
                for (int j = i; j >= 0; j--) {
                    dst[j]->setEnd(dst[j + 1]->getStart() - d_s);
                    dst[j]->setStart(dst[j + 1]->getStart() - d_s - dst[j]->getLength());
                    dst[j]->setEOT(dst[j]->getStart() - d_s - dst[j]->getOt());
                }
            }
        }

        if (dst[i]->getEnd() > dst[i + 1]->getStart() - d_s) {
            // The gap is not big enough c_1 <----> c_2 -- c_1 must be given by EOT
            dst[i + 1]->setStart(dst[i]->getEnd() + d_s);
            dst[i + 1]->setEnd(dst[i]->getEnd() + d_s + dst[i + 1]->getLength());
            dst[i + 1]->setEOT(dst[i + 1]->getStart() - dst[i + 1]->getOt());
        }
    }
}

void TrainAssembler::ctaTrainTruncate(std::vector<SchedulerUnit *> &dst) {
    // add extra d_s for the first car in order to give time for switching
    //dst[0]->setStart(dst[0]->getStart() + d_s);
    EV << "Curbed Train Assembly " << endl;

    // Smoothening process
    for (int i = 0; i < dst.size() - 1; i++) {
        // Calculate the gap between two OT
        simtime_t t_space = dst[i+1]->getOt()-dst[i]->getOt();

        // Evaluate whether current car is not longer than space
        if( dst[i]->getLength() > t_space ){
           Car *car= (Car *) dst[i]->getEncapsulatedPacket();
           cQueue queue = car->getPayload();
           cQueue toSend; toSend.setName( queue.getName() );

           simtime_t tmp_current=0;
           int64_t tmp_current_size=0;
           while (!queue.empty()) {
                Payload *msg = (Payload *) queue.pop();
                simtime_t t_inc = (simtime_t) msg->getBitLength() / C;
                EV << " + " << msg->getDst() << " size " << t_inc;
                if (tmp_current > t_space) {
                    // there is no room so I must return Payload packet back to AQ
                    queue.remove(msg);
                    send(msg,"in$o");
                    EV << " - return" << endl;
                    continue;
                }
                tmp_current_size += msg->getBitLength();
                tmp_current += t_inc;
                toSend.insert(msg);
                EV << " - send" << endl;
           }

           // stop inspecting it in TKenv
           queue.removeFromOwnershipTree();

           // Informations about car and its scheduler container must be updated
           car->setPayload(toSend);
           car->setBitLength(tmp_current_size);
           dst[i]->setBitLength(tmp_current_size);
           dst[i]->setLength(tmp_current);
           dst[i]->setLengthB(car->getByteLength());
           dst[i]->setEnd( dst[i]->getStart() + tmp_current);
//           EV << "Reduced to="<< tmp_current<<"s "<<tmp_current_size<<"b ("<<tmp_current_size/8<<"B)"<<endl;
        }

    }

}
