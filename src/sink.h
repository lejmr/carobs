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
/**
 * TODO - Generated class
 */
class Sink : public cSimpleModule
{
  protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
    int64_t received;
    simtime_t total_delay;

    std::map<int,int> counts;
    std::map<int,cOutVector *> vects;
    std::map<int,cOutVector *> throughputs;
    std::map<int, long double> throughput;

    cOutVector avg_delay;
    cOutVector avg_throughput;
    simtime_t avg_e2e;
    int address;
    std::map<int, int64_t> misdelivered;

};

#endif
