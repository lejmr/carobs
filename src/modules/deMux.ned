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

simple DeMux
{
    parameters:
        bool mux = default(false);
        @display("i=block/classifier2;is=s");
        
        // O band	original	1260 to 1360 nm
        // E band	extended	1360 to 1460 nm
        // S band	short wavelengths	1460 to 1530 nm
        // C band	conventional ("erbium window")	1530 to 1565 nm
        // L band	long wavelengths	1565 to 1625 nm
        // U band	ultralong wavelengths	1625 to 1675 nm
        double c_0 @unit(m) = default(1550.12e-9m);	// Base wavelength of channel 0

        // Quality of tranmission
        double att = default(3);		// dB	- power loss caused by WDM demultiplexation
        double OSNR_drop = default(10);	// dB	- OSNR when the optical signal is dropped

        // Amplifier
        bool amp = default(true);		// Allows turn off amplification as pre-amp/booster
        double B_0 @unit(Hz) = default(100GHz);	// Channel spacing
        double G_0 = default(30);	// dB	- Non-saturated gain
        double P_sat = default(16);	// dBm	- when the amplifier is satureted and distorts optical signal
        double NF = default(4.77);	// dB 	- static noise figure
        double A1 = default(500);	//Noise factor model parameter	.. based on [22] from Pedrola et al.
        double A2 @unit(mW) = default(2W);	//Noise factor model parameter .. based on [22] from Pedrola et al.
}


simple MUX extends DeMux
{
	parameters:
        double Lmux = default(3);	// dB	- power loss caused by WDM demultiplexation
        att = this.Lmux;
    
    mux = true;
    gates:
        input control;
        input soa;
        output out;
}

simple DEMUX extends DeMux
{
	parameters:
        double Ldemux = default(3);	// dB	- power loss caused by WDM multiplexation
        att = this.Ldemux;
        
    mux = false;
    gates:
        input in;
        output control;
        output soa;
}
