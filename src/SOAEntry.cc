#include <stdio.h>
#include <sstream>
#include "SOAEntry.h"


SOAEntry::SOAEntry(int ip, int il, int op, int ol){
	this->inPort_var= ip;
	this->inLambda_var= il;
	this->outPort_var= op;
	this->outLambda_var= ol;
	this->valid = true;
}

std::string SOAEntry::info() const
{
    std::stringstream out;
    out << "In>> Port:" << inPort_var << " ";
    out << "@" << inLambda_var << " ";
    out << "Out>> Port:" << outPort_var << " ";
    out << "@" << outLambda_var << " ";
    out << "for= "<<start_var<<"-"<<stop_var<< " ";
    return out.str();
}

