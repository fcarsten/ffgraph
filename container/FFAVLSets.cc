#if __GNUG__ >= 2
#  pragma implementation
#endif

#include <globdefs.h>

#ifdef USE_AVLSET

#include <ffgraph/FFPoint.h>
#include <ffgraph/FFGraph.h>


#include "AVL.cc"
#include "AVLSet.cc"

#include "compare.cc"

//---- newclass ---------------------------------------------------------

class FFNodeLabel;
class FFEdgeLabel;
class FFGraphLabel;

class FFGraphicEdge;
class FFGraphicNode;

class FFEdgeClass;
class FFNodeClass;

// FFNodeClass AVLSet

template class AVLSet<FFNodeClass *>;
template class AVLNode<FFNodeClass *>;
template void set_bf(AVLNode<FFNodeClass *> *, int);
template void set_lthread(AVLNode<FFNodeClass *> *, int);
template void set_rthread(AVLNode<FFNodeClass *> *, int);
template int bf(AVLNode<FFNodeClass *> *);
template int lthread(AVLNode<FFNodeClass *> *);
template int rthread(AVLNode<FFNodeClass *> *);


// FFEgdeClass AVLSet

template class AVLSet<FFEdgeClass *>;
template class AVLNode<FFEdgeClass *>;
template void set_bf(AVLNode<FFEdgeClass *> *, int);
template void set_lthread(AVLNode<FFEdgeClass *> *, int);
template void set_rthread(AVLNode<FFEdgeClass *> *, int);
template int bf(AVLNode<FFEdgeClass *> *);
template int lthread(AVLNode<FFEdgeClass *> *);
template int rthread(AVLNode<FFEdgeClass *> *);

// FFNodeLabel AVLSet

template class AVLSet<FFNodeLabel *>;
template class AVLNode<FFNodeLabel *>;
template void set_bf(AVLNode<FFNodeLabel *> *, int);
template void set_lthread(AVLNode<FFNodeLabel *> *, int);
template void set_rthread(AVLNode<FFNodeLabel *> *, int);
template int bf(AVLNode<FFNodeLabel *> *);
template int lthread(AVLNode<FFNodeLabel *> *);
template int rthread(AVLNode<FFNodeLabel *> *);

// FFEdgeLabel AVLSet

template class AVLSet<FFEdgeLabel *>;
template class AVLNode<FFEdgeLabel *>;
template void set_bf(AVLNode<FFEdgeLabel *> *, int);
template void set_lthread(AVLNode<FFEdgeLabel *> *, int);
template void set_rthread(AVLNode<FFEdgeLabel *> *, int);
template int bf(AVLNode<FFEdgeLabel *> *);
template int lthread(AVLNode<FFEdgeLabel *> *);
template int rthread(AVLNode<FFEdgeLabel *> *);

// FFGraphLabel AVLSet

template class AVLSet<FFGraphLabel *>;
template class AVLNode<FFGraphLabel *>;
template void set_bf(AVLNode<FFGraphLabel *> *, int);
template void set_lthread(AVLNode<FFGraphLabel *> *, int);
template void set_rthread(AVLNode<FFGraphLabel *> *, int);
template int bf(AVLNode<FFGraphLabel *> *);
template int lthread(AVLNode<FFGraphLabel *> *);
template int rthread(AVLNode<FFGraphLabel *> *);

// FFLayoutLabel AVLSet

template class AVLSet<FFLayoutLabel *>;
template class AVLNode<FFLayoutLabel *>;
template void set_bf(AVLNode<FFLayoutLabel *> *, int);
template void set_lthread(AVLNode<FFLayoutLabel *> *, int);
template void set_rthread(AVLNode<FFLayoutLabel *> *, int);
template int bf(AVLNode<FFLayoutLabel *> *);
template int lthread(AVLNode<FFLayoutLabel *> *);
template int rthread(AVLNode<FFLayoutLabel *> *);

// FFGraphicEdge AVLSet

template class AVLSet<FFGraphicEdge *>;
template class AVLNode<FFGraphicEdge *>;
template void set_lthread(AVLNode<FFGraphicEdge *> *, int);
template void set_rthread(AVLNode<FFGraphicEdge *> *, int);
template void set_bf(AVLNode<FFGraphicEdge *> *, int);
template int rthread(AVLNode<FFGraphicEdge *> *);
template int lthread(AVLNode<FFGraphicEdge *> *);
template int bf(AVLNode<FFGraphicEdge *> *);

// FFGraphicNode Set

template class AVLSet<FFGraphicNode *>;
template class AVLNode<FFGraphicNode *>;
template void set_lthread(AVLNode<FFGraphicNode *> *, int);
template void set_rthread(AVLNode<FFGraphicNode *> *, int);
template void set_bf(AVLNode<FFGraphicNode *> *, int);
template int rthread(AVLNode<FFGraphicNode *> *);
template int lthread(AVLNode<FFGraphicNode *> *);
template int bf(AVLNode<FFGraphicNode *> *);


#endif
