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

package nsf;
import carobs.modules.CoreNode;

import carobs.link.FiberChannel;

//
// Network generated from /carobs/references/nsf/Book1.csv_lines.csv and /carobs/references/nsf/Book1.csv_nodes.csv
//
network unitedstates
{
    @display("bgb=989,592");
    submodules:
        CA1: CoreNode {
            @display("p=64,206");
        }
        CA2: CoreNode {
            @display("p=144,440");
        }
        CO: CoreNode {
            @display("p=363,281");
        }
        DC: CoreNode {
            @display("p=874,380");
        }
        GA: CoreNode {
            @display("p=716,522");
        }
        IL: CoreNode {
            @display("p=610,281");
        }
        MI: CoreNode {
            @display("p=691,167");
        }
        NE: CoreNode {
            @display("p=478,281");
        }
        NJ: CoreNode {
            @display("p=953,289");
        }
        NY: CoreNode {
            @display("p=849,156");
        }
        PA: CoreNode {
            @display("p=775,281");
        }
        TX: CoreNode {
            @display("p=439,546");
        }
        UT: CoreNode {
            @display("p=249,206");
        }
        WA: CoreNode {
            @display("p=160,83");
        }
    connections:
        CA2.gate++ <--> FiberChannel {  length = 834 bps; } <--> CA1.gate++;
        NE.gate++ <--> FiberChannel {  length = 870 bps; } <--> CO.gate++;
        NE.gate++ <--> FiberChannel {  length = 864 bps; } <--> IL.gate++;
        NJ.gate++ <--> FiberChannel {  length = 312 bps; } <--> DC.gate++;
        NJ.gate++ <--> FiberChannel {  length = 942 bps; } <--> MI.gate++;
        NY.gate++ <--> FiberChannel {  length = 468 bps; } <--> DC.gate++;
        NY.gate++ <--> FiberChannel {  length = 720 bps; } <--> MI.gate++;
        PA.gate++ <--> FiberChannel {  length = 1008 bps; } <--> GA.gate++;
        PA.gate++ <--> FiberChannel {  length = 846 bps; } <--> IL.gate++;
        PA.gate++ <--> FiberChannel {  length = 540 bps; } <--> NJ.gate++;
        PA.gate++ <--> FiberChannel {  length = 438 bps; } <--> NY.gate++;
        TX.gate++ <--> FiberChannel {  length = 2520 bps; } <--> CA2.gate++;
        TX.gate++ <--> FiberChannel {  length = 1746 bps; } <--> CO.gate++;
        TX.gate++ <--> FiberChannel {  length = 2364 bps; } <--> DC.gate++;
        TX.gate++ <--> FiberChannel {  length = 1350 bps; } <--> GA.gate++;
        UT.gate++ <--> FiberChannel {  length = 1152 bps; } <--> CA1.gate++;
        UT.gate++ <--> FiberChannel {  length = 684 bps; } <--> CO.gate++;
        UT.gate++ <--> FiberChannel {  length = 2820 bps; } <--> MI.gate++;
        WA.gate++ <--> FiberChannel {  length = 1338 bps; } <--> CA1.gate++;
        WA.gate++ <--> FiberChannel {  length = 2056 bps; } <--> CA2.gate++;
        WA.gate++ <--> FiberChannel {  length = 3408 bps; } <--> IL.gate++;
}
