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

#include "CoreNodeMAC.h"

Define_Module(CoreNodeMAC);

void CoreNodeMAC::initialize() {

    // Communication link with soaManager
    cModule *calleeModule = getParentModule()->getSubmodule("soaManager");
    SM = check_and_cast<SOAManager *>(calleeModule);

    //bufferedSOAqueue.setName("Buffered trains");
    // Statistics and Watchers
    burst_send = 0;
    capacity = 0;
    outAged = 0;
    avg_waitingtime= 0;
    max_buffersize=0;
    avg_buffersize=0;
    wrong_output=0;
    total_waitingtime=0;
    aggregated=0;

    total_buffertime=0;
    avg_buffertime=0;
    buffered=0;
    vbuffertime.setName("Buffering delay [s]");
    vwaitingtime.setName("Access delay [s]");
    buffered_data.setName("Data stored in memory [B]");
    carMsgs.setName("Number of packets in a car [-]");
    carLength.setName("Car's length [s]");

    // Address asignement
    address = par("address").doubleValue();

    // Initialising port monitoring
    for( int i =0; i<gateSize("soa");i++){
        vbuffertime_port[i] = new cOutVector();
        std::stringstream out;
        out << "Buffering delay - port #" << i <<" [s]";
        vbuffertime_port[i]->setName(out.str().c_str());
    }

    WATCH(capacity);
}

void CoreNodeMAC::handleMessage(cMessage *msg) {
    /**
     *  This part of the code is executed when an OpticalLayer message approaches
     *  the MAC layer. This might happen only in case of buffering .. this is the
     *  code which buffers incoming Cars and creates scheduler to resend them back
     */
    if (dynamic_cast<OpticalLayer *>(msg) != NULL) {
        OpticalLayer *ol= dynamic_cast<OpticalLayer *>(msg);
        Car *car= (Car *) ol->decapsulate();    // OE conversion
        car->setBuffered( car->getBuffered() + 1 );
        EV << "The car "<< car->getName() << " is to be stored in buffer";

        // Calculate buffer usage and create record for statistics
        capacity += car->getByteLength();
        if( capacity > max_buffersize )max_buffersize= capacity;
        if( avg_buffersize == 0 ) avg_buffersize= capacity;
        avg_buffersize= (avg_buffersize+capacity)/2;
        buffered_data.record(capacity);

        // Determine description of incoming signal. Since it is optical only port and WL
        int inPort= msg->getArrivalGate()->getIndex();
        int inWL= ol->getWavelengthNo();
        double ot= ol->par("ot").doubleValue();

        // Withdraws the marker from OL
        SOAEntry *olsw= (SOAEntry *) ol->par("marker").pointerValue();

        // Drop OL, cause all data are already in electrical domain
        delete ol;

//        EV << "List of SOAEntries" << endl;
//        std::map< SOAEntry *, simtime_t>::iterator it;
//        for(it=waitings.begin(); it!=waitings.end();it++){
//            if( not  bufferedSOAqueue.contains((*it).first) ){
//                EV << "there is a pointer for: "<< (*it).first << " : " << (*it).second << endl;
//                continue;
//            }
//            EV <<inPort<<"#"<<inWL;
//            EV << " -> ";
//            EV << ((*it).first)->info() << " : " << (*it).second << endl;
//        }

        // Schedule unbuffering
        cMessage *msg = new cMessage("ReleaseBuffer");
        msg->addPar("RelaseStoredCar");
        msg->addPar("RelaseStoredCar_CAR");
        msg->par("RelaseStoredCar").setPointerValue(olsw->bound);
        msg->par("RelaseStoredCar_CAR").setPointerValue(car);
        msg->setSchedulingPriority(1);
        msg->addPar("ot").setDoubleValue( ot );  // Testing purpose.. a car can pass as many CoreNodes as its OT supports

        usage[olsw->bound]++;
        EV << " and released in: " << (simtime_t) waitings[olsw->bound];
        EV << " at=" << simTime() + waitings[olsw->bound] << endl;

        // Security test of waiting assignment for the SOAEntry
        if (waitings.find(olsw->bound) == waitings.end()) {
            EV << "Unable to find waiting time" << endl;
        }

        // Schedules out of buffer CAR release
        scheduleAt(simTime() + waitings[olsw->bound], msg);

        // Put it to the planner
        self_agg[olsw->bound]= msg;
        printScheduler();

        EV << "Scheduling stack> " <<endl;
            for(std::map<cObject *, cObject *>::iterator it=self_agg.begin(); it!=self_agg.end(); ++it )
                EV << "SENDINGS"<< ((SOAEntry *)it->first)->info() << ": " << it->second << endl;

        // Finish buffering procedure
        return;
    }

    /** Continue of previous part - withdrawing the Car from waiting queue and send back to SOA **/
    if( msg->isSelfMessage() and msg->hasPar("RelaseStoredCar")){
        SOAEntry *se=(SOAEntry *) msg->par("RelaseStoredCar").pointerValue();
        Car *car= (Car *) msg->par("RelaseStoredCar_CAR").pointerValue();
        int ident= (int)uniform(100, 1e9);

        /*
        if( !bufferedSOAqueue.contains(se) ){
            // This might happend when is there a problem with timing. se is deleted
            // from bufferedSOAqueue and after the scheduler wants to release car
            EV << "Takova volba neexistuje - cotains ";
            EV << car->getName() << endl;
            outAged++;
            opp_terminate("Jdeme zkoumat kde je problem");
            return;
        }
        */

        EV << "The car "<< car->getName() << "is to be released to SOA";
        EV << " based on " << se->info() << endl;

        // Convert Car->OpticalLayer == E/O conversion
        OpticalLayer *ol= new OpticalLayer( car->getName() );
        ol->setWavelengthNo( se->getOutLambda() );
        ol->encapsulate( car );
        ol->addPar("ot").setDoubleValue( msg->par("ot").doubleValue() );  // Testing purpose.. a car can pass as many CoreNodes as its OT supports
        ol->addPar("SOAEntry_identifier").setLongValue( ident );

        // Calculate buffer usage and create record for statistics
        capacity -= car->getByteLength();
        if( capacity > max_buffersize )max_buffersize= capacity;
        if( avg_buffersize == 0 ) avg_buffersize= capacity;
        avg_buffersize= (avg_buffersize+capacity)/2;

        // Send the OL to SOA
        send(ol, "soa$o", se->getOutPort() );

        // Clean waitings and bufferedSOAe - maintenance
        if( usage[se]-- <= 0 ){
            EV << "Last car now clean up" << endl;
            waitings.erase(se);
            usage.erase(se);
            EV << "Erasing " << se->info() << endl;
        }

        // Drop the self-message since is not needed anymore
        se->identifier= ident;
        self_agg.erase(se);
        printScheduler();
        delete msg; return;
    }


    if( msg->isSelfMessage() and msg->hasPar("StartAggregation")){

        // Statistics
        simtime_t delta= simTime() - (simtime_t)msg->par("StartAggregation_start").doubleValue();
        vwaitingtime.record(delta);

        // Sending ..
        SOAEntry *e= (SOAEntry *)msg->par("StartAggregation_switching").pointerValue();
        MACContainer *MAC= (MACContainer *)msg->par("StartAggregation").pointerValue();
        int outPort= e->getOutPort();
        int WL= e->getOutLambda();
        int ident= (int)uniform(100, 1e9);

        // 1. Header
        CAROBSHeader *H= (CAROBSHeader *)msg->par("StartAggregation_H").pointerValue();
        int dst= H->getDst();

        // Set wavelength of Car train to CAROBS header
        H->setWL(WL);

        // Console info
        EV << "Sending CAROBS train to " << dst ;

        // CAROBS Header
        char buffer_name[50];
        sprintf(buffer_name, "Header %d", dst);
        OpticalLayer *OH = new OpticalLayer(buffer_name);
        OH->setWavelengthNo(0); //Signalisation is always on the first channel
        OH->encapsulate(H);
        OH->setByteLength(50);
        OH->addPar("SOAEntry_identifier").setLongValue(ident);
        send(OH, "control");


        // 2. Cars
        EV << " OT= " << H->getOT() << endl;
        // Schedule associate cars
        while (!MAC->getCars().empty()) {
            SchedulerUnit *su = (SchedulerUnit *) MAC->getCars().pop();
            Car *tcar = (Car *) su->decapsulate();

            // Encapsulate Car into OpticalLayer
            char buffer_name[50];
            sprintf(buffer_name, "Car %d", su->getDst());
            OpticalLayer *OC = new OpticalLayer(buffer_name);
            OC->setWavelengthNo(WL);
            tcar->addPar("dst");
            tcar->par("dst").setDoubleValue( su->getDst() );
            tcar->addPar("src");
            tcar->par("src").setDoubleValue( address );
            OC->encapsulate(tcar);
            OC->addPar("ot").setDoubleValue( su->getStart().dbl() );  // Testing purpose.. a car can pass as many CoreNodes as its OT supports
            OC->addPar("SOAEntry_identifier").setLongValue(ident);

            // Car loading statistics
            carMsgs.record(tcar->getPayload().length());
            carLength.record(su->getLength());

            // Console info
            EV << " + Sending car to " << su->getDst() << " at " << simTime()+su->getStart() << " of length=" << su->getLength() << " == "<< tcar->getPayload().length() << " packets" << endl;

            // Send the car onto proper wl at proper time
            sendDelayed(OC, su->getStart(), "soa$o", outPort);

            // Statistics
            burst_send++;

            // Drop the empty scheduler unit
            delete su;
        }

        // Unset mapper
        self_agg.erase(e);
        e->identifier= ident;
        printScheduler();
        delete msg; return;
    }



    /**
     *  This part of the code takes care of aggregation of Payload packet to
     *  SOA in case of mixed usage scenario when CoreNode can behave as an
     *  EdgeNode.
     */

    // Lets make the sending done
    MACContainer *MAC = dynamic_cast<MACContainer *>(msg);

    // Dencapsulate CARBOSHeader
    CAROBSHeader *H = dynamic_cast<CAROBSHeader *>(MAC->decapsulate());

    // Sending parameters
    int dst = H->getDst();

    // Obtain waiting and wavelength
    SOAEntry *e;
    simtime_t t0 = SM->getAggregationWaitingTime(H->getLabel(), H->getOT(), H->getLength(), e);
    int WL = e->getOutLambda();
    int outPort=e->getOutPort();

    if( outPort < 0 ){
        EV << "Unable to find output path" << endl;
        wrong_output++;
        return ;
    }

    // Schedule sending
    cMessage *msg1 = new cMessage("StartAggregation");
    msg1->addPar("StartAggregation");
    msg1->par("StartAggregation").setPointerValue(MAC);
    msg1->addPar("StartAggregation_H");
    msg1->par("StartAggregation_H").setPointerValue(H);
    msg1->addPar("StartAggregation_switching");
    msg1->par("StartAggregation_switching").setPointerValue(e);
    msg1->addPar("StartAggregation_start");
    msg1->par("StartAggregation_start").setDoubleValue( simTime().dbl() );
    //msg1->setSchedulingPriority(5);

    scheduleAt(simTime() + t0, msg1);

    // Set mapper
    self_agg[e]= msg1;
    printScheduler();
}

void CoreNodeMAC::storeCar( SOAEntry *e, simtime_t wait ){
    Enter_Method("storeCar()");
    EV << "Adding scheduler " << e->info() << "with waiting=" << wait << endl;
    //bufferedSOAqueue.insert(e);
    waitings[e]= wait;
    usage[e]= 0;

    // Statistics
    if( avg_buffertime==0 ) avg_buffertime=wait;
    avg_buffertime= (avg_buffertime+wait)/2;
    vbuffertime.record(wait);
    vbuffertime_port[e->getInPort()]->record(wait);

    // Overall ones
    total_buffertime+=wait;
    buffered++;
}

void CoreNodeMAC::removeCar( SOAEntry *e ){
    Enter_Method("removeCar()");
    EV << "Removing scheduler at " << simTime() << " for release="<<e->getStart()<<endl;
    //if( bufferedSOAqueue.contains(e) ) bufferedSOAqueue.remove( e );
    waitings.erase(e);
    usage.erase(e);
}

void CoreNodeMAC::delaySwitchingTableEntry(cObject *e, simtime_t time){
    Enter_Method("delaySwitchingTableEntry()");
    // EV << "Postponing "<< ((SOAEntry *) e)->info() << " about " << time << endl;

    // Only for better readability
    SOAEntry *sw= (SOAEntry *) e;

    if( self_agg.find(sw) == self_agg.end()  ){

        EV << "Delaying: "<< sw->info() << endl;
        printScheduler();
        opp_terminate("Rescheduling not existing scheduling");

    }

    // Get self-message
    cMessage *msg_agg= (cMessage *) self_agg[ sw ];

    // It exists
    simtime_t arr= ( msg_agg )->getArrivalTime();
    msg_agg->setArrivalTime( arr + time );

}

void CoreNodeMAC::printScheduler(){

    EV << "Scheduling: " <<endl;
    for(std::map<cObject *, cObject *>::iterator it=self_agg.begin(); it!=self_agg.end(); ++it ){
        EV << ( (SOAEntry *)it->first )->info() << " : " << it->second << endl;
    }
}


void CoreNodeMAC::finish(){

    /* Performance monitoring */
    recordScalar("Bursts sent", burst_send);
    recordScalar("Average buffer size [B]", avg_buffersize);
    recordScalar("Maximal buffer size [B]", max_buffersize);
    recordScalar("Access delay", avg_waitingtime);
    if(aggregated>0)recordScalar("Access delay (total)", total_waitingtime/aggregated);
    if(wrong_output>0) recordScalar("Wrong routing policy",wrong_output);

    if(buffered > 0){
        recordScalar("Buffering delay", avg_waitingtime);
        recordScalar("Buffering delay (total)", total_waitingtime/buffered);
    }

    /* Monitoring */
    if(outAged>0)recordScalar(" ! Late release", outAged);
}
