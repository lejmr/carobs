#include <stdio.h>
#include <sstream>
#include "CplexRouteEntry.h"


CplexRouteEntry::CplexRouteEntry(int pid, int s, int d, bool hops){
    this->path_id_var= pid;
    this->source_var= s;
    this->dest_var= d;
	this->inPort_var= -1;
	this->inLambda_var= -1;
	this->outPort_var= -1;
	this->outLambda_var= -1;
	this->hops_var=-1;
	this->vis_hops= hops;
}


void CplexRouteEntry::setInput(int port, int wl) {
    inPort_var = port;
    inLambda_var = wl;
}

void CplexRouteEntry::setOutput(int port, int wl) {
    outPort_var = port;
    outLambda_var = wl;
}


void CplexRouteEntry::countOT(simtime_t d_p, simtime_t d_s) {
    OT_var = 0;

    // There was only one routing rule .. one line, which means that destination is just next hop
    // Then we need to provide atleast one d_p!
    if( hops_var <= 0 ) OT_var= d_s + d_p;
    else OT_var= d_p * hops_var + d_s;
}




std::string CplexRouteEntry::info() const
{
    std::stringstream out;

    out << "Path "<< path_id_var << "= " << source_var << "->"<<dest_var<<" ... ";

    if( inPort_var < 0 ) out << "Emitting to ";
    if( inPort_var >= 0 ) out << inPort_var<<"#"<<inLambda_var;

    out <<  " >> ";

    if( outPort_var < 0 ) out << "Terminating ";
    if( outPort_var >= 0 ) out << outPort_var<<"#"<<outLambda_var;

    if( vis_hops ) out <<" hops="<<hops_var  <<";OT="<<OT_var;

    /*
    out <<" DEBUG: ";
    out << inPort_var<<"#"<<inLambda_var;
    out <<  " >> ";
    out << outPort_var<<"#"<<outLambda_var;
    */

    return out.str();
}

/*
const char *CplexRouteEntry::getName() {
    std::stringstream out;
    out << "Path " << path_id_var << ":" << source_var << " -> "<<dest_var;
    return out.str();
}
*/

CplexRouteEntry *CplexRouteEntry::dup() const
{
    return new CplexRouteEntry(*this);
}
