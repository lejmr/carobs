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

package carobs.simulations;

import carobs.modules.CoreNode;
import carobs.modules.EdgeNode;
import carobs.modules.Endpoint;

network City
{
    @display("bgb=540,184");
    submodules:
        coreNode: CoreNode {
            @display("p=189,89");
        }
        coreNode1: CoreNode {
            @display("p=328,89");
        }
        city: carobs.modules.City {
            @display("p=64,90");
        }
        city1: carobs.modules.City {
            @display("p=451,90");
        }
    connections:
        city.gate <--> coreNode.tributary++;
        coreNode.gate++ <--> coreNode1.gate++;
        coreNode1.tributary++ <--> city1.gate;
}

network GroomIn
{
    @display("bgb=619,223");
    submodules:
        h0: Endpoint {
            @display("p=48,65");
        }
        edgeNode: EdgeNode {
            @display("p=136,65");
        }
        coreNode: CoreNode {
            @display("p=136,158");
        }
        h1: Endpoint {
            @display("p=48,158");
        }
        coreNode1: CoreNode {
            @display("p=231,158");
        }
        coreNode2: CoreNode {
            @display("p=331,158");
        }
        coreNode3: CoreNode {
            @display("p=436,158");
        }
        edgeNode1: EdgeNode {
            @display("p=537,158");
        }
        h2: Endpoint {
            @display("p=231,65");
        }
        h3: Endpoint {
            @display("p=331,65");
        }
        h4: Endpoint {
            @display("p=436,65");
        }
        h5: Endpoint {
            @display("p=537,65");
        }
    connections:
        h0.gate <--> edgeNode.tributary++;
        h1.gate <--> coreNode.tributary++;
        edgeNode.gate++ <--> coreNode.gate++;
        coreNode.gate++ <--> coreNode1.gate++;
        coreNode1.gate++ <--> coreNode2.gate++;
        coreNode2.gate++ <--> coreNode3.gate++;
        coreNode3.gate++ <--> edgeNode1.gate++;
        h2.gate <--> coreNode1.tributary++;
        h3.gate <--> coreNode2.tributary++;
        h4.gate <--> coreNode3.tributary++;
        h5.gate <--> edgeNode1.tributary++;
}


network DisaggregationTest
{
    @display("bgb=533,211");
    submodules:
        h0: Endpoint {
            @display("p=37,88");
        }
        edgeNode: EdgeNode {
            @display("p=142,88");
        }
        coreNode: CoreNode {
            @display("p=248,88");
        }
        edgeNode1: EdgeNode {
            @display("p=360,88");
        }
        h2: Endpoint {
            @display("p=478,88");
        }
        h1: Endpoint {
            @display("p=248,175");
        }
    connections:
        h0.gate <--> edgeNode.tributary++;
        edgeNode.gate++ <--> coreNode.gate++;
        coreNode.gate++ <--> edgeNode1.gate++;
        edgeNode1.tributary++ <--> h2.gate;
        h1.gate <--> coreNode.tributary++;
}
network AggregationTest
{
    @display("bgb=464,189");
    submodules:
        coreNode: CoreNode {
            @display("p=178,89");
        }
        h0: Endpoint {
            @display("p=75,89");
        }
        EdgeNode: EdgeNode {
            @display("p=293,89");
        }
        h1: Endpoint {
            @display("p=398,89");
        }
    connections:
        h0.gate <--> coreNode.tributary++;
        coreNode.gate++ <--> EdgeNode.gate++;
        EdgeNode.tributary++ <--> h1.gate;
}

//
// Burst car creation process
//
network AggregationTest1
{
    @display("bgb=522,216");
    submodules:
        coreNode: CoreNode {
            @display("p=157,54");
        }
        h0: Endpoint {
            @display("p=54,54");
        }
        EdgeNode: EdgeNode {
            @display("p=371,54");
        }
        h1: Endpoint {
            @display("p=465,54");
        }
        coreNode1: CoreNode {
            @display("p=266,159");
        }
        h2: Endpoint {
            @display("p=371,159");
        }
        coreNode2: CoreNode {
            @display("p=266,54");
        }
    connections:
        h0.gate <--> coreNode.tributary++;
        EdgeNode.tributary++ <--> h1.gate;
        coreNode.gate++ <--> coreNode1.gate++;
        h2.gate <--> coreNode1.tributary++;
        coreNode.gate++ <--> coreNode2.gate++;
        coreNode2.gate++ <--> EdgeNode.gate++;
}

//
// Burst car creation process
//
network AggregationPoolTest
{
    @display("bgb=779,243");
    submodules:
        coreNode: CoreNode {
            @display("p=179,89");
        }
        h0: Endpoint {
            @display("p=75,89");
        }
        EdgeNode: EdgeNode {
            @display("p=530,89");
        }
        h1: Endpoint {
            @display("p=635,89");
        }
        coreNode1: CoreNode {
            @display("p=415,89");
        }
        h2: Endpoint {
            @display("p=415,201");
        }
        coreNode2: CoreNode {
            @display("p=296,89");
        }
        h3: Endpoint {
            @display("p=296,201");
        }
    connections:
        h0.gate <--> coreNode.tributary++;
        EdgeNode.tributary++ <--> h1.gate;
        coreNode1.gate++ <--> EdgeNode.gate++;
        h2.gate <--> coreNode1.tributary++;
        coreNode.gate++ <--> coreNode2.gate++;
        coreNode2.gate++ <--> coreNode1.gate++;
        h3.gate <--> coreNode2.tributary++;
}

network AggregationPoolTest1
{
    @display("bgb=631,282");
    submodules:
        v0: CoreNode {
            @display("p=153,150");
        }
        v2: CoreNode {
            @display("p=361,150");
        }
        v1: CoreNode {
            @display("p=259,150");
        }
        v4: CoreNode {
            @display("p=322,69");
        }
        v3: CoreNode {
            @display("p=463,150");
        }
        h4: Endpoint {
            @display("p=424,69");
        }
        h0: Endpoint {
            @display("p=60,150");
        }
        h3: Endpoint {
            @display("p=569,150");
        }
        h1: Endpoint {
            @display("p=259,230");
        }
        h2: Endpoint {
            @display("p=361,230");
        }
        EdgeNode: EdgeNode {
            @display("p=463,230");
        }
        endpoint: Endpoint {
            @display("p=569,230");
        }
    connections:
        v0.gate++ <--> v1.gate++;
        v1.gate++ <--> v2.gate++;
        v2.gate++ <--> v3.gate++;
        h2.gate <--> v2.tributary++;
        h1.gate <--> v1.tributary++;
        h0.gate <--> v0.tributary++;
        v3.tributary++ <--> h3.gate;
        v4.tributary++ <--> h4.gate;
        EdgeNode.tributary++ <--> endpoint.gate;
        EdgeNode.gate++ <--> v3.gate++;
        v1.gate++ <--> v4.gate++;
}
//
// TODO documentation
//
network BufferTest
{
    @display("bgb=591,245");
    submodules:
        h0: Endpoint {
            @display("p=46,50");
        }
        h1: Endpoint {
            @display("p=46,185");
        }
        edgeNode: EdgeNode {
            @display("p=138,50");
        }
        edgeNode1: EdgeNode {
            @display("p=138,185");
        }
        coreNode: CoreNode {
            @display("p=256,119");
        }
        coreNode1: CoreNode {
            @display("p=381,119");
        }
        h2: Endpoint {
            @display("p=506,119");
        }
    connections:
        h0.gate <--> edgeNode.tributary++;
        h1.gate <--> edgeNode1.tributary++;
        edgeNode1.gate++ <--> coreNode.gate++;
        edgeNode.gate++ <--> coreNode.gate++;
        coreNode.gate++ <--> coreNode1.gate++;
        coreNode1.tributary++ <--> h2.gate;
}

//
// TODO documentation
//
network BufferTest1
{
    @display("bgb=669,271");
    submodules:
        h0: Endpoint {
            @display("p=46,50");
        }
        h1: Endpoint {
            @display("p=46,185");
        }
        edgeNode: EdgeNode {
            @display("p=138,50");
        }
        edgeNode1: EdgeNode {
            @display("p=138,185");
        }
        coreNode: CoreNode {
            @display("p=256,119");
        }
        coreNode1: CoreNode {
            @display("p=478,119");
        }
        h2: Endpoint {
            @display("p=370,200");
        }
        coreNode2: CoreNode {
            @display("p=370,119");
        }
        coreNode3: CoreNode {
            @display("p=580,119");
        }
        h3: Endpoint {
            @display("p=478,200");
        }
        h4: Endpoint {
            @display("p=580,200");
        }
    connections:
        h0.gate <--> edgeNode.tributary++;
        h1.gate <--> edgeNode1.tributary++;
        edgeNode1.gate++ <--> coreNode.gate++;
        edgeNode.gate++ <--> coreNode.gate++;
        coreNode.gate++ <--> coreNode2.gate++;
        coreNode2.gate++ <--> coreNode1.gate++;
        coreNode1.gate++ <--> coreNode3.gate++;
        h2.gate <--> coreNode2.tributary++;
        h3.gate <--> coreNode1.tributary++;
        h4.gate <--> coreNode3.tributary++;
}

//
// TODO documentation
//
network BufferTest2
{
    @display("bgb=669,271");
    submodules:
        h0: Endpoint {
            @display("p=46,50");
        }
        h1: Endpoint {
            @display("p=46,185");
        }
        edgeNode: EdgeNode {
            @display("p=138,50");
        }
        edgeNode1: EdgeNode {
            @display("p=138,185");
        }
        coreNode: CoreNode {
            @display("p=256,119");
        }
        coreNode1: CoreNode {
            @display("p=478,119");
        }
        h2: Endpoint {
            @display("p=370,200");
        }
        coreNode2: CoreNode {
            @display("p=370,119");
        }
        coreNode3: CoreNode {
            @display("p=580,119");
        }
        h3: Endpoint {
            @display("p=478,200");
        }
        h4: Endpoint {
            @display("p=580,200");
        }
        h5: Endpoint {
            @display("p=256,200");
        }
    connections:
        h0.gate <--> edgeNode.tributary++;
        h1.gate <--> edgeNode1.tributary++;
        edgeNode1.gate++ <--> coreNode.gate++;
        edgeNode.gate++ <--> coreNode.gate++;
        coreNode.gate++ <--> coreNode2.gate++;
        coreNode2.gate++ <--> coreNode1.gate++;
        coreNode1.gate++ <--> coreNode3.gate++;
        h2.gate <--> coreNode2.tributary++;
        h3.gate <--> coreNode1.tributary++;
        h4.gate <--> coreNode3.tributary++;
        coreNode.tributary++ <--> h5.gate;
}

