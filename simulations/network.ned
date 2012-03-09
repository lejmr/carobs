package carobs.simulations;

import carobs.modules.CoreNode;
import carobs.modules.Endnode;
import carobs.modules.Endpoint;


network Network
{
    @display("bgb=560,232");
    submodules:
        coreNode: CoreNode {
            @display("p=194,65");
        }
        coreNode1: CoreNode {
            @display("p=342,65");
        }
        endnode: Endnode {
            @display("p=60,65");
        }
        endnode1: Endnode {
            @display("p=488,65");
        }
        coreNode2: CoreNode {
            @display("p=267,162");
        }
        endpoint: Endpoint {
            @display("p=60,159");
        }
        endpoint1: Endpoint {
            @display("p=488,159");
        }
        endpoint2: Endpoint {
            @display("p=426,159");
        }
    connections:
        endnode.gate++ <--> coreNode.gate++;
        coreNode.gate++ <--> coreNode1.gate++;
        coreNode1.gate++ <--> endnode1.gate++;
        coreNode.gate++ <--> coreNode2.gate++;
        coreNode2.gate++ <--> coreNode1.gate++;

        endpoint.gate <--> endnode.edge++;
        endpoint1.gate <--> endnode1.edge++;
        endpoint2.gate <--> endnode1.edge++;
}

network Network1
{
    @display("bgb=806,268");
    submodules:
        coreNode: CoreNode {
            @display("p=151,151");
        }
        coreNode1: CoreNode {
            @display("p=266,151");
        }
        endnode: Endnode {
            @display("p=47,151");
        }
        endnode1: Endnode {
            @display("p=516,151");
        }
        endpoint: Endpoint {
            @display("p=47,227");
        }
        endpoint1: Endpoint {
            @display("p=648,119");
        }
        endpoint2: Endpoint {
            @display("p=648,202");
        }
        coreNode2: CoreNode {
            @display("p=388,151");
        }
        coreNode3: CoreNode {
            @display("p=151,62");
        }
        endnode2: Endnode {
            @display("p=266,62");
        }
        endpoint3: Endpoint {
            @display("p=388,62");
        }
    connections:
        endnode.gate++ <--> coreNode.gate++;
        coreNode.gate++ <--> coreNode1.gate++;
        endpoint.gate <--> endnode.edge++;
        endpoint1.gate <--> endnode1.edge++;
        endpoint2.gate <--> endnode1.edge++;
        coreNode.gate++ <--> coreNode3.gate++;
        coreNode3.gate++ <--> endnode2.gate++;
        endnode2.edge++ <--> endpoint3.gate;
        coreNode1.gate++ <--> coreNode2.gate++;
        coreNode2.gate++ <--> endnode1.gate++;
}

network Network2
{
    @display("bgb=719,304");
    submodules:
        coreNode: CoreNode {
            @display("p=151,151");
        }
        coreNode1: CoreNode {
            @display("p=266,151");
        }
        endnode: Endnode {
            @display("p=47,151");
        }
        endnode1: Endnode {
            @display("p=516,151");
        }
        endpoint: Endpoint {
            @display("p=47,227");
        }
        endpoint1: Endpoint {
            @display("p=648,119");
        }
        endpoint2: Endpoint {
            @display("p=648,202");
        }
        coreNode2: CoreNode {
            @display("p=388,151");
        }
        coreNode3: CoreNode {
            @display("p=151,62");
        }
        endnode2: Endnode {
            @display("p=266,62");
        }
        endpoint3: Endpoint {
            @display("p=388,62");
        }
        endpoint4: Endpoint {
            @display("p=388,234");
        }
        endnode3: Endnode {
            @display("p=266,234");
        }
    connections:
        endnode.gate++ <--> coreNode.gate++;
        coreNode.gate++ <--> coreNode1.gate++;
        endpoint.gate <--> endnode.edge++;
        endpoint1.gate <--> endnode1.edge++;
        endpoint2.gate <--> endnode1.edge++;
        coreNode.gate++ <--> coreNode3.gate++;
        coreNode3.gate++ <--> endnode2.gate++;
        endnode2.edge++ <--> endpoint3.gate;
        coreNode1.gate++ <--> coreNode2.gate++;
        coreNode2.gate++ <--> endnode1.gate++;
        endnode3.edge++ <--> endpoint4.gate;
        endnode3.gate++ <--> coreNode1.gate++;
}

network Network3
{
    @display("bgb=719,304");
    submodules:
        coreNode: CoreNode {
            @display("p=151,151");
        }
        coreNode1: CoreNode {
            @display("p=266,151");
        }
        endnode: Endnode {
            @display("p=47,151");
        }
        endnode1: Endnode {
            @display("p=642,151");
        }
        endpoint: Endpoint {
            @display("p=47,227");
        }
        endpoint1: Endpoint {
            @display("p=642,227");
        }
        coreNode2: CoreNode {
            @display("p=388,151");
        }
        coreNode3: CoreNode {
            @display("p=519,151");
        }
    connections:
        endnode.gate++ <--> {  delay = 1ms; } <--> coreNode.gate++;
        coreNode.gate++ <--> {  delay = 1ms; } <--> coreNode1.gate++;
        coreNode1.gate++ <--> {  delay = 1ms; } <--> coreNode2.gate++;
        coreNode2.gate++ <--> {  delay = 1ms; } <--> coreNode3.gate++;
        coreNode3.gate++ <--> {  delay = 1ms; } <--> endnode1.gate++;
        endpoint1.gate <--> endnode1.edge++;
        endpoint.gate <--> endnode.edge++;
}

network NetworkWC_test
{
    @display("bgb=719,304");
    submodules:
        coreNode: CoreNode {
            @display("p=294,151");
        }
        coreNode1: CoreNode {
            @display("p=409,151");
        }
        endnode: Endnode {
            @display("p=174,227");
        }
        endnode1: Endnode {
            @display("p=174,92");
        }
        endpoint: Endpoint {
            @display("p=47,227");
        }
        endpoint4: Endpoint {
            @display("p=47,91");
        }
        endnode2: Endnode {
            @display("p=530,151");
        }
        endpoint2: Endpoint {
            @display("p=637,151");
        }
    connections:
        endnode.gate++ <--> {  delay = 1ms; } <--> coreNode.gate++;
        coreNode.gate++ <--> {  delay = 1ms; } <--> coreNode1.gate++;
        endpoint4.gate <--> endnode1.edge++;
        endpoint.gate <--> endnode.edge++;
        coreNode1.gate++ <--> endnode2.gate++;
        endnode2.edge++ <--> endpoint2.gate;
        endnode1.gate++ <--> coreNode.gate++;
}

