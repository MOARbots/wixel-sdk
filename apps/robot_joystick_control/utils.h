#ifndef __RJCUTILS_H__
#define __RJCUTILS_H__

#define STUB(name) void name(void) { \
	}

#define MODE_UNTETHERED 1
#define MODE_TETHERED   0

#define CHECK_BIT(data, index) !!((data) & (1 << (index)))

#endif