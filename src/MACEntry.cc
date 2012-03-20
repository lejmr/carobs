#include <stdio.h>
#include <sstream>
#include "MACEntry.h"


MACEntry::MACEntry(int op, int ol, simtime_t from, simtime_t to){
	this->outPort_var= op;
	this->outLambda_var= ol;
	this->from_var= from;
	this->to_var= to;

}

std::string MACEntry::info() const
{
    std::stringstream out;
    out << "Port:" << outPort_var << " ";
    out << "WL: " << outLambda_var << " ";
    out << "Reserved: "<< from_var<< "-"<<to_var;
    return out.str();
}

