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
Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/


#ifndef _MFBag_h
#ifdef __GNUG__
#pragma interface
#endif
#define _MFBag_h 1

#include <Pix.h>

template<class Type>
class MFBag
{
protected:
  int count;

public:
  virtual ~MFBag(){}

  int length();                // current number of items
  int empty();

  virtual Pix add(Type  item) = 0;      // add item; return Pix

  virtual void del(Type  item) = 0;      // delete 1 occurrence of item
#ifdef remove
#undef remove
#endif
  virtual void remove(Type  item);       // delete all occurrences
  virtual void          clear();                 // delete all items

  virtual int contains(Type  item);     // is item in Bag?
  virtual int nof(Type  item);          // how many in Bag?

  virtual Pix first() = 0;             // Pix of first item or 0
  virtual void next(Pix& i) = 0;        // advance to next or 0

  virtual Type& operator () (Pix i) = 0; // access item at i

  virtual Pix seek(Type  item, Pix from=0); // Pix of next occurrence
  virtual int owns(Pix i);             // is i a valid Pix  ?

  void error(const char* msg);
  virtual int OK() = 0;                // rep invariant
};

template<class Type>
inline int MFBag<Type>::length()
{
  return count;
}

template<class Type>
inline int MFBag<Type>::empty()
{
  return count == 0;
}

template<class Type>
inline int MFBag<Type>::contains(Type  item)
{
  return seek(item) != 0;
}
 
#endif
