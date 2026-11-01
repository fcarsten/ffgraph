#include <vector>
#include <cstdint>
#include <limits>
#include <stdexcept>
#if __GNUG__ >= 2
#  pragma implementation
#endif

#include <container/MFPlex.h>

//---- newclass ---------------------------------------------------------

// int ffBreak();

template<class Type>
int MFIChunk<Type>:: size() const
{
  return top - base;
}


template<class Type>
int MFIChunk<Type>:: base_index() const
{
  return base;
}

template<class Type>
int MFIChunk<Type>:: low_index() const
{
  return low;
}

template<class Type>
int  MFIChunk<Type>:: fence_index() const
{
  return fence;
}

template<class Type>
int  MFIChunk<Type>:: top_index() const
{
  return top;
}

template<class Type>
Type* MFIChunk<Type>:: pointer_to(int i) const
{
  return &(data[i-base]);
}

template<class Type>
int MFIChunk<Type>:: index_of(const Type* p) const
{
  return static_cast<int>(p - data) + base;
}

template<class Type>
int  MFIChunk<Type>:: possible_index(int i) const
{
  return i >= base && i < top;
}

template<class Type>
int MFIChunk<Type>:: possible_pointer(const Type* p) const
{
  return p >= data && p < &(data[top-base]);
}

template<class Type>
int MFIChunk<Type>:: actual_index(int i) const
{
  return i >= low && i < fence;
}

template<class Type>
int MFIChunk<Type>:: actual_pointer(const Type* p) const
{
  return p >= data && p < &(data[fence-base]);
}

template<class Type>
int MFIChunk<Type>:: can_grow_high () const
{
  return fence < top;
}

template<class Type>
int  MFIChunk<Type>:: can_grow_low () const
{
  return base < low;
}

template<class Type>
Type* MFIChunk<Type>:: invalidate()
{
  Type* p = data;
  data = 0;
  return p;
}


template<class Type>
 MFIChunk<Type>* MFIChunk<Type>::prev() const
{
  return prv;
}

template<class Type>
MFIChunk<Type>* MFIChunk<Type>::next() const
{
  return nxt;
}

template<class Type>
 void MFIChunk<Type>::link_to_prev(MFIChunk<Type>* prev)
{
  nxt = prev->nxt;
  prv = prev;
  nxt->prv = this;
  prv->nxt = this;
}

template<class Type>
 void MFIChunk<Type>::link_to_next(MFIChunk<Type>* next)
{
  prv = next->prv;
  nxt = next;
  nxt->prv = this;
  prv->nxt = this;
}

template<class Type>
 void MFIChunk<Type>::unlink()
{
  MFIChunk<Type>* n = nxt;
  MFIChunk<Type>* p = prv;
  n->prv = p;
  p->nxt = n;
  prv = nxt = this;
}

template<class Type>
int MFIChunk<Type>:: empty() const
{
  return low == fence;
}

template<class Type>
int  MFIChunk<Type>:: full() const
{
  return top - base == fence - low;
}

template<class Type>
int MFIChunk<Type>:: first_index() const
{
  return (low == fence)? fence : low;
}

template<class Type>
int MFIChunk<Type>:: last_index() const
{
  return (low == fence)? low - 1 : fence - 1;
}

template<class Type>
int  MFIChunk<Type>:: succ(int i) const
{
  return (i < low) ? low : i + 1;
}

template<class Type>
int  MFIChunk<Type>:: pred(int i) const
{
  return (i > fence) ? (fence - 1) : i - 1;
}

template<class Type>
int  MFIChunk<Type>:: valid_index(int i) const
{
  return i >= low && i < fence;
}

template<class Type>
int  MFIChunk<Type>:: valid_pointer(const Type* p) const
{
  return p >= &(data[low - base]) && p < &(data[fence - base]);
}

template<class Type>
Type* MFIChunk<Type>:: grow_high ()
{
  if (!can_grow_high()) full_error();
  return &(data[fence++ - base]);
}

template<class Type>
Type* MFIChunk<Type>:: grow_low ()
{
  if (!can_grow_low()) full_error();
  return &(data[--low - base]);
}

template<class Type>
void MFIChunk<Type>:: shrink_high ()
{
  if (empty()) empty_error();
  --fence;
}

template<class Type>
void MFIChunk<Type>:: shrink_low ()
{
  if (empty()) empty_error();
  ++low;
}

template<class Type>
Type* MFIChunk<Type>::first_pointer() const
{
  return (low == fence)? 0 : &(data[low - base]);
}

template<class Type>
Type* MFIChunk<Type>::last_pointer() const
{
  return (low == fence)? 0 : &(data[fence - base - 1]);
}

template<class Type>
Type* MFIChunk<Type>::succ(Type* p) const
{
  return ((p+1) <  &(data[low - base]) || (p+1) >= &(data[fence - base])) ? 
    0 : (p+1);
}

template<class Type>
Type* MFIChunk<Type>::pred(Type* p) const
{
  return ((p-1) <  &(data[low - base]) || (p-1) >= &(data[fence - base])) ? 
    0 : (p-1);
}

template<class Type>
int MFPlex<Type>::chunk_size() const
{
  return csize;
}

template<class Type>
int MFPlex<Type>::ecnef () const
{
  if (lo == std::numeric_limits<int>::min())
    throw std::out_of_range("MFPlex: predecessor outside int bounds");
  return lo - 1;
}


template<class Type>
int MFPlex<Type>::fence () const
{
  return fnc;
}

template<class Type>
int MFPlex<Type>::length () const
{
  const std::int64_t span = static_cast<std::int64_t>(fnc) - lo;
  if (span < 0 || span > std::numeric_limits<int>::max())
    throw std::out_of_range("MFPlex: length exceeds int bounds");
  return static_cast<int>(span);
}

template<class Type>
int MFPlex<Type>::empty () const
{
  return fnc == lo;
}

template<class Type>
MFIChunk<Type>* MFPlex<Type>::tl() const
{
	if(hd != NULL)
	  return hd->prev();
	else
		return NULL;
}

template<class Type>
int MFPlex<Type>::one_chunk() const
{
  return hd == hd->prev();
}

template<class Type>
void MFIChunk<Type>::error(const char* msg) const
{
//  ffBreak();
 
  std::cerr << "MFIChunk:" <<  msg << std::endl;
}

template<class Type>
void MFIChunk<Type>::index_error() const
{
  error("attempt to use invalid index");
}

template<class Type>
void MFIChunk<Type>::empty_error() const
{
  error("invalid use of empty chunk");
}

template<class Type>
void MFIChunk<Type>::full_error() const
{
  error("attempt to extend chunk beyond bounds");
}

template<class Type>
MFIChunk<Type>:: ~MFIChunk() {}

template<class Type>
MFIChunk<Type>::MFIChunk(Type*     d,    
                     int      baseidx,
                     int      lowidx,
                     int      fenceidx,
                     int      topidx)
{
  if (d == 0 || baseidx > lowidx || lowidx > fenceidx || fenceidx > topidx)
    error("inconsistent specification");
  data = d;
  base = baseidx;
  low = lowidx;
  fence = fenceidx;
  top = topidx;
  nxt = prv = this;
}

template<class Type>
void MFIChunk<Type>:: re_index(int lo)
{
  const std::int64_t delta = static_cast<std::int64_t>(lo) - low;
  const auto shifted = [delta](int value) {
    const std::int64_t result = static_cast<std::int64_t>(value) + delta;
    if (result < std::numeric_limits<int>::min() ||
        result > std::numeric_limits<int>::max())
      throw std::out_of_range("MFIChunk: reindex outside int bounds");
    return static_cast<int>(result);
  };
  const int newbase = shifted(base), newfence = shifted(fence), newtop = shifted(top);
  base = newbase;
  low = lo;
  fence = newfence;
  top = newtop;
}


template<class Type>
void MFIChunk<Type>::clear(int lo)
{
  int s = top - base;
  low = base = fence = lo;
  top = base + s;
}

template<class Type>
void MFIChunk<Type>::cleardown(int hi)
{
  int s = top - base;
  low = top = fence = hi;
  base = top - s;
}

template<class Type>
int MFIChunk<Type>:: OK() const
{
  int v = data != 0;             // have some data
  v &= base <= low;              // ok, index-wise
  v &= low <= fence;
  v &= fence <= top;

  v &=  nxt->prv == this;      // and links are OK
  v &=  prv->nxt == this;
  if (!v) error("invariant failure");
  return(v);
}


// error handling


template<class Type>
void MFPlex<Type>::error(const char* msg) const
{
//  ffBreak();
 
  std::cerr << "Plex: " << msg << std::endl;
}

template<class Type>
void MFPlex<Type>::index_error() const
{
  error("attempt to access invalid index");
}

template<class Type>
void MFPlex<Type>::empty_error() const
{
  error("attempted operation on empty plex");
}

template<class Type>
void MFPlex<Type>::full_error() const
{
  error("attempt to increase size of plex past limit");
}

// generic plex ops

template<class Type>
MFPlex<Type>:: ~MFPlex()
{
  invalidate();
}  


template<class Type>
void MFPlex<Type>::append (const MFPlex& a)
{
  std::vector<Type> values;
  for (int i = a.low(); i < a.fence(); a.nextInt(i)) values.push_back(a[i]);
  for (const Type& value : values) add_high(value);
}

template<class Type>
void MFPlex<Type>::prepend (const MFPlex& a)
{
  std::vector<Type> values;
  for (int i = a.high(); i > a.ecnef(); a.prevInt(i)) values.push_back(a[i]);
  for (const Type& value : values) add_low(value);
}

template<class Type>
void MFPlex<Type>::reverse()
{
  Type tmp;
  int l = low();
  int h = high();
  while (l < h)
  {
    tmp = (*this)[l];
    (*this)[l] = (*this)[h];
    (*this)[h] = tmp;
    nextInt(l);
    prevInt(h);
  }
}


template<class Type>
void MFPlex<Type>::fill(const Type  x)
{
  for (int i = lo; i < fnc; ++i) (*this)[i] = x;
}

template<class Type>
void MFPlex<Type>::fill(const Type  x, int lo, int hi)
{
  for (int i = lo; i <= hi; ++i) (*this)[i] = x;
}


template<class Type>
void MFPlex<Type>::del_chunk(MFIChunk<Type>* x)
{
  if (x != 0)
  {
    x->unlink();
    Type* data = (Type*)(x->invalidate());
    delete [] data;
    delete x;
  }
}


template<class Type>
void MFPlex<Type>::invalidate()
{
  MFIChunk<Type>* t = hd;
  if (t != 0)
  {
    MFIChunk<Type>* tail = tl();
    while (t != tail)
    {
      MFIChunk<Type>* nxt = t->next();
      del_chunk(t);
      t = nxt;
    } 
    del_chunk(t);
    hd = 0;
  }
}

template<class Type>
int MFPlex<Type>::reset_low(int l)
{
  const int old = lo;
  const std::int64_t delta = static_cast<std::int64_t>(l) - lo;
  const std::int64_t newfence = static_cast<std::int64_t>(fnc) + delta;
  if (newfence < std::numeric_limits<int>::min() ||
      newfence > std::numeric_limits<int>::max())
    throw std::out_of_range("MFPlex: shifted fence outside int bounds");
  if (delta == 0) return old;
  // Validate every chunk before modifying any indices. Empty MPlex-backed
  // MFPlex instances have no chunks, and are valid objects too.
  if (hd) {
    MFIChunk<Type>* t = hd;
    do {
      const std::int64_t newlow = static_cast<std::int64_t>(t->low_index()) + delta;
      const std::int64_t newbase = static_cast<std::int64_t>(t->base_index()) + delta;
      const std::int64_t newtop = static_cast<std::int64_t>(t->top_index()) + delta;
      const std::int64_t newchunkfence = static_cast<std::int64_t>(t->fence_index()) + delta;
      if (newlow < std::numeric_limits<int>::min() || newlow > std::numeric_limits<int>::max() ||
          newbase < std::numeric_limits<int>::min() || newbase > std::numeric_limits<int>::max() ||
          newtop < std::numeric_limits<int>::min() || newtop > std::numeric_limits<int>::max() ||
          newchunkfence < std::numeric_limits<int>::min() || newchunkfence > std::numeric_limits<int>::max())
        throw std::out_of_range("MFPlex: shifted chunk outside int bounds");
      t = t->next();
    } while (t != hd);
    t = hd;
    do {
      t->re_index(static_cast<int>(static_cast<std::int64_t>(t->low_index()) + delta));
      t = t->next();
    } while (t != hd);
  }
  lo = l;
  fnc = static_cast<int>(newfence);
  return old;
}





