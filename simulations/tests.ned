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

package carobs.simulations;

import carobs.modules.Endnode;


network EndnodeTest
{
    @display("bgb=281,156");
    submodules:
        endnode: Endnode {
            @display("p=57,112");
        }
        endnode1: Endnode {
            @display("p=147,35");
        }
        endnode2: Endnode {
            @display("p=224,112");
        }
    connections:
        endnode.gate++ <--> endnode1.gate++;
        endnode2.gate++ <--> endnode1.gate++;
        endnode.gate++ <--> endnode2.gate++;
}
