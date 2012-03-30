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
    for(int i=0;i<src;i++)
        getline(infile,line);
    demads = cStringTokenizer( line.c_str() , ",").asDoubleVector();
    WATCH_VECTOR(demads);
    // Lets generate something


    // Initialise sending
    if (par("send").boolValue()) {
        cMessage *init= new cMessage();
        init->addPar("initiateSending");
        scheduleAt(0, init);
    }
}

void CityGenerator::handleMessage(cMessage *msg)
{
    double alpha= par("alpha").doubleValue();
    int length = par("length").longValue();
    int step = 500;

    // Prepare sending
    if ( msg->isSelfMessage() and msg->hasPar("initiateSending") ){
        std::vector<double>::iterator it;
        int dst=0;

        for (it = demads.begin(); it < demads.end(); it++) {
            dst++;
            if( dst == src ) continue;
            double demand= (*it)*alpha;
            if( demand == 0 ) continue;
            EV << "Preparing "<<n<<" payload packets to " << dst << " with demand "<<demand <<"Gbps"<< endl;
            // Mean time between incomens of the demands .. based on A= lambda*mu
            // mu = lenghtB*8 / C  - Where lengthB is mean value of length and C is bitrate in bps
            // demand/C - in conversion of traffic matrix into Erlang notation
            double lambda= demand/C*1e9 * length/C;

            int sent= 0;
            while( sent+step <= n ){
                cMessage *t = new cMessage();
                t->addPar("amount").setDoubleValue(step);
                t->addPar("lambda").setDoubleValue(lambda);
                t->addPar("src").setDoubleValue(src);
                t->addPar("dst").setDoubleValue(dst);
                t->addPar("scheduling");
                scheduleAt(simTime(),t);
                sent+=step;
            }

            if(sent < n ){
                int toSend= n-sent;
                cMessage *t = new cMessage();
                t->addPar("amount").setDoubleValue(toSend);
                t->addPar("lambda").setDoubleValue(lambda);
                t->addPar("src").setDoubleValue(src);
                t->addPar("dst").setDoubleValue(dst);
                t->addPar("scheduling");
                scheduleAt(simTime(), t);
            }

            std::stringstream out;
            out << "Send messages to " << dst;
            recordScalar(out.str().c_str(), n);

        }

    }

    //Initiate sending Payload packets
    if( msg->isSelfMessage() and msg->hasPar("scheduling")){
        sendAmount( msg->par("amount").doubleValue(),
                msg->par("src").doubleValue(),
                msg->par("dst").doubleValue(),
                msg->par("lambda").doubleValue(),
                length );
    }

    // Send prepared Payload packet
    if(dynamic_cast<Payload *>(msg) != NULL){
        send(msg, "out");
    }
}

void CityGenerator::sendAmount(int amount, int src, int dst, double lambda, int length){
    for (int i = 0; i < amount; i++) {
        simtime_t gap = poisson(lambda);
        Payload *pl = new Payload();
        pl->setBitLength(length);
        pl->setDst(dst);
        pl->setSrc(src);
        pl->setT0(simTime() + gap);
        scheduleAt(simTime() + gap, pl);
        EV << " + " << src << "->" << dst << " (" << length / 8 << "B): " << simTime() + gap << endl;
    }
}
