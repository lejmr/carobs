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

    // Obtaining processing time parametr JET
    JET = par("JET").doubleValue();

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
    //int outPort = R->getOutputPort( dst );
    int outPort = 1;

    // Prepare SOA switching table entry
    SOAEntry *se = new SOAEntry(inPort, inWl, outPort, inWl);

    // Assign this SOAEntry to SOA
    if( JET ) soa->assignSwitchingTableEntry(se, H->getOT(), H->getLength() );  //JET
    else soa->assignSwitchingTableEntry(se, 0 , H->getOT()+H->getLength() );    // JIT

    // Put the headers  back to OpticalLayer E-O conversion
    ol->encapsulate(H);

    // Resending CAROBS Header to next CoreNode
    sendDelayed(ol, simTime()+d_p, "control$o", outPort );

}
