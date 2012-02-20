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

#include "generator.h"
#include "messages/Payload_m.h"

Define_Module(Generator);

using namespace std;

void Generator::initialize()
{
    long myMinID = par("myMinID").longValue();
    long myMaxID = par("myMaxID").longValue();
    long maxID = par("maxID").longValue();
    long amount = par("n").longValue();
    double gap = par("l").doubleValue();

    int rnd;
    for(int i=0; i < amount;i++){
        // Generate random destination, but must take care of not generating for myself
        while(true){
            rnd= rand() / double(RAND_MAX) * maxID;
            if ( rnd > myMaxID or rnd < myMinID ) break;
        }
        Payload *msg = new Payload();
        msg->setByteLength( (int) normal(1500,100) ); // Generates frames with mean size of 1500B and deviation 100 for normal distribution
        msg->setDst(rnd);
        msg->setSrc(myMinID);
        scheduleAt(simTime()+i*gap, msg);
    }
}

void Generator::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
    send(msg, "out");
}
