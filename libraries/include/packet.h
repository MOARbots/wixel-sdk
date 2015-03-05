/*
size, location
6, 0 header
5, 5 id
10, 10
10, 19
9, 

*/

#include <queue.h>

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
    if (length > 8) { return 0x0; } //this is a failure; asking for more than 8 bit structure. you could use a flag to indicate that this happened.
    else if ( (pos+length >> 3) > QTupleSize ) { return 0x0; } //this is a failure; structure can't extend outside of QTupleSize
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
		temp1 = packet.bytes[(pos >> 3)+1]; //the next byte after that
		temp = temp << (length  - (8 - rel_pos) ); //move over by the number of bits we're going to grab from the next packet string byte
		temp1 = temp1 >> (16 - length - rel_pos); //align to the right
		temp = temp & (0xFF >> rel_pos); //mask the unneeded MSBs on temp
		temp1 = temp1 & (0xFF << (7 - (rel_pos + length -1)%8 ) );//mask the unneeded LSBs on the temp1
		return (temp | temp1);
	}
    }
}

//extract a structure from a packet, of up to 16 bits in size
//uint16 read2byte(uint8 pos, uint8 length) { return 0;}


 /*   if(radioComRxAvailable() >= QTupleSize){ //If we have enough bytes to constitute a packet
    	for(i = 0; i < QTupleSize; i++) {
		packet.bytes[i] = radioComRxReceiveByte(); //place the received byte in the tuple
	}
	if(!queue1.Push(&packet)){queue1.PushForce(&packet);} //push to the packet. if fails, force push (wraparound case, queue was full)
    }

//actually ought to read ID, then reduce size to 32 bit (4 byte). Then each ID should have its own queue   


	if (usbComRxAvailable()) { //switch to radio version later
	    tempbyte = usbComRxReceiveByte(); //switch to radio version later
	    tempbyte = ( tempbyte >> 5 ) & 0x07 ; //0 0 0 _ _ x x x, now we just need to put it together with saved readID
	    readID = tempbyte | readID; //add
	    readstate = 0;
	    if (usbComTxAvailable()) {usbComTxSendByte(readID);}
	}

*/
