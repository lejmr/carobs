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
// Queues manager with ability to make Aggregation of queues (AQs) 
//
simple AggregationQueues
{
    parameters:
        double bufferLengthT = default(0.1); // ms

        @display("i=block/classifier");
    gates:
        input in;
        output out;
}

simple Merge
{
    @display("i=block/join");
    gates:
        input in[];
        output out;
}
