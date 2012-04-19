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

#include "merge.h"

Define_Module(Merge);

void Merge::initialize(){}

void Merge::handleMessage(cMessage *msg)
{

    // Receice optical layer message
    OpticalLayer *ol=  dynamic_cast<OpticalLayer *>(msg);

    // Convet it into a Car
    Car *car= (Car *)ol->decapsulate();

    // Withraw all Payload packets from Car and send it to Payload router
    cQueue q = car->getPayload();
    while(!q.empty()){
        Payload *p= (Payload *) q.pop();
        send(p, "out");
    }

    // Drop empty container
    delete car;
    delete msg;
}
