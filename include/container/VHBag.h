#ifndef FFGRAPH_STL_VHBAG_H
#define FFGRAPH_STL_VHBAG_H

#include "MFBag.h"
#include <unordered_set>
#include <cstddef>
#include <functional>
#include <type_traits>
#include <memory>

// Compatibility declarations for FFGraph's legacy HASH(T&) / EQ(T&,T&) hooks.
template<class T> unsigned int HASH(T&);
template<class T> int EQ(T&, T&);

namespace ffgraph_legacy_bag_detail {
template<class T> struct Hash {
  std::size_t operator()(const T& value) const {
    if constexpr (std::is_pointer_v<T> || std::is_arithmetic_v<T> || std::is_enum_v<T>) {
      // Pointer types use identity hashing; arithmetic and enum types have
      // standard hash/equality semantics and must not depend on legacy
      // compare.cc overloads that are not visible in this translation unit.
      return std::hash<T>{}(value);
    } else {
      T copy = value;
      return static_cast<std::size_t>(::HASH(copy));
    }
  }
};
template<class T> struct Equal {
  bool operator()(const T& lhs, const T& rhs) const {
    if constexpr (std::is_pointer_v<T> || std::is_arithmetic_v<T> || std::is_enum_v<T>) {
      // Pointer types compare identity; arithmetic and enum types use their
      // ordinary value equality, matching the legacy int overload.
      return lhs == rhs;
    } else {
      T a = lhs, b = rhs;
      return ::EQ(a, b) != 0;
    }
  }
};
}

#ifndef DEFAULT_INITIAL_CAPACITY
#define DEFAULT_INITIAL_CAPACITY 5
#endif

template<class Type>
class VHBag : public MFBag<Type> {
  using Store = std::unordered_multiset<Type,
      ffgraph_legacy_bag_detail::Hash<Type>,
      ffgraph_legacy_bag_detail::Equal<Type>>;
  Store items_;

  typename Store::const_iterator find_pix(Pix p) const {
    if (!p) return items_.end();
    const Type* wanted = static_cast<const Type*>(p);
    for (auto it = items_.begin(); it != items_.end(); ++it)
      if (std::addressof(*it) == wanted) return it;
    return items_.end();
  }
public:
  explicit VHBag(unsigned int sz = DEFAULT_INITIAL_CAPACITY) {
    if (sz) items_.reserve(sz);
    this->count = 0;
  }
  VHBag(const VHBag& other) : items_(other.items_) {
    this->count = static_cast<int>(items_.size());
  }
  VHBag& operator=(const VHBag& other) {
    if (this != &other) { items_ = other.items_; this->count = static_cast<int>(items_.size()); }
    return *this;
  }
  ~VHBag() override = default;

  Pix add(Type item) override {
    auto it = items_.insert(item);
    this->count = static_cast<int>(items_.size());
    return Pix(const_cast<Type*>(std::addressof(*it)));
  }
  void del(Type item) override {
    auto range = items_.equal_range(item);
    if (range.first != range.second) items_.erase(range.first);
    this->count = static_cast<int>(items_.size());
  }
  void remove(Type item) override {
    items_.erase(item);
    this->count = static_cast<int>(items_.size());
  }
  int nof(Type item) override {
    auto range = items_.equal_range(item);
    return static_cast<int>(std::distance(range.first, range.second));
  }
  int contains(Type item) override { return items_.find(item) != items_.end(); }
  void clear() override { items_.clear(); this->count = 0; }
  Pix first() override {
    return items_.empty() ? Pix(0) : Pix(const_cast<Type*>(std::addressof(*items_.begin())));
  }
  void next(Pix& p) override {
    auto it = find_pix(p);
    if (it == items_.end() || ++it == items_.end()) { p = 0; return; }
    p = Pix(const_cast<Type*>(std::addressof(*it)));
  }
  Type& operator()(Pix p) override {
    auto it = find_pix(p);
    if (it == items_.end()) { this->error("invalid or null Pix"); static Type invalid{}; return invalid; }
    return const_cast<Type&>(*it);
  }
  Pix seek(Type item, Pix from = 0) override {
    auto it = from ? find_pix(from) : items_.end();
    if (from && it == items_.end()) return 0;
    if (from) ++it;
    else it = items_.begin();
    for (; it != items_.end(); ++it) {
      Type candidate = *it;
      if (ffgraph_legacy_bag_detail::Equal<Type>{}(candidate, item)) return Pix(const_cast<Type*>(std::addressof(*it)));
    }
    return 0;
  }
  int owns(Pix p) override { return find_pix(p) != items_.end(); }
  int capacity() { return static_cast<int>(items_.bucket_count()); }
  void resize(unsigned int newsize = 0) {
    if (newsize) items_.rehash(newsize); else items_.rehash(0);
  }
  int OK() override { return this->count == static_cast<int>(items_.size()); }
};

#endif
