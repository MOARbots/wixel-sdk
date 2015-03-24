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
    uint8 left_motor_power;
    uint8 right_motor_power;
    BIT left_motor_direction;
    BIT right_motor_direction;

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
            left_motor_power = packet[1];
            right_motor_power = packet[2];
            left_motor_direction = CHECK_BIT(packet[3], 0);
            right_motor_direction = CHECK_BIT(packet[3], 1);

            if (left_motor_power != 0) {
                setLeftPWM(left_motor_power);
                setLeftDirection(left_motor_direction);
            }
            if (right_motor_power != 0) {
                setRightPWM(right_motor_power);
                setRightDirection(right_motor_direction);
            }
            printf("left motor power: %u\n", left_motor_power);
            printf("right motor power: %u\n", right_motor_power);
            printf("left motor direction: %u\n", left_motor_direction);
            printf("right motor direction: %u\n", right_motor_direction);
        }
    }
    LED_RED(packetState);
    sendReportUSB();
}

#endif