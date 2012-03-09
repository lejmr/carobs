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
// TODO auto-generated module
//
simple MAC
{
    parameters:
        int maxWL = default(10);		// Max number number of wavelengths
        double guardTime = default(10ms) @unit(s); // Guarding time, least time when a burst can be send to network on the same wavelength and port
         
        @display("i=block/tx");
    gates:
        input in;
        output out[];
}
