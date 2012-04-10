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
 *  All optical space switching matrix
 */
class SOA : public cSimpleModule
{
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

  private:
    cArray *switchingTable;
    /**
     *  switching time
     */
    simtime_t d_s;
    bool WC;

    virtual SOAEntry * findOutput(int inPort, int inWl);
    virtual void addpSwitchingTableEntry(SOAEntry *e);

    int64_t incm, drpd;


    /**
     *  The number of wavelengths currently used
     */
    int wls;
    int wcs;    // number of WC
    int bigOT;  // If OT goes to the past

  public:
    virtual void assignSwitchingTableEntry(cObject *e, simtime_t ot, simtime_t len);
    virtual void dropSwitchingTableEntry(SOAEntry *e);

};

#endif
