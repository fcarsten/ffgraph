#ifndef FFGRAPH_COMPAT_DLLIST_H
#define FFGRAPH_COMPAT_DLLIST_H

#include <cstddef>
#include <list>
#include <memory>
#include <stdexcept>
#include <utility>
#include "Pix.h"

// Transitional adapter for the GNU libg++ DLList API. Storage and element
// lifetime are managed by std::list; Pix remains an opaque stable element handle
// so the legacy call sites can be migrated incrementally.
template<class T>
class DLList {
  struct Node;
  using Store = std::list<std::unique_ptr<Node>>;
  struct Node {
    T value;
    typename Store::iterator position;
    template<class U> explicit Node(U&& v) : value(std::forward<U>(v)) {}
  };
  Store items_;

  Node* node(Pix p) const { return static_cast<Node*>(p); }
  Pix handle(typename Store::const_iterator it) const {
    return it == items_.end() ? nullptr : static_cast<Pix>(it->get());
  }
  void copy_from(const DLList& other) { for (Pix p=other.first(); p; other.next(p)) append(other(p)); }
public:
  DLList() = default;
  DLList(const DLList& other) { copy_from(other); }
  DLList(DLList&&) noexcept = default;
  DLList& operator=(const DLList& other) { if (this != &other) { clear(); copy_from(other); } return *this; }
  DLList& operator=(DLList&&) noexcept = default;
  ~DLList() = default;

  int empty() const { return items_.empty(); }
  int length() const { return static_cast<int>(items_.size()); }
  void clear() { items_.clear(); }
  Pix first() const { return handle(items_.cbegin()); }
  Pix last() const { return items_.empty() ? nullptr : static_cast<Pix>(items_.back().get()); }
  void next(Pix& p) const {
    if (!p) return;
    Node* n=node(p);
    auto it=n->position;
    ++it;
    p=handle(it);
  }
  void prev(Pix& p) const {
    if (!p) return;
    Node* n=node(p);
    auto it=n->position;
    if (it == items_.cbegin()) { p=nullptr; return; }
    --it;
    p=handle(it);
  }
  T& operator()(Pix p) { if (!p) throw std::out_of_range("DLList: null Pix"); return node(p)->value; }
  const T& operator()(Pix p) const { if (!p) throw std::out_of_range("DLList: null Pix"); return node(p)->value; }
  int owns(Pix p) const { for (const auto& n : items_) if (n.get()==p) return 1; return 0; }
  void append(const T& value) { auto n=std::make_unique<Node>(value); auto it=items_.insert(items_.end(),std::move(n)); (*it)->position=it; }
  void append(T&& value) { auto n=std::make_unique<Node>(std::move(value)); auto it=items_.insert(items_.end(),std::move(n)); (*it)->position=it; }
  void prepend(const T& value) { auto n=std::make_unique<Node>(value); auto it=items_.insert(items_.begin(),std::move(n)); (*it)->position=it; }
  void prepend(T&& value) { auto n=std::make_unique<Node>(std::move(value)); auto it=items_.insert(items_.begin(),std::move(n)); (*it)->position=it; }
  // libg++ compatibility: insert relative to an existing Pix handle.
  // A null handle has no position to insert around, so append in that case.
  void ins_before(Pix p, const T& value) {
    if (!p) { append(value); return; }
    Node* target = node(p);
    auto it = items_.insert(target->position, std::make_unique<Node>(value));
    (*it)->position = it;
  }
  void ins_before(Pix p, T&& value) {
    if (!p) { append(std::move(value)); return; }
    Node* target = node(p);
    auto it = items_.insert(target->position, std::make_unique<Node>(std::move(value)));
    (*it)->position = it;
  }
  void ins_after(Pix p, const T& value) {
    if (!p) { append(value); return; }
    Node* target = node(p);
    auto position = std::next(target->position);
    auto it = items_.insert(position, std::make_unique<Node>(value));
    (*it)->position = it;
  }
  void ins_after(Pix p, T&& value) {
    if (!p) { append(std::move(value)); return; }
    Node* target = node(p);
    auto position = std::next(target->position);
    auto it = items_.insert(position, std::make_unique<Node>(std::move(value)));
    (*it)->position = it;
  }
  T& front() { if (empty()) throw std::out_of_range("DLList: empty front"); return items_.front()->value; }
  const T& front() const { if (empty()) throw std::out_of_range("DLList: empty front"); return items_.front()->value; }
  T& rear() { if (empty()) throw std::out_of_range("DLList: empty rear"); return items_.back()->value; }
  const T& rear() const { if (empty()) throw std::out_of_range("DLList: empty rear"); return items_.back()->value; }
  T remove_front() { if (empty()) throw std::out_of_range("DLList: empty remove_front"); T v=std::move(items_.front()->value); items_.pop_front(); return v; }
  void del_front() { if (!empty()) items_.pop_front(); }
  void del(const T& value) { for (auto it=items_.begin(); it!=items_.end(); ++it) if ((*it)->value == value) { items_.erase(it); return; } }
  void del(Pix& p) { if (!p) return; Node* target=node(p); for (auto it=items_.begin(); it!=items_.end(); ++it) if (it->get()==target) { auto nextIt=std::next(it); p=handle(nextIt); items_.erase(it); return; } p=nullptr; }
  int contains(const T& value) const { for (const auto& n : items_) if (n->value == value) return 1; return 0; }
  int OK() const { return 1; }
};

#endif
