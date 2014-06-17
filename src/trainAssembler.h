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

#ifndef __CAROBS_TRAINASSEMBLER_H_
#define __CAROBS_TRAINASSEMBLER_H_

#include <omnetpp.h>
#include <routing.h>
#include <messages/car_m.h>
#include <messages/Payload_m.h>
#include <messages/schedulerUnit_m.h>
#include <CAROBSCarHeader_m.h>
#include <CAROBSHeader_m.h>
#include <MACContainer_m.h>

//TODO - CTA algorithm

class TrainAssembler : public cSimpleModule
{
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

  private:
    std::map<int, cQueue> schedulerCAR;
    std::map<int, int> tsid_label;

    /**
     *  Pointer to Routing module for Offset time adjustment
     */
    Routing *R;

    virtual void prepareTrain(int TSId);

    /**
     *  Function bubbleSort sorts a vector of Cars according to their
     *  increasing offset time OT. It behaves as a procedure
     *
     *  @param num - is a pointer to vector containing cars in SchedulerUnit
     */
    virtual void bubbleSort(std::vector<SchedulerUnit *> &num);

    /**
     *  BHP processing time at a BCP of a CoreNode
     *  It might be good be changable among CoreNodes in a network and
     *  consider such a variation in the OT calculation .. currently
     *  we are going static - hardcoded.
     */
    simtime_t d_p;

    /**
     *  Switching time of a SOA switches along a flow. For now it is hard-coded
     *  but correctly it should by a parameter of CAROBS CoreNode
     */
    simtime_t d_s;

    /**
     *  Funtion smoothTheTrain takes sorted vector of cars encapsulated into
     *  SchedulerUnit which carries information about start, stop,... time events
     *  and changes the start, stop so the cars go one by one with time difference
     *  given by processing time of BHP at any CoreNode along the path.
     */
    virtual void smoothTheTrain(std::vector<SchedulerUnit *> &num);
    virtual void ctaTrainTruncate(std::vector<SchedulerUnit *> &dst);

    /**
     *  Hardcoded datarate
     */
    int64_t C;

    /**
     *  CTA option
     */
    bool CTA;
};

#endif
