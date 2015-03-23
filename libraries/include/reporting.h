#ifndef __REPORTING_H__
#define __REPORTING_H__

#include <radio_com.h>
#include <usb_com.h>


void putchar(char c);
void sendReportRadio();
void sendReportUSB();

#endif