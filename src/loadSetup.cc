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

#include "loadSetup.h"
#include "manualGenerator.h"

Define_Module(LoadSetup);

using namespace std;

void LoadSetup::initialize()
{
    load= par("load").doubleValue();
    n= par("sources").longValue();
    maxWL= par("maxWL").longValue();
    double datarate= par("datarate").doubleValue();
    std::stringstream out;

    float data[n], sum=0;
    for(int i=0;i<n;i++){
        data[i]= (float) uniform(0, load);
        sum+=data[i];
    }

    for(int i=0;i<n;i++){
        EV << "Recalculated "<<i<<" "<< data[i]<<"->"<<data[i]*load/sum << endl;
        data[i]= data[i]*load/sum;
    }

    int i = 0;
    double allocated= 0;
    for (cSubModIterator iter(*getParentModule()); !iter.end(); iter++)
    {
        if( !strcmp(iter()->getFullName(),"e1")) continue;
        for( cSubModIterator iter2(*iter());!iter2.end(); iter2++){
            if( !strcmp( iter2()->getFullName(), "generator") ){
                float ta= data[i++];
                EV << "Setting up " << iter()->getFullName() << ".generator to ("<<ta<<")" << ta*datarate <<"*"<<maxWL<< endl;
                //ManualGenerator mg= (ManualGenerator) iter2();
                iter2()->par("bandwidth").setLongValue(ta*datarate*maxWL);
                allocated+= ta*datarate*maxWL;

                cModule *calleeModule = iter2();
                ManualGenerator *callee = check_and_cast<ManualGenerator *>(calleeModule);
                callee->updateLambda(ta*datarate*maxWL);

                // Log the value
                out.str("");
                out <<  "Setting up " << iter()->getFullName() << ".generator to";
                recordScalar(out.str().c_str(), ta*datarate*maxWL );
            }
        }
    }

    EV << "Allocated: "<<allocated<<"/"<<load*datarate*maxWL<<" ("<<load<<"*"<<datarate<<"*"<<maxWL<<")" << endl;
    EV << "Allocation coef: "<< allocated/(load*datarate*maxWL) << endl;

}

void LoadSetup::handleMessage(cMessage *msg)
{

}
