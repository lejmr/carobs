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

#ifndef __CAROBS_ROUTINGTABLE_H_
#define __CAROBS_ROUTINGTABLE_H_

#include <omnetpp.h>

/**
 * TODO - Generated class
 */
class RoutingTable : public cSimpleModule
{
  protected:
    virtual void initialize();

  private:
    std::map<int,int> routingTable;
    std::set<int> localIDs;
    std::set<int> remoteIDs;
    int min, max;

  public:
    virtual std::set<int> getLocalIDs();
    virtual void giveMyLocalIds(std::set<int> localIDs);
    virtual int getNthRemoteAddress( int n);
    virtual int dimensionOfRemoteAddressSet();
};

#endif
