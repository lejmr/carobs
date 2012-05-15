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

    // Hard-coded datarte
    C = par("datarate").doubleValue();
    tbdropped=0;

    // W election method
    fifo= par("fifo").boolValue();

    // Speed of O/E conversion
    convPerformance=par("convPerformance").doubleValue();

    WATCH(OBS);
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
        for(int i=0;i<cars.length();i++){
            CAROBSCarHeader *tmpc = (CAROBSCarHeader *) cars.get(i);
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
    if (OBS)
        obsBehaviour(msg, inPort);
    else
        carobsBehaviour(msg, inPort);

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

    // Car train reached its termination Node -> Disaggregation
    if (H->getDst() == address) {
        CAROBSCarHeader *first_car = (CAROBSCarHeader *) H->getCars().get(0);
        SOAEntry *se = new SOAEntry(inPort, inWl, false);
        se->setStart(simTime() + H->getOT() );
        se->setStop(simTime() + H->getOT()+H->getLength());
        // Add Switching Entry to SOA a SOAManager scheduler
        EV << " Car train has reached it destination"<<endl;
        EV << " Dissagregation: " << se->info() << endl;

        soa->assignSwitchingTableEntry(se, H->getOT()-d_s, H->getLength());
        scheduling.add(se);
        // And that is it, nothing more to do
        delete msg;
        return;
    }

    // This node is not termination one, so we must check whether we need to disaggreate something
    int NoToDis = -1;
    int longest_burst_length = 0;
    cQueue cars = H->getCars();
    bool dissaggregation=false;
    EV << "cars we have with OT=" << H->getOT() << ": " << endl;
    for (int i = 0; i < cars.length(); i++) {
        CAROBSCarHeader *tmpc = (CAROBSCarHeader *) cars.get(i);
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
        EV << endl;
    }

    // Whole car-train times
    simtime_t train_start = simTime() + H->getOT();
    simtime_t train_stop = simTime() + H->getOT() + H->getLength();
    simtime_t train_length = H->getLength();

    if (NoToDis>=0 and NoToDis + 1 < H->getN()) {
        // Count times for car-train
        CAROBSCarHeader *tmpc0 = (CAROBSCarHeader *) cars.get(NoToDis);
        CAROBSCarHeader *tmpc = (CAROBSCarHeader *) cars.get(NoToDis + 1);
        train_start += tmpc->getD_c();
        train_length-= tmpc->getD_c();

        // Create disaggregation SOA instructions
        SOAEntry *se = new SOAEntry(inPort, inWl, false); // Disaggregation SOAEntry => false
        se->setStart(simTime()+H->getOT());
        se->setStop(train_start - d_s);
        simtime_t len= train_start - d_s - simTime()+H->getOT();

        // Add Switching Entry to SOA a SOAManager scheduler
        soa->assignSwitchingTableEntry(se, H->getOT()- d_s, tmpc->getD_c()-d_s);
        scheduling.add(se);
        EV << " Dissagregation: " << se->info() << endl;

        // Remove CAR Header from CAROBS Header of this disaggregated car
        // and update every d_c of cars persisting in the train
        delete cars.remove(tmpc0);


        simtime_t shift= tmpc->getD_c();
        EV << "Zkracuji o " << shift << endl;
        for(int i=0;i<cars.length();i++){
            CAROBSCarHeader *tc= (CAROBSCarHeader *) cars.get(i);
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
    }

    // Add Switching Entry to SOA a SOAManager scheduler
    sef->setStart(train_start+d_w_extra);
    sef->setStop(train_stop+d_w_extra);
    soa->assignSwitchingTableEntry(sef, d_w_extra+H->getOT()-d_s, train_length);

    // Update of OT for continuing car train
    H->setOT(H->getOT()-d_p);

    // If there was changed wavelength of train Header must carry such information further
    H->setWL(sef->getOutLambda());

    // Encapsulate the CAROBS Header back to OpticalLayer and send further
//    ol->encapsulate(H);

    // Test whether this is last CoreNode on the path is so CAROBS Header is not passed towards
    if (R->canForwardHeader(H->getDst())) {
        // Resending CAROBS Header to next CoreNode
        sendDelayed(ol, d_p+d_w_extra, "control$o", outPort);
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

    std::map<int, simtime_t> times;
    // table row: outputPort & outputWL -> time the outputWL is ready to be used again
    for (int i = 0; i < scheduling.size(); i++) {
        if( scheduling[i] == NULL ) { continue; }
        SOAEntry *tmp = (SOAEntry *) scheduling[i];

        // Do the find only for One output port OutPort
        if (tmp->getOutPort() == outPort) {
            // test whether table times contains such outputWL .. unless fix it
            if (times.find(tmp->getOutLambda()) == times.end()) {
                times[tmp->getOutLambda()] = tmp->getStop();
                continue;
            }

            // Update outputWL timing if it is not up-to-date
            if (times[tmp->getOutLambda()] < tmp->getStop()) {
                times[tmp->getOutLambda()] = tmp->getStop();
            }
        }
    }

    if (times.find(inWL) == times.end()) {
        EV << " NO BUFFER outWL=" << inWL << endl;
        EV << " ! Strange situation - it seems there is no scheduling for outPort=";
        EV << outPort << "#" << inWL << " do not have to buffer it !" << endl;
        SOAEntry *e = new SOAEntry(inPort, inWL, outPort, inWL);
        e->setStart(start);
        e->setStop(stop);
        scheduling.add(e);
        return e;
    }

    // Minum time the burst must stay in buffer .. it is approximation of limit conversio speed
    double min_buffer= convPerformance*(double)length;

    //  Time to stay in buffer in case we keep the same wavelength
    simtime_t BT = times[inWL] - start + d_s;
    if( BT < min_buffer) BT= min_buffer;
    int outWL = inWL;

    // Look for an output wavelength with shortest waiting time
    simtime_t tmpBT = BT; //1e6
    int betterWL = inWL;
    for (int i = 1; i <= maxWL; i++) {
        if (i == inWL) continue;
        if (times.find(i) == times.end()) {
            EV << " O/E/O only";

            // Set the car-train to buffer
            betterWL= i;
            tmpBT= min_buffer;
            break;
        }

        simtime_t tmpBTloop = times[i] - start;
        if (tmpBTloop < tmpBT and tmpBTloop >= min_buffer) {
            tmpBT = tmpBTloop;
            betterWL = i;
        }
    }

    // The winner is
    outWL = betterWL;
    BT = tmpBT + d_s; // d_s makes time offset for SOA reconfiguration

    //  Return the record important for outgoing car-train from buffer - this
    //  way SOAManager can assume the buffering time
    // Record which sends incoming car-train to buffer
    SOAEntry *e_in = new SOAEntry(inPort, inWL, outPort, outWL);
    e_in->setStart(start);
    e_in->setStop(stop);
    e_in->setBuffer(true);
    e_in->setInBuffer();
    scheduling.add(e_in);
    soa->assignSwitchingTableEntry(e_in, start - simTime() - d_s, stop - start);

    // Withdraw car-train from buffer and reserver output port
    SOAEntry *e_out = new SOAEntry(inPort, inWL, outPort, outWL);
    e_out->setStart(start + BT);
    e_out->setStop(stop + BT);
    e_out->setBuffer(true);
    e_out->setOutBuffer();
    scheduling.add(e_out);

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

    for (int i = 0; i < scheduling.size(); i++) {
        if( scheduling[i] == NULL ) { continue;}
        SOAEntry *tmp = (SOAEntry *) scheduling[i];

        //if( tmp->getBuffer() and tmp->getBufferDirection() ) continue;

        if (tmp->getOutLambda() == outWL and tmp->getOutPort() == outPort) {
            // There is some scheduling for my output combination, lets see whether it is overlapping

            if( stop < tmp->getStart()-d_s and start < tmp->getStart()-d_s ){
                // Overlap start time with respect to d_s
                //   in table:           |-------|
                //the new one:    |----|
                tests.insert(true);
                continue;
            }

            if( start > tmp->getStop()+d_s and stop > tmp->getStop()+d_s ){
                // Overlap start time with respect to d_s
                //   in table:   |-------|
                //the new one:             |----|
                tests.insert(true);
                continue;
            }

            tests.insert(false);
        }
    }

    // Returns True if here is no conflict on the output combination port#WL
    return tests.find(false) == tests.end();
}

void SOAManager::dropSwitchingTableEntry(SOAEntry *e) {
    Enter_Method("dropSwitchingTableEntry()");
    scheduling.remove(e);
}

simtime_t SOAManager::getAggregationWaitingTime(int destination, simtime_t OT, simtime_t len, int &WL, int &outPort) {
    Enter_Method("getAggregationWaitingTime()");

    // Find the output gate
    outPort = R->getOutputPort(destination);
    EV << "Asking for destination=" << destination << " through port=" << outPort;
    EV << " with OT="<<OT<<" len="<<len;

    if (scheduling.size() == 0) {
        // Fast forward - if there is no scheduling .. do bypas
        WL= 1;
        if( !fifo ) WL = (int)uniform(1,maxWL+1);
        SOAEntry *se = new SOAEntry(outPort, WL, true);
        se->setStart(simTime() + OT);
        se->setStop(simTime() + OT + len);
        soa->assignSwitchingTableEntry(se, OT - d_s, len);
        scheduling.add(se);

        // Full-fill output text
        EV << "#" << WL << " without waiting" << endl;
        return 0.0;
    }

    // Make a map of spaces for all WLs
    std::map<int, simtime_t> fitting;
    for (int i = 0; i < scheduling.size(); i++) {
        if( scheduling[i] == NULL ) { continue; EV << "invalid" << endl;}
        SOAEntry *tmp = (SOAEntry *) scheduling[i];

        // Do the find only for One output port OutPort
        if (tmp->getOutPort() == outPort) {

            // Gather spaces before a burst is comming on a WL
            simtime_t fitTime = tmp->getStart() - simTime();
            if (fitting.find(tmp->getOutLambda()) == fitting.end()) {
                // Empty, lets assign it
                fitting[tmp->getOutLambda()] = fitTime;
                if( fitTime < 0) fitting[tmp->getOutLambda()]= 0;
                continue;
            }

            // Update outputWL timing if it is not up-to-date
            if ( fitting[tmp->getOutLambda()] > fitTime ) {
                fitting[tmp->getOutLambda()] = fitTime;
                if( fitTime < 0) fitting[tmp->getOutLambda()]= 0;
            }
        }
    }

//    // Prints out list of wavelengths and its leading space
//    // (time space before a burst caused by dissagreageted burst)
//    EV << "leading spaces::" << endl;
//    std::map<int, simtime_t>::iterator it;
//    for(it=fitting.begin(); it!=fitting.end();it++){
//        EV << "** WL "<<it->first<<" space="<< it->second << endl;
//    }

    if( !fifo ){
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

            // Randomly generate and test the generated WL
            do {
                int i = uniform(0, freeWLs.size());
                std::advance(it, i);
                WL = *it;
            } while (!testOutputCombination(outPort, WL, simTime() + OT, simTime() + OT + len));

            // Create scheduling entry and assign it to the SOA
            SOAEntry *se = new SOAEntry(outPort, WL, true);
            se->setStart(simTime() + OT);
            se->setStop(simTime() + OT + len);
            soa->assignSwitchingTableEntry(se, OT - d_s, len);
            scheduling.add(se);

            // Full-fill output text
            EV << "#" << WL << " without waiting" << endl;
            return 0.0;
        }
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
        for (int i = 0; i < scheduling.size(); i++) {
            if( scheduling[i] == NULL ) { continue; EV << "invalid1" << endl;}
            SOAEntry *tmp = (SOAEntry *) scheduling[i];

            // Do the find only for One output port OutPort
            if (tmp->getOutPort() == outPort) {
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
    scheduling.add(se);

    // Inform MAC how log mus wait before sending CAROBS Header and Cars after OT respectively
    return t0;
}

void SOAManager::finish(){
    recordScalar("Bursts to be dropped", tbdropped);
}
