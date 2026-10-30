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


#ifndef _MFMap_h
#ifdef __GNUG__
#pragma interface
#endif
#define _MFMap_h 1

#include <Pix.h>

template<class T1, class T2>
class MFMap
{
protected:
  unsigned int                   count;
  T2                   def;

public:
                        MFMap(T2  dflt);
  virtual              ~MFMap();

  int                   length() const; // current number of items
  int                   empty() const;

  virtual int           contains(T1  key);      // is key mapped?

  virtual void          clear();                 // delete all items

  virtual T2&          operator [] (T1  key) = 0; // access contents by key

  virtual void          del(T1  key) = 0;       // delete entry

  virtual Pix           first() const = 0;             // Pix of first item or 0
  virtual void          next(Pix& i) const = 0;        // advance to next or 0
  virtual T1&          key(Pix i) const = 0;          // access key at i
  virtual T2&          contents(Pix i) const = 0;     // access contents at i

  virtual int           owns(Pix i);             // is i a valid Pix  ?
  virtual Pix           seek(T1  key);          // Pix of key

  T2&                  dflt();                  // access default val

  void                  error(const char* msg) const;
  virtual int           OK() = 0;                // rep invariant
};


#endif
