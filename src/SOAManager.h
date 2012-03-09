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

#ifndef __CAROBS_SOAMANAGER_H_
#define __CAROBS_SOAMANAGER_H_

#include <omnetpp.h>
#include <routing.h>
#include <SOA.h>
#include <messages/CAROBSHeader_m.h>
#include <messages/OpticalLayer_m.h>
#include <SOAEntry.h>


class SOAManager : public cSimpleModule
{
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

  private:
    /**
     *  Pointer for communication with routing submodule
     */
    Routing *R;

    /**
     *  Pointer for communication with SOA submodule
     */
    SOA *soa;

    /**
     *  Processing and switching time of SOAManager and SOA respectively
     */
    simtime_t d_p, d_s;

    /**
     *  Option whether JET/JIT is going to be used
     */
    bool JET;

    /**
     *  Maximum number of wavelengths which can be used for WC
     */
    int maxWL;

    /**
     *  Variable which tracks Wavelength conversion state .. true= enabled, false= disabled
     *  at this CoreNode
     */
    bool WC;

    /**
     *  Array which keeps informations about time usage of output ports and its
     *  wavelengths over the time. It can be used for finding of free wavelengths
     *  if wavelength conversion is enabled
     */
    cArray scheduling;

    /**
     *  Function tries to find proper output such that there is no collision or
     *  overlap of output port@output wavelength among more inputs
     *
     *  if function returns -1 -1 -1 -1 it means there is no affordable output\
     *  so the burst must be either stored in memory or dropped
     */
    virtual SOAEntry* getOptimalOutput(int outPort, int inPort, int inWL, simtime_t start, simtime_t stop);

    /**
     *  Function testOutputCombination test whether combination of outPort and outWL at given time
     *  can be used. It returns response
     */
    virtual bool testOutputCombination(int outPort, int outWL, simtime_t start, simtime_t stop );


  public:
    /**
     *  When the SOAEntry is not needed SOA informas SOAManager to tear it down through
     *  this interface taking as input parameter pointer to the SOAEntry.
     */
    virtual void dropSwitchingTableEntry(SOAEntry *e);
};

#endif
