// -*- c-basic-offset: 4 -*-
#ifndef CLICK_STOREDATA_HH
#define CLICK_STOREDATA_HH
#include <click/element.hh>
CLICK_DECLS

/* =c
 * StoreData(OFFSET, DATA)
 * =s modification
 * changes packet data
 * =d
 *
 * Changes packet data starting at OFFSET to DATA.
 *
 * =a AlignmentInfo, click-align(1) */

class StoreData : public Element { public:
  
    StoreData();
    ~StoreData();
    
    const char *class_name() const		{ return "StoreData"; }
    const char *port_count() const		{ return PORTS_1_1; }
    const char *processing() const		{ return AGNOSTIC; }
    int configure(Vector<String> &, ErrorHandler *);
    
    Packet *simple_action(Packet *);

  private:

    unsigned _offset;
    String _data;
  
};

CLICK_ENDDECLS
#endif
