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

#ifdef __GNUG__
#pragma implementation
#endif
#include <builtin.h>
#include <container/MFBag.h>

// error handling

template<class Type>
void MFBag<Type>::error(const char* msg)
{
  (*lib_error_handler)("Bag", msg);
}


template<class Type>
Pix MFBag<Type>::seek(Type  item, Pix i)
{
 extern int EQ(Type&, Type&);
 if (i == 0) 
    i = first();
  else
    next(i);
  for (;i != 0 && (!(EQ((*this)(i), item))); next(i));
  return i;
}

template<class Type>
int MFBag<Type>::owns(Pix p)
{
  if (p == 0) return 0;
  for (Pix i = first(); i != 0; next(i)) if (i == p) return 1;
  return 0;
}

template<class Type>
void MFBag<Type>::remove(Type  item)
{
  int i = nof(item);
  while (i-- > 0) del(item);
}


template<class Type>
int MFBag<Type>::nof(Type  item)
{
 extern int EQ(Type&, Type&);

  int n = 0;
  for (Pix p = first(); p; next(p)) if (EQ((*this)(p), item)) ++n;
  return n;
}

template<class Type>
void MFBag<Type>::clear()
{
  Pix i = first(); 
  while (i != 0)
  {
    del((*this)(i));
    i = first();
  }
}


