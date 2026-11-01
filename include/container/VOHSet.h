#ifndef FFGRAPH_STL_VOHSET_H
#define FFGRAPH_STL_VOHSET_H

#include "MFSet.h"
#include <unordered_set>
#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>
#include <vector>
#include <memory>
#include <stdexcept>

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
  // A Pix is the address of a node's key. A second index lets us validate
  // membership without dereferencing a stale address, in expected O(1) time.
  std::unordered_set<const T*> live_pix_;

  void rebuild_live_pix() {
    std::unordered_set<const T*> addresses;
    addresses.reserve(items_.size());
    for (const auto& item : items_) addresses.insert(std::addressof(item));
    live_pix_.swap(addresses);
  }

  typename Store::const_iterator find_pix(Pix p) const {
    if (!p) return items_.end();
    const T* wanted = static_cast<const T*>(p);
    if (live_pix_.find(wanted) == live_pix_.end()) return items_.end();
    // Address membership is established; this value is still allocated.
    auto it = items_.find(*wanted);
    return it != items_.end() && std::addressof(*it) == wanted ? it : items_.end();
  }
public:
  VOHSet() : VOHSet(5) {}
  explicit VOHSet(int sz) {
    if (sz > 0) {
      items_.reserve(static_cast<std::size_t>(sz));
      live_pix_.reserve(static_cast<std::size_t>(sz));
    }
    this->count = 0;
  }
  VOHSet(const VOHSet& other) : items_(other.items_) {
    rebuild_live_pix();
    this->count = static_cast<int>(items_.size());
  }
  VOHSet& operator=(const VOHSet& other) {
    if (this != &other) {
      VOHSet copy(other);
      items_.swap(copy.items_);
      live_pix_.swap(copy.live_pix_);
      this->count = static_cast<int>(items_.size());
    }
    return *this;
  }
  ~VOHSet() override = default;

  Pix add(T item) override {
    auto result = items_.insert(item);
    if (result.second) {
      try { live_pix_.insert(std::addressof(*result.first)); }
      catch (...) { items_.erase(result.first); throw; }
    }
    this->count = static_cast<int>(items_.size());
    return Pix(const_cast<T*>(std::addressof(*result.first)));
  }
  void del(T item) override {
    auto it = items_.find(item);
    if (it != items_.end()) {
      live_pix_.erase(std::addressof(*it));
      items_.erase(it);
    }
    this->count = static_cast<int>(items_.size());
  }
  int contains(T item) const override { return items_.find(item) != items_.end(); }
  void clear() override { items_.clear(); live_pix_.clear(); this->count = 0; }
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
  // Legacy API returns a mutable reference to an element used as a hash key.
  // Do NOT modify that value in place: std::unordered_* requires stable keys.
  // To change it, erase the old entry and insert a new one (preserving mapped
  // values as needed). Read-only key access and mapped-value edits are safe.
  T& operator()(Pix p) const override {
    auto it = find_pix(p);
    if (it == items_.end()) throw std::out_of_range("VOHSet: invalid or null Pix");
    return const_cast<T&>(*it);
  }
  int owns(Pix p) override { return find_pix(p) != items_.end(); }

  void resize(int newsize = 0) {
    if (newsize > 0) items_.rehash(static_cast<std::size_t>(newsize));
    else items_.rehash(0);
  }
  int capacity() { return static_cast<int>(items_.bucket_count()); }
  int OK() override {
    if (this->count != static_cast<int>(items_.size()) ||
        live_pix_.size() != items_.size()) return 0;
    for (const auto& item : items_)
      if (!live_pix_.count(std::addressof(item))) return 0;
    return 1;
  }

  void operator|=(const VOHSet& b) {
    if (this == &b) return;
    for (const auto& x : b.items_) add(x);
  }
  void operator-=(const VOHSet& b) {
    if (this == &b) { clear(); return; }
    for (auto it = items_.begin(); it != items_.end();) {
      if (b.contains(*it)) {
        live_pix_.erase(std::addressof(*it));
        it = items_.erase(it);
      } else ++it;
    }
    this->count = static_cast<int>(items_.size());
  }
  void operator&=(const VOHSet& b) {
    if (this == &b) return;
    for (auto it = items_.begin(); it != items_.end();) {
      if (!b.contains(*it)) {
        live_pix_.erase(std::addressof(*it));
        it = items_.erase(it);
      } else ++it;
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
