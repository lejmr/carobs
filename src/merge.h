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

#ifndef __CAROBS_MERGE_H_
#define __CAROBS_MERGE_H_

#include <omnetpp.h>
#include <messages/OpticalLayer_m.h>
#include <messages/Payload_m.h>
#include <messages/car_m.h>

/**
 *  Object Merge converts CAROBS Car into a sequence of
 *  Payload packets which then sends onto output gate
 */
class Merge : public cSimpleModule
{
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    cOutVector number_of_hops;
};

#endif
