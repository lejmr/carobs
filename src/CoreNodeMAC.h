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

#ifndef __CAROBS_CORENODEMAC_H_
#define __CAROBS_CORENODEMAC_H_

#include <omnetpp.h>
#include <SOAManager.h>
#include <messages/schedulerUnit_m.h>
#include <messages/MACContainer_m.h>
#include <messages/car_m.h>
#include <messages/OpticalLayer_m.h>

class CoreNodeMAC : public cSimpleModule
{
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
  private:

    /**
     *  Communication link with SOA manager
     */
    SOAManager *SM;

    /**
     *  Helper variables for waiting time and SOAEntry storing
     */
    cArray bufferedSOAe;
    std::map< SOAEntry *, simtime_t> waitings;
    std::map< SOAEntry *, int> usage;

    /**
     *  How much bites was stored in memory - buffered cars in Bytes
     */
    int64_t capacity;

    /**
     *  Statistics of buffer use
     */
    cOutVector bufferSize;

  public:

    /**
     *  Function called by SOA manager informing MAC about new burst coming to
     *  MAC to be stored for a given time wait;
     */
    virtual void storeCar( SOAEntry *e, simtime_t wait );
};
#endif
