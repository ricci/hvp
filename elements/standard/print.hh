#ifndef CLICK_PRINT_HH
#define CLICK_PRINT_HH
#include <click/element.hh>
#include <click/string.hh>
CLICK_DECLS

/*
=c

Print([TAG, NBYTES, I<KEYWORDS>])

=s debugging

prints packet contents

=d

Prints up to NBYTES bytes of data from each packet, in hex, preceded by the
TAG text. Default NBYTES is 24.

Keyword arguments are:

=over 8

=item NBYTES

Number of bytes to print. Default is 24.

=item TIMESTAMP

Boolean. Determines whether to print each packet's timestamp in seconds since
1970. Default is false.

=item PRINTANNO

Boolean. Determines whether to print each packet's user annotation bytes.  Default is false.

=item CPU

Boolean; available only in the Linux kernel module. Determines whether to
print the current CPU ID for every packet. Default is false.

=back

=a

IPPrint */

class Print : public Element { public:

  Print();
  ~Print();
  
  const char *class_name() const		{ return "Print"; }
  const char *port_count() const		{ return PORTS_1_1; }
  const char *processing() const		{ return AGNOSTIC; }
  
  int configure(Vector<String> &, ErrorHandler *);
  bool can_live_reconfigure() const		{ return true; }
  
  Packet *simple_action(Packet *);
  
 private:
  
  String _label;
  unsigned _bytes;		// How many bytes of a packet to print
  bool _timestamp : 1;
#ifdef CLICK_LINUXMODULE
  bool _cpu : 1;
#endif
  bool _print_anno;
};

CLICK_ENDDECLS
#endif
