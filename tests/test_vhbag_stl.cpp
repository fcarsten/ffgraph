#include <iostream>
#include <cassert>
inline unsigned int HASH(int& x) { return static_cast<unsigned int>(x * 2654435761u); }
inline int EQ(int& a, int& b) { return a == b; }
void (*lib_error_handler)(const char*, const char*) = nullptr;
#include "../include/container/VHBag.h"
template<class T> void MFBag<T>::error(const char* msg) { if (lib_error_handler) lib_error_handler("Bag", msg); }
template<class T> void MFBag<T>::remove(T item) { while (nof(item)) del(item); }
template<class T> void MFBag<T>::clear() { while (first()) del((*this)(first())); }
template<class T> int MFBag<T>::nof(T item) { int n=0; for (Pix p=first(); p; next(p)) { T v=(*this)(p); if (EQ(v,item)) ++n; } return n; }
template<class T> Pix MFBag<T>::seek(T item, Pix p) { if (!p) p=first(); else next(p); for (; p; next(p)) { T v=(*this)(p); if (EQ(v,item)) return p; } return 0; }
template<class T> int MFBag<T>::owns(Pix p) { for (Pix i=first(); i; next(i)) if (i==p) return 1; return 0; }
int main() {
  VHBag<int> b(2); b.add(7); b.add(7); b.add(9);
  assert(b.length() == 3 && b.nof(7) == 2 && b.contains(9) && b.OK());
  Pix p = b.seek(7); assert(p && b(p) == 7 && b.owns(p));
  Pix p2 = b.seek(7, p); assert(p2 && p2 != p && b(p2) == 7);
  int seen=0; for (Pix i=b.first(); i; b.next(i)) ++seen; assert(seen == 3);
  b.del(7); assert(b.nof(7) == 1 && b.length() == 2);
  b.remove(7); assert(b.nof(7) == 0 && b.length() == 1 && b.OK());
  VHBag<int> copy(b); assert(copy.length() == 1 && copy.contains(9));
  copy.clear(); assert(copy.empty() && copy.OK());
}
