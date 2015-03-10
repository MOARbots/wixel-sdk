/* Wireless_serial_robot app: wireless_serial + PWM on timer 3
 * When plugged into battery power (MODE_UART_RADIO) and NOT connect via USB
 * this app will respond to key presses w,a,s,d, and Space to drive around the robot.
 * Program another Wixel with a wireless_serial app on the same channel
 * Use a serial communication program like PuTTY or screen to send the keypresses
 * to control the robot.
 */

/** Dependencies **************************************************************/
#include <wixel.h>
#include <usb.h>
#include <usb_com.h>
#include <radio_com.h>
#include <radio_link.h>
#include <uart1.h>
#include <packet.h>
#include <queue.h>
#include <string.h>
#include <stdio.h>

/** Parameters ****************************************************************/
#define SERIAL_MODE_AUTO        0
#define SERIAL_MODE_USB_RADIO   1
#define SERIAL_MODE_UART_RADIO  2
#define SERIAL_MODE_USB_UART    3

int32 CODE param_serial_mode = SERIAL_MODE_AUTO;

int32 CODE param_baud_rate = 9600;

int32 CODE param_nDTR_pin = -1;
int32 CODE param_nRTS_pin = -1;
int32 CODE param_nDSR_pin = -1;
int32 CODE param_nCD_pin = -1;

int32 CODE param_DTR_pin = -1;
int32 CODE param_RTS_pin = -1;
int32 CODE param_DSR_pin = -1;
int32 CODE param_CD_pin = -1;

// Approximate number of milliseconds to disable UART's receiver for after a
// framing error is encountered.
// Valid values are 0-250.
// A value of 0 disables the feature (the UART's receiver will not be disabled).
// The actual number of milliseconds that the receiver is disabled for will be
// between param_framing_error_ms and param_framing_error_ms + 1.
int32 CODE param_framing_error_ms = 0;

/** Global Variables **********************************************************/

// This bit is 1 if the UART's receiver has been disabled due to a framing error.
// This bit should be equal to !U1CSR.RE, but we need this variable because we
// don't want to be reading U1CSR in the main loop, because reading it might
// cause the FE or ERR bits to be cleared and then the ISR
// would not receive notice of those errors.
BIT uartRxDisabled = 0;

uint8 DATA currentSerialMode;

BIT framingErrorActive = 0;

BIT errorOccurredRecently = 0;
uint8 lastErrorTime;

// A big buffer for holding a report.  This allows us to print more than
// 128 bytes at a time to USB.
uint8 XDATA report[1024];

// The length (in bytes) of the report currently in the report buffer.
// If zero, then there is no report in the buffer.
uint16 DATA reportLength = 0;

// The number of bytes of the current report that have already been
// send to the computer over USB.
uint16 DATA reportBytesSent = 0;

/** Functions *****************************************************************/


// This gets called by puts, printf. The result is sent by sendReport()
void putchar(char c)
{
    report[reportLength] = c;
    reportLength++;
}

//This sends data over the radio
void sendReportRadio()
{

    uint8 bytesToSend, i;

    // Send the report to radio in chunks.
    if (reportLength > 0)
    {
        bytesToSend = radioComTxAvailable();
        if (bytesToSend > reportLength - reportBytesSent)
        {
            // Iterate through byte by byte, sending the data until the end of the report.
            for (i=reportBytesSent; i < reportLength; i++  ) {
	    	radioComTxSendByte(report[i]);
	    }
            reportLength = 0;
        }
        else //Not enough space on the buffers, send only what will fit
        {
	    for (i=reportBytesSent; i < (reportBytesSent+bytesToSend); i++) {
            	radioComTxSendByte(report[i]);
	    }
            reportBytesSent += bytesToSend;
        }
    }
}

//This sends data over the USB
void sendReportUSB()
{

    uint8 bytesToSend;

    // Send the report to USB in chunks.
    if (reportLength > 0)
    {
        bytesToSend = usbComTxAvailable();
        if (bytesToSend > reportLength - reportBytesSent)
        {
            // Send the last part of the report.
            usbComTxSend(report+reportBytesSent, reportLength - reportBytesSent);
            reportLength = 0;
        }
        else
        {
            usbComTxSend(report+reportBytesSent, bytesToSend);
            reportBytesSent += bytesToSend;
        }
    }
}

void timer3Init()
{
    // Start the timer in free-running mode and set the prescaler.
    T3CTL = 0b01110000;   // Prescaler 1:8, frequency = (24000 kHz)/8/256 = 11.7 kHz
    //T3CTL = 0b01010000; // Use this line instead if you want 23.4 kHz (1:4)

    // Set the duty cycles to zero.
    T3CC0 = T3CC1 = 0;

    // Enable PWM on both channels.  We choose the mode where the channel
    // goes high when the timer is at 0 and goes low when the timer value
    // is equal to T3CCn.
    T3CCTL0 = T3CCTL1 = 0b00100100;

    // Configure Timer 3 to use Alternative 1 location, which is the default.
    PERCFG &= ~(1<<5);  // PERCFG.T3CFG = 0;

    // Configure P1_3 and P1_4 to be controlled by a peripheral function (Timer 3)
    // instead of being general purpose I/O.
    P1SEL |= (1<<3) | (1<<4);

    // After calling this function, you can set the duty cycles by simply writing
    // to T3CC0 and T3CC1.  A value of 255 results in a 100% duty cycle, and a
    // value of N < 255 results in a duty cycle of N/256.
}

void updateLeds()
{
    static BIT dimYellowLed = 0;
    static uint16 lastRadioActivityTime;
    uint16 now;

    usbShowStatusWithGreenLed();

    now = (uint16)getMs();

    if (currentSerialMode == SERIAL_MODE_USB_UART)
    {
        // The radio is not being used, so turn off the yellow LED.
        LED_YELLOW(0);
    }
    else if (!radioLinkConnected())
    {
        // We have not connected to another device wirelessly yet, so do a
        // 50% blink with a period of 1024 ms.
        LED_YELLOW(now & 0x200 ? 1 : 0);
    }
    else
    {
        // We have connected.

        if ((now & 0x3FF) <= 20)
        {
            // Do a heartbeat every 1024ms for 21ms.
            LED_YELLOW(1);
        }
        else if (dimYellowLed)
        {
            static uint8 DATA count;
            count++;
            LED_YELLOW((count & 0x7)==0);
        }
        else
        {
            LED_YELLOW(0);
        }
    }

    if (radioLinkActivityOccurred)
    {
        radioLinkActivityOccurred = 0;
        dimYellowLed ^= 1;
        //dimYellowLed = 1;
        lastRadioActivityTime = now;
    }

    if ((uint16)(now - lastRadioActivityTime) > 32)
    {
        dimYellowLed = 0;
    }

    if ((uint8)(now - lastErrorTime) > 100)
    {
        errorOccurredRecently = 0;
    }

    LED_RED(errorOccurredRecently || uartRxDisabled);
}

void errorOccurred()
{
    lastErrorTime = (uint8)getMs();
    errorOccurredRecently = 1;
}

void errorService()
{
    static uint8 lastRxLowTime;

    if (uart1RxBufferFullOccurred)
    {
        uart1RxBufferFullOccurred = 0;
        errorOccurred();
    }

    if (uart1RxFramingErrorOccurred)
    {
        uart1RxFramingErrorOccurred = 0;

        // A framing error occurred.
        framingErrorActive = 1;
        errorOccurred();

        if (param_framing_error_ms > 0)
        {
            // Disable the UART's receiver.
            U1CSR &= ~0x40;    // U1CSR.RE = 0.  Disables reception of bytes on the UART.
            uartRxDisabled = 1;
            lastRxLowTime = (uint8)getMs();  // Initialize lastRxLowTime even if the line isn't low right now.
        }
    }

    if (framingErrorActive)
    {
        if (!isPinHigh(17))
        {
            errorOccurred();
        }
        else
        {
            framingErrorActive = 0;
        }
    }

    if (uartRxDisabled)
    {
        if (!isPinHigh(17))
        {
            // The line is low.
            lastRxLowTime = (uint8)getMs();
        }
        else if ((uint8)(getMs() - lastRxLowTime) > param_framing_error_ms)
        {
            // The line has been high for long enough, so re-enable the receiver.
            U1CSR |= 0x40;
            uartRxDisabled = 0;
        }
    }
}

void updateSerialMode()
{
    if ((uint8)param_serial_mode > 0 && (uint8)param_serial_mode <= 3)
    {
        currentSerialMode = (uint8)param_serial_mode;
        return;
    }

    if (usbPowerPresent())
    {
        if (vinPowerPresent())
        {
            currentSerialMode = SERIAL_MODE_USB_UART;
        }
        else
        {
            currentSerialMode = SERIAL_MODE_USB_RADIO;
        }
    }
    else
    {
        currentSerialMode = SERIAL_MODE_UART_RADIO;
    }
}

//This mode is active when the USB is plugged in only (no battery)
//Test Streaming Data to USB. Data is received over the radio.
//If the packet is properly formatted (111111 header, 5 bit ID, 10 bit Y pos, 10 bit X pos, 9 bit rotation, total 5 bytes)
//Then the Wixel will send this back via the PC it is connected to via USB.
void usbToRadioService()
{
    if (!readstate) { //the idle state	
	if (radioComRxAvailable()) { checkHeader( radioComRxReceiveByte() ); }
    }

    else { //readstate
	if ( i + 1 < QTupleSize ) { //packet not yet full
	    if (radioComRxAvailable()) { //if byte available
	    	packet.bytes[i+1] = radioComRxReceiveByte(); //take it off the receiving buffer
		i = i+1;
	    }
	}
	else { //We filled up one packet
	    readstate = 0; //return to idle state next loop
	    i=0;
	    
	    //Store or process the packet. Here we are formatting with printf, which calls putchar, then we call sendReport
	    printf("ID: %u, ", readID());
	    printf("Y: %u, ", readY());
	    printf("X: %u, ", readX());
	    printf("R: %u", readR());
	    putchar('\r');
	    sendReportUSB();
	}
    }
}


void uartToRadioService()
{
    //This piece of code controls the robot.
    //This code operates only if USB power is not present (battery power)

}

void usbToUartService()
{

    // Data
    while(usbComRxAvailable() && uart1TxAvailable())
    {
        uart1TxSendByte(usbComRxReceiveByte());
    }

    while(uart1RxAvailable() && usbComTxAvailable())
    {
        usbComTxSendByte(uart1RxReceiveByte());
    }

}

void main()
{
    systemInit();

    usbInit();

    uart1Init();
    uart1SetBaudRate(param_baud_rate);

    if (param_serial_mode != SERIAL_MODE_USB_UART)
    {
        radioComRxEnforceOrdering = 1;
        radioComInit();
    }
	
    setDigitalOutput(0,PULLED);
    setDigitalOutput(1,PULLED);
    setDigitalOutput(2,PULLED);
    setDigitalOutput(3,PULLED);
    setDigitalOutput(15,PULLED);

    timer3Init(); //Timer 3 will now control the Enable A and Enable B pins on the motor driver
    setDigitalOutput(0,LOW); // Initializing A1, A2, B1, B2 to LOW so the robot doesn't move, but not brake mode
    setDigitalOutput(1,LOW);
    setDigitalOutput(2,LOW);
    setDigitalOutput(3,LOW);
    setDigitalOutput(15,HIGH); //Standby mode: Motor driver turns off when LOW, on when HIGH
    T3CC0 = T3CC1 = 255; //this means we init PWM val to 255 unless otherwise specified


    while(1)
    {
        updateSerialMode();
        boardService();
        updateLeds();  
        errorService();

        if (param_serial_mode != SERIAL_MODE_USB_UART)
        {
            radioComTxService();
        }

        usbComService();

        switch(currentSerialMode)
        {
        case SERIAL_MODE_USB_RADIO:  usbToRadioService();  break;
        case SERIAL_MODE_UART_RADIO: uartToRadioService(); break;
        case SERIAL_MODE_USB_UART:   usbToUartService();   break;
        }
    }
}
