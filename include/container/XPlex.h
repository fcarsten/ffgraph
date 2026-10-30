// This may look like C code, but it is really -*- C++ -*-
/* 
Copyright (C) 1988 Free Software Foundation
    written by Doug Lea (dl@rocky.oswego.edu)
    based on code by Marc Shapiro (shapiro@sor.inria.fr)

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

#ifndef _XPlex_h
#ifdef __GNUG__
#pragma interface
#endif
#define _XPlex_h 1

#include "MFPlex.h"

template<class Type>
class XPlex: public MFPlex<Type>
{
  MFIChunk<Type>*       ch;           // cached chunk

  void             make_initial_chunks(int up = 1);

  void             cache(int idx) const;
  void             cache(const Type* p) const;

  Type*             dopred(const Type* p) const;
  Type*             dosucc(const Type* p) const;

  void             set_cache(const MFIChunk<Type>* t) const; // logically, 
                                               // not physically const
public:
  XPlex();                 // set low = 0;
                                // fence = 0;
                                // csize = default
  
  XPlex(int ch_size);      // low = 0; 
                                // fence = 0;
                                // csize = ch_size
  
  XPlex(int lo,            // low = lo; 
        int ch_size);      // fence=lo
                                // csize = ch_size
  
  XPlex(int lo,            // low = lo
        int hi,            // fence = hi+1
        const Type  initval,// fill with initval,
        int ch_size = 0);  // csize= ch_size
                                // or fence-lo if 0

  XPlex(const XPlex<Type>&);
  
  void operator= (const XPlex<Type>&);

// virtuals


  Type& high_element ();
  Type& low_element ();

  const Type& high_element () const;
  const Type& low_element () const;

  Pix first() const;
  Pix last() const;
  void prev(Pix& ptr) const;
  void next(Pix& ptr) const;
  int owns(Pix p) const;
  Type& operator () (Pix p);
  const Type& operator () (Pix p) const;

  int low() const; 
  int high() const;
  int valid(int idx) const;
  void prev(int& idx) const;
  void next(int& x) const;
  Type& operator [] (int index);
  const Type& operator [] (int index) const;
    
  int Pix_to_index(Pix p) const;
  Pix index_to_Pix(int idx) const;    

  int can_add_high() const;
  int can_add_low() const;
  int full() const;

  int add_high(const Type  elem);
  int del_high ();
  int add_low (const Type  elem);
  int del_low ();

  void fill(const Type  x);
  void fill(const Type  x, int from, int to);
  void clear();
  void reverse();
    
  int OK () const; 

};


template<class Type>
void XPlex<Type>::prev(int& idx) const
{
  --idx;
}

template<class Type>
void XPlex<Type>::next(int& idx) const
{
  ++idx;
}

template<class Type>
int XPlex<Type>::full () const
{
  return 0;
}

template<class Type>
int XPlex<Type>::can_add_high() const
{
  return 1;
}

template<class Type>
int XPlex<Type>::can_add_low() const
{
  return 1;
}

template<class Type>
int XPlex<Type>::valid (int idx) const
{
  return idx >= lo && idx < fnc;
}

template<class Type>
int XPlex<Type>::low() const
{
  return lo;
}

template<class Type>
int XPlex<Type>::high() const
{
  return fnc - 1;
}

template<class Type>
Type& XPlex<Type>:: operator [] (int idx)
{
  if (!ch->actual_index(idx)) cache(idx);
  return *(ch->pointer_to(idx));
}

template<class Type>
const Type& XPlex<Type>:: operator [] (int idx) const
{
  if (!ch->actual_index(idx)) cache(idx);
  return *((const Type*)(ch->pointer_to(idx)));
}

template<class Type>
Type& XPlex<Type>::low_element ()
{
  if (empty()) index_error();
  return *(hd->pointer_to(lo));
}

template<class Type>
const Type& XPlex<Type>::low_element () const
{
  if (empty()) index_error();
  return *((const Type*)(hd->pointer_to(lo)));
}

template<class Type>
Type& XPlex<Type>::high_element ()
{
  if (empty()) index_error();
  return *(tl()->pointer_to(fnc - 1));
}

template<class Type>
const Type& XPlex<Type>::high_element () const
{
  if (empty()) index_error();
  return *((const Type*)(tl()->pointer_to(fnc - 1)));
}

template<class Type>
int XPlex<Type>::Pix_to_index(Pix px) const
{
  Type* p = (Type*)px;
  if (!ch->actual_pointer(p)) cache(p);
  return ch->index_of(p);
}

template<class Type>
Pix XPlex<Type>::index_to_Pix(int idx) const
{
  if (!ch->actual_index(idx)) cache(idx);
  return (Pix)(ch->pointer_to(idx));
}

template<class Type>
Pix XPlex<Type>::first() const
{
  return Pix(hd->MFIChunk<Type>::first_pointer());
}

template<class Type>
Pix XPlex<Type>::last() const
{
  return Pix(tl()->MFIChunk<Type>::last_pointer());
}

template<class Type>
void XPlex<Type>::prev(Pix& p) const
{
  Pix q = Pix(ch->MFIChunk<Type>::pred((Type*) p));
  p = (q == 0)? Pix(dopred((const Type*) p)) : q;
}

template<class Type>
void XPlex<Type>::next(Pix& p) const
{
  Pix q = Pix(ch->MFIChunk<Type>::succ((Type*) p));
  p = (q == 0)? Pix(dosucc((const Type*)p)) : q;
}

template<class Type>
Type& XPlex<Type>:: operator () (Pix p)
{
  return *((Type*)p);
}

template<class Type>
const Type& XPlex<Type>:: operator () (Pix p) const
{
  return *((const Type*)p);
}

template<class Type>
void XPlex<Type>::set_cache(const MFIChunk<Type>* t) const
{
  ((XPlex<Type>*)(this))->ch = (MFIChunk<Type>*)t;
}

template<class Type>
XPlex<Type>:: XPlex()
{
  lo = fnc = 0;
  csize = PLEX_DEFAULT_INITIAL_CAPACITY;
  Type* data = new Type[csize];
  set_cache(new MFIChunk<Type>(data,  lo, lo, fnc, lo+csize));
  hd = ch;
}

template<class Type>
XPlex<Type>:: XPlex(int chunksize)
{
  if (chunksize == 0) error("invalid constructor specification");
  lo = fnc = 0;
  if (chunksize > 0)
  {
    csize = chunksize;
    Type* data = new Type[csize];
    set_cache(new MFIChunk<Type>(data,  lo, lo, fnc, csize));
    hd = ch;
  }
  else
  {
    csize = -chunksize;
    Type* data = new Type[csize];
    set_cache(new MFIChunk<Type>(data,  chunksize, lo, fnc, fnc));
    hd = ch;
  }
}


template<class Type>
XPlex<Type>:: XPlex(int l, int chunksize)
{
  if (chunksize == 0) error("invalid constructor specification");
  lo = fnc = l;
  if (chunksize > 0)
  {
    csize = chunksize;
    Type* data = new Type[csize];
    set_cache(new MFIChunk<Type>(data,  lo, lo, fnc, csize+lo));
    hd = ch;
  }
  else
  {
    csize = -chunksize;
    Type* data = new Type[csize];
    set_cache(new MFIChunk<Type>(data,  chunksize+lo, lo, fnc, fnc));
    hd = ch;
  }
}

template<class Type>
void XPlex<Type>::make_initial_chunks(int up)
{
  int need = fnc - lo;
  hd = 0;
  if (up)
  {
    int l = lo;
    do
    {
      int sz;
      if (need >= csize)
        sz = csize;
      else
        sz = need;
      Type* data = new Type [csize];
      MFIChunk<Type>* h = new MFIChunk<Type>(data,  l, l, l+sz, l+csize);
      if (hd != 0)
        h->link_to_next(hd);
      else
        hd = h;
      l += sz;
      need -= sz;
    } while (need > 0);
  }
  else
  {
    int hi = fnc;
    do
    {
      int sz;
      if (need >= csize)
        sz = csize;
      else
        sz = need;
      Type* data = new Type [csize];
      MFIChunk<Type>* h = new MFIChunk<Type>(data,  hi-csize, hi-sz, hi, hi);
      if (hd != 0)
        h->link_to_next(hd);
      hd = h;
      hi -= sz;
      need -= sz;
    } while (need > 0);
  }
  set_cache(hd);
}

template<class Type>
XPlex<Type>:: XPlex(int l, int hi, const Type  initval, int chunksize)
{
  lo = l;
  fnc = hi + 1;
  if (chunksize == 0)
  {
    csize = fnc - l;
    make_initial_chunks(1);
  }
  else if (chunksize < 0)
  {
    csize = -chunksize;
    make_initial_chunks(0);
  }
  else
  {
    csize = chunksize;
    make_initial_chunks(1);
  }
  fill(initval);
}

template<class Type>
XPlex<Type>::XPlex(const XPlex<Type>& a)
{
  lo = a.lo;
  fnc = a.fnc;
  csize = a.csize;
  make_initial_chunks();
  for (int i = a.low(); i < a.fence(); a.next(i)) (*this)[i] = a[i];
}

template<class Type>
void XPlex<Type>::operator= (const XPlex<Type>& a)
{
  if (&a != this)
  {
    invalidate();
    lo = a.lo;
    fnc = a.fnc;
    csize = a.csize;
    make_initial_chunks();
    for (int i = a.low(); i < a.fence(); a.next(i)) (*this)[i] = a[i];
  }
}


template<class Type>
void XPlex<Type>::cache(int idx) const
{
  const MFIChunk<Type>* tail = tl();
  const MFIChunk<Type>* t = ch;
  while (idx >= t->fence_index())
  {
    if (t == tail) index_error();
    t = (t->next());
  }
  while (idx < t->low_index())
  {
    if (t == hd) index_error();
    t = (t->prev());
  }
  set_cache(t);
}


template<class Type>
void XPlex<Type>::cache(const Type* p) const
{
  const MFIChunk<Type>* old = ch;
  const MFIChunk<Type>* t = ch;
  while (!t->actual_pointer(p))
  {
    t = (t->next());
    if (t == old) index_error();
  }
  set_cache(t);
}

template<class Type>
int XPlex<Type>::owns(Pix px) const
{
  Type* p = (Type*)px;
  const MFIChunk<Type>* old = ch;
  const MFIChunk<Type>* t = ch;
  while (!t->actual_pointer(p))
  {
    t = (t->next());
    if (t == old) { set_cache(t); return 0; }
  }
  set_cache(t);
  return 1;
}


template<class Type>
Type* XPlex<Type>::dosucc(const Type* p) const
{
  if (p == 0) return 0;
  const MFIChunk<Type>* old = ch;
  const MFIChunk<Type>* t = ch;
 
  while (!t->actual_pointer(p))
  {
    t = (t->next());
    if (t == old)  return 0;
  }
  int i = t->index_of(p) + 1;
  if (i >= fnc) return 0;
  if (i >= t->fence_index()) t = (t->next());
  set_cache(t);
  return (t->pointer_to(i));
}

template<class Type>
Type* XPlex<Type>::dopred(const Type* p) const
{
  if (p == 0) return 0;
  const MFIChunk<Type>* old = ch;
  const MFIChunk<Type>* t = ch;
  while (!t->actual_pointer(p))
  {
    t = (t->prev());
    if (t == old) return 0;
  }
  int i = t->index_of(p) - 1;
  if (i < lo) return 0;
  if (i < t->low_index()) t = (t->prev());
  set_cache(t);
  return (t->pointer_to(i));
}


template<class Type>
int XPlex<Type>::add_high(const Type  elem)
{
  MFIChunk<Type>* t = tl();
  if (!t->can_grow_high())
  {
    if (t->MFIChunk<Type>::empty() && one_chunk())
      t->clear(fnc);
    else
    {
      Type* data = new Type [csize];
      t = (new MFIChunk<Type>(data,  fnc, fnc, fnc,fnc+csize));
      t->link_to_prev(tl());
    }
  }
  *((t->MFIChunk<Type>::grow_high())) = elem;
  set_cache(t);
  return fnc++;
}

template<class Type>
int XPlex<Type>::del_high ()
{
  if (empty()) empty_error();
  MFIChunk<Type>* t = tl();
  t->MFIChunk<Type>::shrink_high();
  if (t->MFIChunk<Type>::empty() && !one_chunk())
  {
    MFIChunk<Type>* pred = t->prev();
    del_chunk(t);
    t = pred;
  }
  set_cache(t);
  return --fnc - 1;
}

template<class Type>
int XPlex<Type>::add_low (const Type  elem)
{
  MFIChunk<Type>* t = hd;
  if (!t->can_grow_low())
  {
    if (t->MFIChunk<Type>::empty() && one_chunk())
      t->cleardown(lo);
    else
    {
      Type* data = new Type [csize];
      hd = new MFIChunk<Type>(data,  lo-csize, lo, lo, lo);
      hd->link_to_next(t);
      t = hd;
    }
  }
  *((t->MFIChunk<Type>::grow_low())) = elem;
  set_cache(t);
  return --lo;
}


template<class Type>
int XPlex<Type>::del_low ()
{
  if (empty()) empty_error();
  MFIChunk<Type>* t = hd;
  t->MFIChunk<Type>::shrink_low();
  if (t->MFIChunk<Type>::empty() && !one_chunk())
  {
    hd = t->next();
    del_chunk(t);
    t = hd;
  }
  set_cache(t);
  return ++lo;
}

template<class Type>
void XPlex<Type>::reverse()
{
  Type tmp;
  int l = lo;
  int h = fnc - 1;
  MFIChunk<Type>* loch = hd;
  MFIChunk<Type>* hich = tl();
  while (l < h)
  {
    Type* lptr = loch->pointer_to(l);
    Type* hptr = hich->pointer_to(h);
    tmp = *lptr;
    *lptr = *hptr;
    *hptr = tmp;
    if (++l >= loch->fence_index()) loch = loch->next();
    if (--h < hich->low_index()) hich = hich->prev();
  }
}

template<class Type>
void XPlex<Type>::fill(const Type  x)
{
  for (int i = lo; i < fnc; ++i) (*this)[i] = x;
}

template<class Type>
void XPlex<Type>::fill(const Type  x, int l, int hi)
{
  for (int i = l; i <= hi; ++i) (*this)[i] = x;
}


template<class Type>
void XPlex<Type>::clear()
{
  if (fnc != lo)
  {
    MFIChunk<Type>* t = tl();
    while (t != hd)
    {
      MFIChunk<Type>* prv = t->prev();
      del_chunk(t);
      t = prv;
    }
    t->MFIChunk<Type>::clear(lo);
    set_cache(t);
    fnc = lo;
  }
}


template<class Type>
int XPlex<Type>::OK () const
{
  int v = hd != 0 && ch != 0;     // at least one chunk

  v &= fnc == tl()->fence_index();// last chunk fence == plex fence
  v &= lo == ((hd))->MFIChunk<Type>::low_index();    // first lo == plex lo

// loop for others:
  int found_ch = 0;                   // to make sure ch is in list;
  const MFIChunk<Type>* t = (hd);
  for (;;)
  {
    if (t == ch) ++found_ch;
    v &= t->MFIChunk<Type>::OK();              // each chunk is OK
    if (t == tl())
      break;
    else                              // and has indices contiguous to succ
    {
      v &= t->top_index() == t->next()->base_index();
      if (t != hd)                  // internal chunks full
      {
        v &= !t->empty();
        v &= !t->can_grow_low();
        v &= !t->can_grow_high();
      }
      t = t->next();
    }
  }
  v &= found_ch == 1;
  if (!v) error("invariant failure");
  return v;
}

#endif
