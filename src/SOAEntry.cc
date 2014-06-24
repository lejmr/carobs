#include <stdio.h>
#include <sstream>
#include "SOAEntry.h"


SOAEntry::SOAEntry(int ip, int il, int op, int ol){
	this->inPort_var= ip;
	this->inLambda_var= il;
	this->outPort_var= op;
	this->outLambda_var= ol;
	this->valid = true;
	this->aggregation_var = false;
	this->disaggregation_var= false;
	this->buffer_var = false;
	this->identifier = -1;
	this->H_sent=false;
}


/**
 *  This is Aggregation/ Disaggregation SOA table entry only!
 *  port - in/out port which is used by a burst
 *  wl   - represents wavelength of the given burst
 *  aggreagtion - informs whether the entry is true=aggregation or false=disaggregation
 */
SOAEntry::SOAEntry(int port, int wl, bool aggregation){
    this->inPort_var= -1;
    this->inLambda_var= -1;
    this->outPort_var= -1;
    this->outLambda_var= -1;

    if( aggregation ) {
        this->outPort_var= port;
        this->outLambda_var= wl;
    }else{
        this->inPort_var = port;
        this->inLambda_var = wl;
    }

    this->valid = true;
    this->aggregation_var = aggregation;
    this->disaggregation_var= not aggregation;
    this->buffer_var = false;
    this->identifier = -1;
    this->H_sent=false;
}

std::string SOAEntry::info() const
{
    std::stringstream out;

    if( buffer_var and not buffer_in_var ){
            out << "In>> Buffer ";
    } else if (  not buffer_var and aggregation_var ){
        out << "In>> Aggregation ";
    }else{
        out << "In>> Port:" << inPort_var ;
        out << "#" << inLambda_var << " ";
    }



    if (buffer_var and buffer_in_var) {
        out << "Out>> Buffer ";
    } else if (not buffer_var and disaggregation_var) {
        out << "Out>> Disaggregation ";
    } else {
        out << "Out>> Port:" << outPort_var;
        out << "#" << outLambda_var << " ";
    }



    out << "for= "<<start_var<<"-"<<stop_var<< " ";
    return out.str();
}

SOAEntry *SOAEntry::dup() const
{
    return new SOAEntry(*this);
}
