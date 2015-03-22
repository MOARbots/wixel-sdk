#ifndef __REPORTING_H__
#define __REPORTING_H__

#define REPORTING_BUFFER \
	uint8 XDATA report[1024]; \
	uint16 DATA reportLength = 0; \
	uint16 DATA reportBytesSent = 0;

void putchar(char c);
void sendReportRadio();
void sendReportUSB();

#endif