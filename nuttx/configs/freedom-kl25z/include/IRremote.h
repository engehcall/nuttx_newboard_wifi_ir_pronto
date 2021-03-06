/*
 * IRremote
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * For details, see http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.htm http://arcfn.com
 * Edited by Mitra to add new controller SANYO
 *
 * Interrupt code based on NECIRrcv by Joe Knapp
 * http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
 * Also influenced by http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
 *
 * JVC and Panasonic protocol added by Kristian Lauszus (Thanks to zenwheel and other people at the original blog post)
 */

#ifndef IRremote_h
#define IRremote_h
//#include "IRremoteInt.h"
#include <stdio.h>

// The following are compile-time library options.
// If you change them, recompile the library.
// If DEBUG is defined, a lot of debugging output will be printed during decoding.
// TEST must be defined for the IRtest unittests to work.  It will make some
// methods virtual, which will be slightly slower, which is why it is optional.
// #define DEBUG
// #define TEST
//#include "common.h"


// Results returned from the decoder
typedef struct decode_results {
  int decode_type; // NEC, SONY, RC5, UNKNOWN
  unsigned int panasonicAddress; // This is only used for decoding Panasonic data
  unsigned long value; // Decoded value
  int bits; // Number of bits in decoded value
  volatile unsigned int *rawbuf; // Raw intervals in .5 us ticks
  int rawlen; // Number of records in rawbuf.
} decode_results_t;

// Values for decode_type
#define NEC 1
#define SONY 2
#define RC5 3
#define RC6 4
#define DISH 5
#define SHARP 6
#define PANASONIC 7
#define JVC 8
#define SANYO 9
#define MITSUBISHI 10
#define UNKNOWN -1

// Decoded value for NEC when a repeat code is received
#define REPEAT 0xffffffff

int periodic_read_ir(int irq, void *context);

// main class for receiving IR
//class IRrecv
//{
//public:
  void IRrecv(int recvpin);
  void blink13(int blinkflag);
  int decode(struct decode_results *results);
  void enableIRIn(void);
  void resume(void);
//private:
  // These are called by decode
  int getRClevel(struct decode_results *results, int *offset, int *used, int t1);
  long decodeNEC(struct decode_results *results);
  long decodeSony(struct decode_results *results);
  long decodeSanyo(struct decode_results *results);
  long decodeMitsubishi(struct decode_results *results);
  long decodeRC5(struct decode_results *results);
  long decodeRC6(struct decode_results *results);
  long decodePanasonic(struct decode_results *results);
  long decodeJVC(struct decode_results *results);
  long decodeHash(struct decode_results *results);
  int compare(unsigned int oldval, unsigned int newval);

//}
//;

// Only used for testing; can remove virtual for shorter code
#ifdef TEST
#define VIRTUAL virtual
#else
#define VIRTUAL
#endif

//class IRsend
//{
//public:
  //void IRsend() {}
  volatile void delayMicroseconds(unsigned int us);
  void sendRaw(unsigned int *buf, int len, int hz);
  void sendPronto(unsigned int *buf);

  void sendNEC(unsigned long data, int nbits);
  void sendSony(unsigned long data, int nbits);
  // Neither Sanyo nor Mitsubishi send is implemented yet
  //  void sendSanyo(unsigned long data, int nbits);
  //  void sendMitsubishi(unsigned long data, int nbits);
  void geProtocol();
  void sendRC5(unsigned long data, int nbits);
  void sendRC6(unsigned long data, int nbits);
  void sendDISH(unsigned long data, int nbits);
  void sendSharp(unsigned long data, int nbits);
  void sendPanasonic(unsigned int address, unsigned long data);
  void sendJVC(unsigned long data, int nbits, int repeat); // *Note instead of sending the REPEAT constant if you want the JVC repeat signal sent, send the original code value and change the repeat argument from 0 to 1. JVC protocol repeats by skipping the header NOT by sending a separate code value like NEC does.
  extern void delay(unsigned int length_ms);

  // private
  void enableIROut(int khz);
  void mark(unsigned int usec);
  void space(unsigned int usec);
//}
//;

#define USECPERTICK 50  // microseconds per clock interrupt tick
#define RAWBUF 120 // Length of raw duration buffer

// Marks tend to be 100us too long, and spaces 100us too short
// when received due to sensor lag.
#define MARK_EXCESS 100

#endif
