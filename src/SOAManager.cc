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

#include "SOAManager.h"
#include "CoreNodeMAC.h"

Define_Module(SOAManager);

void SOAManager::initialize() {
    // Obtaining processing time parametr d_p
    d_p = par("d_p").doubleValue();
    d_s = par("d_s").doubleValue();

    // reading parameters
    maxWL = par("maxWL").longValue();
    WC = par("WC").boolValue();

    // Determin OBS/CAROBS mode
    OBS = getParentModule()->gateSize("tributary") == 0;

    // Making link with Routing module
    cModule *calleeModule = getParentModule()->getSubmodule("routing");
    R = check_and_cast<Routing *>(calleeModule);

    // Address of this CoreNode
    address = getParentModule()->par("address").longValue();

    // Making link with SOA module
    calleeModule = getParentModule()->getSubmodule("soa");
    soa = check_and_cast<SOA *>(calleeModule);

    // Hard-coded datarte
    C = par("datarate").doubleValue();

    WATCH(OBS);

}

void SOAManager::handleMessage(cMessage *msg) {
    // Obtaining information about source port
    int inPort = msg->getArrivalGate()->getIndex();

    // Behaves as full-featured or not CoreNode
    if (OBS) obsBehaviour(msg, inPort);
    else carobsBehaviour(msg, inPort);

}

void SOAManager::carobsBehaviour(cMessage *msg, int inPort) {
    EV << "> CAROBS Behaviour" << endl;


    // Obtaining optical signal
    OpticalLayer *ol = dynamic_cast<OpticalLayer *>(msg);

    // Detection of optical signal -> Electrical CARBOS Header
    CAROBSHeader *H = (CAROBSHeader *) ol->decapsulate();

    /* Aggregation - Data coming from aggregation port */
    if (!strcmp(msg->getArrivalGate()->getName(), "aggregation")) {
        EV << " ! Aggregation process" << endl;
        int inWl = H->getWL();
        int dst = H->getDst();
        int outPort = R->getOutputPort(dst);
        simtime_t tmpOT = H->getOT();
        H->setOT(H->getOT() - d_p);
        SOAEntry *se = new SOAEntry(outPort, inWl, true);

        se->setStart(simTime() + H->getOT() - d_s);
        se->setStop(simTime() + H->getOT() + H->getLength() - d_s);

        soa->assignSwitchingTableEntry(se, tmpOT - d_s, H->getLength());
        scheduling.add(se);

        ol->encapsulate(H);
        if (R->canForwardHeader(H->getDst())) {
            // Resending CAROBS Header to next CoreNode
            sendDelayed(ol, d_p, "control$o", outPort);
        }

        return;
    }


    /*  Disaggregation - This burst train is destined here   */
    EV << " ! Disaggregation process " << endl;
    int inWl = H->getWL();
    int dst = H->getDst();

    // Car train reached its termination Node -> Disaggregation
    if (H->getDst() == address) {
        SOAEntry *se = new SOAEntry(inPort, inWl, false);
        se->setStart(simTime() + H->getOT() - d_s);
        se->setStop(simTime() + H->getOT() + H->getLength() - d_s);
        // Add Switching Entry to SOA a SOAManager scheduler
        soa->assignSwitchingTableEntry(se, H->getOT() - d_s, H->getLength());
        scheduling.add(se);
        // And that is it, nothing more to do
        return;
    }

    // This node is not termination one, so we must check whether we need to disaggreate something
    int NoToDis = 0;
    cQueue cars = H->getCars();
    EV << "cars we have with OT="<<H->getOT()<<": " <<endl;
    for (int i = 0; i < cars.length(); i++) {
        CAROBSCarHeader *tmpc = (CAROBSCarHeader *) cars.get(i);
        EV << " * car=" << tmpc->getDst();
        EV << " start=" << simTime() + H->getOT() + tmpc->getD_c();
        EV << " stop=" << (simtime_t) tmpc->getSize()*8/C + simTime() + H->getOT() + tmpc->getD_c();
        EV << " length="<< (simtime_t) tmpc->getSize()*8/C;
        EV << " d_c=" << tmpc->getD_c();

        if (tmpc->getDst() == address) {
            EV << " - disaggregate";
            NoToDis = i;
        }
        EV << endl;
    }

    // Whole car-train times
    simtime_t train_start = simTime() + H->getOT();
    simtime_t train_stop = simTime() + H->getOT() + H->getLength();
    simtime_t train_length = H->getLength();

    if ( NoToDis + 1 > 0 and NoToDis + 1 < H->getN() ) {
        // Count times for car-train
        CAROBSCarHeader *tmpc = (CAROBSCarHeader *) cars.get(NoToDis + 1);
        train_start= simTime() + H->getOT() + tmpc->getD_c();
        train_length= train_length - tmpc->getD_c();
        EV << "disaggregate start="<<simTime() + H->getOT()<<" stop="<<train_start-d_s<<" length="<<tmpc->getD_c()-d_s<<endl;

        // Remove CAR Header from CAROBS Header of this disaggregated car
        CAROBSCarHeader *tmpc2 = (CAROBSCarHeader *) cars.get(NoToDis);
        delete cars.remove(tmpc2);
        H->setN( H->getN()-1 );

        // Create disaggregation SOA instructions
        SOAEntry *se = new SOAEntry(inPort, inWl, false);   // Disaggregation SOAEntry => false
        se->setStart(simTime()+H->getOT());
        se->setStop(train_start-d_s);
        // Add Switching Entry to SOA a SOAManager scheduler
        soa->assignSwitchingTableEntry(se, H->getOT()-d_s, tmpc->getD_c()-d_s);
        scheduling.add(se);
        //TODO: inform AQ that there is a time window  //simTime()+H->getOT()-d_s// to //c_n_start//
    }
    EV << "train start="<<train_start<<" stop="<<train_stop<<" length="<<train_length<<endl;

    // 2nd half of the burst train
    int outPort = R->getOutputPort(dst);

    // Assign this SOAEntry to SOA
    SOAEntry *sef = getOptimalOutput(outPort, inPort, inWl, train_start, train_stop );
    sef->setStart(train_start);
    sef->setStop(train_stop);

    // Add Switching Entry to SOA a SOAManager scheduler
    soa->assignSwitchingTableEntry(sef, train_start-simTime()-d_s, train_length );

    // Update of OT for continuing car train
    H->setOT(H->getOT() - d_p);

    // Encapsulate the CAROBS Header back to OpticalLayer and send further
    ol->encapsulate(H);

    // Test whether this is last CoreNode on the path is so CAROBS Header is not passed towards
    if (R->canForwardHeader(H->getDst())) {
        // Resending CAROBS Header to next CoreNode
        sendDelayed(ol, d_p, "control$o", outPort);
    }

    return;
}

void SOAManager::obsBehaviour(cMessage *msg, int inPort) {
    EV << "> OBS Behaviour" << endl;
    // Obtaining optical signal
    OpticalLayer *ol = dynamic_cast<OpticalLayer *>(msg);
    // Detection of optical signal -> Electrical CARBOS Header
    CAROBSHeader *H = (CAROBSHeader *) ol->decapsulate();

    // Obtaining information about wavelength of incoming Car train
    int inWl = H->getWL();

    // Obtaining information about Car train destination
    int dst = H->getDst();

    // Resolving output port
    int outPort = R->getOutputPort(dst);

    // Assign this SOAEntry to SOA
    SOAEntry *se = getOptimalOutput(outPort, inPort, inWl, simTime()+H->getOT(), simTime()+H->getOT()+H->getLength() );
    simtime_t d_p_extra=0;

    if( se->getBuffer() ){
        simtime_t SOA_set= simTime()+H->getOT();
        simtime_t SOA_nset= se->getStart();
        d_p_extra= SOA_nset-SOA_set;

        EV << "Extra cekani t="<<d_p_extra;
        EV << " inPort="<<inPort;
        EV << " inWL="<<inWl;
        EV << " outPort="<<outPort;
        EV << " outWL="<<se->getOutLambda();
        EV << endl;

        cModule *calleeModule = getParentModule()->getSubmodule("MAC");
        CoreNodeMAC *mac = check_and_cast<CoreNodeMAC *>(calleeModule);
        mac->storeCar(se);
        // Comunication SOAmanager -> MAC pass the waiting time

    }

    soa->assignSwitchingTableEntry(se, H->getOT()+d_p_extra-d_s, H->getLength());

    // Update WL in case of WC
    H->setWL( se->getOutLambda() );

    // Put the headers  back to OpticalLayer E-O conversion
    ol->encapsulate(H);
    H->setOT(H->getOT() - d_p + d_p_extra );
    // Test whether this is last CoreNode on the path is so CAROBS Header is not passed towards
    if (R->canForwardHeader(H->getDst())) {
        // Resending CAROBS Header to next CoreNode
        sendDelayed(ol, d_p, "control$o", outPort);
    }
}

SOAEntry* SOAManager::getOptimalOutput(int outPort, int inPort, int inWL, simtime_t start, simtime_t stop) {
    EV << "Get scheduling for " << inPort << "#" << inWL << " to port=" << outPort << " for:" << start << "-" << stop;

    if (scheduling.size() == 0) {
        // Fast forward - if there is no scheduling .. do bypas
        SOAEntry *e = new SOAEntry(inPort, inWL, outPort, inWL);
        e->setStart(start);
        e->setStop(stop);

        // And add it to scheduling table
        scheduling.add(e);
        EV << " outWL=" << inWL << endl;
        return e;
    }

    // Test output combination
    if (testOutputCombination(outPort, inWL, start, stop)) {
        // There is no blocking of output port at a  given time
        SOAEntry *e = new SOAEntry(inPort, inWL, outPort, inWL);
        e->setStart(start);
        e->setStop(stop);
        // And add it to scheduling table
        scheduling.add(e);
        EV << " outWL=" << inWL << endl;
        return e;
    }

    if (WC) {
        /* Perform wavelength conversion */
        // FIFO approach
        for (int i = 1; i <= maxWL; i++) {
            if (testOutputCombination(outPort, i, start, stop)) {
                // Wavelength i is free at the given time, we can use it
                SOAEntry *e = new SOAEntry(inPort, inWL, outPort, i);
                e->setStart(start);
                e->setStop(stop);
                // And add it to scheduling table
                scheduling.add(e);
                EV << " WC->outWL=" << i << endl;
                return e;
            }
        }

    }

    /**
     *  This part is reached because of full usage of all lambdas in the outPort.
     *  So the approach here is to find output combination with delayed time such
     *  that the delay is minimal. Car-train is meanwhile stored in MAC buffers
     *  Solution:
     *      1. Try to find least output time for given WL
     *      2. If WC is enabled find the least output time at any WL
     *          1. If enabled the result can be dropped if it was to expensive ..
     *             through configuration of simulation
     */

    std::map<int, simtime_t> times;
    // ! NO WC
    // table row: outputPort & outputWL -> time the outputWL is ready to be used again
    for (int i = 0; i < scheduling.size(); i++) {
        SOAEntry *tmp = (SOAEntry *) scheduling[i];

        // Do the find only for One output port OutPort
        if( tmp->getOutPort() == outPort ){
            // test whether table times contains such outputWL .. unless fix it
            if( times.find( tmp->getOutLambda() ) == times.end() ){
                times[tmp->getOutLambda()]= tmp->getStop();
                continue;
            }

            // Update outputWL timing if it is not up-to-date
            if( times[tmp->getOutLambda()] < tmp->getStop() ){
                times[tmp->getOutLambda()]= tmp->getStop();
            }
        }
    }

    if( times.find(inWL) == times.end() ){
        EV << " NO BUFFER outWL=" << inWL << endl;
        EV << " ! Strange situation - it seems there is no scheduling for outPort="<<outPort<<"#"<<inWL<<" do not have to buffer it !"<<endl;
        SOAEntry *e = new SOAEntry(inPort, inWL, outPort, inWL);
        e->setStart(start);
        e->setStop(stop);
        scheduling.add(e);
        return e;
    }

    //  Time to stay in buffer
    simtime_t BT= times[inWL]-start;
    int outWL = inWL;
    // Check WC options
    if( WC ){
        simtime_t tmpBT = BT;
        int betterWL = inWL;
        for(int i=1; i<maxWL;i++){
            if( i==inWL) continue;
            if (times.find(i) == times.end()) {
                EV << " NO BUFFER WC->outWL=" << i << endl;
                EV<<" ! Strange situation - it seems there is no scheduling for outPort="<<outPort<<"#"<<i<<" do not have to buffer it !" << endl;

                // Set the car-train to buffer
                SOAEntry *e = new SOAEntry(inPort, inWL, outPort, inWL);
                e->setStart(start);
                e->setStop(stop);
                return e;
            }

            simtime_t tmpBTloop = times[i]-start;
            if( tmpBTloop < tmpBT ){
                // There is a candidate but WC is needed
                tmpBT= tmpBTloop;
                betterWL= i;
            }
        }

        outWL= betterWL;
        BT= tmpBT;

        EV << " BUFFERED="<<tmpBT<<" WC->outWL=" << outWL << endl;
        //  Return the record important for outgoing car-train from buffer - this
        //  way SOAManager can assume the buffering time
    }

    // Record which sends incoming car-train to buffer
    SOAEntry *e_in = new SOAEntry(inPort, inWL, outPort, outWL);
    e_in->setStart(start);
    e_in->setStop(stop);
    e_in->setBuffer(true);
    e_in->setInBuffer();
    scheduling.add(e_in);
    soa->assignSwitchingTableEntry(e_in, start-simTime()-d_s, stop - start);

    // Withdraw car-train from buffer and reserver output port
    SOAEntry *e_out = new SOAEntry(inPort, inWL, outPort, outWL);
    e_out->setStart(start + BT);
    e_out->setStop(stop + BT);
    e_out->setBuffer(true);
    e_out->setOutBuffer();
    scheduling.add(e_out);

    //  Buffered result
    if(not WC) EV << " BUFFERED=" << BT << " outWL=" << inWL << endl;

    //  Return the record important for outgoing car-train from buffer - this
    //  way SOAManager can assume the buffering time
    return e_out;
}

bool SOAManager::testOutputCombination(int outPort, int outWL, simtime_t start, simtime_t stop) {
    for (int i = 0; i < scheduling.size(); i++) {
        SOAEntry *tmp = (SOAEntry *) scheduling[i];

        if (tmp->getOutLambda() == outWL and tmp->getOutPort() == outPort) {
            // There is some scheduling for my output combination, lets see whether it is overlapping
            if (start < tmp->getStart() and stop > tmp->getStart()) {
                // Overlap start time
                //   in table:       |-------|
                //the new one:    |----|
                return false;
            }

            if (start < tmp->getStop() and stop > tmp->getStart()) {
                // inside of it .. body overlap
                //   in table:       |-------|
                //the new one:         |----|
                return false;
            }

            if (start < tmp->getStop() and stop > tmp->getStop()) {
                // overlap stop time
                //   in table:       |-------|
                //the new one:            |----|
                return false;
            }

            return true;
        }
    }
    return true;
}

void SOAManager::dropSwitchingTableEntry(SOAEntry *e) {
    Enter_Method("dropSwitchingTableEntry()");
    scheduling.remove(e);
}

simtime_t SOAManager::getAggregationWaitingTime(int destination, simtime_t OT, int &WL, int &outPort) {

    outPort = R->getOutputPort(destination);
    EV << "Dotazuji se na cil " << destination << " port=" << outPort << ":" << R->getOutputPort(destination) << endl;

    WL = 5;

    return 2;
}
