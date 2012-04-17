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

#ifndef __CAROBS_FIBERCHANNEL_H_
#define __CAROBS_FIBERCHANNEL_H_

#include <omnetpp.h>
#include <cdelaychannel.h>
#include <messages/OpticalLayer_m.h>

class FiberChannel : public cDelayChannel
{
  protected:
    virtual void initialize();
    virtual void finish();
    void processMessage(cMessage *msg, simtime_t t, result_t& result);

    std::map<int,simtime_t> free_time;
    std::map<int,simtime_t> stop_time;
    int64_t overlap, scheduling, trans;

    /**
     *  Hardcoded datarate
     */
    int64_t C;
    simtime_t d_s;
};

#endif