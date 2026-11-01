#ifndef FFGRAPH_STL_MPLEX_H
#define FFGRAPH_STL_MPLEX_H

#include "MFPlex.h"
#include <map>
#include <vector>
#include <stdexcept>
#include <utility>
#include <limits>
#include <cstdint>
#include <algorithm>

// Compatibility implementation of the old GNU libg++ MPlex API.
// Slots retain their integer indices and can be logically deleted/undeleted;
// std::map keeps slot addresses stable, which is important because Pix is an
// opaque pointer to a slot's value in the historical interface.
template<class Type>
class MPlex : public MFPlex<Type>
{
  struct Slot {
    Type value{};
    bool present = false;
    Slot() = default;
    Slot(const Type& v, bool p) : value(v), present(p) {}
  };

  std::map<int, Slot> slots_;
  int unused_ = 0;

  typename std::map<int, Slot>::iterator slot_for_pix(Pix p) {
    if (!p) return slots_.end();
    for (auto it = slots_.begin(); it != slots_.end(); ++it)
      if (&it->second.value == static_cast<Type*>(p)) return it;
    return slots_.end();
  }
  typename std::map<int, Slot>::const_iterator slot_for_pix(Pix p) const {
    if (!p) return slots_.end();
    for (auto it = slots_.cbegin(); it != slots_.cend(); ++it)
      if (&it->second.value == static_cast<const Type*>(p)) return it;
    return slots_.cend();
  }
  typename std::map<int, Slot>::iterator next_present(int idx) {
    auto it = slots_.upper_bound(idx);
    while (it != slots_.end() && !it->second.present) ++it;
    return it;
  }
  typename std::map<int, Slot>::const_iterator next_present(int idx) const {
    auto it = slots_.upper_bound(idx);
    while (it != slots_.end() && !it->second.present) ++it;
    return it;
  }
  typename std::map<int, Slot>::iterator prev_present(int idx) {
    auto it = slots_.lower_bound(idx);
    while (it != slots_.begin()) {
      --it;
      if (it->second.present) return it;
    }
    return slots_.end();
  }
  typename std::map<int, Slot>::const_iterator prev_present(int idx) const {
    auto it = slots_.lower_bound(idx);
    while (it != slots_.begin()) {
      --it;
      if (it->second.present) return it;
    }
    return slots_.cend();
  }
  void require_slot(int idx) const {
    auto it = slots_.find(idx);
    if (it == slots_.end() || !it->second.present)
      throw std::out_of_range("MPlex: invalid or deleted index");
  }
  void init_empty(int low, int chunk_size) {
    this->lo = this->fnc = low;
    this->csize = chunk_size > 0 ? chunk_size : PLEX_DEFAULT_INITIAL_CAPACITY;
    unused_ = 0;
    slots_.clear();
  }

public:
  MPlex() { init_empty(0, PLEX_DEFAULT_INITIAL_CAPACITY); }
  explicit MPlex(int chunk_size) { init_empty(0, chunk_size); }
  MPlex(int low, int chunk_size) { init_empty(low, chunk_size); }
  MPlex(int low, int high, const Type initval, int chunk_size = 0) {
    if (high < low) { init_empty(low, chunk_size); return; }
    // The fence is one past the final slot, and all indices use signed int.
    const std::int64_t span = static_cast<std::int64_t>(high) - low + 1;
    if (high == std::numeric_limits<int>::max() ||
        span > std::numeric_limits<int>::max())
      throw std::out_of_range("MPlex: index range exceeds int bounds");
    init_empty(low, chunk_size > 0 ? chunk_size : static_cast<int>(span));
    for (int i = low; i <= high; ++i) {
      slots_.emplace(i, Slot(initval, true));
      this->fnc = i + 1;
    }
  }
  MPlex(const MPlex& other)
      : MFPlex<Type>(), slots_(other.slots_), unused_(other.unused_) {
    this->lo = other.lo;
    this->fnc = other.fnc;
    this->csize = other.csize;
  }
  void operator=(const MPlex& other) {
    if (this == &other) return;
    // Build the replacement before touching existing slots or bounds.
    std::map<int, Slot> copy(other.slots_);
    slots_.swap(copy);
    unused_ = other.unused_;
    this->lo = other.lo;
    this->fnc = other.fnc;
    this->csize = other.csize;
  }

  Type& operator[](int idx) override { require_slot(idx); return slots_.at(idx).value; }
  const Type& operator[](int idx) const override { require_slot(idx); return slots_.at(idx).value; }
  Type& operator()(Pix p) override {
    auto it = slot_for_pix(p);
    if (it == slots_.end()) throw std::out_of_range("MPlex: invalid Pix");
    return it->second.value;
  }
  const Type& operator()(Pix p) const override {
    auto it = slot_for_pix(p);
    if (it == slots_.cend()) throw std::out_of_range("MPlex: invalid Pix");
    return it->second.value;
  }
  const Type& high_element() const override {
    int i = high(); if (i < this->lo || i >= this->fnc) throw std::out_of_range("MPlex: empty");
    return (*this)[i];
  }
  const Type& low_element() const override {
    int i = low(); if (i < this->lo || i >= this->fnc) throw std::out_of_range("MPlex: empty");
    return (*this)[i];
  }

  int valid(int idx) const override {
    auto it = slots_.find(idx); return it != slots_.end() && it->second.present;
  }
  int low() const override {
    for (const auto& entry : slots_) if (entry.second.present) return entry.first;
    return this->fnc;
  }
  int high() const override {
    for (auto it = slots_.rbegin(); it != slots_.rend(); ++it)
      if (it->second.present) return it->first;
    if (this->lo == std::numeric_limits<int>::min())
      throw std::out_of_range("MPlex: empty high sentinel outside int bounds");
    return this->lo - 1;
  }
  Pix first() const override {
    for (const auto& entry : slots_) if (entry.second.present) return Pix(const_cast<Type*>(&entry.second.value));
    return 0;
  }
  Pix last() const override {
    for (auto it = slots_.rbegin(); it != slots_.rend(); ++it)
      if (it->second.present) return Pix(const_cast<Type*>(&it->second.value));
    return 0;
  }
  void prev(Pix& p) const override {
    if (!p) return;
    auto current = slot_for_pix(p);
    if (current == slots_.cend()) { p = 0; return; }
    auto it = prev_present(current->first);
    p = it == slots_.cend() ? Pix(0) : Pix(const_cast<Type*>(&it->second.value));
  }
  void next(Pix& p) const override {
    if (!p) return;
    auto current = slot_for_pix(p);
    if (current == slots_.cend()) { p = 0; return; }
    auto it = next_present(current->first);
    p = it == slots_.cend() ? Pix(0) : Pix(const_cast<Type*>(&it->second.value));
  }
  int owns(Pix p) const override {
    auto it = slot_for_pix(p); return it != slots_.cend() && it->second.present;
  }
  void prevInt(int& idx) const override {
    auto it = prev_present(idx);
    if (it == slots_.cend() && this->lo == std::numeric_limits<int>::min())
      throw std::out_of_range("MPlex: predecessor outside int bounds");
    idx = it == slots_.cend() ? this->lo - 1 : it->first;
  }
  void nextInt(int& idx) const override {
    auto it = next_present(idx);
    idx = it == slots_.cend() ? this->fnc : it->first;
  }
  int Pix_to_index(Pix p) const override {
    auto it = slot_for_pix(p);
    if (it == slots_.cend()) throw std::out_of_range("MPlex: Pix does not belong to this container");
    return it->first;
  }
  Pix index_to_Pix(int idx) const override {
    require_slot(idx); return Pix(const_cast<Type*>(&slots_.at(idx).value));
  }

  int can_add_high() const override {
    return this->fnc < std::numeric_limits<int>::max() &&
           static_cast<std::int64_t>(this->fnc) - this->lo < std::numeric_limits<int>::max();
  }
  int can_add_low() const override {
    return this->lo > std::numeric_limits<int>::min() &&
           static_cast<std::int64_t>(this->fnc) - this->lo < std::numeric_limits<int>::max();
  }
  int full() const override { return !can_add_high() && !can_add_low(); }
  int add_high(const Type elem) override {
    if (!can_add_high()) throw std::out_of_range("MPlex: cannot grow high beyond int bounds");
    const int idx = this->fnc;
    slots_.emplace(idx, Slot(elem, true));
    ++this->fnc; // commit only after successful allocation/copy
    return idx;
  }
  int add_low(const Type elem) override {
    if (!can_add_low()) throw std::out_of_range("MPlex: cannot grow low beyond int bounds");
    const int idx = this->lo - 1;
    slots_.emplace(idx, Slot(elem, true));
    this->lo = idx; // commit only after successful allocation/copy
    return idx;
  }
  int del_high() override {
    const int idx = high();
    if (idx < this->lo || idx >= this->fnc)
      throw std::out_of_range("MPlex: delete high from empty plex");
    // Remove the highest present element and any trailing deleted slots.
    slots_.erase(slots_.lower_bound(idx), slots_.end());
    this->fnc = idx;
    unused_ = 0;
    for (const auto& e : slots_) if (!e.second.present) ++unused_;
    return high();
  }
  int del_low() override {
    const int idx = low();
    if (idx < this->lo || idx >= this->fnc)
      throw std::out_of_range("MPlex: delete low from empty plex");
    slots_.erase(slots_.begin(), slots_.upper_bound(idx));
    this->lo = idx + 1;
    unused_ = 0;
    for (const auto& e : slots_) if (!e.second.present) ++unused_;
    return low();
  }
  void clear() override { slots_.clear(); this->fnc = this->lo; unused_ = 0; }
  int reset_low(int newlow) override {
    const int old = this->lo;
    const std::int64_t delta = static_cast<std::int64_t>(newlow) - old;
    const std::int64_t newfence = static_cast<std::int64_t>(this->fnc) + delta;
    if (newfence > std::numeric_limits<int>::max() ||
        newfence < std::numeric_limits<int>::min())
      throw std::out_of_range("MPlex: shifted fence outside int bounds");
    if (delta) {
      // Node extraction preserves value addresses, hence existing Pix values.
      std::map<int, Slot> shifted;
      while (!slots_.empty()) {
        auto node = slots_.extract(slots_.begin());
        node.key() = static_cast<int>(static_cast<std::int64_t>(node.key()) + delta);
        shifted.insert(std::move(node));
      }
      slots_.swap(shifted);
      this->lo = newlow;
      this->fnc = static_cast<int>(newfence);
    }
    return old;
  }
  void fill(const Type value) override {
    for (auto& e : slots_) if (e.second.present) e.second.value = value;
  }
  void fill(const Type value, int from, int to) override {
    for (auto it = slots_.lower_bound(from); it != slots_.end() && it->first <= to; ++it)
      if (it->second.present) it->second.value = value;
  }
  void reverse() override {
    std::vector<int> valid_indices;
    for (const auto& e : slots_) if (e.second.present) valid_indices.push_back(e.first);
    for (std::size_t i = 0; i < valid_indices.size() / 2; ++i)
      std::swap(slots_.at(valid_indices[i]).value, slots_.at(valid_indices[valid_indices.size()-1-i]).value);
  }
  void append(const MFPlex<Type>& other) override {
    // Snapshot before mutation: other may alias *this.
    std::vector<Type> values;
    for (int i = other.low(); i < other.fence(); other.nextInt(i))
      values.push_back(other[i]);
    for (const Type& value : values) add_high(value);
  }
  void prepend(const MFPlex<Type>& other) override {
    std::vector<Type> values;
    for (int i = other.low(); i < other.fence(); other.nextInt(i))
      values.push_back(other[i]);
    // Adding at the low end naturally reverses order. Snapshot first for aliasing.
    for (auto it = values.rbegin(); it != values.rend(); ++it) add_low(*it);
  }

  int count() const { return static_cast<int>(slots_.size()) - unused_; }
  int available() const { return unused_; }
  int unused_index() const {
    if (!unused_) throw std::out_of_range("MPlex: no unused index");
    for (const auto& e : slots_) if (!e.second.present) return e.first;
    throw std::logic_error("MPlex: inconsistent unused count");
  }
  Pix unused_Pix() const {
    if (!unused_) return 0;
    return Pix(const_cast<Type*>(&slots_.at(unused_index()).value));
  }
  int del_index(int idx) {
    auto it = slots_.find(idx);
    if (it == slots_.end()) throw std::out_of_range("MPlex: delete index outside bounds");
    if (!it->second.present) return 0;
    it->second.present = false; ++unused_; return 1;
  }
  int del_Pix(Pix p) { return del_index(Pix_to_index(p)); }
  void undel_index(int idx) {
    auto it = slots_.find(idx);
    if (it == slots_.end()) throw std::out_of_range("MPlex: undelete index outside bounds");
    if (!it->second.present) { it->second.present = true; --unused_; }
  }
  void undel_Pix(Pix p) { undel_index(Pix_to_index(p)); }
  void adjust_bounds() {
    while (this->lo < this->fnc) {
      auto it = slots_.find(this->lo);
      if (it != slots_.end() && it->second.present) break;
      if (it != slots_.end()) { slots_.erase(it); --unused_; }
      ++this->lo;
    }
    while (this->fnc > this->lo) {
      auto it = slots_.find(this->fnc - 1);
      if (it != slots_.end() && it->second.present) break;
      if (it != slots_.end()) { slots_.erase(it); --unused_; }
      --this->fnc;
    }
    if (this->lo == this->fnc) { slots_.clear(); unused_ = 0; }
  }
  int add(const Type elem) {
    if (unused_ == 0) return add_high(elem);
    int idx = unused_index();
    auto it = slots_.find(idx); it->second.value = elem; it->second.present = true; --unused_;
    return idx;
  }
  int OK() const {
    const std::int64_t span = static_cast<std::int64_t>(this->fnc) - this->lo;
    if (span < 0 || span > std::numeric_limits<int>::max() ||
        slots_.size() != static_cast<std::size_t>(span)) return 0;
    int holes = 0;
    int expected = this->lo;
    for (const auto& entry : slots_) {
      if (entry.first != expected++) return 0;
      if (!entry.second.present) ++holes;
    }
    return holes == unused_;
  }
};

#endif
