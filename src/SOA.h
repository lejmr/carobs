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

#ifndef __CAROBS_SOA_H_
#define __CAROBS_SOA_H_

#include <omnetpp.h>
#include <SOAEntry.h>
#include <messages/OpticalLayer_m.h>
//#include <messages/swe_m.h>

/**
 * TODO - Generated class
 */
class SOA : public cSimpleModule
{
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
  private:
    cArray *switchingTable;
    /**
     *  switching time
     */
    simtime_t d_s;

    virtual SOAEntry * findOutput(int inPort, int inWl);


  public:
    virtual void assignSwitchingTableEntry(cObject *e, simtime_t ot, simtime_t len);
    virtual void dropSwitchingTableEntry(SOAEntry *e);

};

#endif