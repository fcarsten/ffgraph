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

#ifdef __GNUG__
#pragma implementation
#endif
#include "SLSet.h"

template < class T1 >
int SLSet<T1>::OK()
{
  int v = p.OK();
  v &= count == p.length();
  if (!v) error("invariant failure");
  return v;
}

template < class T1 >
Pix SLSet<T1>::seek(T1  item)
{
  Pix i;
  for ( i = p.first(); i != 0 && !EQ(p(i),item); p.next(i));
  return i;
}

template < class T1 >
Pix SLSet<T1>::add(T1  item)
{
  Pix i = seek(item);
  if (i == 0)
  {
    ++count;
    i = p.append(item);
  }
  return i;
}

template < class T1 >
void SLSet<T1>::del(T1  item)
{
  Pix i = p.first();
  if (i == 0)
    return;
  else if (EQ(p(i), item))
  {
    --count;
    p.del_front();
  }
  else
  {
    Pix trail = i;
    p.next(i);
    while (i != 0)
    {
      if (EQ(p(i), item))
      {
        --count;
        p.del_after(trail);
        return;
      }
      trail = i;
      p.next(i);
    }
  }
}    

