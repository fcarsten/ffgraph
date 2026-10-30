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


#ifndef _AVLSet_h
#ifdef __GNUG__
#pragma interface
#endif
#define _AVLSet_h 1

#include <string.h>
#include <stream.h>
#include <stdlib.h>
#include "MFSet.h"
#include "AVL.h"

/*
 constants & inlines for maintaining balance & thread status in tree nodes
*/

template<class T>
struct AVLNode
{
	AVLNode<T>*         lt;
	AVLNode<T>*         rt;
  T                 item;
  char                stat;
	AVLNode(T  h, AVLNode<T>* l=0, AVLNode<T>* r=0);
	~AVLNode(){};
};


template<class T>
class AVLSet : public MFSet<T>
{
protected:
  AVLNode<T>*   root;

  AVLSet(AVLNode<T>* p, int l);

	int _need_rebalancing;   // to send back balance info from rec. calls
	T*   _target_item;     // add/del_item target
	AVLNode<T>* _found_node; // returned added/deleted node
	int    _already_found;   // for deletion subcases
	AVLNode<T>** _hold_nodes;       // used for rebuilding trees
	int  _max_hold_index;              // # elements-1 in _hold_nodes

  AVLNode<T>*   leftmost() const;
  AVLNode<T>*   rightmost() const;
  AVLNode<T>*   pred(AVLNode<T>* t);
  AVLNode<T>*   succ(AVLNode<T>* t) const;
  void          _kill(AVLNode<T>* t);
  void          _add(AVLNode<T>*& t);
  void          _del(AVLNode<T>* p, AVLNode<T>*& t);

	AVLNode<T>* _treeify(int n);
	AVLNode<T>* _do_treeify(int lo, int hi, int& h);

public:

	AVLSet();
	AVLSet(const AVLSet<T>& a);
	~AVLSet();

  Pix   add(T  item);
  void          del(T  item);
  int           contains(T  item) const;

  void          clear();

  Pix           first() const;
  void          next(Pix& i) const;
  T&          operator () (Pix i) const;
  int           owns(Pix i);
  Pix           seek(T  item) const;

  Pix           last();
  void          prev(Pix& i);

  AVLSet<T>&    operator =  (const AVLSet<T>& b);
  void          operator |= (const AVLSet<T>& b);
  void          operator -= (const AVLSet<T>& b);
  void          operator &= (const AVLSet<T>& b);

  int           operator == (const AVLSet<T>& b);
  int           operator != (const AVLSet<T>& b);
  int           operator <= (const AVLSet<T>& b); 

  int           OK();
};


#endif


