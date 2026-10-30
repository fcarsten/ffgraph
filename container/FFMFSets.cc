#if __GNUG__ >= 2
#  pragma implementation
#endif

#include <globdefs.h>

#include <ffgraph/FFPoint.h>
#include <ffgraph/FFGraph.h>

#include "MFSet.cc"

#include "compare.cc"

//---- newclass ---------------------------------------------------------

class FFNodeLabel;
class FFEdgeLabel;
class FFGraphLabel;

class FFGraphicEdge;
class FFGraphicNode;

class FFEdgeClass;
class FFNodeClass;

template class MFSet<FFEdgeClass *>;
template class MFSet<FFNodeClass *>;
template class MFSet<FFNodeLabel *>;
template class MFSet<FFEdgeLabel *>;
template class MFSet<FFGraphLabel *>;
template class MFSet<FFLayoutLabel *>;
template class MFSet<FFGraphicEdge *>;
template class MFSet<FFGraphicNode *>;

