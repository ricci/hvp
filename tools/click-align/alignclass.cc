/*
 * alignclass.{cc,hh} -- element classes that know about alignment constraints
 * Eddie Kohler
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

#include "alignclass.hh"
#include <click/confparse.hh>
#include <click/error.hh>
#include "routert.hh"
#include <cstring>

Alignment
common_alignment(const Vector<Alignment> &a, int off, int n)
{
  if (n == 0)
    return Alignment(1, 0);
  Alignment m = a[off];
  for (int i = 1; i < n; i++)
    m |= a[off+i];
  return m;
}

Alignment
combine_alignment(const Vector<Alignment> &a, int off, int n)
{
  if (n == 0)
    return Alignment(1, 0);
  Alignment m = a[off];
  for (int i = 1; i < n; i++)
    m &= a[off+i];
  return m;
}

void
Aligner::have_flow(const Vector<Alignment> &ain, int offin, int nin, Vector<Alignment> &aout, int offout, int nout)
{
  Alignment a = common_alignment(ain, offin, nin);
  for (int j = 0; j < nout; j++)
    aout[offout + j] = a;
}

void
Aligner::want_flow(Vector<Alignment> &ain, int offin, int nin, const Vector<Alignment> &aout, int offout, int nout)
{
  Alignment a = combine_alignment(aout, offout, nout);
  for (int j = 0; j < nin; j++)
    ain[offin + j] = a;
}

void
Aligner::adjust_flow(Vector<Alignment> &, int, int, const Vector<Alignment> &, int, int)
{
}

void
NullAligner::have_flow(const Vector<Alignment> &, int, int, Vector<Alignment> &, int, int)
{
}

void
NullAligner::want_flow(Vector<Alignment> &, int, int, const Vector<Alignment> &, int, int)
{
}

void
CombinedAligner::have_flow(const Vector<Alignment> &ain, int offin, int nin, Vector<Alignment> &aout, int offout, int nout)
{
  _have->have_flow(ain, offin, nin, aout, offout, nout);
}

void
CombinedAligner::want_flow(Vector<Alignment> &ain, int offin, int nin, const Vector<Alignment> &aout, int offout, int nout)
{
  _want->want_flow(ain, offin, nin, aout, offout, nout);
}

void
GeneratorAligner::have_flow(const Vector<Alignment> &, int, int, Vector<Alignment> &aout, int offout, int nout)
{
  for (int j = 0; j < nout; j++)
    aout[offout + j] = _alignment;
}

void
GeneratorAligner::want_flow(Vector<Alignment> &ain, int offin, int nin, const Vector<Alignment> &, int, int)
{
  for (int j = 0; j < nin; j++)
    ain[offin + j] = Alignment();
}

void
ShifterAligner::have_flow(const Vector<Alignment> &ain, int offin, int nin, Vector<Alignment> &aout, int offout, int nout)
{
  Alignment a = common_alignment(ain, offin, nin);
  a += _shift;
  for (int j = 0; j < nout; j++)
    aout[offout + j] = a;
}

void
ShifterAligner::want_flow(Vector<Alignment> &ain, int offin, int nin, const Vector<Alignment> &aout, int offout, int nout)
{
  Alignment a = combine_alignment(aout, offout, nout);
  a -= _shift;
  for (int j = 0; j < nin; j++)
    ain[offin + j] = a;
}

void
WantAligner::want_flow(Vector<Alignment> &ain, int offin, int nin, const Vector<Alignment> &, int, int)
{
  for (int j = 0; j < nin; j++)
    ain[offin + j] = _alignment;
}

void
ClassifierAligner::adjust_flow(Vector<Alignment> &ain, int offin, int nin, const Vector<Alignment> &, int, int)
{ 
  Alignment a = common_alignment(ain, offin, nin);
  if (a.chunk() < 4)
    a = Alignment(4, a.offset());
  for (int j = 0; j < nin; j++)
    ain[offin + j] = a;
}


Aligner *
default_aligner()
{
  static Aligner *a;
  if (!a) a = new Aligner;
  return a;
}


AlignClass::AlignClass(const String &name)
  : ElementClassT(name), _aligner(default_aligner())
{
}

AlignClass::AlignClass(const String &name, Aligner *a)
  : ElementClassT(name), _aligner(a)
{
}

Aligner *
AlignClass::create_aligner(ElementT *, RouterT *, ErrorHandler *)
{
  return _aligner;
}

void *
AlignClass::cast(const char *s)
{
  if (strcmp(s, "AlignClass") == 0)
    return (void *)this;
  else
    return 0;
}


StripAlignClass::StripAlignClass()
  : AlignClass("Strip")
{
}

Aligner *
StripAlignClass::create_aligner(ElementT *e, RouterT *, ErrorHandler *errh)
{
  int m;
  ContextErrorHandler cerrh(errh, "While analyzing alignment for `" + e->declaration() + "':");
  if (cp_va_parse(e->configuration(), &cerrh,
		  cpInteger, "amount to strip", &m,
		  0) < 0)
    return default_aligner();
  return new ShifterAligner(m);
}


CheckIPHeaderAlignClass::CheckIPHeaderAlignClass(const String &name, int argno)
  : AlignClass(name), _argno(argno)
{
}

Aligner *
CheckIPHeaderAlignClass::create_aligner(ElementT *e, RouterT *, ErrorHandler *errh)
{
  unsigned offset = 0;
  Vector<String> args;
  cp_argvec(e->configuration(), args);
  if (args.size() > _argno) {
    if (!cp_unsigned(args[_argno], &offset)) {
      ContextErrorHandler cerrh(errh, "While analyzing alignment for `" + e->declaration() + "':");
      cerrh.error("argument %d should be IP header offset (unsigned)", _argno + 1);
      return default_aligner();
    }
  }
  return new WantAligner(Alignment(4, 0) - (int)offset);
}


AlignAlignClass::AlignAlignClass()
  : AlignClass("Align")
{
}

Aligner *
AlignAlignClass::create_aligner(ElementT *e, RouterT *, ErrorHandler *errh)
{
  int offset, chunk;
  ContextErrorHandler cerrh(errh, "While analyzing alignment for `" + e->declaration() + "':");
  if (cp_va_parse(e->configuration(), &cerrh,
		  cpUnsigned, "alignment modulus", &chunk,
		  cpUnsigned, "alignment offset", &offset,
		  0) < 0)
    return default_aligner();
  return new GeneratorAligner(Alignment(chunk, offset));
}
