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

Define_Module(SOAManager);

void SOAManager::initialize()
{
    // Obtaining processing time parametr d_p
    d_p = par("d_p").doubleValue();
    d_s = par("d_s").doubleValue();

    // reading parameters
    maxWL = par("maxWL").longValue();
    WC = par("WC").boolValue();

    // Obtaining processing time parametr JET
    JET = par("JET").boolValue();

    // Making link with Routing module
    cModule *calleeModule = getParentModule()->getSubmodule("routing");
    R = check_and_cast<Routing *>(calleeModule);

    // Making link with SOA module
    calleeModule = getParentModule()->getSubmodule("soa");
    soa = check_and_cast<SOA *>(calleeModule);

}

void SOAManager::handleMessage(cMessage *msg)
{
    // Obtaining optical signal
    OpticalLayer *ol=  dynamic_cast<OpticalLayer *>(msg);

    // Detection of optical signal -> Electrical CARBOS Header
    CAROBSHeader *H= (CAROBSHeader *) ol->decapsulate();

    // Obtaining information about source port
    int inPort= msg->getArrivalGate()->getIndex();

    // Obtaining information about wavelength of incoming Car train
    int inWl= H->getWL();

    // Obtaining information about Car train destination
    int dst= H->getDst();

    // Resolving output port
    int outPort = R->getOutputPort( dst );

    // Update OT
    simtime_t tmpOT= H->getOT();
    H->setOT( H->getOT() - d_p );


    // Prepare SOA switching table entry
    //SOAEntry *se = new SOAEntry(inPort, inWl, outPort, inWl);

    EV << "start "<< simTime()+tmpOT-d_s;
    EV << " stop "<< simTime()+tmpOT-d_s+ H->getLength() << endl;

    // Assign this SOAEntry to SOA
    SOAEntry *se = getOptimalOutput(outPort, inPort, inWl, simTime()+tmpOT-d_s, simTime()+tmpOT-d_s+H->getLength() ); // JET
    soa->assignSwitchingTableEntry(se, tmpOT-d_s, H->getLength() );


    /*  JIT is blocked for now
     * else soa->assignSwitchingTableEntry(se, d_s , tmpOT+H->getLength() );    // JIT
     */

    // Put the headers  back to OpticalLayer E-O conversion
    ol->encapsulate(H);

    // Test whether this is last CoreNode on the path is so CAROBS Header is not passed towards
    if( R->canForwardHeader(H->getDst()) ){
        // Resending CAROBS Header to next CoreNode
        sendDelayed(ol, d_p, "control$o", outPort );
    }else{
        delete msg; return;
    }
}

SOAEntry* SOAManager::getOptimalOutput(int outPort, int inPort, int inWL, simtime_t start, simtime_t stop){
    EV << "Get scheduling for "<<inPort<<"@"<<inWL<<" to "<<outPort<<" for:"<<start<<"-"<<stop;

    if( scheduling.size() == 0  ){
        // Fast forward - if there is no scheduling .. do bypas
        SOAEntry *e= new SOAEntry(inPort, inWL, outPort, inWL);
        e->setStart(start);
        e->setStop(stop);

        // And add it to scheduling table
        scheduling.add(e);
        EV << " outWL="<<inWL<<endl;
        return e;
    }

    // Test output combination
    if( testOutputCombination(outPort, inWL, start, stop ) ){
        // There is no blocking of output port at a  given time
        SOAEntry *e = new SOAEntry(inPort, inWL, outPort, inWL);
        e->setStart(start);
        e->setStop(stop);
        // And add it to scheduling table
        scheduling.add(e);
        EV << " outWL="<<inWL<<endl;
        return e;
    }

    if( WC ){
        /* Perform wavelength conversion */

        // FIFO approach
        for( int i=1;i<=maxWL;i++){
            if( testOutputCombination(outPort, i, start, stop ) ){
                // Wavelength i is free at the given time, we can use it
                SOAEntry *e = new SOAEntry(inPort, i, outPort, inWL);
                e->setStart(start);
                e->setStop(stop);
                // And add it to scheduling table
                scheduling.add(e);
                EV << " WC->outWL="<<i<<endl;
                return e;
            }
        }
    }

    // Sorry I did my best but there is not output
    // So burst buffering if enabled?
    return new SOAEntry(-1,-1,-1,-1);
}


bool SOAManager::testOutputCombination(int outPort, int outWL, simtime_t start, simtime_t stop ){
    for ( int i=0 ; i<scheduling.size(); i++ ){
            SOAEntry *tmp= (SOAEntry *) scheduling[i];

            if( tmp->getOutLambda() == outWL and tmp->getOutPort() == outPort ){
                // There is some scheduling for my output combination, lets see whether it is overlapping
                if( start < tmp->getStart() and stop > tmp->getStart() ){
                    // Overlap start time
                    //   in table:       |-------|
                    //the new one:    |----|
                    return false;
                }

                if( start < tmp->getStop() and stop > tmp->getStart()  ){
                    // inside of it .. body overlap
                    //   in table:       |-------|
                    //the new one:         |----|
                    return false;
                }

                if( start < tmp->getStop() and stop > tmp->getStop() ){
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

void SOAManager::dropSwitchingTableEntry(SOAEntry *e){
    Enter_Method("dropSwitchingTableEntry()");
    scheduling.remove(e);
}


