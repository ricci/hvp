// -*- c-basic-offset: 4 -*-
/*
 * randomseed.{cc,hh} -- test element that sets random seed
 * Eddie Kohler
 *
 * Copyright (c) 2004 Regents of the University of California
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
#include "randomseed.hh"
#include <click/confparse.hh>
#include <click/error.hh>
CLICK_DECLS

RandomSeed::RandomSeed()
{
    MOD_INC_USE_COUNT;
}

RandomSeed::~RandomSeed()
{
    MOD_DEC_USE_COUNT;
}

int
RandomSeed::configure(Vector<String> &conf, ErrorHandler *errh)
{
    if (conf.size() == 1 && !conf[0])
	conf.clear();
    bool truly_random = (conf.size() == 0);
    uint32_t seed;
    if (cp_va_parse(conf, this, errh, cpOptional,
		    cpUnsigned, "random seed", &seed,
		    cpEnd) < 0)
	return -1;
    if (truly_random)
	click_random_srandom();
    else
	srandom(seed);
    return 0;
}

void
RandomSeed::add_handlers()
{
    add_write_handler("seed", Element::reconfigure_positional_handler, (void*)0);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(RandomSeed)