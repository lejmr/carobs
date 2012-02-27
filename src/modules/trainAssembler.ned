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
        int poolTreshold = default(9000); // Bytes
        double d_p = default(10us) @unit(s); // Default is 10microseconds
        
    @display("i=block/join,#FF8000");
    gates:
        input in;
        output out;
}


