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

package germany;
import carobs.modules.City;
import carobs.modules.CoreNode;
import carobs.link.FiberChannel;
import carobs.modules.Endpoint;


network Germany
{
    @display("bgb=1016,974;bgi=,s");
    submodules:
        B: CoreNode {
            parameters:
                address = 1;
                @display("p=820,239");
        }
        D: CoreNode {
            @display("p=126,450");
        }
        Do: CoreNode {
            @display("p=264,318");
        }
        E: CoreNode {
            @display("p=144,327");
        }
        F: CoreNode {
            @display("p=373,495");
        }
        H: CoreNode {
            @display("p=502,278");
        }
        HB: CoreNode {
            @display("p=379,225");
        }
        HH: CoreNode {
            @display("p=512,175");
        }
        K: CoreNode {
            @display("p=256,439");
        }
        Ka: CoreNode {
            @display("p=320,681");
        }
        L: CoreNode {
            @display("p=805,470");
        }
        M: CoreNode {
            @display("p=795,816");
        }
        Ma: CoreNode {
            @display("p=373,601");
        }
        N: CoreNode {
            @display("p=661,626");
        }
        No: CoreNode {
            @display("p=281,175");
        }
        S: CoreNode {
            @display("p=435,759");
        }
        Ul: CoreNode {
            @display("p=551,816");
        }
       
    connections:
        E.gate++ <--> FiberChannel {  length = 37 km; } <--> Do.gate++;
        E.gate++ <--> FiberChannel {  length = 36 km; } <--> D.gate++;
        HH.gate++ <--> FiberChannel {  length = 306 km; } <--> B.gate++;
        HH.gate++ <--> FiberChannel {  length = 114 km; } <--> HB.gate++;
        H.gate++ <--> FiberChannel {  length = 298 km; } <--> B.gate++;
        H.gate++ <--> FiberChannel {  length = 120 km; } <--> HB.gate++;
        H.gate++ <--> FiberChannel {  length = 208 km; } <--> Do.gate++;
        H.gate++ <--> FiberChannel {  length = 316 km; } <--> F.gate++;
        H.gate++ <--> FiberChannel {  length = 157 km; } <--> HH.gate++;
        K.gate++ <--> FiberChannel {  length = 88 km; } <--> Do.gate++;
        K.gate++ <--> FiberChannel {  length = 41 km; } <--> D.gate++;
        K.gate++ <--> FiberChannel {  length = 182 km; } <--> F.gate++;
        L.gate++ <--> FiberChannel {  length = 174 km; } <--> B.gate++;
        L.gate++ <--> FiberChannel {  length = 353 km; } <--> F.gate++;
        L.gate++ <--> FiberChannel {  length = 258 km; } <--> H.gate++;
        Ma.gate++ <--> FiberChannel {  length = 85 km; } <--> F.gate++;
        Ma.gate++ <--> FiberChannel {  length = 64 km; } <--> Ka.gate++;
        No.gate++ <--> FiberChannel {  length = 144 km; } <--> HB.gate++;
        No.gate++ <--> FiberChannel {  length = 278 km; } <--> Do.gate++;
        N.gate++ <--> FiberChannel {  length = 224 km; } <--> F.gate++;
        N.gate++ <--> FiberChannel {  length = 275 km; } <--> L.gate++;
        N.gate++ <--> FiberChannel {  length = 179 km; } <--> M.gate++;
        S.gate++ <--> FiberChannel {  length = 74 km; } <--> Ka.gate++;
        S.gate++ <--> FiberChannel {  length = 187 km; } <--> N.gate++;
        Ul.gate++ <--> FiberChannel {  length = 143 km; } <--> M.gate++;
        Ul.gate++ <--> FiberChannel {  length = 86 km; } <--> S.gate++;
       
}
