#include <stdio.h>
#include <sstream>
#include "MACEntry.h"


MACEntry::MACEntry(int op, int ol, simtime_t a){
	this->outPort_var= op;
	this->outLambda_var= ol;
	this->available_var= a;

}

std::string MACEntry::info() const
{
    std::stringstream out;
    out << "Port:" << outPort_var << " ";
    out << "WL: " << outLambda_var << " ";
    out << "Available: "<< available_var<< " ";
    return out.str();
}

