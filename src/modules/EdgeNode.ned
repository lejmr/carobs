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


module EdgeNode
{
    parameters:
        int address = default(uniform(1, 1000));	// TODO: watchout might overlap
        int maxWL = default(10);		// Max number number of wavelengths

        @display("bgb=300,278;i=abstract/router;t=$address");
    gates:
        inout gate[] @labels(CAROBS);
        inout tributary[] @labels(Payload);

    submodules:
        AQ: AggregationQueues {
            @display("p=56,194");
        }
        TA: TrainAssembler {
            @display("p=137,194");
        }
        co: Merge {
            @display("p=223,125");
            gates:
                in[sizeof(gate)];
        }
        routing: Routing {
            @display("p=223,48");
        }

        MAC: MAC {
            parameters:
                maxWL = maxWL;
                @display("p=223,194");
            gates:
                out[sizeof(gate)];
        }
        INT: PayloadInterface {
            @display("p=56,125");
            gates:
                in[sizeof(tributary)];
        }
    connections:
        INT.outgoing --> AQ.in;
        AQ.out <--> TA.in;
        co.out --> INT.incoming;
        for i=0..sizeof(gate)-1 {
            MAC.out[i] --> gate$o[i];
            gate$i[i] --> co.in[i];
        }


        for i=0..sizeof(tributary)-1 {
            tributary[i] <--> INT.in[i];
        }

        TA.out --> MAC.in;

}

