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

#include "routing.h"

Define_Module(Routing);

using namespace std;

void Routing::initialize() {

    cMessage *mp = new cMessage("PortAndOtDiscovery");
    mp->addPar("PortAndOtDiscovery");
    scheduleAt(simTime(), mp);

    // Read given parameter d_p - processing time of SOAmanager
    d_p = par("d_p").doubleValue();
    d_s = par("d_s").doubleValue();

    // Create watchers
    WATCH_MAP(DestMap);
    WATCH_SET(terminatingIDs);
    RoutingTable.setName("RoutingTable");

    WATCH_MAP(inPort);
    WATCH_MAP(outPort);
    WATCH_MAP(paths);
}

bool cache_sort (std::string i,std::string j) {
    std::vector<int> left= cStringTokenizer(i.c_str(),"-").asIntVector();
    std::vector<int> right= cStringTokenizer(j.c_str(),"-").asIntVector();

    return i[1] != j[0];
}

void Routing::handleMessage(cMessage *msg) {

    if (msg->isSelfMessage() and msg->hasPar("PortAndOtDiscovery")) {

        // Get address of node in the network
        int src = this->getParentModule()->par("address").longValue() - 100;


        // Get info about the neighbours .. map node id with port id
        EV << "Testing "<< getParentModule()->gateSize("gate") << " connected gates (output)."<<endl;
        for (int i=0; i<getParentModule()->gateSize("gate$o"); i++) {
            EV << "gate$o["<<i <<"] = "<< getParentModule()->gate("gate$o",i)->getNextGate()->getOwnerModule()->par("address").longValue() << endl;
            outPort[ getParentModule()->gate("gate$o",i)->getNextGate()->getOwnerModule()->par("address").longValue()-100 ] = i;
        }

        EV << "Testing "<< getParentModule()->gateSize("gate") << " connected gates (input)."<<endl;
        for (int i=0; i<getParentModule()->gateSize("gate$i"); i++) {
            EV << "gate$i["<<i <<"] = "<< getParentModule()->gate("gate$i",i)->getPreviousGate()->getOwnerModule()->par("address").longValue() << endl;
            inPort[ getParentModule()->gate("gate$i",i)->getPreviousGate()->getOwnerModule()->par("address").longValue() - 100] = i;
        }


        // Parse the routing file
        const char *filename = par("trafficFile");
        std::ifstream infile;
        infile.open(filename);
        EV << "Loading data from " << filename << endl;
        if (!infile.is_open())
            opp_error("Error opening routing table file `%s'", filename);

        // Parse input file and obtain the traffic matrix
        std::string line;
        std::vector<double> tmp;
        std::map<int, int> tmpOT;
        std::map<int, CplexRouteEntry *> tmpa;
        std::set<int> paths_to_check;
        while (getline(infile, line)) {


            // Because of unix line end notation..
            if (!line.empty() && line[line.size() - 1] == '\r')
                line.erase(line.size() - 1);

            // Parse line
            tmp = cStringTokenizer(line.c_str(), ";").asDoubleVector();
            // id, s,d,li,lo,wl,phi
            //  0  1 2 3   4  5  6

            // Filter only related lines
            if( (int)tmp[1] == src or (int)tmp[3] == src or (int)tmp[4] == src ){

                // Save for path check
                if( tmp[1] == src ) paths_to_check.insert( tmp[0] );

                // 1. Find the CplexRouteEntry
                if (tmpa.find((int) tmp[0]) == tmpa.end()){
                    EV << " Initiating entry: " << line << endl;
                    tmpa[ tmp[0] ] = new CplexRouteEntry(tmp[0], tmp[1], tmp[2], tmp[1]==src );
                }

                // Increase hop indicator
                tmpa[ tmp[0] ]->addHop();

                // Starting link
                if ( tmp[1] == src and tmp[3] == src) {
                    EV << " Starting link: "<< line << endl;
                    tmpa[ tmp[0] ]->setOutput( outPort[ tmp[4] ], tmp[5]);
                }

                // Switching - input
                if( tmp[1] != src and tmp[4] == src ){
                    tmpa[ tmp[0] ]->setInput( inPort[ tmp[3] ], tmp[5]);
                    //EV << "link from " << tmp[3] << " is " << inPort[ tmp[3] ] << endl;
                }

                // Switching - output
                if (tmp[2] != src and tmp[3] == src) {
                    EV << " Switching - output link: " << line <<endl;
                    tmpa[ tmp[0] ]->setOutput( outPort[ tmp[4] ], tmp[5]);
                    //EV << "link to " << tmp[4] << " is " << outPort[ tmp[4] ] << endl;
                }

                // This this the terminating node
                if( tmp[2]==src and tmp[4] == src){
                    EV << " Terminating node: "<< line << endl;
                    tmpa[ tmp[0] ]->setInput( inPort[ tmp[3] ], tmp[5]);
                }

            }

        }

        // Ordered routing file reading
        infile.close();
        for(std::set<int>::iterator it=paths_to_check.begin();it!=paths_to_check.end();it++){
            /* Every loop focuses on different path_id .. label */

            // Open and read file again
            std::ifstream infile;
            infile.open(filename);
            std::string APname;
            std::vector< std::string > cache, scache;
            while (getline(infile, line)) {

                // Because of unix line end notation..
                if (!line.empty() && line[line.size() - 1] == '\r')
                    line.erase(line.size() - 1);

                // Parse line
                tmp = cStringTokenizer(line.c_str(), ";").asDoubleVector();

                // Filter important one
                if( tmp[0] != *it ) continue;

                // Zpracuji
                std::ostringstream _id, _id2;
                _id << tmp[3] <<"-"<<tmp[4];
                cache.push_back( _id.str() );

                // Construct AP identifier
                _id2 << tmp[5] << "-" << tmp[2] << "-" << tmp[0];
                APname = _id2.str();
            }
            infile.close();

            // Sort links that are related to the path
            int tsrc=src;
            while( cache.size() > scache.size() ){
                for (std::vector<std::string>::iterator i = cache.begin(); i != cache.end(); ++i) {
                    if( i == cache.end()) continue;
                    std::vector<int> l= cStringTokenizer( (*i).c_str(), "-").asIntVector();
                    if( l[0] == tsrc ){
                        scache.push_back(*i);
                        tsrc= l[1];
                    }
                }
            }

            // Convert the sorted cache to the line of nodes on path
            // Opening node
            if (paths.find(APname) == paths.end()) {
                std::ostringstream stm;
                stm << src;
                paths[APname] = stm.str() + " ";
            }
            // All the other nodes
            for (std::vector<std::string>::iterator i = scache.begin(); i != scache.end(); i++) {
                std::vector<int> l = cStringTokenizer((*i).c_str(), "-").asIntVector();
                std::ostringstream stm;
                stm << l[1];
                paths[APname] += stm.str() + " ";
            }

        }


        // Transfer parsed lines to RoutingTable
        for (std::map<int, CplexRouteEntry *>::iterator it=tmpa.begin(); it!=tmpa.end(); ++it){
            EV << (*it->second).info() << endl;
            RoutingTable.insert( it->second );
            it->second->countOT(d_p,d_s);
        }

        delete msg;
        return;
    }
}


simtime_t Routing::getOffsetTime(int destination) {
    // Check whether destination is in the network, otherwise return -1.0
    // which means drop the burst, cause such destination doesn't exist

    for (cQueue::Iterator iter(RoutingTable, 0); !iter.end(); iter++){
        CplexRouteEntry *r = (CplexRouteEntry *) iter();
        if( r->getdest() == destination ){
            return r->getOT();
        }
    }

    return (simtime_t) -1;
}

simtime_t Routing::getNetworkOffsetTime(int dst) {
    if (DestMap.find(dst) == DestMap.end())
        return  (simtime_t) -1.0;

    return getOffsetTime(100+dst);
}

int Routing::getOutputPort(int destination) {
    // Check whether destination is in the DestMap, otherwise return -1.0
    // which means drop the burst, cause such destination doesn't exist
    return outPort[destination];
}

std::set<int> Routing::getTerminatingIDs() {
    return terminatingIDs;
}


int Routing::getTerminationNodeAddress(int dst){
    Enter_Method("getTerminationNodeAddress()");
    if( DestMap.find(dst)==DestMap.end() )
        DestMap[dst]= dst+100;
    return dst + 100;
}

bool Routing::canForwardHeader(int destination){
    int port= getOutputPort(destination);

    cGate *g= getParentModule()->gate("gate$o", port);
    cObject *c = g->getPathEndGate()->getOwner()->getOwner();

    topo.extractByNedTypeName( cStringTokenizer("carobs.modules.EdgeNode").asVector());
        // Calculate translation of destination network vs terminating node
    for (int i = 0; i < topo.getNumNodes(); i++) {
        cTopology::Node *node = topo.getNode(i);
        if( node->getModule() == c ) return false;
    }

    return true;
}

void Routing::sourceAddressUpdate(int min, int max){
    Enter_Method("sourceAddressUpdate");
    for (int j = min; j <= max; j++) terminatingIDs.insert(j);
}


CplexRouteEntry Routing::getRoutingEntry(int label){
    Enter_Method("getRoutingEntry");

    for (cQueue::Iterator iter(RoutingTable, 0); !iter.end(); iter++) {
        CplexRouteEntry *r = (CplexRouteEntry *) iter();
        if (r->getLabel() == label) {
            return *r;
        }
    }

    opp_terminate("!!! No routing for requested label!!! ");
}
