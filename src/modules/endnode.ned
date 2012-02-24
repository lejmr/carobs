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
    @display("bgb=361,254;i=abstract/router");
    gates:
        inout gate[];

    submodules:
        endpoint: Endpoint {
            @display("p=38,137");
        }
        AQ: AggregationQueues {
            @display("p=134,180");
        }
        TA: TrainAssembler {
            @display("p=215,180");
        }
        co: Merge {
            @display("p=215,121");
            gates:
                in[sizeof(gate)];
        }

        routing: Routing {
            @display("p=215,34");
        }
        TS: TrainScheduler {
            @display("p=299,180");
        }
    connections:
        endpoint.gate$o --> AQ.in;
        AQ.out --> TA.in;
        co.out --> endpoint.gate$i;
		TA.out --> TS.in;
        for i=0..sizeof(gate)-1 {
            TS.out[i] --> gate$o[i];
            gate$i[i] --> co.in[i];
        }



}

