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
    int datarate= par("datarate").longValue();

    double to_alloc=load;
    int i = 1;

    for (cSubModIterator iter(*getParentModule()); !iter.end(); iter++)
    {
        if( !strcmp(iter()->getFullName(),"e1")) continue;
        for( cSubModIterator iter2(*iter());!iter2.end(); iter2++){
            if( !strcmp( iter2()->getFullName(), "generator") ){
                double ta= to_alloc;
                if( i++ < n ){
                    ta= uniform(0, to_alloc);
                    to_alloc-=ta;
                }
                EV << "Setting up " << iter()->getFullName() << ".generator to ("<<ta<<")" << ta*datarate << endl;
                //ManualGenerator mg= (ManualGenerator) iter2();
                iter2()->par("bandwidth").setLongValue(ta*datarate);

                cModule *calleeModule = iter2();
                ManualGenerator *callee = check_and_cast<ManualGenerator *>(calleeModule);
                callee->updateLambda(ta*datarate);
            }
        }
    }

}

void LoadSetup::handleMessage(cMessage *msg)
{

}
