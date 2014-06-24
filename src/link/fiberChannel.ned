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

package carobs.link;

import ned.DelayChannel;


channel FiberChannel extends DelayChannel
{
	// OMNeT++ properties
	@class(FiberChannel);
    @display("ls=blue");
     
	// Physical properties parameters   
    double length @unit(km);
    delay = this.length / 200km * 1ms;
	
    // Informational parameters .. in future will be extended
    double d_s @unit(s) = default(10us);	// Switching time of SOA Matrix
    double datarate @unit(bps) = default(1Gbps);	// Datarate	
    double datarate_correction = default(1.00009616);	// Modification used in order overcome gap between datarate and bandwidth speed
    
    // Bandwidth measurements
    double thr_window @unit(s) = default(100us);
	
    
}
