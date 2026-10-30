#ifndef FFGRAPH_STL_VHMAP_H
#define FFGRAPH_STL_VHMAP_H

#include "MFMap.h"
#include <unordered_map>
#include <cstddef>
#include <functional>
#include <type_traits>
#include <memory>

// Compatibility declarations for FFGraph's legacy HASH(T&) / EQ(T&,T&) hooks.
template<class T> unsigned int HASH(T&);
template<class T> int EQ(T&, T&);

namespace ffgraph_legacy_map_detail {
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

template<class Type1, class Type2>
class VHMap : public MFMap<Type1, Type2> {
  using Store = std::unordered_map<Type1, Type2,
      ffgraph_legacy_map_detail::Hash<Type1>,
      ffgraph_legacy_map_detail::Equal<Type1>>;
  Store items_;

  typename Store::const_iterator find_pix(Pix p) const {
    if (!p) return items_.end();
    const Type1* wanted = static_cast<const Type1*>(p);
    for (auto it = items_.begin(); it != items_.end(); ++it)
      if (std::addressof(it->first) == wanted) return it;
    return items_.end();
  }
public:
  explicit VHMap(Type2 dflt, unsigned int sz = DEFAULT_INITIAL_CAPACITY)
      : MFMap<Type1, Type2>(dflt) {
    if (sz) items_.reserve(sz);
  }
  VHMap(const VHMap& other) : MFMap<Type1, Type2>(other.def), items_(other.items_) {
    this->count = static_cast<unsigned int>(items_.size());
  }
  VHMap& operator=(const VHMap& other) {
    if (this != &other) {
      items_ = other.items_;
      this->def = other.def;
      this->count = static_cast<unsigned int>(items_.size());
    }
    return *this;
  }
  ~VHMap() override = default;

  Type2& operator[](Type1 key) override {
    auto result = items_.try_emplace(key, this->def);
    this->count = static_cast<unsigned int>(items_.size());
    return result.first->second;
  }
  void del(Type1 key) override {
    items_.erase(key);
    this->count = static_cast<unsigned int>(items_.size());
  }
  Pix first() const override {
    return items_.empty() ? Pix(0) : Pix(const_cast<Type1*>(std::addressof(items_.begin()->first)));
  }
  void next(Pix& p) const override {
    auto it = find_pix(p);
    if (it == items_.end() || ++it == items_.end()) { p = 0; return; }
    p = Pix(const_cast<Type1*>(std::addressof(it->first)));
  }
  Type1& key(Pix p) const override {
    auto it = find_pix(p);
    if (it == items_.end()) { this->error("invalid or null Pix"); static Type1 invalid{}; return invalid; }
    // Legacy API exposes mutable keys; changing a key directly is unsafe for any hash map.
    return const_cast<Type1&>(it->first);
  }
  Type2& contents(Pix p) const override {
    auto it = find_pix(p);
    if (it == items_.end()) { this->error("invalid or null Pix"); return const_cast<Type2&>(this->def); }
    return const_cast<Type2&>(it->second);
  }
  Pix seek(Type1 key) override {
    auto it = items_.find(key);
    return it == items_.end() ? Pix(0) : Pix(const_cast<Type1*>(std::addressof(it->first)));
  }
  int contains(Type1 key) override { return items_.find(key) != items_.end(); }
  int owns(Pix p) override { return find_pix(p) != items_.end(); }
  void clear() override { items_.clear(); this->count = 0; }
  void resize(unsigned int newsize = 0) {
    if (newsize) items_.rehash(newsize); else items_.rehash(0);
  }
  int OK() override { return this->count == items_.size(); }
};

#endif
