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
    parameters:
        int myMinID;
        int myMaxID;

        @display("i=block/rx");
    gates:
        input in;
}

//
// TODO documentation
//
simple Generator
{
    parameters:
        int n = default(10);
        volatile double l = default(1E-3);
        volatile double length = default(1500);
        int myMinID;
        int myMaxID;
        bool send = default(false);
        int dst = default(-1);

        @display("i=block/tx");

    gates:
        output out;
}

//
// TODO documentation
//
module Endpoint
{
    parameters:
        int myMinID;
        int myMaxID;

        @display("i=misc/house;bgb=270,103;t=$myMinID-$myMaxID;tc=gray");
    gates:
        inout gate @labels(Payload);

    submodules:
        sink: Sink {
            parameters:
                myMinID = myMinID;
                myMaxID = myMaxID;

                @display("p=58,51");
        }
        generator: Generator {
            parameters:
                myMinID = myMinID;
                myMaxID = myMaxID;
                @display("p=215,51");
        }
        routingTable: RoutingTable {
            parameters:
                myMinID = myMinID;
                myMaxID = myMaxID;
                @display("p=133,51");
        }
    connections:
        generator.out --> gate$o;
        sink.in <-- gate$i;
}



simple CityGenerator
{
    parameters:
        int n = default(10);
        int address;
        string ipDemandFile;
        double datarate @unit(bps) = default(1Gbps);	// Datarate
		bool send = default(true);
		double alpha= default(1);
		int length @unit(b)= default(100b);	// bit
		
        @display("i=block/tx,#FF8000");

    gates:
        output out;
}


module City
{
    parameters:
        int address;
        int myMinID = address;
        int myMaxID = address;
        
        @display("i=misc/town;bgb=179,98;t=$address;tc=gray");
    gates:
        inout gate @labels(Payload);

    submodules:
        sink: Sink {
            parameters:
                myMaxID = address;
                myMinID = address;
                @display("p=51,53");
        }
        generator: CityGenerator {
            parameters:
                address = address;
                @display("p=127,53");
        }

    connections:
        generator.out --> gate$o;
        sink.in <-- gate$i;
}

