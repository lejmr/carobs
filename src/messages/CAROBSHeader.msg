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

class noncobject cQueue;

packet CAROBSHeader {
	// Train Section
	int label;		// Label that defines the path which is taken
	int dst;
	simtime_t OT;
	int WL;
	
	// Car section
	int N;
	simtime_t length;	// Duration of all cars with theirs spaces d_s
	cQueue cars;
}

