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


#ifndef _MFSet_h
#ifdef __GNUG__
#pragma interface
#endif
#define _MFSet_h 1

#include <Pix.h>
#include <iostream>

template<class Type> class MFSet
{
protected:

  int                   count;

public:
  virtual              ~MFSet() {};

  int                   length() const {return count;};  // current number of items
  int                   empty() const {return count == 0;};

  virtual Pix           add(Type  item) = 0;      // add item; return Pix
  virtual void          del(Type  item) = 0;      // delete item

  virtual int contains(Type  item) const 
		{return seek(item) != 0;}     // is item in set?

  virtual void clear();                 // delete all items

  virtual Pix           first() const = 0;             // Pix of first item or 0
  virtual void          next(Pix& i) const = 0;        // advance to next or 0
  virtual Type&         operator () (Pix i) const = 0; // access item at i

  virtual int owns(Pix idx);            // is i a valid Pix  ?
  virtual Pix seek(Type item) const;    // Pix of item

  void operator |= (MFSet<Type>& b); // add all items in b
  void operator -= (MFSet<Type>& b); // delete items also in b
  void operator &= (MFSet<Type>& b); // delete items not in b
  int operator == (MFSet<Type>& b);
  int operator != (MFSet<Type>& b) 
		{return !(*this == b);}
  int operator <= (MFSet<Type>& b); 

  void error(const char* msg) const {std::cerr << "Error :" << msg << std::endl;};
  virtual int OK() = 0;                // rep invariant
};


#endif
