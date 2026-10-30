#if __GNUG__ >= 2
#  pragma implementation
#endif

#include "MFStack.cc"
#include "SLStack.h" 

#include "SLList.h"
#include "compare.cc"

//---- newclass ---------------------------------------------------------

class FFNodeClass;

// Node Stack

template class SLStack<FFNodeClass *>;
template class MFStack<FFNodeClass *>;
template class SLList<FFNodeClass *>;
