#ifndef __REPORTING_H__
#define __REPORTING_H__

#include <radio_com.h>
#include <usb_com.h>

#define BUFSIZE			512

extern uint8 XDATA report[BUFSIZE];
extern uint16 head, tail;
extern BIT buffer_overflow;

void bufferInit();

BIT bufferempty();
BIT bufferfull();

uint16 bufferdatasize ();

void putchar(char c);
char getchar();

void sendReportRadio();
void sendReportUSB();

#endif
