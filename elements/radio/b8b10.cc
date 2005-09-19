/*
 * b8b10.{cc,hh} -- element maps each 8-bit pattern into a 10-bit pattern
 * with as many 1 bits as 0 bits (for BIM-4xx-RS232 radios)
 * Robert Morris
 *
 * Copyright (c) 1999-2000 Massachusetts Institute of Technology
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, subject to the conditions
 * listed in the Click LICENSE file. These conditions include: you must
 * preserve this copyright notice, and you cannot mention the copyright
 * holders in advertising related to the Software without their permission.
 * The Software is provided WITHOUT ANY WARRANTY, EXPRESS OR IMPLIED. This
 * notice is a summary of the Click LICENSE file; the license in that file is
 * legally binding.
 */

#include <click/config.h>
#include "b8b10.hh"
#include <click/error.hh>
#include <click/confparse.hh>
#include <click/glue.hh>
CLICK_DECLS

B8B10::B8B10()
{
}

B8B10::~B8B10()
{
}

/*
 * Map each 8 bit pattern to a 10 bit
 * pattern with as many 1s as 0s.
 * There are only 252, so the last four
 * are faked.
 */
static int x8to10[] = {
0x01f, 0x02f, 0x037, 0x03b, 0x03d, 0x03e, 0x04f, 0x057, 
0x05b, 0x05d, 0x05e, 0x067, 0x06b, 0x06d, 0x06e, 0x073, 
0x075, 0x076, 0x079, 0x07a, 0x07c, 0x08f, 0x097, 0x09b, 
0x09d, 0x09e, 0x0a7, 0x0ab, 0x0ad, 0x0ae, 0x0b3, 0x0b5, 
0x0b6, 0x0b9, 0x0ba, 0x0bc, 0x0c7, 0x0cb, 0x0cd, 0x0ce, 
0x0d3, 0x0d5, 0x0d6, 0x0d9, 0x0da, 0x0dc, 0x0e3, 0x0e5, 
0x0e6, 0x0e9, 0x0ea, 0x0ec, 0x0f1, 0x0f2, 0x0f4, 0x0f8, 
0x10f, 0x117, 0x11b, 0x11d, 0x11e, 0x127, 0x12b, 0x12d, 
0x12e, 0x133, 0x135, 0x136, 0x139, 0x13a, 0x13c, 0x147, 
0x14b, 0x14d, 0x14e, 0x153, 0x155, 0x156, 0x159, 0x15a, 
0x15c, 0x163, 0x165, 0x166, 0x169, 0x16a, 0x16c, 0x171, 
0x172, 0x174, 0x178, 0x187, 0x18b, 0x18d, 0x18e, 0x193, 
0x195, 0x196, 0x199, 0x19a, 0x19c, 0x1a3, 0x1a5, 0x1a6, 
0x1a9, 0x1aa, 0x1ac, 0x1b1, 0x1b2, 0x1b4, 0x1b8, 0x1c3, 
0x1c5, 0x1c6, 0x1c9, 0x1ca, 0x1cc, 0x1d1, 0x1d2, 0x1d4, 
0x1d8, 0x1e1, 0x1e2, 0x1e4, 0x1e8, 0x1f0, 0x20f, 0x217, 
0x21b, 0x21d, 0x21e, 0x227, 0x22b, 0x22d, 0x22e, 0x233, 
0x235, 0x236, 0x239, 0x23a, 0x23c, 0x247, 0x24b, 0x24d, 
0x24e, 0x253, 0x255, 0x256, 0x259, 0x25a, 0x25c, 0x263, 
0x265, 0x266, 0x269, 0x26a, 0x26c, 0x271, 0x272, 0x274, 
0x278, 0x287, 0x28b, 0x28d, 0x28e, 0x293, 0x295, 0x296, 
0x299, 0x29a, 0x29c, 0x2a3, 0x2a5, 0x2a6, 0x2a9, 0x2aa, 
0x2ac, 0x2b1, 0x2b2, 0x2b4, 0x2b8, 0x2c3, 0x2c5, 0x2c6, 
0x2c9, 0x2ca, 0x2cc, 0x2d1, 0x2d2, 0x2d4, 0x2d8, 0x2e1, 
0x2e2, 0x2e4, 0x2e8, 0x2f0, 0x307, 0x30b, 0x30d, 0x30e, 
0x313, 0x315, 0x316, 0x319, 0x31a, 0x31c, 0x323, 0x325, 
0x326, 0x329, 0x32a, 0x32c, 0x331, 0x332, 0x334, 0x338, 
0x343, 0x345, 0x346, 0x349, 0x34a, 0x34c, 0x351, 0x352, 
0x354, 0x358, 0x361, 0x362, 0x364, 0x368, 0x370, 0x383, 
0x385, 0x386, 0x389, 0x38a, 0x38c, 0x391, 0x392, 0x394, 
0x398, 0x3a1, 0x3a2, 0x3a4, 0x3a8, 0x3b0, 0x3c1, 0x3c2, 
0x3c4, 0x3c8, 0x3d0, 0x3e0, 
0x01e, 0x11c, 0x258, 0x393 /* faked */
};

/*
 * Map 10 bit values back to 8.
 * Computed by initialize().
 * -1 means not valid.
 */
static int x10to8[1024];

int
B8B10::initialize(ErrorHandler *errh)
{
  int i;

  if(sizeof(x8to10)/sizeof(x8to10[0]) != 256)
    return errh->error("sizeof(x8to10) != 256");

  for(i = 0; i < 1024; i++)
    x10to8[i] = -1;
  for(i = 0; i < 256; i++){
    if(x10to8[x8to10[i]] != -1)
      return errh->error("duplicate x8to10 entry");
    x10to8[x8to10[i]] = i;
  }

  return(0);
}

int
B8B10::configure(Vector<String> &conf, ErrorHandler *errh)
{
  if (cp_va_parse(conf, this, errh,
		  cpInteger, "encode flag", &_flag,
		  cpEnd) < 0)
    return -1;
  return(0);
}

Packet *
B8B10::simple_action(Packet *p)
{
  if (_flag) {
    /* encode */
    int qbits = p->length() * 10;
    int qbytes = (qbits + 7) / 8;
    WritablePacket *q = Packet::make(qbytes);
    memset(q->data(), '\0', q->length());
    int qbit = 0;
    int pi;
    for(pi = 0; pi < (int)p->length(); pi++){
      int x10 = x8to10[p->data()[pi] & 0xff];
      assert(x10 >= 0 && x10 < 1024);
      int qi = qbit / 8;
      int qj = qbit % 8;
      assert(qi+1 < (int)q->length());
      q->data()[qi] |= (x10 >> (qj+2));
      q->data()[qi+1] |= (x10 << (6-qj)) & 0xff;
      qbit += 10;
    }
    p->kill();
    return(q);
  } else {
    /* decode */
    int babbling = 0;
    int pbits = p->length() * 8;
    WritablePacket *q = Packet::make(pbits / 10);
    int qi = 0;
    int pbit;
    for(pbit = 0; pbit < pbits; pbit += 10){
      int pi = pbit / 8;
      int pj = pbit % 8;
      int x10 = ((p->data()[pi] << pj) & 0xff) << 2;
      x10 |= (p->data()[pi+1] & 0xff) >> (6-pj);
      assert(x10 >= 0 && x10 < 1024);
      int x8 = x10to8[x10];
      if(x8 == -1 && babbling == 0){
        click_chatter("b8b10 bad code 0x%03x", x10);
        babbling = 1;
      }
      q->data()[qi++] = x8;
    }
    p->kill();
    return(q);
  }
}

CLICK_ENDDECLS
ELEMENT_REQUIRES(BIM)
EXPORT_ELEMENT(B8B10)
