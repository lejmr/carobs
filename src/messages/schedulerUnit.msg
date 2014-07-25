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

packet SchedulerUnit
{
    int dst;			// Car destination
    int label;
    simtime_t ot;		// Minumum offset time that must have been sattisfied
 	simtime_t start;	// car's time of the start
    simtime_t end; 		// car's time of the end
    simtime_t length; 	// In seconds
    int lengthB; 		// In Bytes
    
    simtime_t EOT;		//Artificial variable for statistics mainly
}