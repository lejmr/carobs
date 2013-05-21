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

#ifndef __CAROBS_CityGENERATOR_H_
#define __CAROBS_CityGENERATOR_H_

#include <omnetpp.h>
#include <messages/Payload_m.h>

class CityGenerator : public cSimpleModule
{
  protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);

  private:
    int src;
    int64_t n;
    std::vector<double> demads;
    int64_t psend;

    std::map<int,int> counts;

    /**
     *  Function sendAmount sends a number of packets specified by param amount to destination
     *  dst.
     *  Interarrival intervals are specified by lambda which is feed to exponential random
     *  number generator.
     *  Returns time for next sending.
     */
    virtual simtime_t sendAmount(int amount, int src, int dst, double lambda, int length);
    std::map<int,simtime_t> arrivals;

};

#endif
