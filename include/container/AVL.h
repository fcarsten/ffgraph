#ifndef _AVL_h_
#define _AVL_h_

#if __GNUG__ >= 2
#  pragma interface
#endif

#define AVLBALANCEMASK    3
#define AVLBALANCED       0
#define AVLLEFTHEAVY      1
#define AVLRIGHTHEAVY     2

#define LTHREADBIT        4
#define RTHREADBIT        8

template<class T> int bf(T*);
template<class T> void set_bf(T*, int);
template<class T> int rthread(T*);
template<class T> void set_rthread(T*, int);
template<class T> int lthread(T*);
template<class T> void set_lthread(T*, int);

#endif
