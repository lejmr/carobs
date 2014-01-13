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
simple SOAManager
{
    parameters:
        double d_p = default(10ms) @unit(s);	// Processing time of SOA Manager
        double d_s = default(10us) @unit(s);	// Switching time of SOA Matrix
        int maxWL = default(10);		// Max number number of wavelengths
        bool WC = default(true);				// Wavelength conversion
        bool OBS = default(true);	// Sets OBS mode or CAROBS mode = false
        double datarate= default(1Gbps) @unit(bps);	// Datarate
        double datarate_correction = default(1.00009616);	// Modification used in order overcome gap between datarate and bandwidth speed
        bool buffering = default(true);			// All buffering at the CoreNode in order of loss-less
        bool fifo = default(true);				// Wavelength election method .. false= rand
        double convPerformance = default(0.1e-9);	// ns/bit
        bool prioritizeBuffered = default(false);				// Gives extra priority (extra OT) when the burst train was buffered
        
    @display("i=block/cogwheel");
    gates:
        inout control[];
        input aggregation;
}
