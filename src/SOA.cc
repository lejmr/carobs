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

Define_Module(SOA);

void SOA::initialize() {

    //  Initialisation of switching time
    d_s = par("d_s").doubleValue();

    // Initialisation of WC variable
    WC = par("WC").boolValue();

    //switchingTable = new cQueue("switchingTable");
    switchingTable = new cArray("switchingTable");
}

void SOA::handleMessage(cMessage *msg) {

    if (msg->isSelfMessage() and msg->hasPar("ActivateSwitchingTableEntry")) {
        /*  Self-message for adding of switching entry */
        SOAEntry *tse =
                (SOAEntry *) msg->par("ActivateSwitchingTableEntry").pointerValue();

        // Add SwitchingTableEntry
        addpSwitchingTableEntry(tse);
        delete msg; return;
    }

    if (msg->isSelfMessage() and msg->hasPar("DeactivateSwitchingTableEntry")) {
        /*  Self-message for destruction of switching entry */
        SOAEntry *tse = (SOAEntry *) msg->par("DeactivateSwitchingTableEntry").pointerValue();
        dropSwitchingTableEntry(tse);
        delete msg;
        return;
    }

    if( !strcmp(msg->getArrivalGate()->getName(), "gate$i") ){
        /*  Ordinary Cars which are going to be transfered or disaggregated  */

        // Obtain informations about optical signal -- wavelength and incoming port
        OpticalLayer *ol = dynamic_cast<OpticalLayer *>(msg);
        int inPort = msg->getArrivalGate()->getIndex();
        int inWl = ol->getWavelengthNo();

        // Find output configuration for incoming burst
        SOAEntry *sw = findOutput(inPort, inWl);

        /*      Disaggregation     */
        if( sw->isDisaggregation() ){
            EV << " Disaggregation " << endl;
            send(ol,"disaggregation");
            return ;
        }

        // Not disaggregated thus only switched out onto exit
        EV << "Burst is to be switched from " << sw->info() << endl;

        int outPort = sw->getOutPort();
        int outWl = sw->getOutLambda();
        // Chceck whether exit is real or faked one
        if (outPort >= 0 and outWl >= 0) {
            // Wavelength conversion
            ol->setWavelengthNo(sw->getOutLambda());

            // Sending to output port
            send(ol, "gate$o", sw->getOutPort());
        }
    }

    if( !strcmp(msg->getArrivalGate()->getName(), "aggregation$i") ){
        /*      Cars coming from aggregation port    */
        OpticalLayer *ol = dynamic_cast<OpticalLayer *>(msg);
        int inPort = msg->getArrivalGate()->getIndex();
        if (inPort >= 0 ) {
            send(ol, "gate$o", inPort );
        }
    }
    // TODO: take statistics because car is to be dropped
    //delete msg;
}

void SOA::addpSwitchingTableEntry(SOAEntry *e){
    EV << " ADD " << e->info() << endl;
    bool add= true;
    for (int i = 0; i < switchingTable->size(); i++) {
        SOAEntry *se = (SOAEntry *) switchingTable->get(i);
        if (se->getOutPort() == e->getOutPort() and se->getInLambda() == e->getInLambda()){
            add= false;
            break;
        }
    }


    switchingTable->add(e);
}

void SOA::assignSwitchingTableEntry(cObject *e, simtime_t ot, simtime_t len) {
    Enter_Method("assignSwitchingTableEntry()");
    cMessage *amsg = new cMessage("ActivateSTE");
    amsg->addPar("ActivateSwitchingTableEntry");
    amsg->par("ActivateSwitchingTableEntry") = e;
    scheduleAt(simTime() + d_s + ot, amsg);

    // Scheduling to drop SwitchingTableEntry
    cMessage *amsg2 = new cMessage("DeactivateSTE");
    amsg2->addPar("DeactivateSwitchingTableEntry");
    amsg2->par("DeactivateSwitchingTableEntry") = e;
    scheduleAt(simTime() + d_s + ot + len, amsg2);
}

void SOA::dropSwitchingTableEntry(SOAEntry *e) {
    Enter_Method("dropSwitchingTableEntry()");

    cModule *calleeModule = getParentModule()->getSubmodule("soaManager");
    SOAManager *sm = check_and_cast<SOAManager *>(calleeModule);
    sm->dropSwitchingTableEntry(e);

    EV << " DROP " << e->info() << endl;
    delete switchingTable->remove(e);
}

SOAEntry * SOA::findOutput(int inPort, int inWl) {
    for (int i = 0; i < switchingTable->size(); i++) {
        SOAEntry *se = (SOAEntry *) switchingTable->get(i);
        if (se->getInPort() == inPort and se->getInLambda() == inWl )
            return se;
    }

    // Nothing found.. butst is going to be lost
    return new SOAEntry(0, 1, -1, -1);
}
