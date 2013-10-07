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

#ifndef __CAROBS_SINK_H_
#define __CAROBS_SINK_H_

#include <omnetpp.h>
#include <messages/Payload_m.h>


class Sink : public cSimpleModule
{
  protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);

    // Address of the endnode
    int address;

    // How many packets were handled
    int64_t received;

    // Packet measurement - how many has been received
    std::map<int,int> counts;

    // Ent-to-End delay measurement
    std::map<int,cOutVector *> e2e;

    // Throughput measurements
    simtime_t thr_window;
    std::map<int, simtime_t> thr_t0;
    std::map<int, int64_t> thr_bites;
    std::map<int,cOutVector *> throughput;

    // Verification of proper behaviour
    std::map<int, int64_t> misdelivered;
};

#endif
