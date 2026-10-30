#if __GNUG__ >= 2
#  pragma implementation
#endif

#include <ffgraph/FFPoint.h>
#include <ffgraph/FFGraph.h>

#include "MFPlex.cc"
#include "MPlex.cc"
#include "compare.cc"

//---- newclass ---------------------------------------------------------

class FFNodeLabel;
class FFEdgeLabel;
class FFGraphLabel;

class FFGraphicEdge;
class FFGraphicNode;

class FFEdgeClass;
class FFNodeClass;


// FFGraphLabel MPlex

template class MPlex<FFGraphLabel *>;
template class MFPlex<FFGraphLabel *>;
template class MFIChunk<FFGraphLabel *>;

// FFEdgeLabel MPlex

template class MFPlex<FFEdgeLabel *>;
template class MPlex<FFEdgeLabel *>;
template class MFIChunk<FFEdgeLabel *>;

// FFNodeLabel MPlex

template class MPlex<FFNodeLabel *>;
template class MFPlex<FFNodeLabel *>;
template class MFIChunk<FFNodeLabel *>;

// FFPoint MPlex

template class MPlex<FFPoint>;
template class MFPlex<FFPoint>;
template class MFIChunk<FFPoint>;

