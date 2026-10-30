#ifndef FFGRAPH_COMPAT_PIX_H
#define FFGRAPH_COMPAT_PIX_H
// Compatibility token used by FFGraph's pre-STL container APIs.
// Container implementations return opaque element handles; nullptr means end.
using Pix = void*;
#endif
