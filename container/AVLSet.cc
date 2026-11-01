#if __GNUG__ >= 2
#  pragma implementation
#endif

#include <container/AVLSet.h>

//---- newclass ---------------------------------------------------------
template<class T>
AVLNode<T>::AVLNode(T  h, AVLNode<T>* l, AVLNode<T>* r)
						:lt(l),rt(r),item(h),stat(0) 
{}

template<class T>
AVLSet<T>::~AVLSet()
{
  _kill(root);
}

template<class T>
AVLSet<T>::AVLSet()
{
  root = 0;
  this->count = 0;
}

template<class T>
AVLSet<T>::AVLSet(AVLNode<T>* p, int l)
{
  root = p;
  this->count = l;
}

template<class T>
AVLSet<T>& AVLSet<T>::operator =  (const AVLSet<T>& b)
{
  if (this != &b) {
    clear();
    *this |= b;
  }
  return *this;
}

template<class T>
int AVLSet<T>::operator != (const AVLSet<T>& b)
{
  return ! ((*this) == b);
}

template<class T>
Pix AVLSet<T>::first() const
{
  return Pix(leftmost());
}

template<class T>
Pix AVLSet<T>::last()
{
  return Pix(rightmost());
}

template<class T>
void AVLSet<T>::next(Pix& i) const
{
  if (i != 0) i = Pix(succ((AVLNode<T>*)i));
}

template<class T>
void AVLSet<T>::prev(Pix& i)
{
  if (i != 0) i = Pix(pred((AVLNode<T>*)i));
}

template<class T>
T& AVLSet<T>::operator () (Pix i) const
{
  if (i == 0) this->error("null Pix");
  return ((AVLNode<T>*)i)->item;
}

template<class T>
void AVLSet<T>::clear()
{
  _kill(root);
  this->count = 0;
  root = 0;
}

template<class T>
int AVLSet<T>::contains(T  key) const
{
  return seek(key) != 0;
}

template<class T>
AVLNode<T>* AVLSet<T>::leftmost() const
{
  AVLNode<T>* t = root;
  if (t != 0) while (t->lt != 0) t = t->lt;
  return t;
}

template<class T>
AVLNode<T>* AVLSet<T>::rightmost() const
{
  AVLNode<T>* t = root;
  if (t != 0) while (t->rt != 0) t = t->rt;
  return t;
}

template<class T>
AVLNode<T>* AVLSet<T>::succ(AVLNode<T>* t) const
{
  AVLNode<T>* r = t->rt;
  if (!rthread(t)) while (!lthread(r)) r = r->lt;
  return r;
}

template<class T>
AVLNode<T>* AVLSet<T>::pred(AVLNode<T>* t)
{
  AVLNode<T>* l = t->lt;
  if (!lthread(t)) while (!rthread(l)) l = l->rt;
  return l;
}

template<class T>
Pix AVLSet<T>::seek(T  key) const
{
  AVLNode<T>* t = root;
  if (t == 0)
    return 0;
  for (;;)
  {
    extern int CMP(T&,T&);
    int cmp = CMP(key, t->item);
    if (cmp == 0)
      return Pix(t);
    else if (cmp < 0)
    {
      if (lthread(t))
        return 0;
      else
        t = t->lt;
    }
    else if (rthread(t))
      return 0;
    else
      t = t->rt;
  }
}


/*
 The combination of threads and AVL bits make adding & deleting
 interesting, but very awkward.

 We use the following statics to avoid passing them around recursively
*/


template<class T>
void AVLSet<T>:: _add(AVLNode<T>*& t)
{
  extern int CMP(T&,T&);
  int cmp = CMP(*_target_item, t->item);
  if (cmp == 0)
  {
    _found_node = t;
    return;
  }
  else if (cmp < 0)
  {
    if (lthread(t))
    {
      ++this->count;
      _found_node = new AVLNode<T>(*_target_item);
      set_lthread(_found_node, 1);
      set_rthread(_found_node, 1);
      _found_node->lt = t->lt;
      _found_node->rt = t;
      t->lt = _found_node;
      set_lthread(t, 0);
      _need_rebalancing = 1;
    }
    else
      _add(t->lt);
    if (_need_rebalancing)
    {
      switch(bf(t))
      {
      case AVLRIGHTHEAVY:
        set_bf(t, AVLBALANCED);
        _need_rebalancing = 0;
        return;
      case AVLBALANCED:
        set_bf(t, AVLLEFTHEAVY);
        return;
      case AVLLEFTHEAVY:
	{
        AVLNode<T>* l = t->lt;
        if (bf(l) == AVLLEFTHEAVY)
        {
          if (rthread(l))
            t->lt = l;
          else
            t->lt = l->rt;
          set_lthread(t, rthread(l));
          l->rt = t;
          set_rthread(l, 0);
          set_bf(t, AVLBALANCED);
          set_bf(l, AVLBALANCED);
          t = l;
          _need_rebalancing = 0;
        }
        else
        {
          AVLNode<T>* r = l->rt;
          set_rthread(l, lthread(r));
          if (lthread(r))
            l->rt = r;
          else
            l->rt = r->lt;
          r->lt = l;
          set_lthread(r, 0);
          set_lthread(t, rthread(r));
          if (rthread(r))
            t->lt = r;
          else
            t->lt = r->rt;
          r->rt = t;
          set_rthread(r, 0);
          if (bf(r) == AVLLEFTHEAVY)
            set_bf(t, AVLRIGHTHEAVY);
          else
            set_bf(t, AVLBALANCED);
          if (bf(r) == AVLRIGHTHEAVY)
            set_bf(l, AVLLEFTHEAVY);
          else
            set_bf(l, AVLBALANCED);
          set_bf(r, AVLBALANCED);
          t = r;
          _need_rebalancing = 0;
          return;
        }
	}
      }
    }
  }
  else
  {
    if (rthread(t))
    {
      ++this->count;
      _found_node = new AVLNode<T>(*_target_item);
      set_rthread(t, 0);
      set_lthread(_found_node, 1);
      set_rthread(_found_node, 1);
      _found_node->lt = t;
      _found_node->rt = t->rt;
      t->rt = _found_node;
      _need_rebalancing = 1;
    }
    else
      _add(t->rt);
    if (_need_rebalancing)
    {
      switch(bf(t))
      {
      case AVLLEFTHEAVY:
        set_bf(t, AVLBALANCED);
        _need_rebalancing = 0;
        return;
      case AVLBALANCED:
        set_bf(t, AVLRIGHTHEAVY);
        return;
      case AVLRIGHTHEAVY:
	{
        AVLNode<T>* r = t->rt;
        if (bf(r) == AVLRIGHTHEAVY)
        {
          if (lthread(r))
            t->rt = r;
          else
            t->rt = r->lt;
          set_rthread(t, lthread(r));
          r->lt = t;
          set_lthread(r, 0);
          set_bf(t, AVLBALANCED);
          set_bf(r, AVLBALANCED);
          t = r;
          _need_rebalancing = 0;
        }
        else
        {
          AVLNode<T>* l = r->lt;
          set_lthread(r, rthread(l));
          if (rthread(l))
            r->lt = l;
          else
            r->lt = l->rt;
          l->rt = r;
          set_rthread(l, 0);
          set_rthread(t, lthread(l));
          if (lthread(l))
            t->rt = l;
          else
            t->rt = l->lt;
          l->lt = t;
          set_lthread(l, 0);
          if (bf(l) == AVLRIGHTHEAVY)
            set_bf(t, AVLLEFTHEAVY);
          else
            set_bf(t, AVLBALANCED);
          if (bf(l) == AVLLEFTHEAVY)
            set_bf(r, AVLRIGHTHEAVY);
          else
            set_bf(r, AVLBALANCED);
          set_bf(l, AVLBALANCED);
          t = l;
          _need_rebalancing = 0;
          return;
        }
	}
      }
    }
  }
}

template<class T>
Pix AVLSet<T>::add(T  item)
{
  if (root == 0)
  {
    ++this->count;
    root = new AVLNode<T>(item);
    set_rthread(root, 1);
    set_lthread(root, 1);
    return Pix(root);
  }
  else
  {
    _target_item = &item;
    _need_rebalancing = 0;
    _add(root);
    return Pix(_found_node);
  }
}

template<class T>
void AVLSet<T>::_del(AVLNode<T>* par, AVLNode<T>*& t)
{
  int comp;
  extern int CMP(T&,T&);
  
  if (_already_found)
  {
    if (rthread(t))
      comp = 0;
    else
      comp = 1;
  }
  else 
    comp = CMP(*_target_item, t->item);
  if (comp == 0)
  {
    if (lthread(t) && rthread(t))
    {
      _found_node = t;
      if (t == par->lt)
      {
        set_lthread(par, 1);
        par->lt = t->lt;
      }
      else
      {
        set_rthread(par, 1);
        par->rt = t->rt;
      }
      _need_rebalancing = 1;
      return;
    }
    else if (lthread(t))
    {
      _found_node = t;
      AVLNode<T>* s = succ(t);
      if (s != 0 && lthread(s))
        s->lt = t->lt;
      t = t->rt;
      _need_rebalancing = 1;
      return;
    }
    else if (rthread(t))
    {
      _found_node = t;
      AVLNode<T>* p = pred(t);
      if (p != 0 && rthread(p))
        p->rt = t->rt;
      t = t->lt;
      _need_rebalancing = 1;
      return;
    }
    else                        // replace item & find someone deletable
    {
      AVLNode<T>* p = pred(t);
      t->item = p->item;
      _already_found = 1;
      comp = -1;                // fall through below to left
    }
  }

  if (comp < 0)
  {
    if (lthread(t))
      return;
    _del(t, t->lt);
    if (!_need_rebalancing)
      return;
    switch (bf(t))
    {
    case AVLLEFTHEAVY:
      set_bf(t, AVLBALANCED);
      return;
    case AVLBALANCED:
      set_bf(t, AVLRIGHTHEAVY);
      _need_rebalancing = 0;
      return;
    case AVLRIGHTHEAVY:
      {
      AVLNode<T>* r = t->rt;
      switch (bf(r))
      {
      case AVLBALANCED:
        if (lthread(r))
          t->rt = r;
        else
          t->rt = r->lt;
        set_rthread(t, lthread(r));
        r->lt = t;
        set_lthread(r, 0);
        set_bf(t, AVLRIGHTHEAVY);
        set_bf(r, AVLLEFTHEAVY);
        _need_rebalancing = 0;
        t = r;
        return;
      case AVLRIGHTHEAVY:
        if (lthread(r))
          t->rt = r;
        else
          t->rt = r->lt;
        set_rthread(t, lthread(r));
        r->lt = t;
        set_lthread(r, 0);
        set_bf(t, AVLBALANCED);
        set_bf(r, AVLBALANCED);
        t = r;
        return;
      case AVLLEFTHEAVY:
	{
        AVLNode<T>* l = r->lt;
        set_lthread(r, rthread(l));
        if (rthread(l))
          r->lt = l;
        else
          r->lt = l->rt;
        l->rt = r;
        set_rthread(l, 0);
        set_rthread(t, lthread(l));
        if (lthread(l))
          t->rt = l;
        else
          t->rt = l->lt;
        l->lt = t;
        set_lthread(l, 0);
        if (bf(l) == AVLRIGHTHEAVY)
          set_bf(t, AVLLEFTHEAVY);
        else
          set_bf(t, AVLBALANCED);
        if (bf(l) == AVLLEFTHEAVY)
          set_bf(r, AVLRIGHTHEAVY);
        else
          set_bf(r, AVLBALANCED);
        set_bf(l, AVLBALANCED);
        t = l;
        return;
	}
      }
    }
    }
  }
  else
  {
    if (rthread(t))
      return;
    _del(t, t->rt);
    if (!_need_rebalancing)
      return;
    switch (bf(t))
    {
    case AVLRIGHTHEAVY:
      set_bf(t, AVLBALANCED);
      return;
    case AVLBALANCED:
      set_bf(t, AVLLEFTHEAVY);
      _need_rebalancing = 0;
      return;
    case AVLLEFTHEAVY:
      {
      AVLNode<T>* l = t->lt;
      switch (bf(l))
      {
      case AVLBALANCED:
        if (rthread(l))
          t->lt = l;
        else
          t->lt = l->rt;
        set_lthread(t, rthread(l));
        l->rt = t;
        set_rthread(l, 0);
        set_bf(t, AVLLEFTHEAVY);
        set_bf(l, AVLRIGHTHEAVY);
        _need_rebalancing = 0;
        t = l;
        return;
      case AVLLEFTHEAVY:
        if (rthread(l))
          t->lt = l;
        else
          t->lt = l->rt;
        set_lthread(t, rthread(l));
        l->rt = t;
        set_rthread(l, 0);
        set_bf(t, AVLBALANCED);
        set_bf(l, AVLBALANCED);
        t = l;
        return;
      case AVLRIGHTHEAVY:
	{
        AVLNode<T>* r = l->rt;
        set_rthread(l, lthread(r));
        if (lthread(r))
          l->rt = r;
        else
          l->rt = r->lt;
        r->lt = l;
        set_lthread(r, 0);
        set_lthread(t, rthread(r));
        if (rthread(r))
          t->lt = r;
        else
          t->lt = r->rt;
        r->rt = t;
        set_rthread(r, 0);
        if (bf(r) == AVLLEFTHEAVY)
          set_bf(t, AVLRIGHTHEAVY);
        else
          set_bf(t, AVLBALANCED);
        if (bf(r) == AVLRIGHTHEAVY)
          set_bf(l, AVLLEFTHEAVY);
        else
          set_bf(l, AVLBALANCED);
        set_bf(r, AVLBALANCED);
        t = r;
        return;
	}
      }
      }
    }
  }
}

        

template<class T>
void AVLSet<T>::del(T item)
{
  if (root == 0) return;
  _need_rebalancing = 0;
  _already_found = 0;
  _found_node = 0;
  _target_item = &item;
  _del(root, root);
  if (_found_node)
  {
    delete(_found_node);
    if (--this->count == 0)
      root = 0;
  }
}

// build an ordered array of pointers to tree nodes back into a tree
// we know that at least one element exists

template<class T>
AVLNode<T>* AVLSet<T>::_do_treeify(int lo, int hi, int& h)
{
  int lh, rh;
  int mid = (lo + hi) / 2;
  AVLNode<T>* t = _hold_nodes[mid];
  if (lo > mid - 1)
  {
    set_lthread(t, 1);
    if (mid == 0)
      t->lt = 0;
    else
      t->lt = _hold_nodes[mid-1];
    lh = 0;
  }
  else
  {
    set_lthread(t, 0);
    t->lt = _do_treeify(lo, mid-1, lh);
  }
  if (hi < mid + 1)
  {
    set_rthread(t, 1);
    if (mid == _max_hold_index)
      t->rt = 0;
    else
      t->rt = _hold_nodes[mid+1];
    rh = 0;
  }
  else 
  {
    set_rthread(t, 0);
    t->rt = _do_treeify(mid+1, hi, rh);
  }
  if (lh == rh)
  {
    set_bf(t, AVLBALANCED);
    h = lh + 1;
  }
  else if (lh == rh - 1)
  {
    set_bf(t, AVLRIGHTHEAVY);
    h = rh + 1;
  }
  else if (rh == lh - 1)
  {
    set_bf(t, AVLLEFTHEAVY);
    h = lh + 1;
  }
  else                          // can't happen
    abort();

  return t;
}

template<class T>
AVLNode<T>* AVLSet<T>::_treeify(int n)
{
  AVLNode<T>* t;
  if (n == 0)
    t = 0;
  else
  {
    int b;
    _max_hold_index = n-1;
    t = _do_treeify(0, _max_hold_index, b);
  }
  delete [] _hold_nodes;
  return t;
}


template<class T>
void AVLSet<T>::_kill(AVLNode<T>* t)
{
  if (t != 0)
  {
    if (!lthread(t)) _kill(t->lt);
    if (!rthread(t)) _kill(t->rt);
    delete t;
  }
}


template<class T>
AVLSet<T>::AVLSet(const AVLSet<T>& b)
{
  if ((this->count = b.count) == 0)
  {
    root = 0;
  }
  else
  {
//    typedef AVLNode<T>* AVLNodePtr<T>;
    _hold_nodes = new AVLNode<T> *[this->count];
    AVLNode<T>* t = b.leftmost();
    int i = 0;
    while (t != 0)
    {
      _hold_nodes[i++] = new AVLNode<T>(t->item);
      t = b.succ(t);
    }
    root = _treeify(this->count);
  }
}

template<class T>
int AVLSet<T>::operator == (const AVLSet<T>& y)
{
  if (this->count != y.count)
    return 0;
  else
  {
    AVLNode<T>* t = leftmost();
    AVLNode<T>* u = y.leftmost();
    for (;;)
    {
     extern int EQ(T&,T&);
      if (t == 0)
        return 1;
      else if (!(EQ(t->item, u->item)))
        return 0;
      else
      {
        t = succ(t);
        u = y.succ(u);
      }
    }
  }
}

template<class T>
int AVLSet<T>::operator <= (const AVLSet<T>& y)
{
  if (this->count > y.count)
    return 0;
  else
  {
		AVLNode<T>* t = leftmost();
    AVLNode<T>* u = y.leftmost();
    for (;;)
    {
      if (t == 0)
        return 1;
      else if (u == 0)
        return 0;
      extern int CMP(T&,T&);
      int cmp = CMP(t->item, u->item);
      if (cmp == 0)
      {
        t = succ(t);
        u = y.succ(u);
      }
      else if (cmp < 0)
        return 0;
      else
        u = y.succ(u);
    }
  }
}

template<class T>
void AVLSet<T>::operator |=(const AVLSet<T>& y)
{
  if (this == &y) return;
  AVLNode<T>* t = leftmost();
  AVLNode<T>* u = y.leftmost();
  int rsize = this->count + y.count;
	_hold_nodes = new AVLNode<T> *[rsize];
  int k = 0;
  for (;;)
  {
    if (t == 0)
    {
      while (u != 0)
      {
        _hold_nodes[k++] = new AVLNode<T>(u->item);
        u = y.succ(u);
      }
      break;
    }
    else if (u == 0)
    {
      while (t != 0)
      {
        _hold_nodes[k++] = t;
        t = succ(t);
      }
      break;
    }
    extern int CMP(T&,T&);
    int cmp = CMP(t->item, u->item);
    if (cmp == 0)
    {
      _hold_nodes[k++] = t;
      t = succ(t);
      u = y.succ(u);
    }
    else if (cmp < 0)
    {
      _hold_nodes[k++] = t;
      t = succ(t);
    }
    else
    {
      _hold_nodes[k++] = new AVLNode<T>(u->item);
      u = y.succ(u);
    }
  }
  root = _treeify(k);
  this->count = k;
}

template<class T>
void AVLSet<T>::operator &= (const AVLSet<T>& y)
{
  if (this == &y) return;
  AVLNode<T>* t = leftmost();
  AVLNode<T>* u = y.leftmost();
  int rsize = (this->count < y.count)? this->count : y.count;
  _hold_nodes = new AVLNode<T> *[rsize];
  int k = 0;
  for (;;)
  {
    if (t == 0)
      break;
    if (u == 0)
    {
      while (t != 0)
      {
        AVLNode<T>* tmp = succ(t);
        delete t;
        t = tmp;
      }
      break;
    }
    extern int CMP(T&,T&);
    int cmp = CMP(t->item, u->item);
    if (cmp == 0)
    {
      _hold_nodes[k++] = t;
      t = succ(t);
      u = y.succ(u);
    }
    else if (cmp < 0)
    {
      AVLNode<T>* tmp = succ(t);
      delete t;
      t = tmp;
    }
    else
      u = y.succ(u);
  }
  root = _treeify(k);
  this->count = k;
}

template<class T>
void AVLSet<T>::operator -=(const AVLSet<T>& y)
{
  if (this == &y) { clear(); return; }
  AVLNode<T>* t = leftmost();
  AVLNode<T>* u = y.leftmost();
  int rsize = this->count;
  _hold_nodes = new AVLNode<T> *[rsize];
  int k = 0;
  for (;;)
  {
    if (t == 0)
      break;
    else if (u == 0)
    {
      while (t != 0)
      {
        _hold_nodes[k++] = t;
        t = succ(t);
      }
      break;
    }
    extern int CMP(T&,T&);
    int cmp = CMP(t->item, u->item);
    if (cmp == 0)
    {
      AVLNode<T>* tmp = succ(t);
      delete t;
      t = tmp;
      u = y.succ(u);
    }
    else if (cmp < 0)
    {
      _hold_nodes[k++] = t;
      t = succ(t);
    }
    else
      u = y.succ(u);
  }
  root = _treeify(k);
  this->count = k;
}

template<class T>
int AVLSet<T>::owns(Pix i)
{
  if (i == 0) return 0;
  for (AVLNode<T>* t = leftmost(); t != 0; t = succ(t)) 
    if (Pix(t) == i) return 1;
  return 0;
}

template<class T>
int AVLSet<T>::OK()
{
  int v = 1;
  if (root == 0) 
    v = this->count == 0;
  else
  {
    int n = 1;
    extern int CMP(T&,T&);
    
		AVLNode<T>* trail = leftmost();
    AVLNode<T>* t = succ(trail);
    while (t != 0)
    {
      ++n;
      v &= CMP(trail->item, t->item) < 0;
      trail = t;
      t = succ(t);
    }
    v &= n == this->count;
  }
  if (!v) this->error("invariant failure");
  return v;
}
