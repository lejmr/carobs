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

package carobs.simulations.carobs;
import carobs.modules.*;
import carobs.link.*;
//
// Network generated from /carobs/references/pan-europe/europe.csv_lines.csv and /carobs/references/pan-europe/europe.csv_nodes.csv
//
network Europe
{
    @display("bgb=1428,1560");
    submodules:
        Amsterdam: CoreNode {
            @display("p=508,511");
        }
        Athens: CoreNode {
            @display("p=1082,1408");
        }
        Barcelona: CoreNode {
            @display("p=440,1216");
        }
        Belgrade: CoreNode {
            @display("p=1193,1139");
        }
        Berlin: CoreNode {
            @display("p=788,560");
        }
        Bordeaux: CoreNode {
            @display("p=341,1065");
        }
        Brussels: CoreNode {
            @display("p=532,787");
        }
        Budapest: CoreNode {
            @display("p=1090,953");
        }
        Copenhagen: CoreNode {
            @display("p=764,378");
        }
        Dublin: CoreNode {
            @display("p=186,448");
        }
        Frankfurt: CoreNode {
            @display("p=683,797");
        }
        Glasgow: CoreNode {
            @display("p=302,331");
        }
        Hamburg: CoreNode {
            @display("p=650,576");
        }
        London: CoreNode {
            @display("p=355,615");
        }
        Lyon: CoreNode {
            @display("p=532,1072");
        }
        Madrid: CoreNode {
            @display("p=210,1240");
        }
        Milan: CoreNode {
            @display("p=734,1122");
        }
        Munich: CoreNode {
            @display("p=812,924");
        }
        Oslo: CoreNode {
            @display("p=715,241");
        }
        Paris: CoreNode {
            @display("p=462,924");
        }
        Prague: CoreNode {
            @display("p=966,787");
        }
        Rome: CoreNode {
            @display("p=773,1255");
        }
        Stockholm: CoreNode {
            @display("p=915,266");
        }
        Strasbourg: CoreNode {
            @display("p=635,914");
        }
        Vienna: CoreNode {
            @display("p=960,943");
        }
        Warsaw: CoreNode {
            @display("p=1043,599");
        }
        Zagreb: CoreNode {
            @display("p=954,1150");
        }
        Zurich: CoreNode {
            @display("p=650,1016");
        }
        glasgow: City {
            @display("p=307,241;is=s");
        }
        oslo: City {
            @display("p=635,241;is=s");
        }
        copenhagen: City {
            @display("p=691,378;is=s");
        }
        berlin: City {
            @display("p=726,502;is=s");
        }
        hamburg: City {
            @display("p=650,511;is=s");
        }
        amsterdam: City {
            @display("p=432,502;is=s");
        }
        london: City {
            @display("p=284,629;is=s");
        }
        dublin: City {
            @display("p=120,448;is=s");
        }
        lyon: City {
            @display("p=539,1139;is=s");
        }
        barcelona: City {
            @display("p=440,1285;is=s");
        }
        madrid: City {
            @display("p=143,1246;is=s");
        }
        bordeaux: City {
            @display("p=271,1065;is=s");
        }
        paris: City {
            @display("p=386,924;is=s");
        }
        zurich: City {
            @display("p=642,1086;is=s");
        }
        strasbourg: City {
            @display("p=707,914;is=s");
        }
        vienna: City {
            @display("p=1006,986;is=s");
        }
        budapest: City {
            @display("p=1185,953;is=s");
        }
        belgrade: City {
            @display("p=1260,1139;is=s");
        }
        zagreb: City {
            @display("p=1006,1186;is=s");
        }
        athens: City {
            @display("p=1012,1408;is=s");
        }
        milan: City {
            @display("p=795,1113;is=s");
        }
        rome: City {
            @display("p=707,1255;is=s");
        }
        warsaw: City {
            @display("p=1113,599;is=s");
        }
        prague: City {
            @display("p=900,797;is=s");
        }
        munich: City {
            @display("p=756,924;is=s");
        }
        frankfurt: City {
            @display("p=746,797;is=s");
        }
        stockholm: City {
            @display("p=985,257;is=s");
        }
        brussel: City {
            @display("p=462,787;is=s");
        }
    connections:
        Belgrade.gate++ <--> FiberChannel {  length = 1209 km; } <--> Athens.gate++;
        Brussels.gate++ <--> FiberChannel {  length = 259 km; } <--> Amsterdam.gate++;
        Budapest.gate++ <--> FiberChannel {  length = 474 km; } <--> Belgrade.gate++;
        Copenhagen.gate++ <--> FiberChannel {  length = 540 km; } <--> Berlin.gate++;
        Frankfurt.gate++ <--> FiberChannel {  length = 474 km; } <--> Brussels.gate++;
        Glasgow.gate++ <--> FiberChannel {  length = 1067 km; } <--> Amsterdam.gate++;
        Glasgow.gate++ <--> FiberChannel {  length = 462 km; } <--> Dublin.gate++;
        Hamburg.gate++ <--> FiberChannel {  length = 552 km; } <--> Amsterdam.gate++;
        Hamburg.gate++ <--> FiberChannel {  length = 381 km; } <--> Berlin.gate++;
        Hamburg.gate++ <--> FiberChannel {  length = 592 km; } <--> Frankfurt.gate++;
        London.gate++ <--> FiberChannel {  length = 540 km; } <--> Amsterdam.gate++;
        London.gate++ <--> FiberChannel {  length = 690 km; } <--> Dublin.gate++;
        Lyon.gate++ <--> FiberChannel {  length = 796 km; } <--> Barcelona.gate++;
        Madrid.gate++ <--> FiberChannel {  length = 760 km; } <--> Barcelona.gate++;
        Madrid.gate++ <--> FiberChannel {  length = 834 km; } <--> Bordeaux.gate++;
        Munich.gate++ <--> FiberChannel {  length = 757 km; } <--> Berlin.gate++;
        Munich.gate++ <--> FiberChannel {  length = 456 km; } <--> Frankfurt.gate++;
        Munich.gate++ <--> FiberChannel {  length = 522 km; } <--> Milan.gate++;
        Oslo.gate++ <--> FiberChannel {  length = 722 km; } <--> Copenhagen.gate++;
        Paris.gate++ <--> FiberChannel {  length = 747 km; } <--> Bordeaux.gate++;
        Paris.gate++ <--> FiberChannel {  length = 393 km; } <--> Brussels.gate++;
        Paris.gate++ <--> FiberChannel {  length = 514 km; } <--> London.gate++;
        Paris.gate++ <--> FiberChannel {  length = 594 km; } <--> Lyon.gate++;
        Prague.gate++ <--> FiberChannel {  length = 420 km; } <--> Berlin.gate++;
        Prague.gate++ <--> FiberChannel {  length = 668 km; } <--> Budapest.gate++;
        Rome.gate++ <--> FiberChannel {  length = 1500 km; } <--> Athens.gate++;
        Rome.gate++ <--> FiberChannel {  length = 720 km; } <--> Milan.gate++;
        Stockholm.gate++ <--> FiberChannel {  length = 623 km; } <--> Oslo.gate++;
        Strasbourg.gate++ <--> FiberChannel {  length = 271 km; } <--> Frankfurt.gate++;
        Strasbourg.gate++ <--> FiberChannel {  length = 600 km; } <--> Paris.gate++;
        Vienna.gate++ <--> FiberChannel {  length = 534 km; } <--> Munich.gate++;
        Vienna.gate++ <--> FiberChannel {  length = 376 km; } <--> Prague.gate++;
        Warsaw.gate++ <--> FiberChannel {  length = 775 km; } <--> Berlin.gate++;
        Warsaw.gate++ <--> FiberChannel {  length = 1213 km; } <--> Stockholm.gate++;
        Zagreb.gate++ <--> FiberChannel {  length = 551 km; } <--> Belgrade.gate++;
        Zagreb.gate++ <--> FiberChannel {  length = 783 km; } <--> Rome.gate++;
        Zagreb.gate++ <--> FiberChannel {  length = 400 km; } <--> Vienna.gate++;
        Zurich.gate++ <--> FiberChannel {  length = 507 km; } <--> Lyon.gate++;
        Zurich.gate++ <--> FiberChannel {  length = 327 km; } <--> Milan.gate++;
        Zurich.gate++ <--> FiberChannel {  length = 218 km; } <--> Strasbourg.gate++;
        Warsaw.gate++ <--> FiberChannel {  length = 819 km; } <--> Budapest.gate++;
        Stockholm.tributary++ <--> stockholm.gate;
        Oslo.tributary++ <--> oslo.gate;
        copenhagen.gate <--> Copenhagen.tributary++;
        berlin.gate <--> Berlin.tributary++;
        hamburg.gate <--> Hamburg.tributary++;
        amsterdam.gate <--> Amsterdam.tributary++;
        glasgow.gate <--> Glasgow.tributary++;
        dublin.gate <--> Dublin.tributary++;
        london.gate <--> London.tributary++;
        paris.gate <--> Paris.tributary++;
        bordeaux.gate <--> Bordeaux.tributary++;
        madrid.gate <--> Madrid.tributary++;
        barcelona.gate <--> Barcelona.tributary++;
        lyon.gate <--> Lyon.tributary++;
        rome.gate <--> Rome.tributary++;
        zurich.gate <--> Zurich.tributary++;
        milan.gate <--> Milan.tributary++;
        strasbourg.gate <--> Strasbourg.tributary++;
        munich.gate <--> Munich.tributary++;
        frankfurt.gate <--> Frankfurt.tributary++;
        athens.gate <--> Athens.tributary++;
        zagreb.gate <--> Zagreb.tributary++;
        belgrade.gate <--> Belgrade.tributary++;
        Budapest.tributary++ <--> budapest.gate;
        vienna.gate <--> Vienna.tributary++;
        prague.gate <--> Prague.tributary++;
        warsaw.gate <--> Warsaw.tributary++;
        brussel.gate <--> Brussels.tributary++;
}
