#ifndef __RJCSERVICES_H__
#define __RJCSERVICES_H__

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

void robot() {
	uint8 mybyte; //for the incoming byte storage
    uint8 static set_pwm = 0; //flag that we saw the set pwm symbol 'p'
    uint8 static pwm;

    while(radioComRxAvailable()){
    	mybyte = radioComRxReceiveByte();

        if (set_pwm) {
            pwm = mybyte;
            setLeftPWM(pwm);
	    setRightPWM(pwm);
            set_pwm = 0; //restore flag to default state
            continue;
        }

    	switch(mybyte) {
            case 0x70:
                set_pwm = 1; //set flag to receive next input byte as PWM value
            break;
            case 0x77: //char 'w'
        	Forward();	
    		break;
            case 0x61: //char 'a'
        	Left();
    		break;
            case 0x73: //char 's'
        	Reverse();
    		break;
            case 0x64: //char 'd'
        	Right();
    		break;
            case 0x20: //char ' '
        	Brake();
    		break;
            }
    }
}

#endif