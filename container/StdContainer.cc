#if __GNUG__ >= 2
#  pragma implementation
#endif

#include <String.h>
#include <DLList.h>
#include <SLList.h>
#include "AVL.cc"
#include "MFPlex.cc"
#include "MFSet.cc"
#include "MFMap.cc"

#include "MPlex.cc"
#include "AVLSet.cc"
#include "AVLMap.cc"

#include "Tuple.cc"

#include "compare.cc"

//---- newclass ---------------------------------------------------------

// int - int Map

template class MFMap<int, int>;
template class AVLMap<int, int>;
template class AVLMNode<int, int>;
template void set_lthread(AVLMNode<int, int> *, int);
template void set_rthread(AVLMNode<int, int> *, int);
template void set_bf(AVLMNode<int, int> *, int);
template int rthread(AVLMNode<int, int> *);
template int lthread(AVLMNode<int, int> *);
template int bf(AVLMNode<int, int> *);

// int AVLSet

template class MFSet<int>;
template class AVLSet<int>;
template class AVLNode<int>;
template void set_lthread(AVLNode<int> *, int);
template void set_rthread(AVLNode<int> *, int);
template void set_bf(AVLNode<int> *, int);
template int rthread(AVLNode<int> *);
template int lthread(AVLNode<int> *);
template int bf(AVLNode<int> *);


// int - String Map

template class MFMap<int, String>;
template class AVLMap<int, String>;
template class AVLMNode<int, String>;
template void set_lthread(AVLMNode<int, String> *, int);
template void set_rthread(AVLMNode<int, String> *, int);
template void set_bf(AVLMNode<int, String> *, int);
template int rthread(AVLMNode<int, String> *);
template int lthread(AVLMNode<int, String> *);
template int bf(AVLMNode<int, String> *);

// String - Pix Map

template class MFMap<String, Pix>;
template class AVLMap<String, Pix>;
template class AVLMNode<String, Pix>;
template void set_lthread(AVLMNode<String, Pix> *, int);
template void set_rthread(AVLMNode<String, Pix> *, int);
template void set_bf(AVLMNode<String, Pix> *, int);
template int rthread(AVLMNode<String, Pix> *);
template int lthread(AVLMNode<String, Pix> *);
template int bf(AVLMNode<String, Pix> *);

// String - AVLSet<int> Map

template class MFMap<String, AVLSet<int> >;
template class AVLMap<String, AVLSet<int> >;
template class AVLMNode<String, AVLSet<int> >;
template void set_lthread(AVLMNode<String, AVLSet<int> > *, int);
template void set_rthread(AVLMNode<String, AVLSet<int> > *, int);
template void set_bf(AVLMNode<String, AVLSet<int> > *, int);
template int rthread(AVLMNode<String, AVLSet<int> > *);
template int lthread(AVLMNode<String, AVLSet<int> > *);
template int bf(AVLMNode<String, AVLSet<int> > *);

// String - String Map

template class MFMap<String, String>;
template class AVLMap<String, String>;
template class AVLMNode<String, String>;
template void set_lthread(AVLMNode<String, String> *, int);
template void set_rthread(AVLMNode<String, String> *, int);
template void set_bf(AVLMNode<String, String> *, int);
template int rthread(AVLMNode<String, String> *);
template int lthread(AVLMNode<String, String> *);
template int bf(AVLMNode<String, String> *);

// String -  int Map

template class MFMap<String, int>;
template class AVLMap<String, int>;
template class AVLMNode<String, int>;
template void set_lthread(AVLMNode<String, int> *, int);
template void set_rthread(AVLMNode<String, int> *, int);
template void set_bf(AVLMNode<String, int> *, int);
template int rthread(AVLMNode<String, int> *);
template int lthread(AVLMNode<String, int> *);
template int bf(AVLMNode<String, int> *);

// String - AVLSet<String> Map

template class MFMap<String, AVLSet<String> >;
template class AVLMap<String, AVLSet<String> >;
template class AVLMNode<String, AVLSet<String> >;
template void set_lthread(AVLMNode<String, AVLSet<String> > *, int);
template void set_rthread(AVLMNode<String, AVLSet<String> > *, int);
template void set_bf(AVLMNode<String, AVLSet<String> > *, int);
template int rthread(AVLMNode<String, AVLSet<String> > *);
template int lthread(AVLMNode<String, AVLSet<String> > *);
template int bf(AVLMNode<String, AVLSet<String> > *);


// String Set

template class MFSet<String>;
template class AVLSet<String>;
template class AVLNode<String>;
template void set_lthread(AVLNode<String> *, int);
template void set_rthread(AVLNode<String> *, int);
template void set_bf(AVLNode<String> *, int);
template int rthread(AVLNode<String> *);
template int lthread(AVLNode<String> *);
template int bf(AVLNode<String> *);

// String DLList

template class Tuple<int, int>;

template class Triple<String, short, String>;
template class Tuple<String, String>;
// String -> short MAP (fuer SibParams)
template class MFMap<String, short >;
template class AVLMap<String, short>;
template class AVLMNode<String, short >;
template void set_lthread(AVLMNode<String, short > *, int);
template void set_rthread(AVLMNode<String, short > *, int);
template void set_bf(AVLMNode<String, short > *, int);
template int rthread(AVLMNode<String, short > *);
template int lthread(AVLMNode<String, short > *);
template int bf(AVLMNode<String, short > *);
 
// String -> long MAP (fuer SibParams)
template class MFMap<String, long >;
template class AVLMap<String, long>;
template class AVLMNode<String, long >;
template void set_lthread(AVLMNode<String, long > *, int);
template void set_rthread(AVLMNode<String, long > *, int);
template void set_bf(AVLMNode<String, long > *, int);
template int rthread(AVLMNode<String, long > *);
template int lthread(AVLMNode<String, long > *);
template int bf(AVLMNode<String, long > *);

