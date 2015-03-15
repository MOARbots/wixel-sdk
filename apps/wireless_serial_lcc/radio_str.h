#ifndef RADIO_STR_H
#define RADIO_STR_H

// ref: jude's implemtation

void sendBackString(char s[]) { 
  uint8 i = 0; 
  if (currentMode != SERIAL_MODE_USB_UART && s != NULL) { 
    if (!radioComTxAvailable()) { radioComTxService(); radioLinkTxQueueReset();  } 
    while (s[i] != '\0') { 
      if ( radioComTxAvailable() ) { radioComTxSendByte(s[i]);  } 
      else { radioComTxService();  } 
      i++; 
    } 
    radioComTxService(); 
  } 
}

void SendBackStr(char s[]) {
  XDATA char buf[16];
  sprintf(buf, "%s ", s);
  sendBackString(buf);
}

void SendBackStr1(char s[], int v) {
  XDATA char buf[16];
  sprintf(buf, "%s, %d ", s, v);
  sendBackString(buf);
}

void SendBackStr2(char s[], int v1, int v2) {
  XDATA char buf[16];
  sprintf(buf, "%s, %d, %d ", s, v1, v2);
  sendBackString(buf);
}



#endif
