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

void TrainAssembler::initialize()
{
    // set d_p
    d_p = (simtime_t) par("d_p").doubleValue();
    d_p = 0;

    // Making link for communication with Routing module
    cModule *calleeModule = getParentModule()->getSubmodule("routing");
    R = check_and_cast<Routing *>(calleeModule);

    // Structure watchers in TK
    WATCH_MAP(schedulerCAR);
}


void TrainAssembler::handleMessage(cMessage *msg)
{
    int C = 10e9; // data rate in bps -- currently hardcoded but will be resolved from channel rate

    if ( dynamic_cast<Car *>(msg) != NULL){
        Car *tcar= dynamic_cast<Car *>(msg);
        int TSId=msg->par("TSId").longValue();
        int AQ=msg->par("AQ").longValue();

        EV << "A new car has arrived" << " with random ID="<<TSId << " and AQ="<<AQ;

        simtime_t ot= R->getOffsetTime( AQ );
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
        su->setLengthB( tcar->getByteLength() );
        su->setEnd(ot + tlength);
        su->setEOT(0);

        su->encapsulate(tcar);
        schedulerCAR[TSId].insert(su);

        char buffer_name[50];
        sprintf(buffer_name, "Queue %d", TSId);
        schedulerCAR[TSId].setName(buffer_name);

        EV << endl;
    }

    if( msg->hasPar("allCarsHaveBeenSend")){
        // Initialise process of ordering and sending
        int TSId= msg->par("allCarsHaveBeenSend").longValue();
        EV << "Prisel uvolnovaci paket " << TSId << endl;
        prepareTrain(TSId);
        delete msg;
    }
}

void TrainAssembler::prepareTrain(int TSId){
    std::vector<SchedulerUnit *> dst;
    std::vector<SchedulerUnit *>::iterator it;

    // conversion for cQueue into std::vector because of sorting
    for( cQueue::Iterator iter(schedulerCAR[TSId],0); !iter.end(); iter++){
        SchedulerUnit *msg = (SchedulerUnit *) iter();
        dst.push_back(msg);
    }

    // Sort cars according the OT for further car assignement into a train
    bubbleSort(dst);

    // Make the train dense
    smoothTheTrain(dst);

    /**
     *  Prepare CAROBS header
     */
    int N = dst.size();
    simtime_t OT= dst[0]->getStart();
    int dt= dst[dst.size()-1]->getDst();

    // Train Section
    CAROBSHeader *H= new CAROBSHeader();
    H->setDst( dt );
    H->setOT( OT );
    H->setN( N );

    // Car section
    for(int i=0; i<dst.size();i++){
        CAROBSCarHeader *ch= new CAROBSCarHeader();
        ch->setDst( dst[i]->getDst() );
        ch->setSize( dst[i]->getLengthB() );
        ch->setD_c( dst[i]->getStart() - OT );
        H->getCars().insert(ch);
    }

    /**
     *  Put CAROBS Header and cars into storage container MACContainer which
     *  is send onto output interface waiting there until outgoing port is empty
     */
    MACContainer *MAC= new MACContainer();
    MAC->setDst(dt);
    MAC->encapsulate(H);    //  Storing CAROBS header into MAC packet

    // Storing scheduled units with encapsulated cars into MAC->cars
    while (!dst.empty())
     {
        SchedulerUnit *tmp=dst.back();
        dst.pop_back();
        schedulerCAR[TSId].remove(tmp);
        MAC->getCars().insert( tmp );
    }

    // Sending MAC Packet to MAC controller which will take care of sending
    send(MAC, "out");
}

// Think about std::sort instead of this hand-made function
void TrainAssembler::bubbleSort(std::vector<SchedulerUnit *> &num){
      int i, j, flag = 1;    // set flag to 1 to start first pass
      SchedulerUnit *temp;             // holding variable
      int numLength = num.size( );
      for(i = 1; (i <= numLength) && flag; i++)
     {
          flag = 0;
          for (j=0; j < (numLength -1); j++)
         {
               if (num[j+1]->getOt() < num[j]->getOt())      // ascending order simply changes to <
              {
                    temp = num[j];             // swap elements
                    num[j] = num[j+1];
                    num[j+1] = temp;
                    flag = 1;               // indicates that a swap occurred.
               }
          }
     }
     return;   //arrays are passed to functions by address; nothing is returned
}

void TrainAssembler::smoothTheTrain(std::vector<SchedulerUnit *> &dst){
    for (int i = 0; i < dst.size()-1; i++) {
        if( dst[i]->getEnd() < dst[i+1]->getStart()-d_p ){
            // Big gap between c_1 <----> c_2 -- c_1 must be given by EOT
            dst[i]->setEnd( dst[i+1]->getStart()-d_p );
            dst[i]->setStart( dst[i+1]->getStart()-d_p - dst[i]->getLength() );
            dst[i]->setEOT( dst[i]->getStart() - dst[i]->getOt() );
            if( i>0 ){
                for (int j = i; j >= 0 ; j--) {
                    dst[j]->setEnd(dst[j + 1]->getStart() - d_p);
                    dst[j]->setStart(dst[j + 1]->getStart() - d_p - dst[j]->getLength());
                    dst[j]->setEOT(dst[j]->getStart() -d_p - dst[j]->getOt());
                }
            }
        }

        if( dst[i]->getEnd() > dst[i+1]->getStart()-d_p ){
            // The gap is not big enough c_1 <----> c_2 -- c_1 must be given by EOT
            dst[i+1]->setStart(dst[i]->getEnd() + d_p );
            dst[i+1]->setEnd(  dst[i]->getEnd() + d_p + dst[i+1]->getLength() );
            dst[i+1]->setEOT(  dst[i+1]->getStart() - dst[i+1]->getOt() );
        }
    }
}
