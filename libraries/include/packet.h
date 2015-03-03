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
void checkHeader(uint8 header) {
	if ((header & 0xFC) == 0xFC) { //Our header is six leading bits (111111xx for the first byte) so we check against 0xFC = 11111100
		readstate = 1;
		i=0;
		packet.bytes[0] = header;
		//readID = (header & 0x03) << 3; //0 0 0 x x _ _ _, we now have the first two blanks of the ID stored in readID
	}
}


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
