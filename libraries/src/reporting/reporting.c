#include <reporting.h>

// A big buffer for holding a report.
uint8 XDATA report[512];

//Put data at the head. Take data at the tail.
uint16 head, tail;

//A way to check if a buffer overflow occurred recently
//(aka if putchar was not successful when it was last called)
BIT buffer_overflow = 0; //flag to check if a buffer overflow occured

//Initialize the head and tail
void bufferInit() {
    head = 0;
    tail = 0;
}

//True if the buffer is empty
BIT bufferempty() {
    if (head == tail) {return 1;}
    else {return 0;}
}

//True if the buffer is full
BIT bufferfull() {
    if ( ((head+1) % BUFSIZE ) == tail ) {return 1;}
    else {return 0;}
}

//Returns the number of bytes of data in the buffer now
uint16 bufferdatasize () {
    if (bufferempty()) {return 0;}
    else if (bufferfull()) {return (BUFSIZE-1);}
    else { //neither empty nor full
	if (head > tail) { return (head-tail); }
	else { return (BUFSIZE - tail + head);}
    }

}

// This gets called by puts, printf. The result is sent by sendReport functions
void putchar(char c)
{
    if (bufferfull()) { buffer_overflow = 1; }
    else {
	report[head] = c;
	if (head < (BUFSIZE-1)) {head++;}
	else {head = 0;} //head == (BUFSIZE-1) or we somehow got to a state we should never reach
	buffer_overflow = 0;
    }
}

char getchar(){
    char c;
    if (bufferempty()) { return 0x00; } //return NULL if buffer is empty
    else {
	c = report[tail];
	if (tail < (BUFSIZE-1)) {tail++;}
	else {tail = 0;} //tail == (BUFSIZE-1) or we somehow got to a state we should never reach
        return c;
   } 
}

//This sends data over the radio
void sendReportRadio()
{
    uint8 bytesToSend,count;
    uint8 txAvailable = radioComTxAvailable();
    if (bufferempty()) {return;} //don't send if no data

    if (bufferdatasize() >= (uint16)txAvailable) {bytesToSend = txAvailable;} //if txAvailable is the bottleneck, send only this many
    else {bytesToSend = bufferdatasize(); } //the bottleneck is the amount of data, so send this many

    for (count =0; count < bytesToSend; count++) {
	radioComTxSendByte(getchar());
	//until either buffer empty or radioComTxAvailable runs out, send a byte
    }
}

//This sends data over the USB
void sendReportUSB()
{
    uint8 bytesToSend,count;
    uint8 txAvailable = usbComTxAvailable();
    if (bufferempty()) {return;} //don't send if no data

    if (bufferdatasize() >= (uint16)txAvailable) {bytesToSend = txAvailable;} //if txAvailable is the bottleneck, send only this many
    else {bytesToSend = bufferdatasize(); } //the bottleneck is the amount of data, so send this many

    for (count =0; count < bytesToSend; count++) {
	usbComTxSendByte(getchar());
	//until either buffer empty or usbComTxAvailable runs out, send a byte
    }
}
