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

#include "cityGenerator.h"
#include "messages/Payload_m.h"
#include <iostream>
#include <fstream>

Define_Module(CityGenerator);

using namespace std;

void CityGenerator::initialize()
{
    src = par("address").longValue();
    n = par("n").longValue();
    C = par("datarate").doubleValue();

    // Read data from ipDemandFile
    const char *filename = par("ipDemandFile");
    std::ifstream infile;
    infile.open (filename);
    EV<< "Loading data from " << filename << endl;
    if (!infile.is_open())
        opp_error("Error opening routing table file `%s'", filename);

    std::string line;
    for(int i=0;i<src;i++){
        getline(infile,line);
        // Because of unix line end notation..
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);
    }

    // Resolving which char is delimiter
    char *del = ",";
    if( line.find(',') == std::string::npos )
        del = ";";
    else demads = cStringTokenizer( line.c_str() , del ).asDoubleVector();

    WATCH_VECTOR(demads);

    // Initialise sending
    if (par("send").boolValue()) {
        cMessage *init= new cMessage();
        init->addPar("initiateSending");
        scheduleAt(0, init);
    }

    psend=0;
}

void CityGenerator::handleMessage(cMessage *msg)
{
    double alpha= par("alpha").doubleValue();
    int length = par("length").longValue();
    int step = 100;

    // Prepare sending
    if ( msg->isSelfMessage() and msg->hasPar("initiateSending") ){
        std::vector<double>::iterator it;
        int dst=0;

        for (it = demads.begin(); it < demads.end(); it++) {
            dst++;
            if( dst == src ) continue;
            double demand= (*it)*alpha;
            if( demand == 0 ) continue;

            // First arrival time
            arrivals[dst]=simTime();

            EV << "Preparing "<<n<<" payload packets to " << dst << " with demand "<<demand <<"Gbps"<< endl;
            // Mean time between incomes of the demands .. based on A= lambda*tos
            // tos = lenght / C  - Where lengthB is mean value of length and C is bitrate in bps
            // demand/C - in conversion of traffic matrix into Erlang notation
            double lambda= demand*1e9 / length ;
            EV << " + alpha="<<alpha<<" demand="<<(*it)<<" alpha*demand="<<demand;
            EV << " lambda="<<lambda <<endl;

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

simtime_t CityGenerator::sendAmount(int amount, int src, int dst, double lambda, int length){
    for (int i = 0; i < amount; i++) {
        // E[t]=int(t*lambda*exp(-lambda*t),t=0..oo)=1/lambda
        simtime_t gap = exponential(1/lambda);
        Payload *pl = new Payload();
        pl->setBitLength(length);
        pl->setDst(dst);
        pl->setSrc(src);
        pl->setT0(arrivals[dst]);
        scheduleAt(arrivals[dst], pl);
        EV << " + " << src << "->" << dst << " (" << length / 8 << "B): " << arrivals[dst] << endl;
        arrivals[dst]+=gap;
        psend++;
    }

    EV << "Next sending is planned at " << arrivals[dst] << endl;
    return arrivals[dst];
}

void CityGenerator::finish(){
    recordScalar("Packets sent", psend);
}
