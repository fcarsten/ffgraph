#ifndef FFGRAPH_STL_VHSET_H
#define FFGRAPH_STL_VHSET_H

#include "VOHSet.h"
#ifndef DEFAULT_INITIAL_CAPACITY
#define DEFAULT_INITIAL_CAPACITY 5
#endif

// The old VHSet and VOHSet were two variants of libg++ open-addressed hash
// sets. Both now share the std::unordered_set-backed compatibility adapter.
template<class T>
class VHSet : public VOHSet<T> {
public:
  explicit VHSet(unsigned int sz = DEFAULT_INITIAL_CAPACITY)
      : VOHSet<T>(static_cast<int>(sz)) {}
  VHSet(const VHSet& other) : VOHSet<T>(other) {}
  VHSet& operator=(const VHSet& other) {
    VOHSet<T>::operator=(other);
    return *this;
  }
};

#endif
