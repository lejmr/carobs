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
    buffering= par("buffering").boolValue();

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

    // Hard-coded datarate ...
    // TODO: fix and make in more scalable from d_s point of view .. this constant assumes 10e-9
    C = par("datarate").doubleValue() * par("datarate_correction").doubleValue();
    tbdropped=0;

    // W election method
    fifo= par("fifo").boolValue();
    prioritizeBuffered= par("prioritizeBuffered").boolValue();

    // Speed of O/E conversion
    convPerformance=par("convPerformance").doubleValue();

    // Naming helper container
    inSplitTable.setName("Items managed by split tables");
    scheduling.setName("Scheduling table");

    // Merging Flows counters
    inMFcounter.setName("Merging Flows");

    // Buffering statistics
    bbp_switched= 0;
    bbp_buffered= 0;
    bbp_dropped=0;
    bbp_total=0;
    bbp_interval_max=100;
    BBP.setName("Burst buffering probability");
    BLP.setName("Burst loss probability");
    BOKP.setName("Burst cut-throught probability");
    BTOTAL.setName("Number of bursts in statistics");

    WATCH(OBS);
    WATCH_MAP(mf_max);
    WATCH_MAP(reg_max);
}

bool myfunction (SOAEntry *i, SOAEntry *j) { return (i->getStart()>j->getStart()); }

void SOAManager::countProbabilities(){
    /* Function that is used for evaluation of probabilities during the simulation */

    if(bbp_total > 0){
        // BLP
        BLP.record( (double)bbp_dropped/bbp_total );

        // BBP
        BBP.record( (double)bbp_buffered/bbp_total );

        // BOKP
        BOKP.record( (double)bbp_switched/bbp_total );

        // Backkground of the number of stuff :)
        BTOTAL.record(bbp_total);
    }

    // Erease for another counting
    bbp_switched= 0;
    bbp_buffered= 0;
    bbp_dropped=0;
    bbp_total=0;
}

void SOAManager::handleMessage(cMessage *msg) {
    // Obtaining information about source port
    int inPort = msg->getArrivalGate()->getIndex();

    // Obtaining optical signal
    OpticalLayer *ol = dynamic_cast<OpticalLayer *>(msg);

    if( dynamic_cast<CAROBSHeader *>(ol->getEncapsulatedPacket()) == NULL ){
        opp_error("This is not a CAROBS Header !!! ");
    }

    /* Aggregation - Data coming from aggregation port */
    if (!strcmp(msg->getArrivalGate()->getName(), "aggregation")) {

        // Detection of optical signal -> Electrical CARBOS Header
        CAROBSHeader *H = (CAROBSHeader *) ol->getEncapsulatedPacket();

        EV << " ! Aggregation process";
        int dst = H->getDst();

        // Obtain output port
        int outPort = R->getOutputPort(dst);

        EV << " to " << outPort<<"#"<<H->getWL() << endl;
        EV << "cars we have with OT=" << H->getOT() << ": " << endl;
        cQueue cars = H->getCars();
        for( cQueue::Iterator iter(cars,0); !iter.end(); iter++){
            CAROBSCarHeader *tmpc = (CAROBSCarHeader *) iter();
            simtime_t tmplen= (simtime_t) ((double) tmpc->getSize() * 8 / C);
            EV << " * car=" << tmpc->getDst();
            EV << " start=" << simTime() + H->getOT() + tmpc->getD_c();
            EV << " stop="  << simTime() + H->getOT() + tmpc->getD_c() + tmplen;
            EV << " length=" << tmplen;
            EV << " d_c=" << tmpc->getD_c();
            EV << endl;
        }

        H->setOT(H->getOT() - d_p);
        if (R->canForwardHeader(H->getDst())) {
            // Resending CAROBS Header to next CoreNode
            sendDelayed(ol, d_p, "control$o", outPort);
        }

        return;
    }

    // Behaves as full-featured or not CoreNode
    bbp_total++;
    if (OBS)
        obsBehaviour(msg, inPort);
    else
        carobsBehaviour(msg, inPort);

    // When there is enough data calculate statistics
    if( bbp_total >= bbp_interval_max ){
        countProbabilities();
    }

}

void SOAManager::carobsBehaviour(cMessage *msg, int inPort) {
    EV << " > CAROBS Behaviour" << endl;

    // Obtaining optical signal
    OpticalLayer *ol = dynamic_cast<OpticalLayer *>(msg);

    // Detection of optical signal -> Electrical CARBOS Header
    CAROBSHeader *H = (CAROBSHeader *) ol->getEncapsulatedMsg();

    /*  Disaggregation - This burst train is destined here   */
    int inWl = H->getWL();
    int dst = H->getDst();
    cQueue cars = H->getCars();

    // Car train reached its termination Node -> Disaggregation
    if (H->getDst() == address) {
        cQueue::Iterator iter(cars,0);
        CAROBSCarHeader *first_car = (CAROBSCarHeader *) iter();
        SOAEntry *se = new SOAEntry(inPort, inWl, false);
        se->setStart(simTime() + H->getOT() );
        se->setStop(simTime() + H->getOT()+H->getLength());
        // Add Switching Entry to SOA a SOAManager scheduler
        EV << " Car train has reached it destination"<<endl;
        EV << " Dissagregation: " << se->info() << endl;

        soa->assignSwitchingTableEntry(se, H->getOT()-d_s, H->getLength());
        addSwitchingTableEntry(se);
        // And that is it, nothing more to do
        delete msg;
        return;
    }

    // This node is not terminating one, so we must check whether we need to disaggreate somthing or not
    int NoToDis = -1;
    int longest_burst_length = 0;
    bool dissaggregation=false;
    EV << "cars we have with OT=" << H->getOT() << ": " << endl;
    int i = 0;
    for( cQueue::Iterator iter(cars,0); !iter.end(); iter++){
        CAROBSCarHeader *tmpc = (CAROBSCarHeader *) iter();
        EV << " * car=" << tmpc->getDst();
        EV << " start=" << simTime() + H->getOT() + tmpc->getD_c();
        EV << " stop=" << (simtime_t) ((double)tmpc->getSize()*8/C) + simTime() + H->getOT() + tmpc->getD_c();
        EV << " length=" << (simtime_t) ((double)tmpc->getSize()*8/C);
        EV << " d_c=" << tmpc->getD_c();

        // Capturing lenght of longest burst .. important for minimum buffering time
        if( tmpc->getSize()*8 > longest_burst_length){
            longest_burst_length= tmpc->getSize()*8;
        }

        if (tmpc->getDst() == address) {
            EV << " - disaggregate";
            NoToDis = i;
            dissaggregation=true;
        }
        i++;
        EV << endl;
    }

    // Whole car-train times
    simtime_t train_start = simTime() + H->getOT();
    simtime_t train_stop = simTime() + H->getOT() + H->getLength();
    simtime_t train_length = H->getLength();

    if (NoToDis>=0 and NoToDis + 1 < H->getN()) {
        // Count times for car-train
        cQueue::Iterator iter(cars,0);
        CAROBSCarHeader *tmpc0 = (CAROBSCarHeader *) iter(); iter++;
        CAROBSCarHeader *tmpc = (CAROBSCarHeader *) iter();
        train_start += tmpc->getD_c();
        train_length-= tmpc->getD_c();

        // Create disaggregation SOA instructions
        SOAEntry *se = new SOAEntry(inPort, inWl, false); // Disaggregation SOAEntry => false
        se->setStart(simTime()+H->getOT());
        se->setStop(train_start - d_s);
        simtime_t len= train_start - d_s - simTime()+H->getOT();

        // Add Switching Entry to SOA a SOAManager scheduler
        soa->assignSwitchingTableEntry(se, H->getOT()- d_s, tmpc->getD_c()-d_s);
        addSwitchingTableEntry(se);
        EV << " Dissagregation: " << se->info() << endl;

        // Remove CAR Header from CAROBS Header of this disaggregated car
        // and update every d_c of cars persisting in the train
        delete cars.remove(tmpc0);


        simtime_t shift= tmpc->getD_c();
        EV << "Zkracuji o " << shift << endl;
        for( cQueue::Iterator iter(cars,0); !iter.end(); iter++){
            CAROBSCarHeader *tc = (CAROBSCarHeader *) iter();
            EV << " - CAR " << tc->getDst() << " d_c="<<tc->getD_c()-shift << endl;
            tc->setD_c( tc->getD_c()-shift );
        }
        EV << "Akutalizuji OT z " << H->getOT();
        H->setOT( H->getOT()+shift);
        EV << " to " << H->getOT() << endl;

        // Update CAROBS Header properties
        H->setN(cars.length());     // Number of carried cars
        H->setCars(cars);
        H->setLength( H->getLength()-shift );   // Length of the train
    }

    // 2nd half of the burst train
    int outPort = R->getOutputPort(dst);

    // Assign this SOAEntry to SOA
    SOAEntry *sef = getOptimalOutput(outPort, inPort, inWl, train_start, train_stop, longest_burst_length);
    EV << " Switching: " << sef->info() << endl;

    // sef==NULL happens only if the buffering is turned off
    if( sef == NULL ){
        // No buffering allowed so the CAROBS Header is to be dropped and processing is stopped
        delete msg;
        bbp_dropped++;
        return;
    }

    simtime_t d_w_extra = 0;
    if (sef->getBuffer()) {
        // Calculate extra waiting time
        simtime_t SOA_nset = sef->getStart();
        d_w_extra = SOA_nset - train_start;

        // Inform the MAC to store and restore Car in a given moment
        cModule *calleeModule = getParentModule()->getSubmodule("MAC");
        CoreNodeMAC *mac = check_and_cast<CoreNodeMAC *>(calleeModule);
        mac->storeCar(sef, d_w_extra);
        //opp_terminate("Bufferuje");
        bbp_buffered++;
    }
    else{
        // Burst is simply switched
        bbp_switched++;
    }


    // Add Switching Entry to SOA a SOAManager scheduler
    sef->setStart(train_start+d_w_extra);
    sef->setStop(train_stop+d_w_extra);
    soa->assignSwitchingTableEntry(sef, d_w_extra+H->getOT()-d_s, train_length);

    // Priorite buffered burst trains for next HOPs
    simtime_t OT_extra= 0;
    simtime_t DELAY_extra= d_w_extra;
    if( prioritizeBuffered ){
        simtime_t OT_extra= d_w_extra;
        simtime_t DELAY_extra= 0;
    }

    // Update of OT for continuing car train
    H->setOT(H->getOT()-d_p+OT_extra);

    // If there was changed wavelength of train Header must carry such information further
    H->setWL(sef->getOutLambda());

    // Encapsulate the CAROBS Header back to OpticalLayer and send further
//    ol->encapsulate(H);

    // Test whether this is last CoreNode on the path is so CAROBS Header is not passed towards
    if (R->canForwardHeader(H->getDst())) {
        // Resending CAROBS Header to next CoreNode
        sendDelayed(ol, d_p+DELAY_extra, "control$o", outPort);
    }else
        delete msg;

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
    SOAEntry *se = getOptimalOutput(outPort, inPort, inWl, simTime() + H->getOT(), simTime() + H->getOT() + H->getLength());
    EV << " Switching: " << se->info() << endl;

    // se==NULL happens only if the buffering is turned off
    if (se == NULL) {
        // No buffering allowed so the CAROBS Header is to be dropped and processing is stopped
        delete msg; delete H;
        bbp_dropped++;
        return;
    }

    simtime_t d_w_extra = 0;
    if (se->getBuffer()) {
        // Calculate extra waiting time
        simtime_t SOA_set = simTime() + H->getOT();
        simtime_t SOA_nset = se->getStart();
        d_w_extra = SOA_nset - SOA_set;

        // Inform the MAC to store and restore Car in a given moment
        cModule *calleeModule = getParentModule()->getSubmodule("MAC");
        CoreNodeMAC *mac = check_and_cast<CoreNodeMAC *>(calleeModule);
        mac->storeCar(se, d_w_extra);
        bbp_buffered++;
    }
    else{
        // Burst is simply switched
        bbp_switched++;
    }

    // Send switching configuration to SOA - with/without extra waiting time in case of buffering
    soa->assignSwitchingTableEntry(se, H->getOT() + d_w_extra - d_s, H->getLength());

    // Update WL in case of WC
    H->setWL(se->getOutLambda());

    //  Update offset time in-order of processing time d_p
    H->setOT(H->getOT() - d_p);

    // Put the headers  back to OpticalLayer E-O conversion
    ol->encapsulate(H);

    // Test whether this is last CoreNode on the path is so CAROBS Header is not passed towards
    if (R->canForwardHeader(H->getDst())) {
        // Resending CAROBS Header to next CoreNode
        sendDelayed(ol, d_p+d_w_extra, "control$o", outPort);
    }else
        delete msg;
}

SOAEntry* SOAManager::getOptimalOutput(int outPort, int inPort, int inWL, simtime_t start, simtime_t stop, int length) {
    EV << "Get scheduling for " << inPort << "#" << inWL << " to port="
              << outPort << " for:" << start << "-" << stop;

    if (scheduling.length() == 0) {
        // Fast forward - if there is no scheduling .. do bypas
        SOAEntry *e = new SOAEntry(inPort, inWL, outPort, inWL);
        e->setStart(start);
        e->setStop(stop);

        // And add it to scheduling table
        addSwitchingTableEntry(e);
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
        addSwitchingTableEntry(e);
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
                addSwitchingTableEntry(e);
                EV << " WC->outWL=" << i << endl;
                return e;
            }
        }

    }

    if( not buffering ){
        // The buffering is turned off so We will take some statistics and finish it
        tbdropped++;
        EV << " DROPPED" << endl;
        return NULL;
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

    // Minum time the burst must stay in buffer .. it is approximation of limit conversio speed
    double min_buffer= convPerformance*(double)length;
    EV << " Min buffer> "<< min_buffer << endl;

    // SOAEntry mapping for sorting
    std::map<int, simtime_t> waiting;
    std::map<int, simtime_t>::iterator it_waiting;
    std::map<int, std::vector<SOAEntry *> > mapa;
    std::map<int, std::vector<SOAEntry *> >::iterator it2;
    std::set<int> free_wl;
    for (int i = 1; i <= maxWL; i++) free_wl.insert(i);
    std::set<int>::iterator wlit;

    for( cQueue::Iterator iter(splitTable[outPort],0); !iter.end(); iter++){
            SOAEntry *tmp = (SOAEntry *) iter();
            mapa[ tmp->getOutLambda() ].push_back( tmp);
    }

    // Sorting function and finding a space
    for (it2=mapa.begin(); it2!=mapa.end(); ++it2){
        //EV << it2->first << " => " << endl;
        //for (std::vector<SOAEntry *>::iterator itv = (it2->second).begin() ; itv != (it2->second).end(); ++itv)
        //    EV << " " << (*itv)->info();
        //EV << endl;

        // Soting
        std::sort ((it2->second).begin(), (it2->second).end(), myfunction);

        // Finding of an useful space
        std::vector<SOAEntry *>::iterator itv = (it2->second).begin();
        simtime_t tmp_start=  (*itv)->getStart();
        simtime_t tmp_stop=  (*itv)->getStop();
        int tmp_wl= (*itv)->getOutLambda();
        bool space_found= false;
        for (itv = (it2->second).begin() ; itv != (it2->second).end(); ++itv){
            simtime_t t_space= (*itv)->getStart() - tmp_stop - 2*d_s;
            simtime_t t_ot = tmp_stop - start;

            if( t_ot >= min_buffer and t_space >= (stop-start) ){
                waiting[tmp_wl]=t_ot;
                space_found= true;
            }

            // Next round
            simtime_t tmp_start= (*itv)->getStart();
            simtime_t tmp_stop=  (*itv)->getStop();
        }

        // There was no space so simply append
        if( ! space_found ){
            waiting[tmp_wl]= tmp_stop - start;
        }
    }

    int outWL= inWL;
    simtime_t BT = 1e6;
    for (it_waiting = waiting.begin(); it_waiting != waiting.end(); ++it_waiting) {
        EV << " WL#" << it_waiting->first << " .. " << it_waiting->second << endl;
        if( it_waiting->second < BT ){
            BT= it_waiting->second;
            outWL= it_waiting->first;
        }
    }

    // Dont forget about inter burst space
    BT += d_s;

    //  Return the record important for outgoing car-train from buffer - this
    //  way SOAManager can assume the buffering time
    // Record which sends incoming car-train to buffer
    SOAEntry *e_in = new SOAEntry(inPort, inWL, outPort, outWL);
    e_in->setStart(start);
    e_in->setStop(stop);
    e_in->setBuffer(true);
    e_in->setInBuffer();
    addSwitchingTableEntry(e_in);
    soa->assignSwitchingTableEntry(e_in, start - simTime() - d_s, stop - start);

    // Withdraw car-train from buffer and reserver output port
    SOAEntry *e_out = new SOAEntry(inPort, inWL, outPort, outWL);
    e_out->setStart(start + BT);
    e_out->setStop(stop + BT);
    e_out->setBuffer(true);
    e_out->setOutBuffer();
    addSwitchingTableEntry(e_out);

    // Bond this two together
    e_out->bound= e_in;
    e_in->bound= e_out;

    //  Buffered result
    EV << " BUFFERED=" << BT << " outWL=" << outWL << endl;
    EV << " Buffering: " << e_in->info() << endl;

    //  Return the record important for outgoing car-train from buffer - this
    //  way SOAManager can assume the buffering time
    return e_out;
}

bool SOAManager::testOutputCombination(int outPort, int outWL, simtime_t start, simtime_t stop) {
    std::set<bool>::iterator it;
    std::set<bool> tests;

    EV << "Visualizce> "<< start<<" - "<< stop << "("<<start+d_s<<"-"<<stop+d_s<<") "<< endl;
    for( cQueue::Iterator iter(splitTable[outPort],0); !iter.end(); iter++){
        SOAEntry *tmp = (SOAEntry *) iter();

        if (tmp->getOutLambda() == outWL and tmp->getOutPort() == outPort) {
            // There is some scheduling for my output combination, lets see whether it is overlapping
            EV << tmp->info() <<" : ";
            if( stop <= tmp->getStart()-d_s and start < tmp->getStart() ){
                // Overlap stop time with respect to d_s
                //   in table:           |-------|
                //the new one:    |----|
                tests.insert(true);
                EV << " start " << endl;
                continue;
            }

            if( start >= tmp->getStop()+d_s and stop > tmp->getStop() ){
                // Overlap start time with respect to d_s
                //   in table:   |-------|
                //the new one:             |----|
                tests.insert(true);
                EV << " stop " << endl;
                continue;
            }

            EV << " both " << endl;
            tests.insert(false);
        }
    }

    // Returns True if here is no conflict on the output combination port#WL
    return tests.find(false) == tests.end();
}

void SOAManager::dropSwitchingTableEntry(SOAEntry *e) {
    Enter_Method("dropSwitchingTableEntry()");

    // Erase res record from split scheduling table
    if( e->getOutPort() > -1  and inSplitTable.length() > 0 and inSplitTable.contains(e) ){

        // Test if this split table exists ..needs to exists otherwise it is incausal
        if( splitTable.find(e->getOutPort()) == splitTable.end() ){
            opp_terminate("Deleting from splittable which does not exist!!!");
        }

        splitTable[e->getOutPort()].remove(e);
        inSplitTable.remove(e);
    }

    // Erase MFcounter
    if (e->getBuffer() and e->getBufferDirection() and inMFcounter.contains(e)) {
        // Test if this split table exists ..needs to exists otherwise it is incausal
        if (mf_table.find(e->getInLambda()) == mf_table.end()) {
            opp_terminate("Deleting from mf_table which does not exist!!!");
        }

        mf_table[e->getInLambda()].remove(e);
        inMFcounter.remove(e);
    }

    // Remove from scheduling
    scheduling.remove(e);
}

void SOAManager::addSwitchingTableEntry(SOAEntry *e){
    int outPort= e->getOutPort();
    if( e->getOutPort() > -1 and not (e->getBuffer() and e->getBufferDirection() ) ){
        // initiate switching table associated with output port if needed
        if( splitTable.find(e->getOutPort()) == splitTable.end() ){
            std::stringstream out;
            out << "ST for output#"<<e->getOutPort();
            splitTable[ e->getOutPort() ]= cQueue();
            splitTable[ e->getOutPort() ].setName(out.str().c_str());
        }

        // Reference switching table entry with one output port
        splitTable[ e->getOutPort() ].insert(e);
        inSplitTable.insert(e);
    }

    if( e->getBuffer() and e->getBufferDirection() ){
        // SOAEntry which directs to the buffer

        if (mf_table.find( e->getInLambda() ) == mf_table.end()) {
            std::stringstream out;
            out << "MF for wl#" << e->getInLambda();
            mf_table[e->getInLambda()] = cQueue();
            mf_table[e->getInLambda()].setName(out.str().c_str());
        }

        // Adding itself
        mf_table[e->getInLambda()].insert(e);
        inMFcounter.insert(e);

        // Counting of MF
        countMergingFlows(e->getInLambda(), e);
    }

    // Add to the global switching table
    scheduling.insert(e);
}


simtime_t SOAManager::getAggregationWaitingTime(int destination, simtime_t OT, simtime_t len, int &WL, int &outPort) {
    Enter_Method("getAggregationWaitingTime()");

    // Find the output gate
    outPort = R->getOutputPort(destination);
    EV << "Asking for destination=" << destination << " through port=" << outPort;
    EV << " with OT="<<OT<<" len="<<len;

    if (scheduling.length() == 0) {
        // Fast forward - if there is no scheduling .. do bypas
        WL= 1;
        if( !fifo ) WL = (int)uniform(1,maxWL+1);
        SOAEntry *se = new SOAEntry(outPort, WL, true);
        se->setStart(simTime() + OT);
        se->setStop(simTime() + OT + len);
        soa->assignSwitchingTableEntry(se, OT - d_s, len);
        addSwitchingTableEntry(se);

        // Full-fill output text
        EV << "#" << WL << " without waiting" << endl;
        return 0.0;
    }

    // SOAEntry mapping for sorting
    std::map<int, simtime_t> agg;
    std::map<int, simtime_t>::iterator it_agg;
    std::map<int, std::vector<SOAEntry *> > mapa;
    std::map<int, std::vector<SOAEntry *> >::iterator it2;
    std::set<int> free_wl;
    for (int i = 1; i <= maxWL; i++) free_wl.insert(i);
    std::set<int>::iterator wlit;

    for( cQueue::Iterator iter(splitTable[outPort],0); !iter.end(); iter++){
            SOAEntry *tmp = (SOAEntry *) iter();
            mapa[ tmp->getOutLambda() ].push_back( tmp);
    }

    // Sorting function
    for (it2=mapa.begin(); it2!=mapa.end(); ++it2){
        /*EV << it2->first << " => " << endl;
        for (std::vector<SOAEntry *>::iterator itv = (it2->second).begin() ; itv != (it2->second).end(); ++itv)
            EV << " " << (*itv)->info();
        EV << endl; */

        // Soting
        std::sort ((it2->second).begin(), (it2->second).end(), myfunction);

        // Finding of an useful space
        std::vector<SOAEntry *>::iterator itv = (it2->second).begin();
        simtime_t tmp_start=  (*itv)->getStart();
        simtime_t tmp_stop=  (*itv)->getStop();
        int tmp_wl= (*itv)->getOutLambda();
        bool space_found= false;
        for (itv = (it2->second).begin() ; itv != (it2->second).end(); ++itv){
            simtime_t t_space= (*itv)->getStart() - tmp_stop - 2*d_s;
            simtime_t t_ot = tmp_stop - simTime();

            if( t_ot>= OT and t_space >= len ){
                agg[tmp_wl]=t_ot;
                space_found= true;
                break;
            }

            // Next round
            simtime_t tmp_start= (*itv)->getStart();
            simtime_t tmp_stop=  (*itv)->getStop();
        }

        // There was no space so simply append
        if( ! space_found ){
            agg[tmp_wl]= tmp_stop - simTime();
        }

        // Erase this wavelength from set of free ones
        if( free_wl.find( tmp_wl  ) != free_wl.end() )
            free_wl.erase( tmp_wl );
    }

    if( free_wl.size() > 0 ){
        // There are some free wavelength that can be used for allocation
        WL= 0;
        if( !fifo ) WL = (int)uniform(0,free_wl.size());
        wlit= free_wl.begin();
        std::advance( wlit, WL );
        WL= *wlit;

        SOAEntry *se = new SOAEntry(outPort, WL, true);
        se->setStart(simTime() + OT);
        se->setStop(simTime() + OT + len);
        soa->assignSwitchingTableEntry(se, OT - d_s, len);
        addSwitchingTableEntry(se);

        // Full-fill output text
        EV << "#" << WL << " without waiting" << endl;
        return 0.0;
    }

    // Find closest space
    simtime_t shortest= 1e6;
    for (it_agg=agg.begin(); it_agg!=agg.end(); ++it_agg){
        if( shortest > it_agg->second ){
            shortest= (it_agg->second);
            WL= it_agg->first;
        }
    }

    //EV << "Volim WL " << WL << " s cekanim "<< shortest-OT << endl;

    simtime_t waiting= (shortest-OT<0)?0:shortest-OT+d_s;

    SOAEntry *sew = new SOAEntry(outPort, WL, true);
    sew->setStart(simTime() + OT + waiting);
    sew->setStop(simTime() + OT + len + waiting);
    soa->assignSwitchingTableEntry(sew, waiting + OT - d_s, len);
    addSwitchingTableEntry(sew);

    // Full-fill output text
    EV << "#" << WL << " with waiting "<< waiting << endl;
    return waiting;



    /*
    EV << "Setridene" << endl;
    for (it2 = mapa.begin(); it2 != mapa.end(); ++it2) {
        EV << it2->first << " => ";
        for (std::vector<SOAEntry *>::iterator itv = (it2->second).begin();
                itv != (it2->second).end(); ++itv)
            EV << " " << (*itv)->info();
        EV << endl;
    }
    */


    // Make a map of spaces for all WLs
    std::map<int, simtime_t> fitting;
    for( cQueue::Iterator iter(splitTable[outPort],0); !iter.end(); iter++){
        SOAEntry *tmp = (SOAEntry *) iter();

        // Gather spaces before a burst is comming on a WL
        simtime_t fitTime = tmp->getStart() - simTime();
        if (fitting.find(tmp->getOutLambda()) == fitting.end()) {
            // Empty, lets assign it
            fitting[tmp->getOutLambda()] = fitTime;
            if (fitTime < 0)
                fitting[tmp->getOutLambda()] = 0;
            continue;
        }

        // Update outputWL timing if it is not up-to-date
        if (fitting[tmp->getOutLambda()] > fitTime) {
            fitting[tmp->getOutLambda()] = fitTime;
            if (fitTime < 0)
                fitting[tmp->getOutLambda()] = 0;
        }
    }

    //    // Prints out list of wavelengths and its leading space
    //    // (time space before a burst caused by dissagreageted burst)
    //    EV << "leading spaces::" << endl;
    //    std::map<int, simtime_t>::iterator it;
    //    for(it=fitting.begin(); it!=fitting.end();it++){
    //        EV << "** WL "<<it->first<<" space="<< it->second << endl;
    //    }

    /* Random approach for aggreagation through MAC */
    std::set<int> freeWLs;
    std::set<int>::iterator it = freeWLs.begin();

    // Map of used and unsed WLs
    for (int i = 1; i <= maxWL; i++)
        if (fitting.find(i) == fitting.end()) {
            freeWLs.insert(i);
            EV << i << " ";
        }

    // Test whether there are some unused WLs
    if (freeWLs.size() > 0) {
        it = freeWLs.begin();


        if (fifo) {
            // First unused wavelength is used
            do{
                WL = *it++;
                // FIFO but also test if it is really empty
            } while (!testOutputCombination(outPort,WL,simTime()+OT, simTime()+OT+len) );

        } else {
            // Randomly generate and test the generated WL
            do {
                int i = uniform(0, freeWLs.size());
                std::advance(it, i);
                WL = *it;
            } while (!testOutputCombination(outPort,WL,simTime()+OT, simTime()+OT+len) );

        }

        // Create scheduling entry and assign it to the SOA
        SOAEntry *se = new SOAEntry(outPort, WL, true);
        se->setStart(simTime() + OT);
        se->setStop(simTime() + OT + len);
        soa->assignSwitchingTableEntry(se, OT - d_s, len);
        addSwitchingTableEntry(se);

        // Full-fill output text
        EV << "#" << WL << " without waiting" << endl;
        return 0.0;
    }

    /*
     * GROOMING .. using the time of disaggregated car
     * Find WL that can accommodate CAR train in space which was created by car disaggregation
     */
    simtime_t t0 = 0;
    bool fitted=false;
    for(int i=1;i<=maxWL;i++){
        if( fitting[i] >= OT+len+2*d_s and
            testOutputCombination(outPort, WL, simTime()+OT, simTime()+OT+len)){
            WL=i;
            fitted=true;
            EV << " - fitting "<< outPort;
            break;
        }
    }

//    //Print scheduling of designated wavelength
//    EV << "DEBUG WL:" << endl;
//    for (int i = 0; i < scheduling.size(); i++) {
//            if( scheduling[i] == NULL ) { continue;}
//            SOAEntry *tmp = (SOAEntry *) scheduling[i];
//            // Do the find only for One output port OutPort
//            if (tmp->getOutPort() == outPort and tmp->getOutLambda() == WL) {
//                EV << "-- "<<tmp->info()<< " space="<< tmp->getStart()-simTime() <<endl;
//            }
//    }

    /* If there is no space for fitting cars must be appended */
    if (not fitted) {
        // Procedure of find less waiting egress port
        std::map<int, simtime_t> times;
        // table row: outputPort & outputWL -> time the outputWL is ready to be used again
        for( cQueue::Iterator iter(splitTable[outPort],0); !iter.end(); iter++){
            SOAEntry *tmp = (SOAEntry *) iter();

            // Do the find only for One output port OutPort
            // test whether table times contains such outputWL .. unless fix it
            simtime_t waitTime = tmp->getStop() - simTime();
            if (times.find(tmp->getOutLambda()) == times.end()) {
                // Empty, lets assign it
                times[tmp->getOutLambda()] = waitTime;
                continue;
            }

            // Update outputWL timing if it is not up-to-date
            if (times[tmp->getOutLambda()] < waitTime) {
                times[tmp->getOutLambda()] = waitTime;
            }
        }

        // Append approach
        simtime_t waiting = 1e6; // just a really high number
        for (int i = 1; i <= maxWL; i++) {
            // We have found unused wavelength.. lets use it!!!
            if (times.find(i) == times.end()) {
                WL = i;
                t0 = 0;
                break;
            }

            // If the WL is blocked now but wont be blocked at the time of car arrival
            if (times[i] < OT) {
                t0 = 0;
                WL = i;
                //TODO: Mel by zde byt asi break
                break;
            }

            // If the WL is blocked now but wont be blocked at the time of cat arrival
            if (times[i] - OT >= 0 and times[i] < waiting) {
                WL = i;
                waiting = times[i];
                t0 = waiting - OT;
            }
        }

    }

    // The necessary offset between two bursts
    t0= t0 + d_s;

    // Full-fill output text
    EV << "#" << WL << " with waiting=" << t0 << endl;

    // Create
    SOAEntry *se = new SOAEntry(outPort, WL, true);
    se->setStart(simTime()+t0+OT);
    se->setStop( simTime()+t0+OT+len);
    soa->assignSwitchingTableEntry(se,t0+OT-d_s, len);
    addSwitchingTableEntry(se);

    // Inform MAC how log mus wait before sending CAROBS Header and Cars after OT respectively
    return t0;
}

void SOAManager::finish(){
    recordScalar("Bursts to be dropped", tbdropped);

    // Merging flows part
    int total_reg=0;
    for( it_mfc=mf_max.begin(); it_mfc!=mf_max.end(); it_mfc++ ){
        std::stringstream out;
        out << "Number of merging flows at wl#" << (*it_mfc).first;
        recordScalar(out.str().c_str(), (*it_mfc).second);
        total_reg += (*it_mfc).second;
    }
    recordScalar("Total number of merging flows", total_reg);

    // Regenerators
    total_reg = 0;
    for (it_mfc = mf_max.begin(); it_mfc != mf_max.end(); it_mfc++) {
        std::stringstream out;
        out << "Number of regenerators at wl#" << (*it_mfc).first;
        recordScalar(out.str().c_str(), (*it_mfc).second);
        total_reg += (*it_mfc).second;
    }
    recordScalar("Total number of regenerators", total_reg);

    // Calculate vector statistics
    countProbabilities();
}


void SOAManager::countMergingFlows(int inWL, SOAEntry *e){
    simtime_t start = e->getStart();
    simtime_t stop = e->getStop();
    int mfc = 0;
    int regs= 0;


    // MF and Reg# counting
    for( int outPort=0; outPort<getParentModule()->gateSize("gate");outPort++){

        // Color full counting
        for (cQueue::Iterator iter(mf_table[e->getInLambda()], 0); !iter.end(); iter++) {
            SOAEntry *tmp = (SOAEntry *) iter();
            // Check how many merging we have for given period start-stop at output port
            if (outPort != tmp->getOutPort()) continue;

            // Verification of merging flows.. allowed states
            // 1.
            //  ........|----|..... incumbent
            //  .|----|............ the new
            if (tmp->getStart() > stop and tmp->getStop() > stop) continue;
            // 2.
            //  .|----|............ the new
            //  ........|----|..... incumbent
            if (tmp->getStart() < start and tmp->getStop() < start) continue;

            // Otherwise we have met a merging flow..
            regs++;

            // Focus on MF
            if (e->getOutPort() == tmp->getOutPort()) {
                mfc++;
            }
        }
    }

    /** Merging flows */
    // If there is no result for this wavelength, lets make it
    if( mf_max.find(e->getInLambda()) == mf_max.end() )
        mf_max[e->getInLambda()]= mfc;

    // Check if this new record is higher than our current max..
    if( mfc > mf_max[e->getInLambda()])
        mf_max[e->getInLambda()]= mfc;

    /** Regenerators */
    // If there is no result for this wavelength, lets make it
    if( reg_max.find(e->getInLambda()) == reg_max.end() )
        reg_max[e->getInLambda()]= regs;

    // Check if this new record is higher than our current max..
    if( regs > reg_max[e->getInLambda()])
        reg_max[e->getInLambda()]= regs;
}
