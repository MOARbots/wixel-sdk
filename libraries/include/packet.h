/*! \file packet.h
 * This library provides functions for reading the packet bitstring
 * and wrapper functions for specific data pieces.
 * It also has a function checkheader() that will check the first byte
 * for the expected string. The header is included mostly to pad out
 * the data bitstring to a around number of bytes (radio libraries only
 * send out whole bytes) however it has an additional functionality:
 * you could use it to indicate different types of packets coming in.
 * Current this is not implemented.
 * Checkheader() also changes the global readstate, which keeps track
 * of whether we are in idle (waiting for a packet to start streaming)
 * or active (in the middle of reading a packet, not yet at the end)
 * state.
 */

#include <queue.h>

/*In this section define the start points and lengths
 * of data in your packet bitstring.
 * If the packet format changes, you will need to
 * modify this as well as the wrapper functions
 * at the bottom of this document. 
 */

#define ID_START	6
#define ID_LENGTH	5
#define Y_START		11
#define Y_LENGTH	10
#define X_START		21
#define X_LENGTH	10
#define R_START		31
#define R_LENGTH	9

uint8 i;
QTuple packet; //for storing packets, which may come in multiple bytes over multiple CPU loops
BIT readstate = 0; //0 means idle state, 1 means currently reading a packet state

//Run this function on incoming bytes when in idle state
BIT checkHeader(uint8 header) {
	//to do: add check for start trial code, aka 'G' hex code 0x47
	if ((header & 0xFC) == 0xFC) { //Our header is six leading bits (111111xx for the first byte) so we check against 0xFC = 11111100
		readstate = 1;
		i=0;
		packet.bytes[0] = header;
		return 1;
	}
	else {return 0;}
}

//extract a structure from a packet, of up to 8 bits in size
uint8 read1byte(uint8 pos, uint8 length) {
    uint8 temp,rel_pos;
    if (length > 8) { return 0x00; } //this is a failure; asking for more than 8 bit structure. you could use a flag to indicate that this happened.
    else if ( (pos+length) > QTupleSize*8 ) { return 0x0; } //this is a failure; structure can't extend outside of QTupleSize
    else {
	rel_pos = (pos % 8); //pos mod 8 is the relative position in the byte
	temp = packet.bytes[(pos >> 3)]; //floor of (pos divided by 8)
	if ( 8 - rel_pos >= length ) { //Spans a single byte within the packet string
		temp = temp >> (8 - (rel_pos + length) ); //align to the right
		temp = temp & (0xFF >> (8 - length)); //mask any unused MSBs
		return temp;
	}
	else { //spans two bytes within the packet string
		uint8 temp1;
		temp = temp & (0xFF >> rel_pos); //mask the unneeded MSBs on temp
		temp1 = packet.bytes[(pos >> 3)+1]; //grab the next byte
		temp = temp << (length  - (8 - rel_pos) ); //move over by the number of bits we're going to grab from the next packet string byte
		temp1 = temp1 >> (16 - length - rel_pos); //align to the right
		return (temp | temp1);
	}
    }
}

//extract a structure from a packet, of up to 16 bits in size
uint16 read2byte(uint8 pos, uint8 length) {
    if ( (length > 16) | (length <= 8) ) {return 0x00; } //check if failure b/c asking more than 16 bit structure, or should be using read1byte instead
    else if ( ( pos+length) > QTupleSize*8 ) { return  0x00; } //check if failure b/c extending outside of QTupleSize
    else {
	uint16 temp;
	uint8 rel_pos8;
	rel_pos8 = (pos % 8); //Relative position within a single byte
	if ( (length + rel_pos8) <= 16 ) {//Spans 2 bytes within the packet string
	    uint8 temp1;
	    temp = packet.bytes[(pos >> 3)];//Grab the byte at floor of  (pos / 8), this is the MSBs of our data, and already right aligned
	    temp1 = packet.bytes[(pos >> 3) +1 ]; //grab the next byte, which is left aligned
	    temp = temp & (0xFF >> rel_pos8); //mask temp MSBs that aren't needed (from position 0 to start position)
	    temp = temp << (length - 8 + rel_pos8);//shift over to the left by the number of bits we need to get from temp1
	    temp1 = temp1 >> (16 - length - rel_pos8); //right align the temp1 data, this will also zero all other bits
	    temp = (temp | temp1);
	    return temp;
	}
	else { //Spans 3 bytes within the packet string
	    uint16 temp1;
	    uint8 temp2;
	    temp = packet.bytes[ (pos >> 3) ];//grab the byte at floor of (pos/16), MSBs that are right aligned
	    temp1 = packet.bytes[ (pos >> 3) +1 ]; //Middle of our data, 8 bits long
	    temp1 = temp1 & (0xFF); //going from uint8 to uint16 is putting 1s on all the MSBs, therefore we mask here.
	    temp2 = packet.bytes[ (pos >> 3) +2 ]; //End of ur data, LSBs, left aligned.
	    temp2 = temp2 & (0xFF); //going from uint8 to uint16, mask the MSBs
	    temp = temp & (0xFF >> rel_pos8); //Mask MSBs from zero to start
	    temp = temp << ( length - 8 + rel_pos8); //shift to the left by the number of bytes we need to get from bytes #2 and #3
	    temp1 = temp1 << ( length -16 + rel_pos8); //shift byte #2 by that number - 8
	    temp2 = temp2 >> (8 - ((pos+length) % 8)); //align bye #3 to the right, which also zeros all other bits
	    temp = (temp | temp1 | temp2); //add all three together
	    return (temp);
	}
    }
}


//wrapper functions
uint8 readID() {
	return read1byte(ID_START,ID_LENGTH);
}

uint16 readY() {
	return read2byte(Y_START,Y_LENGTH);
}

uint16 readX() {
	return read2byte(X_START,X_LENGTH);
}

uint16 readR() {
	return read2byte(R_START,R_LENGTH);
}

