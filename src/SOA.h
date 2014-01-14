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
    cQueue switchingTable;
    /**
     *  switching time
     */
    simtime_t d_s;
    bool WC;

    virtual SOAEntry * findOutput(int inPort, int inWl);

    /**
     *  Internal function which verifies the loaded SOAEntry and provides mechanisms
     *  for SOAEntry adding into SOA switching matrix..
     *  Function is called from handleMessage() and is triggered by self-message: ActivateSTE
     */
    virtual void addpSwitchingTableEntry(SOAEntry *e);

    /**
     *  Function dropSwitchingTableEntry removes SOAEntry when is no longer needed in the
     *  SOA. By removing SOAEntry from SOA is removed also from SOAmanager and MAC if it was
     *  buffering entry.
     *  Function is called from handleMessage() and is triggered by self-message: DeactivateSTE
     */
    virtual void dropSwitchingTableEntry(SOAEntry *e);

    /**
     *  Loss statistics
     *  incm - all processed OpticalLayer packets which are not dropped
     *  drpd - a number of dropped bursts
     */
    int64_t incm, drpd, buff;
    int64_t wrong_scheduling;
    cOutVector blpevo;

    /**
     *  The number of wavelengths currently used
     */
    int wls;
    int wcs;    // number of WC
    int bigOT;  // If OT goes to the past

  public:

    /**
     *  Public function called by SOA Manager in order to perform switching.. SOAm says
     *  when and for how long the switching must be cross connected. After the time is
     *  passed dropSwitchingTableEntry function is called.
     *  When function is called initiates two self-messages: ActivateSTE and DeactivateSTE
     */
    virtual void assignSwitchingTableEntry(cObject *e, simtime_t ot, simtime_t len);

};

#endif
