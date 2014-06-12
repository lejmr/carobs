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

#include "cplexGenerator.h"
#include "messages/Payload_m.h"
#include <iostream>
#include <fstream>

Define_Module(CplexGenerator);

using namespace std;

void CplexGenerator::initialize()
{
    src = par("address").longValue();
    n = par("n").longValue();

    // Read data from ipDemandFile
    const char *filename = par("trafficFile");
    std::ifstream infile;
    infile.open (filename);
    EV<< "Loading data from " << filename << endl;
    if (!infile.is_open())
        opp_error("Error opening routing table file `%s'", filename);

    // Parse input file and obtain the traffic matrix
    std::string line;
    std::vector<double> tmp;
    int max_dst=0;
    while ( getline (infile,line) ){

        // Because of unix line end notation..
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);

        // Parse line
        tmp= cStringTokenizer( line.c_str() , ";" ).asDoubleVector();

        if( (int)tmp[0] > max_dst ) max_dst= (int)tmp[0];
        if( (int)tmp[1] > max_dst ) max_dst= (int)tmp[1];

        // Data for this particular source
        if( (int)tmp[0] == src and (int)tmp[2] == src  ){

            // Initialize request to
            if( demands.find( (int)tmp[1] ) == demands.end() ) demands[ (int)tmp[1] ]=0;

            // Add requests
            demands[ (int)tmp[1] ]+= tmp[5];

        }
    }

    // Visualize traffic matrix
    WATCH_MAP(demands);


    // Initialise sending
    if (par("send").boolValue()) {
        cMessage *init= new cMessage();
        init->addPar("initiateSending");
        scheduleAt(0, init);
    }

    psend=0;
    // Informations for Routing
    cMessage *r = new cMessage("MyIDs");
    r->addPar("myMinID").setLongValue(src);
    r->addPar("myMaxID").setLongValue(src);
    r->setSchedulingPriority(-10);
    scheduleAt(0,r);
}

void CplexGenerator::handleMessage(cMessage *msg)
{

    int length = par("length").longValue();
    int step = par("blast").longValue();
    int alpha = par("alpha").doubleValue();

    if( msg->isSelfMessage() and msg->hasPar("myMinID") ){
        send(msg,"out");
        return;
    }

    // Prepare sending
    if ( msg->isSelfMessage() and msg->hasPar("initiateSending") ){
        std::vector<double>::iterator it;
        int dst=0;

        for (std::map<int, double>::iterator it=demands.begin(); it!=demands.end(); ++it){
            //EV << it->first << " => " << it->second << endl;
            dst= it->first;
            double demand= alpha * it->second;

            //for (it = demands.begin(); it < demads.end(); it++) {
            if( dst == src ) continue;
            if( demand <= 0 ) continue;

            // First arrival time
            arrivals[dst]=simTime();

            EV << "Preparing "<<n<<" payload packets to " << dst << " with demand "<<demand <<"Mbps";
            // Mean time between incomes of the demands .. based on A= lambda*tos
            // tos = lenght / C  - Where lengthB is mean value of length and C is bitrate in bps
            // demand/C - in conversion of traffic matrix into Erlang notation
            // 1e6 .. SNDlib comes with data in Mbps .. so to make it bps
            double lambda= demand*1e6 / length ;
            EV << " => lambda="<<lambda <<endl;

            // Initiate sending
            cMessage *t = new cMessage();
            t->addPar("lambda").setDoubleValue(lambda);
            t->addPar("src").setDoubleValue(src);
            t->addPar("dst").setDoubleValue(dst);
            t->addPar("sent").setLongValue(0);
            t->addPar("scheduling");
            scheduleAt(simTime(), t);

        }

        delete msg;
        return;
    }

    //Initiate sending Payload packets
    if( msg->isSelfMessage() and msg->hasPar("scheduling")){

        // Test how many packet has been send
        int64_t sent = msg->par("sent").longValue();
        if (sent >= n) {
            EV << " All (" << sent << "/" << n << ") packets has been sent to " << msg->par("dst").doubleValue() << endl;
            delete msg;
            return;
        }

        // There are some packets to send
        int tosend= step;
        if( sent+step > n ) tosend= n-sent;

        // Send these packets
        simtime_t next = sendAmount( tosend,
                            msg->par("src").doubleValue(), msg->par("dst").doubleValue(),
                            msg->par("lambda").doubleValue(), length);


        // Initiate next sending blast
        cMessage *t = new cMessage();
        t->addPar("lambda").setDoubleValue( msg->par("lambda").doubleValue() );
        t->addPar("src").setDoubleValue(src);
        t->addPar("dst").setDoubleValue( msg->par("dst").doubleValue() );
        t->addPar("sent").setLongValue( msg->par("sent").longValue()+tosend );
        t->addPar("scheduling");
        scheduleAt( next , t);

        // Clean after last scheduling
        delete msg;
        return;
    }

    // Send prepared Payload packet
    if(dynamic_cast<Payload *>(msg) != NULL){
        send(msg, "out");
    }
}

simtime_t CplexGenerator::sendAmount(int amount, int src, int dst, double lambda, int length){
    for (int i = 0; i < amount; i++) {
        // E[t]=int(t*lambda*exp(-lambda*t),t=0..oo)=1/lambda
        simtime_t gap = exponential(1/lambda);
        Payload *pl = new Payload();
        pl->setBitLength(length);
        pl->setDst(dst);
        pl->setSrc(src);
        pl->setT0(arrivals[dst]);
        pl->setSchedulingPriority(10); // Gives way to the autoconfiguration of CoreNode
        scheduleAt(arrivals[dst], pl);
        EV << " + " << src << "->" << dst << " (" << length / 8 << "B): " << arrivals[dst] << endl;
        arrivals[dst]+=gap;
        psend++;
    }

    EV << "Next sending is planned at " << arrivals[dst] << endl;
    return arrivals[dst];
}

void CplexGenerator::finish(){
    recordScalar("Packets sent", psend);
}
