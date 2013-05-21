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
// Train assembler based on chapter 8.2.3 - Offset Time Computation of Coutelen DT
// This implementation sorts based on ot^c size
//
simple TrainAssembler
{
    parameters:
        //int poolTreshold @unit(B) = default(9KiB) ; // Bytes
        double d_p @unit(s) = default(10ms);	// Processing time of SOA Manager
        double d_s @unit(s) = default(10us);	// Switching time of SOA Matrix
        double datarate= default(1Gbps) @unit(bps);	// Datarate
        double datarate_correction = default(1.00009616);	// Modification used in order overcome gap between datarate and bandwidth speed
        bool   CTA = default(false);	// Turns Curbet train algorithm reducing E-to-E delay
        
    @display("i=block/join,#FF8000");
    gates:
        inout in;
        output out;
}


