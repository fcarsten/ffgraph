#ifndef FFGRAPH_STL_VOHSET_H
#define FFGRAPH_STL_VOHSET_H

#include "MFSet.h"
#include <unordered_set>
#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>
#include <vector>

// Adapters for the old libg++ hash/equality hooks. The project supplies
// HASH(T&) and EQ(T&,T&) overloads in compare.cc for its supported types.
template<class T> unsigned int HASH(T&);
template<class T> int EQ(T&, T&);
namespace ffgraph_legacy_hash_detail {
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

template<class T>
class VOHSet : public MFSet<T> {
  using Store = std::unordered_set<T, ffgraph_legacy_hash_detail::Hash<T>,
                                      ffgraph_legacy_hash_detail::Equal<T>>;
  Store items_;

  typename Store::const_iterator find_pix(Pix p) const {
    if (!p) return items_.end();
    const T* value = static_cast<const T*>(p);
    auto it = items_.find(*value);
    return it != items_.end() && std::addressof(*it) == value ? it : items_.end();
  }
public:
  VOHSet() : VOHSet(5) {}
  explicit VOHSet(int sz) { if (sz > 0) items_.reserve(static_cast<std::size_t>(sz)); this->count = 0; }
  VOHSet(const VOHSet& other) : items_(other.items_) { this->count = static_cast<int>(items_.size()); }
  VOHSet& operator=(const VOHSet& other) {
    if (this != &other) { items_ = other.items_; this->count = static_cast<int>(items_.size()); }
    return *this;
  }
  ~VOHSet() override = default;

  Pix add(T item) override {
    auto result = items_.insert(item);
    this->count = static_cast<int>(items_.size());
    return Pix(const_cast<T*>(std::addressof(*result.first)));
  }
  void del(T item) override { items_.erase(item); this->count = static_cast<int>(items_.size()); }
  int contains(T item) const override { return items_.find(item) != items_.end(); }
  void clear() override { items_.clear(); this->count = 0; }
  Pix first() const override {
    return items_.empty() ? Pix(0) : Pix(const_cast<T*>(std::addressof(*items_.begin())));
  }
  void next(Pix& p) const override {
    auto it = find_pix(p);
    if (it == items_.end() || ++it == items_.end()) { p = 0; return; }
    p = Pix(const_cast<T*>(std::addressof(*it)));
  }
  Pix seek(T item) const override {
    auto it = items_.find(item);
    return it == items_.end() ? Pix(0) : Pix(const_cast<T*>(std::addressof(*it)));
  }
  T& operator()(Pix p) const override {
    if (!p) this->error("null Pix");
    return *static_cast<T*>(p);
  }
  int owns(Pix p) override { return find_pix(p) != items_.end(); }

  void resize(int newsize = 0) {
    if (newsize > 0) items_.rehash(static_cast<std::size_t>(newsize));
    else items_.rehash(0);
  }
  int capacity() { return static_cast<int>(items_.bucket_count()); }
  int OK() override { return this->count == static_cast<int>(items_.size()); }

  void operator|=(const VOHSet& b) { for (const auto& x : b.items_) add(x); }
  void operator-=(const VOHSet& b) {
    for (auto it = items_.begin(); it != items_.end();) {
      if (b.contains(*it)) it = items_.erase(it); else ++it;
    }
    this->count = static_cast<int>(items_.size());
  }
  void operator&=(const VOHSet& b) {
    for (auto it = items_.begin(); it != items_.end();) {
      if (!b.contains(*it)) it = items_.erase(it); else ++it;
    }
    this->count = static_cast<int>(items_.size());
  }
  int operator==(const VOHSet& b) const {
    if (items_.size() != b.items_.size()) return 0;
    for (const auto& x : items_) if (!b.contains(x)) return 0;
    return 1;
  }
  int operator!=(const VOHSet& b) const { return !(*this == b); }
  int operator<=(const VOHSet& b) const {
    if (items_.size() > b.items_.size()) return 0;
    for (const auto& x : items_) if (!b.contains(x)) return 0;
    return 1;
  }
};

#endif
