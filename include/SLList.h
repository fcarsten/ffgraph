#ifndef FFGRAPH_COMPAT_SLLIST_H
#define FFGRAPH_COMPAT_SLLIST_H
#include "DLList.h"
// The old SLList and DLList had the same operations at FFGraph call sites.
// Keep a distinct type name while using the same safe STL-backed adapter.
template<class T>
class SLList : public DLList<T> {
public:
  SLList() = default;
  SLList(const SLList& other) : DLList<T>(other) {}
  SLList(SLList&&) noexcept = default;
  SLList& operator=(const SLList& other) { DLList<T>::operator=(other); return *this; }
  SLList& operator=(SLList&&) noexcept = default;
};
#endif
