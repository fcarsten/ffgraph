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


#ifndef _SLStack_h
#ifdef __GNUG__
#pragma interface
#endif
#define _SLStack_h 1

#include "DLList.h"
#include "MFStack.h"

template<class T>
class SLStack : public MFStack<T>
{
 DLList<T> p;

public:
 SLStack();
 SLStack(const SLStack& s);
 inline ~SLStack();

 void operator = (const SLStack<T>&);

 inline void push(T  item);
 inline T pop();
 inline T& top();               
 inline void del_top();

 inline int empty();
 inline int full();
 inline int length();
 
 void clear();
 void del(T item);
 inline int OK();
};

template<class T>
inline SLStack<T>::SLStack() :p() {}
template<class T>
inline SLStack<T>::SLStack(const SLStack& a) : p(a.p) {}
template<class T>
inline SLStack<T>::~SLStack() {}

template<class T>
inline void SLStack<T>::push(T  item)
{
  p.prepend(item);
}

template<class T>
inline T SLStack<T>::pop()
{
  return p.remove_front();
}

template<class T>
inline T& SLStack<T>::top()
{
  return p.front();
}

template<class T>
inline void SLStack<T>::del_top()
{
  p.del_front();
}

template<class T>
inline void SLStack<T>::operator =(const SLStack& s)
{
  p = s.p;
}

template<class T>
inline int SLStack<T>::empty() 
{
  return p.empty();
}

template<class T>
inline int SLStack<T>::full() 
{
  return 0;
}

template<class T>
inline int SLStack<T>::length() 
{
  return p.length();
}

template<class T>
inline int SLStack<T>::OK() 
{
  return p.OK();
}


#endif
