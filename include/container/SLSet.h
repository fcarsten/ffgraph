// This may look like C code, but it is really -*- C++ -*-
/* 
Copyright (C) 1988 Free Software Foundation
    written by Doug Lea (dl@rocky.oswego.edu)

This file is part of the GNU C++ Library.  This library is free
software; you can redistribute it and/or modify it under the terms of
the GNU Library General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your
option) any later version.  This library is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU Library General Public License for more details.
You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free Software
Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#ifndef _SLMFSet_h
#ifdef __GNUG__
#pragma interface
#endif
#define _SLMFSet_h 1

#include "MFSet.h"
#include "SLList.h"

template < class T1 >
class SLSet : public MFSet<T1>
{
protected:
  SLList<T1>     p;

public:
                SLSet();
                SLSet(const SLSet<T1>&);

  Pix           add(T1  item);
  void          del(T1  item);
  int           contains(T1  item) const;

  void          clear();

  Pix           first() const;
  void          next(Pix& i) const;
  T1&          operator () (Pix i) const;
  int           owns(Pix i);
  Pix           seek(T1  item) const;

  int           OK();
};

template < class T1 >
inline SLSet<T1>::SLSet() : p() { count = 0; }

template < class T1 >
inline SLSet<T1>::SLSet(const SLSet<T1>& s) : p(s.p) { count = s.count; }

template < class T1 >
inline Pix SLSet<T1>::first() const 
{
  return p.first();
}

template < class T1 >
inline void SLSet<T1>::next(Pix  & idx) const
{
  p.next(idx);
}

template < class T1 >
inline T1& SLSet<T1>::operator ()(Pix   idx) const
{
  return p(idx);
}

template < class T1 >
inline void SLSet<T1>::clear()
{
  count = 0;  p.clear();
}

template < class T1 >
inline int SLSet<T1>::contains (T1  item) const
{
  return seek(item) != 0;
}

template < class T1 >
inline int SLSet<T1>::owns (Pix   idx)
{
  return p.owns(idx);
}

#endif
