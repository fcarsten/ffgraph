#if __GNUG__ >= 2
#  pragma implementation
#endif

#include<stream.h>
#include <globdefs.h>

#ifdef USE_VOHSET

#include <VOHSet.cc>
#include <MFSet.cc>
#include <String.h>

#include "compare.cc"

template class VOHSet<FFEdgeClass *>;
template class VOHSet<FFEdgeLabel *>;
template class VOHSet<FFGraphLabel *>;
template class VOHSet<FFNodeLabel *>;
template class VOHSet<FFLayoutLabel *>;
template class VOHSet<FFNodeClass *>;
template class VOHSet<FFGraphicEdge *>;
template class VOHSet<FFGraphicNode *>;
// template class VOHSet<String>;
template class VOHSet<int>;
#endif
