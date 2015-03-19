#ifndef PACKET_H
#define PACKET_H

/*! \file packet.h
 * This library provides functions for reading the packet bitstring
 * and wrapper functions for specific data pieces.
 * It also has a function checkheader() that will check the first byte
 * for the expected string. The header is included mostly to pad out
 * the data bitstring to a around number of bytes (radio libraries only
 * send out whole bytes) however it has an additional functionality:
 * you could use it to indicate different types of packets coming in.
 * Current this is not implemented.
 * Checkheader() also changes the global readstate, which keeps track
 * of whether we are in idle (waiting for a packet to start streaming)
 * or active (in the middle of reading a packet, not yet at the end)
 * state.
 */

#include <queue.h>

extern uint8 i;
extern QTuple packet; //for storing packets, which may come in multiple bytes over multiple CPU loops

BIT checkHeader(uint8 header);

uint8 read1byte(uint8 pos, uint8 length, QTuple* mypacket) __reentrant; 
uint16 read2byte(uint8 pos, uint8 length, QTuple* mypacket) __reentrant;

uint8 readID(QTuple* mypacket) __reentrant;
uint16 readY(QTuple* mypacket) __reentrant;
uint16 readX(QTuple* mypacket) __reentrant;
uint16 readR(QTuple* mypacket) __reentrant;

#endif
