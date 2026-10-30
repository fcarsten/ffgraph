#if __GNUG__ >= 2
#  pragma implementation
#endif

#include <ffgraph/FFPoint.h>
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFKaese.h>

#include "AVL.cc"
#include "MFMap.cc"
#include "AVLMap.cc"
#include "compare.cc"

//---- newclass ---------------------------------------------------------

class FFNodeLabel;
class FFEdgeLabel;
class FFGraphLabel;

class FFGraphicEdge;
class FFGraphicNode;

class FFEdgeClass;
class FFNodeClass;

// FFNode - Cuts AVLMap

template class MFMap<FFNode, Cuts *>;
template class AVLMap<FFNode, Cuts *>;
template class AVLMNode<FFNode, Cuts *>;
template void set_lthread(AVLMNode<FFNode, Cuts *> *, int);
template void set_rthread(AVLMNode<FFNode, Cuts *> *, int);
template void set_bf(AVLMNode<FFNode, Cuts *> *, int);
template int rthread(AVLMNode<FFNode, Cuts *> *);
template int lthread(AVLMNode<FFNode, Cuts *> *);
template int bf(AVLMNode<FFNode, Cuts *> *);

// String - FFGraphicEdge AVLMap
template class MFMap<String, FFGraphicEdge *>;
template class AVLMap<String, FFGraphicEdge *>;
template class AVLMNode<String, FFGraphicEdge *>;
template void set_lthread(AVLMNode<String, FFGraphicEdge *> *, int);
template void set_rthread(AVLMNode<String, FFGraphicEdge *> *, int);
template void set_bf(AVLMNode<String, FFGraphicEdge *> *, int);
template int rthread(AVLMNode<String, FFGraphicEdge *> *);
template int lthread(AVLMNode<String, FFGraphicEdge *> *);
template int bf(AVLMNode<String, FFGraphicEdge *> *);

// String - FFGraphicNode Map

template class MFMap<String, FFGraphicNode *>;
template class AVLMap<String, FFGraphicNode *>;
template class AVLMNode<String, FFGraphicNode *>;
template void set_lthread(AVLMNode<String, FFGraphicNode *> *, int);
template void set_rthread(AVLMNode<String, FFGraphicNode *> *, int);
template void set_bf(AVLMNode<String, FFGraphicNode *> *, int);
template int rthread(AVLMNode<String, FFGraphicNode *> *);
template int lthread(AVLMNode<String, FFGraphicNode *> *);
template int bf(AVLMNode<String, FFGraphicNode *> *);

// FFNodeClass * - FFNodeClass * Map

template class MFMap<FFNodeClass *, FFNodeClass *>;
template class AVLMap<FFNodeClass *, FFNodeClass *>;
template class AVLMNode<FFNodeClass *, FFNodeClass *>;
template void set_lthread(AVLMNode<FFNodeClass *, FFNodeClass *> *, int);
template void set_rthread(AVLMNode<FFNodeClass *, FFNodeClass *> *, int);
template void set_bf(AVLMNode<FFNodeClass *, FFNodeClass *> *, int);
template int rthread(AVLMNode<FFNodeClass *, FFNodeClass *> *);
template int lthread(AVLMNode<FFNodeClass *, FFNodeClass *> *);
template int bf(AVLMNode<FFNodeClass *, FFNodeClass *> *);

// FFNodeClass * - String Map

template class MFMap<FFNodeClass *, String >;
template class AVLMap<FFNodeClass *, String >;
template class AVLMNode<FFNodeClass *, String >;
template void set_lthread(AVLMNode<FFNodeClass *, String > *, int);
template void set_rthread(AVLMNode<FFNodeClass *, String > *, int);
template void set_bf(AVLMNode<FFNodeClass *, String > *, int);
template int rthread(AVLMNode<FFNodeClass *, String > *);
template int lthread(AVLMNode<FFNodeClass *, String > *);
template int bf(AVLMNode<FFNodeClass *, String > *);

// int - FFNodeClass * Map

template class MFMap<int, FFNodeClass *>;
template class AVLMap<int, FFNodeClass *>;
template class AVLMNode<int, FFNodeClass *>;
template void set_lthread(AVLMNode<int, FFNodeClass *> *, int);
template void set_rthread(AVLMNode<int, FFNodeClass *> *, int);
template void set_bf(AVLMNode<int, FFNodeClass *> *, int);
template int rthread(AVLMNode<int, FFNodeClass *> *);
template int lthread(AVLMNode<int, FFNodeClass *> *);
template int bf(AVLMNode<int, FFNodeClass *> *);

// int - FFNodeClass * Map

template class MFMap<FFNodeClass *, int >;
template class AVLMap<FFNodeClass *, int >;
template class AVLMNode<FFNodeClass *, int >;
template void set_lthread(AVLMNode<FFNodeClass *, int > *, int);
template void set_rthread(AVLMNode<FFNodeClass *, int > *, int);
template void set_bf(AVLMNode<FFNodeClass *, int > *, int);
template int rthread(AVLMNode<FFNodeClass *, int > *);
template int lthread(AVLMNode<FFNodeClass *, int > *);
template int bf(AVLMNode<FFNodeClass *, int > *);

// int - FFEdgeClass * Map

template class MFMap<FFEdgeClass *, int >;
template class AVLMap<FFEdgeClass *, int >;
template class AVLMNode<FFEdgeClass *, int >;
template void set_lthread(AVLMNode<FFEdgeClass *, int > *, int);
template void set_rthread(AVLMNode<FFEdgeClass *, int > *, int);
template void set_bf(AVLMNode<FFEdgeClass *, int > *, int);
template int rthread(AVLMNode<FFEdgeClass *, int > *);
template int lthread(AVLMNode<FFEdgeClass *, int > *);
template int bf(AVLMNode<FFEdgeClass *, int > *);


// int - FFSet<FFNodeClass *> Map

// template class MFMap<int, FFSet<FFNodeClass *>  >;
// template class AVLMap<int, FFSet<FFNodeClass *>  >;
// template class AVLMNode<int, FFSet<FFNodeClass *>  >;
// template void set_lthread(AVLMNode<int, FFSet<FFNodeClass *>  > *, int);
// template void set_rthread(AVLMNode<int, FFSet<FFNodeClass *>  > *, int);
// template void set_bf(AVLMNode<int, FFSet<FFNodeClass *>  > *, int);
// template int rthread(AVLMNode<int, FFSet<FFNodeClass *>  > *);
// template int lthread(AVLMNode<int, FFSet<FFNodeClass *>  > *);
// template int bf(AVLMNode<int, FFSet<FFNodeClass *>  > *);

// String - FFodeClass * Map

template class MFMap<String, FFNodeClass *>;
template class AVLMap<String, FFNodeClass *>;
template class AVLMNode<String, FFNodeClass *>;
template void set_lthread(AVLMNode<String, FFNodeClass *> *, int);
template void set_rthread(AVLMNode<String, FFNodeClass *> *, int);
template void set_bf(AVLMNode<String, FFNodeClass *> *, int);
template int rthread(AVLMNode<String, FFNodeClass *> *);
template int lthread(AVLMNode<String, FFNodeClass *> *);
template int bf(AVLMNode<String, FFNodeClass *> *);

// long - FFNodeClass AVLMap
template class MFMap<long, FFNodeClass *>;
template class AVLMap<long, FFNodeClass *>;
template class AVLMNode<long, FFNodeClass *>;
template void set_lthread(AVLMNode<long, FFNodeClass *> *, int);
template void set_rthread(AVLMNode<long, FFNodeClass *> *, int);
template void set_bf(AVLMNode<long, FFNodeClass *> *, int);
template int rthread(AVLMNode<long, FFNodeClass *> *);
template int lthread(AVLMNode<long, FFNodeClass *> *);
template int bf(AVLMNode<long, FFNodeClass *> *);

