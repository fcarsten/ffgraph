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

#ifndef _MFPlex_h
#ifdef __GNUG__
#pragma interface
#endif
#define _MFPlex_h 1

#include <Pix.h>
#include <iostream>

// Plexes are made out of MFIChunks

#include <stddef.h>
#ifndef PLEX_DEFAULT_INITIAL_CAPACITY
#define PLEX_DEFAULT_INITIAL_CAPACITY 5
#endif

template<class Type>
class MFIChunk
{
//public: // kludge until C++ `protected' policies settled
protected:      

  Type*           data;           // data, from client

  int            base;           // lowest possible index
  int            low;            // lowest valid index
  int            fence;          // highest valid index + 1
  int            top;            // highest possible index + 1

  MFIChunk<Type>*     nxt;            // circular links
  MFIChunk<Type>*     prv;

public:

// constructors

                 MFIChunk(Type*     d,       // ptr to array of elements
                        int      base_idx, // initial indices
                        int      low_idx,  
                        int      fence_idx,
                        int      top_idx);

  virtual       ~MFIChunk();

// status reports

  int            size() const;    // number of slots

  virtual int    empty() const ;
  virtual int    full() const ; 

  int            can_grow_high () const ;  // there is space to add data
  int            can_grow_low () const;        
  
  int            base_index() const;   // lowest possible index;
  int            low_index() const;    // lowest actual index;
  virtual int    first_index() const;  // lowest valid index or fence if none
  virtual int    last_index() const;   // highest valid index or low-1 if none
  int            fence_index() const;  // highest actual index + 1
  int            top_index() const;    // highest possible index + 1

// indexing conversion

  int            possible_index(int i) const; // i between base and top
  int            actual_index(int i) const;   // i between low and fence
  virtual int    valid_index(int i) const;    // i not deleted (mainly for mchunks)

  int            possible_pointer(const Type* p) const; // same for ptr
  int            actual_pointer(const Type* p) const; 
  virtual int    valid_pointer(const Type* p) const; 

  Type*           pointer_to(int i) const ;   // pointer to data indexed by i
                                      // caution: i is not checked for validity
  int            index_of(const Type* p) const; // index of data pointed to by p
                                      // caution: p is not checked for validity

  virtual int    succ(int idx) const;     // next valid index or fence if none
  virtual int    pred(int idx) const;     // previous index or low - 1 if none

  virtual Type*   first_pointer() const;   // pointer to first valid pos or 0
  virtual Type*   last_pointer() const;    // pointer to first valid pos or 0
  virtual Type*   succ(Type*  p) const;     // next pointer or 0
  virtual Type*   pred(Type* p) const;     // previous pointer or 0

// modification

  virtual Type*   grow_high ();      // return spot to add an element
  virtual Type*   grow_low ();  

  virtual void   shrink_high ();    // logically delete top index
  virtual void   shrink_low ();     

  virtual void   clear(int lo);     // reset to empty ch with base = lo
  virtual void   cleardown(int hi); // reset to empty ch with top = hi
  void           re_index(int lo);  // re-index so lo is new low

// chunk traversal

  MFIChunk<Type>*     next() const;
  MFIChunk<Type>*     prev() const;

  void           link_to_prev(MFIChunk<Type>* prev);
  void           link_to_next(MFIChunk<Type>* next);
  void           unlink();

// state checks

  Type*           invalidate();     // mark self as invalid; return data
                                   // for possible deletion

  virtual int    OK() const;             // representation invariant

  void   error(const char*) const;
  void   empty_error() const;
  void   full_error() const;
  void   index_error() const;
};

// MFPlex is a partly `abstract' class: few of the virtuals
// are implemented at the Plex level, only in the subclasses

template<class Type>
class MFPlex
{
protected:      

  MFIChunk<Type>*       hd;          // a chunk holding the data
  int              lo;          // lowest  index
  int              fnc;         // highest index + 1
  int              csize;       // size of the chunk

  void             invalidate();              // mark so OK() is false
  void             del_chunk(MFIChunk<Type>*);        // delete a chunk

  MFIChunk<Type>*       tl() const;                // last chunk;
  int              one_chunk() const;         // true if hd == tl()

public:

// constructors, etc.

                    MFPlex() : hd(0), lo(0), fnc(0), csize(0) {} // empty base state

  virtual           ~MFPlex();

// Access functions 
    
  virtual Type&      operator [] (int idx) = 0; // access by index;
  virtual Type&      operator () (Pix p) = 0;   // access by Pix;

//   virtual Type&      high_element () = 0;      // access high element
//   virtual Type&      low_element () = 0;       // access low element


// read-only versions for const Plexes

  virtual const Type& operator [] (int idx) const = 0; // access by index;
  virtual const Type& operator () (Pix p) const = 0;   // access by Pix;

  virtual const Type& high_element () const = 0;      // access high element
  virtual const Type& low_element () const = 0;       // access low element

  

// Index functions

  virtual int       valid (int idx) const = 0;      // idx is an OK index

  virtual int       low() const = 0;         // lowest index or fence if none
  virtual int       high() const = 0;        // highest index or low-1 if none

  int               ecnef() const;         // low limit index (low-1)
  int               fence() const;         // high limit index (high+1)

//  virtual void      prev(void *& idx) const= 0; // set idx to preceding index
//                                           // caution: pred may be out of bounds

//  virtual void      next(void *&idx) const = 0;       // set to next index
                                          // caution: succ may be out of bounds

  virtual Pix       first() const = 0;        // Pix to low element or 0
  virtual Pix       last() const = 0;         // Pix to high element or 0
  virtual void      prev(Pix& pix) const = 0;  // preceding pix or 0
  virtual void      next(Pix& pix) const = 0;  // next pix or 0
  virtual int       owns(Pix p) const = 0;     // p is an OK Pix

 virtual void             prevInt(int& idx) const =0;
 virtual void             nextInt(int& x) const =0;
 

// index<->Pix 

  virtual int       Pix_to_index(Pix p) const = 0;   // get index via Pix
  virtual Pix       index_to_Pix(int idx) const = 0; // Pix via index

// Growth

  virtual int       add_high(const Type  elem) =0;// add new element at high end
                                                // return new high

  virtual int       add_low(const Type  elem) = 0;   // add new low element,
                                                // return new low

// Shrinkage

  virtual int       del_high() = 0;           // remove the element at high end
                                          // return new high
  virtual int       del_low() = 0;        // delete low element, return new lo

                                          // caution: del_low/high
                                          // does not necessarily 
                                          // immediately call Type::~Type


// operations on multiple elements

  virtual void      fill(const Type  x);          // set all elements = x
  virtual void      fill(const Type  x, int from, int to); // fill from to to
  virtual void      clear() = 0;                // reset to zero-sized Plex
  virtual int       reset_low(int newlow); // change low index,return old
  virtual void      reverse();                   // reverse in-place
  virtual void      append(const MFPlex& a);    // concatenate a copy
  virtual void      prepend(const MFPlex& a);   // prepend a copy

// status

  virtual int       can_add_high() const = 0;
  virtual int       can_add_low() const = 0;
  
  int               length () const;       // number of slots

  int               empty () const;        // is the plex empty?
  virtual int       full() const = 0;      // it it full?

  int               chunk_size() const;    // report chunk size;

  virtual int       OK() const = 0;        // representation invariant

  void		    error(const char* msg) const;
  void		    index_error() const;
  void		    empty_error() const;
  void		    full_error() const;
};


// MFIChunk ops


#endif
