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

#include "SOA.h"
#include "SOAManager.h"
#include "CoreNodeMAC.h"

Define_Module(SOA);

void SOA::initialize() {

    //  Initialisation of switching time
    d_s = par("d_s").doubleValue();

    // Initialisation of WC variable
    WC = par("WC").boolValue();

    // OE limit
    oe_limit= par("oe_limit").doubleValue();
    WATCH(oe_limit);

    //switchingTable = new cQueue("switchingTable");
    switchingTable.setName("switchingTable");

    incm=0; drpd=0;
    wrong_scheduling=0;

    // Wavelength statistics
    wls=0;
    wcs=0;
    buff=0;
    bigOT = 0;
    blpevo.setName("Evolution of BLP");

    // RRPD
    OE.setName("Number of O/E blocks used [-]");
    EO.setName("Number of E/O blocks used [-]");
    LOE.setName("Limited of O/E blocks used [-]");
    BBP.setName("Burst buffering probability");
    BLP.setName("Burst loss probability");
    BOKP.setName("Burst cut-throught probability");
    BDISP.setName("Burst disaggregation probability");
    bbp_interval_max=100;
    bbp_disagg=0;
    bbp_switched= 0;
    bbp_buffered= 0;
    bbp_dropped=0;
    bbp_total=0;
}

void SOA::handleMessage(cMessage *msg) {

    // Set quite high number for lowering message priority.. Since this is EDS a number of
    // events can happen at one moment. Burst has lower priority than ActivateSwitchingTableEntry in SOA
    msg->setSchedulingPriority(10);

    if (msg->isSelfMessage() and msg->hasPar("ActivateSwitchingTableEntry")) {
        /*  Self-message for adding of switching entry */
        SOAEntry *tse = (SOAEntry *) msg->par("ActivateSwitchingTableEntry").pointerValue();

        // Add SwitchingTableEntry
        addpSwitchingTableEntry(tse);
        delete msg; return;
    }

    if (msg->isSelfMessage() and msg->hasPar("DeactivateSwitchingTableEntry")) {
        /*  Self-message for destruction of switching entry */
        SOAEntry *tse = (SOAEntry *) msg->par("DeactivateSwitchingTableEntry").pointerValue();
        dropSwitchingTableEntry(tse);
        delete msg; return;
    }

    if( !strcmp(msg->getArrivalGate()->getName(), "gate$i") ){
        /*  Ordinary Cars which are going to be transfered or disaggregated  */
        // Obtain informations about optical signal -- wavelength and incoming port

        // Taking statistics about incoming bursts
        incm++;

        msg->setSchedulingPriority(10);
        OpticalLayer *ol = dynamic_cast<OpticalLayer *>(msg);
        int inPort = msg->getArrivalGate()->getIndex();
        int inWl = ol->getWavelengthNo();

        // Find output configuration for incoming burst
        SOAEntry *sw = findOutput(inPort, inWl);

        // Count statistics
        if (bbp_total >= bbp_interval_max) {
                countProbabilities();
        }

        /*      Disaggregation     */
        if( sw->isDisaggregation() ){
            EV << "Disaggregated based on: " << sw->info() << endl;
            send(ol,"disaggregation");
            bbp_disagg++; bbp_total++;
            return ;
        }

        /*      Buffering to MAC    */
        if (sw->getBuffer() and sw->getBufferDirection() ) {
            EV << "Buffering based on: " << sw->info() << endl;

            // Marking the buffered OpticalLayer in order of easier SOAEntry resolution
            ol->addPar("marker");
            ol->par("marker").setPointerValue(sw);

            // Send burst to the MAC
            send(ol, "aggregation$o",inPort);
            bbp_buffered++;bbp_total++;

            // Check how many buffering instructions are assigned
            int oe=0, mf=0;
            for (cQueue::Iterator iter(switchingTable, 0); !iter.end();
                    iter++) {
                SOAEntry *se = (SOAEntry *) iter();

                // Check buffering instructions only
                if ( not (se->getBuffer() and se->getBufferDirection()) ) continue;


                // Check whether instructions overlap in time
                if (se->getStart() < sw->getStart() and se->getStop() + d_s < sw->getStart() ) {
                    // 1.
                    //  ........|----|..... sw
                    //  .|----|............ se
                    continue;
                }

                if (se->getStop() > sw->getStop() and se->getStart() - d_s >= sw->getStop()) {
                    // 2.
                    //  .|----|............ sw
                    //  ........|----|..... se
                    continue;
                }

                // Overlapping SE
                oe++;

                // Number of merging flows at one wavelength specified by the input wavelength of sw
                if( sw->getInLambda() == se->getInLambda() and sw->getInPort() == sw->getInPort() )
                    mf++;
            }

            // Statisticis
            buff++;
            OE.record(oe);
            return;
        }

        /*      All-optical Bypass    */
        int outPort = sw->getOutPort();
        int outWl = sw->getOutLambda();
        // Chceck whether exit is real or faked one

        //  and outPort < getParentModule()->gateSize("gate")
        if (outPort >= 0 and outWl >= 0) {
            // Not disaggregated thus only switched out onto exit
            EV << "Burst is to be switched based on: " << sw->info() << endl;

            // Wavelength conversion
            ol->setWavelengthNo(sw->getOutLambda());

            // Update ot of optical layer
            double ot= ol->par("ot").doubleValue();
            ol->par("ot").setDoubleValue( ot - getParentModule()->par("d_p").doubleValue() );

            if ( ot < 0 )
                opp_terminate("ASi tu mame smycku");

            // Monitor bypass into CAR section
            Car *car= (Car *)ol->getEncapsulatedPacket();
            car->setBypass( car->getBypass() + 1 );

            // Sending to output port
            send(ol, "gate$o", sw->getOutPort());
            bbp_switched++;bbp_total++;
            OE.record(0);
        }else{
            EV << "Burst at "<<inPort<<"#"<<inWl<<" is to be dropped !!!" << endl;
            drpd++;
            blpevo.record( (double) drpd/incm );
            bbp_dropped++;bbp_total++;
            delete msg; return;
        }


    }

    if( !strcmp(msg->getArrivalGate()->getName(), "aggregation$i") ){
        /*      Cars coming from aggregation port    */
        OpticalLayer *ol = dynamic_cast<OpticalLayer *>(msg);
        int inPort = msg->getArrivalGate()->getIndex();

        // Update ot of optical layer
        double ot = ol->par("ot").doubleValue();
        ol->par("ot").setDoubleValue(ot-getParentModule()->par("d_p").doubleValue());

        if (ot < 0)
            opp_terminate("ASi tu mame smycku v agregovani");

        EV << "Agreguji pres port " << inPort << endl;
        if (inPort >= 0 ) {
            send(ol, "gate$o", inPort );
        }
    }

}

void SOA::addpSwitchingTableEntry(SOAEntry *e){
    EV << " ADD " << e->info();

    int t_oe=0;

    // AOS mode and different lambdas on output and input
    if( !e->getBuffer() and e->getInLambda() != e->getOutLambda() ) wcs++;

    // Test whether *e does not overlap any switching configuration
    if( !e->getBuffer() and !e->isDisaggregation() ){
        simtime_t start= e->getStart();
        simtime_t stop= e->getStop();

        for( cQueue::Iterator iter(switchingTable,0); !iter.end(); iter++){
            SOAEntry *se = (SOAEntry *) iter();

            // This SE is for buffering
            if( se->getBuffer() and se->getBufferDirection() )
                continue;

            // Same entry as I want to use .. probably is here something to test.. overlap?
            if( e->getOutPort() == se->getOutPort() and e->getOutLambda() == se->getOutLambda()){

                if( se->getStart() - d_s > start and se->getStart() - d_s >= stop ){
                    continue;
                }

                if( se->getStop() + d_s <= start and se->getStop() + d_s < stop ){
                    continue;
                }

                EV << " already used time slot !!! ("<<se->info()<<")" << endl;
                wrong_scheduling++;
                EV << "****** Wrong scheduling" << endl;
                EV << e->info() << endl << se->info() <<endl;
                EV << "Output configuration " <<se->getOutPort() << "#" << se->getOutLambda() << endl;
                opp_terminate("Wrong scheduling");
                return;
            }
        }
    }
    EV << endl;

    /***
     * Count the number of required
     *
     */
    int oe = 0, mf = 0;
    for (cQueue::Iterator iter(switchingTable, 0); !iter.end(); iter++) {
        SOAEntry *se = (SOAEntry *) iter();

        // Check buffering instructions only
        if (not (se->getBuffer() and se->getBufferDirection()))
            continue;

        // Check whether instructions overlap in time
        if (se->getStart() < e->getStart() and se->getStop() + d_s < e->getStart()) {
            // 1.
            //  ........|----|..... sw
            //  .|----|............ se
            continue;
        }

        if (se->getStop() > e->getStop() and se->getStart() - d_s >= e->getStop()) {
            // 2.
            //  .|----|............ sw
            //  ........|----|..... se
            continue;
        }

        // Overlapping SE
        oe++;
    }

    LOE.record(oe);

    // Block adding more buffering SOAentries
    if( oe < oe_limit){

        // Reserve WL if it is not to buffer direction
        switchingTable.insert(e);
    }
    else{
        //opp_terminate("blokuji");
    }

}

void SOA::assignSwitchingTableEntry(cObject *e, simtime_t ot, simtime_t len) {

    if( d_s + ot < 0 ){
        bigOT++;
        EV << "Car train reached the Header - unable to set switching matrix";
        EV << " lacking="<< d_s+ot <<"s"<< endl;
        return;
    }

    Enter_Method("assignSwitchingTableEntry()");
    cMessage *amsg = new cMessage("ActivateSTE");
    amsg->addPar("ActivateSwitchingTableEntry");
    amsg->par("ActivateSwitchingTableEntry") = e;
    amsg->setSchedulingPriority(5);
    scheduleAt(simTime() + d_s + ot, amsg);

    // Scheduling to drop SwitchingTableEntry
    cMessage *amsg2 = new cMessage("DeactivateSTE");
    amsg2->addPar("DeactivateSwitchingTableEntry");
    amsg2->par("DeactivateSwitchingTableEntry") = e;
    scheduleAt(((SOAEntry *)e)->getStop(), amsg2);
}

void SOA::dropSwitchingTableEntry(SOAEntry *e) {
    Enter_Method("dropSwitchingTableEntry()");

    // Drop Entry from SOAmanager
    cModule *calleeModule = getParentModule()->getSubmodule("soaManager");
    SOAManager *sm = check_and_cast<SOAManager *>(calleeModule);
    sm->dropSwitchingTableEntry(e);

    // Drop Entry from MAC if it was used
    if( e->getBuffer() and ! e->getBufferDirection() ){
        calleeModule = getParentModule()->getSubmodule("MAC");
        CoreNodeMAC *mac = check_and_cast<CoreNodeMAC *>(calleeModule);
        mac->removeCar(e);
    }

    EV << " DROP " << e->info() << endl;

    // Reserve WL if it is not buffered
    delete switchingTable.remove(e);
}

SOAEntry * SOA::findOutput(int inPort, int inWl) {

    for( cQueue::Iterator iter(switchingTable,0); !iter.end(); iter++){
        SOAEntry *se = (SOAEntry *) iter();

        // It searches proper port#wl combination and the combination is not out-of-buffer records
        if (se->getInPort() == inPort and se->getInLambda() == inWl ){
            if( se->getBuffer() and not se->getBufferDirection()) continue;
            return se;
        }
    }

    // Nothing found.. butst is going to be lost
    return new SOAEntry(-1, -1, -1, -1);
}


void SOA::finish() {
    /* Performance statistics */
    recordScalar("Total switched bursts", incm);    // Does not mean they are not dropped
    recordScalar("Total dropped bursts", drpd);
    recordScalar("Total buffered bursts", buff);
    recordScalar("Wavelength conversion used", wcs);

    /* Monitoring statistics */
    if(incm>0){
        recordScalar("Loss probability", (double) drpd/incm );
        recordScalar("Buffering probability", (double) buff/incm );
    }
    if(bigOT>0)recordScalar(" ! Train reached header", bigOT );
    if(wrong_scheduling>0) recordScalar(" ! Not enough time for switching", wrong_scheduling);
    countProbabilities();
}


void SOA::countProbabilities(){
    /* Function that is used for evaluation of probabilities during the simulation */

    if(bbp_total > 0){
        // BLP
        BLP.record( (double)bbp_dropped/bbp_total );

        // BBP
        BBP.record( (double)bbp_buffered/bbp_total );

        // BOKP
        BOKP.record( (double)bbp_switched/bbp_total );

        // Disaggregace
        BDISP.record( (double)bbp_disagg/bbp_total );

        // Backkground of the number of stuff :)
        BTOTAL.record(bbp_total);
    }

    // Erease for another counting
    bbp_switched= 0;
    bbp_buffered= 0;
    bbp_dropped=0;
    bbp_total=0;
}
