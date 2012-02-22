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

#ifndef __CAROBS_ROUTING_H_
#define __CAROBS_ROUTING_H_

#include <omnetpp.h>

/**
 * TODO - Generated class
 */
class Routing : public cSimpleModule
{
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

  public:
    /**
     * Function calculates minumum offset time needed for OBS system to work.
     * It sums all delays introduced by fibers, CoreNodes,.. along a flow
     * @param destination - refers to address of destination nodde
     * @return offset time ot^c - which stands for processing time of all intermediate nodes ot^c=l^c*d^H [8.2.3 - Coutelen DT]
     */
    virtual simtime_t getOffsetTime(int destination);



  private:
    std::map<int, simtime_t> OT;   // First step development, later it obsoletes by cTopolog
};

#endif
