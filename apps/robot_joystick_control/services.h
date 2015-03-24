#ifndef __RJCSERVICES_H__
#define __RJCSERVICES_H__

BIT packetState = 0;
uint8 DATA packet[4];
uint8 DATA recvLength = 0;

void computerLink() //runs during TETHERED mode, relays info between USB and radio
{
    // Data
    while(usbComRxAvailable() && radioComTxAvailable())
    {
        radioComTxSendByte(usbComRxReceiveByte());
    }

    while(radioComRxAvailable() && usbComTxAvailable())
    {
        usbComTxSendByte(radioComRxReceiveByte());
    }
}

BIT checkHeader(uint8 header) {
    if ((header & 0xFC) == 0xFC) { //Our header is six leading bits (111111xx for the first byte) so we check against 0xFC = 11111100
        recvLength = 0;
        packet[0] = header;
        return 1;
    }
    else {return 0;}
}

void robot() {
    if (!packetState) {
        if (usbComRxAvailable()) {
            if (checkHeader(usbComRxReceiveByte())) {
                packetState = 1;
            }
        }
    } else {
        if (recvLength + 1 < 4) {
            if (usbComRxAvailable()) {
                packet[recvLength + 1] = usbComRxReceiveByte();

                recvLength++;
            }
        } else {
            packetState = 0;
            recvLength = 0;
            printf("packet filled!\r\n");
            printf("l: %u\n", packet[0]);
            printf("l: %u\n", packet[1]);
            printf("r: %u\n", packet[2]);
            printf("d: %u\n", packet[3]);
        }
    }
    LED_RED(packetState);
    sendReportUSB();
}

#endif