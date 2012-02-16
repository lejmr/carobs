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
simple Sink
{
    @display("i=block/rx");
    gates:
        input in;
}

//
// TODO documentation
//
simple Generator
{
    @display("i=block/tx");
    gates:
        output out;
}

//
// TODO documentation
//
module Endpoint
{
    @display("i=misc/town");
    gates:
        inout gate;

    submodules:
        sink: Sink;
        generator: Generator;
    connections:
        generator.out --> gate$o;
        sink.in <-- gate$i;
}

