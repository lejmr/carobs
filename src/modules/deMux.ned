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
}


simple MUX extends DeMux
{
    mux = true;
    gates:
        input control;
        input soa;
        output out;
}

simple DEMUX extends DeMux
{
    mux = false;
    gates:
        input in;
        output control;
        output soa;
}
