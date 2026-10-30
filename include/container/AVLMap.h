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


#ifndef _AVLMap_h
#ifdef __GNUG__
#pragma interface
#endif
#define _AVLMap_h 1

#include "MFMap.h"
#include "AVL.h"

template<class T1, class T2>
struct AVLMNode
{
  AVLMNode<T1,T2>*      lt;
  AVLMNode<T1,T2>*      rt;
  T1                 item;
  T2                 cont;
  char                stat;
                     
 AVLMNode(T1, T2, AVLMNode* l=0, AVLMNode* r=0);
 ~AVLMNode();
};

template<class T1, class T2>
class AVLMap : public MFMap<T1,T2>
{
private:

  AVLMap<T1,T2>& operator = (const AVLMap<T1,T2>&);

protected:
  AVLMNode<T1,T2>*   root;

  AVLMNode<T1,T2>*   leftmost() const;
  AVLMNode<T1,T2>*   rightmost();
  AVLMNode<T1,T2>*   pred(AVLMNode<T1,T2>*);
  AVLMNode<T1,T2>*   succ(AVLMNode<T1,T2>*) const;
  void  _kill(AVLMNode<T1,T2>*);
  void _add(AVLMNode<T1,T2>*&);
  void _del(AVLMNode<T1,T2>*, AVLMNode<T1,T2>*&);

	T1*   _target_item;     // add/del_item target
	AVLMNode<T1,T2>* _found_node; // returned added/deleted node
	int _need_rebalancing;   // to send back balance info from rec. calls
	int _already_found;   // for deletion subcases

public:
	AVLMap(T2);
	AVLMap(const AVLMap<T1,T2>&);
  ~AVLMap();

  T2& operator [] (T1);

  void del(T1);
  Pix first() const;
  void next(Pix&) const;
  T1& key(Pix) const;
  T2& contents(Pix) const;

  Pix seek(T1);
  int contains(T1);

  void clear(); 

  Pix last();
  void prev(Pix&);

  int OK();
};


#endif

