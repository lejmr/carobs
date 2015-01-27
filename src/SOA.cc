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
    OE.setName("Number of O/E blocks [-]");
    OEbuf.setName("Number of O/E blocks used for buffering [-]");
    OEdis.setName("Number of O/E blocks used for disaggregation [-]");
    EO.setName("Number of used E/O blocks [-]");

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
        self_act.erase( (cObject *) msg->par("ActivateSwitchingTableEntry").pointerValue() );
        delete msg; return;
    }

    if (msg->isSelfMessage() and msg->hasPar("DeactivateSwitchingTableEntry")) {
        /*  Self-message for destruction of switching entry */
        SOAEntry *tse = (SOAEntry *) msg->par("DeactivateSwitchingTableEntry").pointerValue();
        dropSwitchingTableEntry(tse);
        self_dea.erase( (cObject *) msg->par("DeactivateSwitchingTableEntry").pointerValue() );
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

        /*      Disaggregation     */
        if( sw->isDisaggregation() ){
            EV << "Disaggregated based on: " << sw->info() << endl;
            send(ol,"disaggregation");
            return ;
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
            simtime_t ot= ol->par("ot").doubleValue();
            ol->par("ot").setDoubleValue( ( ot - (simtime_t)getParentModule()->par("d_p").doubleValue() ).dbl() );

            if ( ot < 0 ){
                EV << "ERROR: OT=" << ot << endl;
                opp_terminate("ASi tu mame smycku");
            }

            // Monitor bypass into CAR section
            Car *car= (Car *)ol->getEncapsulatedPacket();
            car->setBypass( car->getBypass() + 1 );

            // Sending to output port
            send(ol, "gate$o", sw->getOutPort());
        }else{
            EV << "Burst at "<<inPort<<"#"<<inWl<<" is to be dropped !!!" << endl;
            drpd++;
            blpevo.record( (double) drpd/incm );
            delete msg; return;
        }


    }

    if( !strcmp(msg->getArrivalGate()->getName(), "aggregation$i") ){
        /*      Cars coming from aggregation port    */
        OpticalLayer *ol = dynamic_cast<OpticalLayer *>(msg);
        int inPort = msg->getArrivalGate()->getIndex();


        if( ol->hasPar("marker") ){
            /** Car that was buffered and now is put back onto the optical network */

            // Check correctness of the un-buffering scheduling
            SOAEntry *se = (SOAEntry *) ol->par("marker").pointerValue();
            if( not switchingTable.contains( se )  ){
                EV << "NOT IN: " << se->info() << endl;
                opp_terminate("Un-buffering SOAEntry is missing !!! ");
            }

            // Send to the next node
            EV << "Un-buffered through port " << inPort << ": "<< se->info() << endl;
            send(ol, "gate$o", inPort );

        } else {
            /** Just and aggregation from adjacent network */

            // Get the identifier
            int ident = ol->par("SOAEntry_identifier").longValue();
            if (ident < 100) opp_terminate("Wrong SOAEntry identifier");

            // Verify it is correct scheduling
            bool found = false;
            SOAEntry *se;
            for (cQueue::Iterator iter(switchingTable, 0); !iter.end(); iter++) {
                se = (SOAEntry *) iter();
                EV << "SOA: " << se->info() << endl;

                if (se->identifier == ident) {
                    found = true;
                    break;
                }
            }
            if (not found) opp_terminate("Aggregate without the rule is set !!! ");

            // It is correct so it can be forwarded
            double ot = ol->par("ot").doubleValue();
            ol->par("ot").setDoubleValue(ot-getParentModule()->par("d_p").doubleValue());
            if (ot < 0) opp_terminate("ASi tu mame smycku v agregovani");

            // Send to the next node
            EV << "Aggreagating traffic through port" << inPort << ": "<< se->info() << endl;
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
                EV << "New: " << e->info() << endl << "Set: " << se->info() <<endl;
                EV << "Output configuration " <<se->getOutPort() << "#" << se->getOutLambda() << endl;
                opp_terminate("Wrong scheduling");
                return;
            }
        }
    }
    EV << endl;

    // Reserve WL if it is not to buffer direction
    switchingTable.insert(e);

    // Count the number of O/E blocks that are necessary
    int oe_total = 0;
    int oe_disagg = 0;
    int oe_buffer = 0;

    for( cQueue::Iterator iter(switchingTable,0); !iter.end(); iter++){
        SOAEntry *se = (SOAEntry *) iter();

        // Count dissaggregation
        if( se->isDisaggregation() ){
            oe_total++;
            oe_disagg++;
        }

        // Count buffering
        if( se->getBuffer() and se->getBufferDirection() ){
            oe_total++;
            oe_buffer++;
        }
    }

    OEbuf.record(oe_buffer);
    OEdis.record(oe_disagg);
    OE.record(oe_total);
}

void SOA::assignSwitchingTableEntry(cObject *e, simtime_t ot, simtime_t len) {

    if( d_s + ot < 0 ){
        bigOT++;
        EV << "Car train reached the Header - unable to set switching matrix";
        EV << " lacking="<< d_s+ot <<"s"<< endl;
        return;
    }

    EV << "Assign SOA: "<< ( (SOAEntry *) e )->info() << endl;

    Enter_Method("assignSwitchingTableEntry()");
    cMessage *amsg = new cMessage("ActivateSTE");
    amsg->addPar("ActivateSwitchingTableEntry");
    amsg->par("ActivateSwitchingTableEntry") = e;
    amsg->setSchedulingPriority(5);
    scheduleAt(simTime() + ot, amsg);
    self_act[ e ]= amsg;

    // Scheduling to drop SwitchingTableEntry
    cMessage *amsg2 = new cMessage("DeactivateSTE");
    amsg2->addPar("DeactivateSwitchingTableEntry");
    amsg2->par("DeactivateSwitchingTableEntry") = e;
    scheduleAt(((SOAEntry *)e)->getStop(), amsg2);
    self_dea[ e ]= amsg2;
}

void SOA::delaySwitchingTableEntry(cObject *e, simtime_t time){
    Enter_Method("delaySwitchingTableEntry()");

    if( self_act.find( e ) == self_act.end() ) opp_terminate("ACT: Switching entry already installed !!! ");
    if( self_dea.find( e ) == self_dea.end() ) opp_terminate("DEA: Switching entry already installed !!! ");

    // Delay it
    cMessage *msg_act= (cMessage *) self_act[ e ];
    cMessage *msg_dea= (cMessage *) self_dea[ e ];

    simtime_t arr_act= ( msg_act )->getArrivalTime();
    simtime_t arr_dea= ( msg_dea )->getArrivalTime();

    cancelEvent( msg_act );
    cancelEvent( msg_dea );

    scheduleAt(arr_act + time , msg_act);
    scheduleAt(arr_dea + time , msg_dea);

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
}
