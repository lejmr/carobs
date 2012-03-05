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
module CoreNode
{
    parameters:
        int address = default(uniform(1, 1000));	// TODO: watchout mi overlap 
        double d_p @unit(s) = default(10ms);	// Processing time of SOA Manager
        double d_s @unit(s) = default(10us);	// Switching time of SOA Matrix

        @display("bgb=326,349;i=abstract/switch;t=$address");
    gates:
        inout gate[];


    submodules:
        routing: Routing {
            @display("p=55,44");
        }

        mux[sizeof(gate)]: MUX {
            @display("p=47,212");
        }

        demux[sizeof(gate)]: DEMUX {
            @display("p=86,248");
        }

        soa: SOA {
            parameters:
                d_s = d_s;
                @display("p=228,221");
            gates:
                gate[sizeof(gate)];
        }
        soaManager: SOAManager {
            parameters:
                d_p = d_p;
                @display("p=228,104");
            gates:
                control[sizeof(gate)];
        }

    connections:
        for i=0..sizeof(gate)-1 {
            demux[i].soa --> soa.gate$i[i];
            soa.gate$o[i] --> mux[i].soa;
            gate$i[i] --> demux[i].in;
            mux[i].out --> gate$o[i];
            demux[i].control --> soaManager.control$i[i];
            soaManager.control$o[i] --> mux[i].control;
        }

}

