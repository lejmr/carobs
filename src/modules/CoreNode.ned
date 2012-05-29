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
//	Full-featured CAROBS CoreNode
//
module CoreNode
{
    parameters:
        int address = default(uniform(1000, 2000));	// TODO: watchout mi overlap 
        double d_p @unit(s) = default(10ms);	// Processing time of SOA Manager
        double d_s @unit(s) = default(10us);	// Switching time of SOA Matrix
        bool WC = default(true);				// Wavelength conversion 
        int maxWL = default(10);				// Max number number of wavelengths
        bool buffering = default(true);			// All buffering at the CoreNode in order of loss-less

        @display("bgb=499,444;t=$address;i=abstract/switch,,30");
    gates:
        inout gate[] @labels(CAROBS);
        inout tributary[] @labels(Payload);


    submodules:
        routing: Routing {
            @display("p=356,127");
        }

        mux[sizeof(gate)]: MUX {
            @display("p=49,236,c,-45");
        }

        demux[sizeof(gate)]: DEMUX {
            @display("p=86,275,c,45");
        }

        soa: SOA {
            parameters:
                d_s = d_s;
                WC = WC;
                @display("p=228,244");
            gates:
                gate[sizeof(gate)];
                aggregation[sizeof(gate)];
        }
        soaManager: SOAManager {
            parameters:
                d_s = d_s;
                d_p = d_p;
                maxWL = maxWL;
                WC = WC;
                buffering = buffering;
                @display("p=228,127");
            gates:
                control[sizeof(gate)];
        }
        INT: PayloadInterface {
            @display("p=458,244");
            gates:
                in[sizeof(tributary)];
        }
        AQ: AggregationQueues {
            @display("p=399,226;is=s");
        }
        disaggregation: Merge {
            @display("p=356,266;is=s");
        }
        TA: TrainAssembler {
            @display("p=355,226;is=s");
        }
        MAC: CoreNodeMAC {
            @display("p=313,226;is=s");
            gates:
                soa[sizeof(gate)];
        }
    connections:
        for i=0..sizeof(gate)-1 {
            demux[i].soa --> soa.gate$i[i];
            soa.gate$o[i] --> mux[i].soa;
            gate$i[i] --> demux[i].in;
            mux[i].out --> gate$o[i];
            demux[i].control --> soaManager.control$i[i];
            soaManager.control$o[i] --> mux[i].control;
            soa.aggregation[i] <--> MAC.soa[i];
        }

        for i=0..sizeof(tributary)-1 {
            tributary[i] <--> INT.in[i];
        }

        INT.outgoing --> AQ.in;
        disaggregation.out --> INT.incoming;

        soa.disaggregation --> disaggregation.in++;

        AQ.out <--> TA.in;
        TA.out --> MAC.in;
        MAC.control --> soaManager.aggregation;
}
