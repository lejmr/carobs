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

package carobs.modules;
//
// TODO documentation
//
module Endnode
{
    @display("bgb=182,193;i=abstract/router");
    gates:
        inout gate[];

    submodules:
        endpoint: Endpoint {
            @display("p=37,45");
        }
        AP: AggregationPool {
            @display("p=37,124");
        }
        iface: OBSLaunch {
            @display("p=123,124");
        }
        co: Conctration {
            @display("p=123,45");
        }

    connections:
        endpoint.gate$o --> AP.in;
        AP.out --> iface.in;
        co.out --> endpoint.gate$i;

        for i=1..sizeof(gate) {
            iface.out[i] --> gate$o[i];
            gate$i[i] --> co.in[i];
        }



}

