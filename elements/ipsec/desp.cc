/*
 * desp.{cc,hh} -- element implements IPsec unencapsulation (RFC 2406)
 * Alex Snoeren
 *
 * Copyright (c) 1999-2000 Massachusetts Institute of Technology
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * Further elaboration of this license, including a DISCLAIMER OF ANY
 * WARRANTY, EXPRESS OR IMPLIED, is provided in the LICENSE file, which is
 * also accessible at http://www.pdos.lcs.mit.edu/click/license.html
 */

#include <click/config.h>
#ifndef HAVE_IPSEC
# error "Must #define HAVE_IPSEC in config.h"
#endif
#include "esp.hh"
#include "desp.hh"
#include <click/ipaddress.hh>
#include <click/confparse.hh>
#include <click/error.hh>
#include <click/sha1.h>
#include <click/glue.hh>

IPsecESPUnencap::IPsecESPUnencap()
  : Element(1, 1)
{
}

IPsecESPUnencap::~IPsecESPUnencap()
{
}

IPsecESPUnencap *
IPsecESPUnencap::clone() const
{
  return new IPsecESPUnencap();
}

int
IPsecESPUnencap::configure(const Vector<String> &conf, ErrorHandler *errh)
{
  _sha1 = false;
  if (cp_va_parse(conf, this, errh,
	          cpOptional, 
		  cpBool, "verify SHA1", &_sha1,
		  0) < 0)
    return -1;
  return 0;
}


Packet *
IPsecESPUnencap::simple_action(Packet *p)
{

  int i, blks;
  const unsigned char * blk;

  // chop off authentication header
  if (_sha1) {
    const u_char *ah = p->data() + p->length() - 12;
    SHA1_ctx ctx;
    SHA1_init (&ctx);
    SHA1_update (&ctx, 
	         ((u_char*) p->data())+sizeof(esp_new), 
		 p->length()-12-sizeof(esp_new));
    SHA1_final (&ctx);
    const unsigned char *digest = SHA1_digest(&ctx);
    if (memcmp(ah, digest, 12)) {
      click_chatter("Invalid SHA1 authentication digest");
      p->kill();
      return(0);
    }
    p->take(12);
  }

  // rip off ESP header
  p->pull(sizeof(esp_new));

  // verify padding
  blks = p->length();
  blk = p->data();
  if((blk[blks - 2] != blk[blks - 3]) && (blk[blks -2] != 0)) {
    click_chatter("Invalid padding length");
    p->kill();
    return(0);
  }
  blks = blk[blks - 2];
  blk = p->data() + p->length() - (blks + 2);
  for(i = 0; (i < blks) && (blk[i] == ++i););    
  if(i<blks) {
    click_chatter("Corrupt padding");
    p->kill();
    return(0);
  }
  
  // chop off padding
  p->take(blks+2);
  return p;
}

EXPORT_ELEMENT(IPsecESPUnencap)
ELEMENT_MT_SAFE(IPsecESPUnencap)
