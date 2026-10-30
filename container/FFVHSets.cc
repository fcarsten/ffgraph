#if __GNUG__ >= 2
#  pragma implementation
#endif

#include<stream.h>
#include <globdefs.h>

#ifdef USE_VHSET

#include <String.h>
#include "compare.cc"
#include <VHSet.cc>
#include <MFSet.cc>

template class VHSet<FFEdgeClass *>;
template class VHSet<FFEdgeLabel *>;
template class VHSet<FFGraphLabel *>;
template class VHSet<FFNodeLabel *>;
template class VHSet<FFLayoutLabel *>;
template class VHSet<FFNodeClass *>;
template class VHSet<FFGraphicEdge *>;
template class VHSet<FFGraphicNode *>;
template class VHSet<String>;
template class VHSet<int>;
#endif
