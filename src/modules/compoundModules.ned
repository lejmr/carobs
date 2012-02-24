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
module AggregationPool
{
    @display("i=block/subqueue;bgb=260,103");
    gates:
        input in[];
        output out[];

    submodules:
        AQ: AggregationQueues {
            @display("p=46,47");
        }
        TA: TrainAssembler {
            @display("p=127,47");
        }
        TS: TrainScheduler {
            @display("p=211,47");
            gates:
                out[sizeof(out)];
        }
    connections:
        AQ.out --> TA.in;
        TA.out --> TS.in;
        for i=0..sizeof(out)-1 {
            TS.out[i] --> out[i];
        }
        for i=0..sizeof(in)-1 {
            in[i] --> AQ[i];
        }
}

