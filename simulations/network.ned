package carobs.simulations;

import carobs.modules.CoreNode;
import carobs.modules.EdgeNode;
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
        EdgeNode: EdgeNode {
            @display("p=60,65");
        }
        EdgeNode1: EdgeNode {
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
        EdgeNode.gate++ <--> coreNode.gate++;
        coreNode.gate++ <--> coreNode1.gate++;
        coreNode1.gate++ <--> EdgeNode1.gate++;
        coreNode.gate++ <--> coreNode2.gate++;
        coreNode2.gate++ <--> coreNode1.gate++;

        endpoint.gate <--> EdgeNode.tributary++;
        endpoint1.gate <--> EdgeNode1.tributary++;
        endpoint2.gate <--> EdgeNode1.tributary++;
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
        EdgeNode: EdgeNode {
            @display("p=47,151");
        }
        EdgeNode1: EdgeNode {
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
        EdgeNode2: EdgeNode {
            @display("p=266,62");
        }
        endpoint3: Endpoint {
            @display("p=388,62");
        }
    connections:
        EdgeNode.gate++ <--> coreNode.gate++;
        coreNode.gate++ <--> coreNode1.gate++;
        endpoint.gate <--> EdgeNode.tributary++;
        endpoint1.gate <--> EdgeNode1.tributary++;
        endpoint2.gate <--> EdgeNode1.tributary++;
        coreNode.gate++ <--> coreNode3.gate++;
        coreNode3.gate++ <--> EdgeNode2.gate++;
        EdgeNode2.tributary++ <--> endpoint3.gate;
        coreNode1.gate++ <--> coreNode2.gate++;
        coreNode2.gate++ <--> EdgeNode1.gate++;
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
        EdgeNode: EdgeNode {
            @display("p=47,151");
        }
        EdgeNode1: EdgeNode {
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
        EdgeNode.gate++ <--> {  delay = 1ms; } <--> coreNode.gate++;
        coreNode.gate++ <--> {  delay = 1ms; } <--> coreNode1.gate++;
        coreNode1.gate++ <--> {  delay = 1ms; } <--> coreNode2.gate++;
        coreNode2.gate++ <--> {  delay = 1ms; } <--> coreNode3.gate++;
        coreNode3.gate++ <--> {  delay = 1ms; } <--> EdgeNode1.gate++;
        endpoint1.gate <--> EdgeNode1.tributary++;
        endpoint.gate <--> EdgeNode.tributary++;
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
        EdgeNode: EdgeNode {
            @display("p=174,227");
        }
        EdgeNode1: EdgeNode {
            @display("p=174,92");
        }
        endpoint: Endpoint {
            @display("p=47,227");
        }
        endpoint4: Endpoint {
            @display("p=47,91");
        }
        EdgeNode2: EdgeNode {
            @display("p=530,151");
        }
        endpoint2: Endpoint {
            @display("p=637,151");
        }
    connections:
        EdgeNode.gate++ <--> {  delay = 1ms; } <--> coreNode.gate++;
        coreNode.gate++ <--> {  delay = 1ms; } <--> coreNode1.gate++;
        endpoint4.gate <--> EdgeNode1.tributary++;
        endpoint.gate <--> EdgeNode.tributary++;
        coreNode1.gate++ <--> EdgeNode2.gate++;
        EdgeNode2.tributary++ <--> endpoint2.gate;
        EdgeNode1.gate++ <--> coreNode.gate++;
}

network NetworkCoreNodeAPm
{
    @display("bgb=719,304");
    submodules:
        coreNode: CoreNode {
            @display("p=294,151");
        }
        coreNode1: CoreNode {
            @display("p=409,151");
        }
        EdgeNode: EdgeNode {
            @display("p=174,227");
        }
        EdgeNode1: EdgeNode {
            @display("p=174,92");
        }
        endpoint: Endpoint {
            @display("p=47,227");
        }
        endpoint4: Endpoint {
            @display("p=47,91");
        }
        EdgeNode2: EdgeNode {
            @display("p=530,151");
        }
        endpoint2: Endpoint {
            @display("p=637,151");
        }
        endpoint1: Endpoint {
            @display("p=409,248");
        }
    connections:
        EdgeNode.gate++ <--> {  delay = 1ms; } <--> coreNode.gate++;
        coreNode.gate++ <--> {  delay = 1ms; } <--> coreNode1.gate++;
        endpoint4.gate <--> EdgeNode1.tributary++;
        endpoint.gate <--> EdgeNode.tributary++;
        coreNode1.gate++ <--> EdgeNode2.gate++;
        EdgeNode2.tributary++ <--> endpoint2.gate;
        EdgeNode1.gate++ <--> coreNode.gate++;
        coreNode1.tributary++ <--> endpoint1.gate;
}

network Network2
{
    @display("bgb=719,304");
    submodules:
        coreNode0: CoreNode {
            @display("p=151,99");
        }
        coreNode1: CoreNode {
            @display("p=266,99");
        }
        h0: Endpoint {
            @display("p=47,227");
        }
        h1: Endpoint {
            @display("p=642,227");
        }
        coreNode2: CoreNode {
            @display("p=388,99");
        }
        coreNode3: CoreNode {
            @display("p=519,99");
        }
        coreNode_start: CoreNode {
            @display("p=47,99");
        }
        coreNode_stop: CoreNode {
            @display("p=642,99");
        }
    connections:
        coreNode0.gate++ <--> {  delay = 1ms; } <--> coreNode1.gate++;
        coreNode1.gate++ <--> {  delay = 1ms; } <--> coreNode2.gate++;
        coreNode2.gate++ <--> {  delay = 1ms; } <--> coreNode3.gate++;
        coreNode_stop.gate++ <--> coreNode3.gate++;
        h1.gate <--> coreNode_stop.tributary++;
        h0.gate <--> coreNode_start.tributary++;
        coreNode_start.gate++ <--> coreNode0.gate++;
}

