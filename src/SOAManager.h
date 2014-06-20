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
#include <SOAEntry.h>
#include <messages/CAROBSHeader_m.h>
#include <messages/OpticalLayer_m.h>
#include <messages/CAROBSCarHeader_m.h>
#include <aggregationQueues.h>
#include <algorithm>    // std::sort

class SOAManager : public cSimpleModule
{
  protected:
    virtual void initialize();
    virtual void finish();
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
     *  Pointer for communication with AQ submodule
     */
    AggregationQueues *AQ;

    /**
     *  Processing and switching time of SOAManager and SOA respectively
     */
    simtime_t d_p, d_s;

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
     *  OBS mode allows swtiching of Burst trains only on the contrary CAROBS mode
     *  allows cars grooming
     */
    bool OBS;

    /**
     *  Denotes O/E conversion speed in sec/bit
     */
    double convPerformance;

    /**
     *  Array which keeps informations about time usage of output ports and its
     *  wavelengths over the time. It can be used for finding of free wavelengths
     *  if wavelength conversion is enabled
     */
    cQueue scheduling;

    /**
     *  Function tries to find proper output such that there is no collision or
     *  overlap of output port@output wavelength among more inputs
     *
     *  if function returns -1 -1 -1 -1 it means there is no affordable output\
     *  so the burst must be either stored in memory or dropped
     */
    virtual SOAEntry* getOptimalOutput(int label, int inPort, int inWL, simtime_t start, simtime_t stop, int length=0);

    /**
     *  Function testOutputCombination test whether combination of outPort and outWL at given time
     *  can be used. It returns response
     */
    virtual bool testOutputCombination(int outPort, int outWL, simtime_t start, simtime_t stop );

    /**
     *  OBS manner burst train SOA manager behaviour - means that car train is switched based on
     *  informations in CAROBS header and no CAR Headers are taken on consideration. Such a mode
     *  is elected automatically based on activation of tributary ports .. sizeof(tributary)==0
     */
    virtual void obsBehaviour(cMessage *msg, int inPort);

    /**
     *  CAROBS manner burst train SOA manager behaviour - means that car train is switched based on
     *  informations in CAR Headers. It means CoreNode can perform Grooming, Dis/Aggregation.
     *  Such a mode is elected automatically based on activation of tributary ports .. sizeof(tributary)!=0
     */
    virtual void carobsBehaviour(cMessage *msg, int inPort);

    /**
     *  Address of this CoreNode
     */
    int address;

    /**
     *  Hardcoded datarate
     */
    int64_t C;

    /**
     *  Trigger which allows buffering
     */
    bool buffering;

    /**
     *  Statistics
     */
    int64_t tbdropped;

    /**
     *  Sending method
     *      True = fifo
     *      False= rand
     */
    bool fifo;

    /**
     * prioritizeBuffered
     */
    bool prioritizeBuffered;

    /**
     *  Merging flows counters
     *  - first int index stands for # of wavelength
     *  - second int index stands for #of concurent merging flows
     */
    std::map<int, int> mf_max;
    std::map<int, int> reg_max;
    std::map<int, int>::iterator it_mfc;


    /**
     *  Implentation of split switching table
     */
    cQueue inSplitTable;
    std::map<int, cQueue> splitTable;
    std::map<int, cQueue>::iterator it_st;

    /**
     * Merging flows counters
     * - Based on a similar mechanism used in splitTable.. we are having separate
     *   set of SOAEntries which are heading to the buffer with same input wavelength
     * - Everytime a new one is added counting is initiated and if there is more of
     *   merging flows than in previous step, statitstics are updated
     *
     *    int - stands for input wavelegth
     *    cQueue- is used for storing of SOAEntries
     */
    std::map<int, cQueue> mf_table;
    cQueue inMFcounter;

    // Function which provides counting
    virtual void countMergingFlows(int inWL, SOAEntry *e);

    /**
     *  Wrapper around adding SOAEntry
     */
    virtual void addSwitchingTableEntry(SOAEntry *e);

    /**
     *
     */
    virtual void rescheduleAggregation(std::vector<SOAEntry *> toBeRescheduled);


    /**
     * Buffering probability counters
     */
    int64 bbp_switched, bbp_buffered, bbp_dropped, bbp_total, bbp_interval_max;
    cOutVector BBP, BLP, BOKP, BTOTAL, SECRATIO;


    // Count function
    virtual void countProbabilities();
    virtual void evaluateSecondaryContentionRatio(int outPort, simtime_t start, simtime_t stop);

  public:
    /**
     *  When the SOAEntry is not needed SOA informas SOAManager to tear it down through
     *  this interface taking as input parameter pointer to the SOAEntry.
     */
    virtual void dropSwitchingTableEntry(SOAEntry *e);



    /**
     *   Function getAggregationWaitingTime return waiting time of burst train such that
     *   SOA is not blocked by any other burst train
     *   @param label : Label of selected path
     *   @param simtime_t:  Offset time betwean CAROBS header and Car train
     *   @param wl: pointer on WL variable which is to be used as wavelength
     *   @return    : Least waiting time when such car train can be send to SOA
     */
    virtual simtime_t getAggregationWaitingTime(int label, simtime_t OT, simtime_t len, int &WL, int &outPort);
};

#endif
// TODO: pri zapnute WC a omezenem poctu bufferovani@WL nutno WC usmernit, aby vybirala i podle intenzity vyuzivani WL
