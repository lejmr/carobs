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
    
    // Simulation parameters
    // O band	original	1260 to 1360 nm
    // E band	extended	1360 to 1460 nm
    // S band	short wavelengths	1460 to 1530 nm
    // C band	conventional ("erbium window")	1530 to 1565 nm
    // L band	long wavelengths	1565 to 1625 nm
    // U band	ultralong wavelengths	1625 to 1675 nm
    double c_0 @unit(m) = default(1550.12e-9m);	// Base wavelength of channel 0
    
	// Physical properties parameters   
    double length @unit(km);
    delay = this.length / 200km * 1ms;
    double att = default(0.2);	// db/km  .. attenuation of optical fiber per kilometer
    double D_pmd = 0.05;	// ps/sqrt(km) .. PMD coefficient
    int WL = default(50);	// Number of waveleghts used in network
    
    // Amplifier
    double B_0 @unit(Hz) = default(100GHz);	// Channel spacing
    double G_0 = default(30);	// dB	- Non-saturated gain
    double P_sat = default(16);	// dBm	- when the amplifier is satureted and distorts optical signal
    double NF = default(4.77);	// dB 	- static noise figure
    double A1 = default(500);	//Noise factor model parameter	.. based on [22] from Pedrola et al.
    double A2 @unit(mW) = default(2W);	//Noise factor model parameter .. based on [22] from Pedrola et al.

    // Quality of tranmission
    double OSNR_qot = default(23);	// dB 	- minimal OSNR, if less the optical signal needs to be regenerated
    double OSNR_drop = default(10);	// dB	- OSNR when the optical signal is dropped

    // Informational parameters .. in future will be extended
    double d_s @unit(s) = default(10us);	// Switching time of SOA Matrix
    double datarate @unit(bps) = default(1Gbps);	// Datarate	
    double datarate_correction = default(1.00009616);	// Modification used in order overcome gap between datarate and bandwidth speed
    
    // Bandwidth measurements
    double thr_window @unit(s) = default(100us);
	
    
}
