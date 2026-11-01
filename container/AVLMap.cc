#if __GNUG__ >= 2
#  pragma implementation
#endif

#include <container/AVLMap.h>

//---- newclass ---------------------------------------------------------

template<class T1, class T2>
AVLMNode<T1,T2>::AVLMNode(T1  h, T2  c, 
		 AVLMNode<T1,T2>* l, AVLMNode<T1,T2>* r)
     :lt(l), rt(r), item(h), cont(c), stat(0) 
{
}

template<class T1, class T2>
AVLMNode<T1,T2>::~AVLMNode() 
{
}

template<class T1, class T2>
 AVLMap<T1,T2>::~AVLMap()
{
  _kill(root);
}

template<class T1, class T2>
 AVLMap<T1,T2>::AVLMap(T2  dflt) :MFMap<T1,T2>(dflt)
{
  root = 0;
}

template<class T1, class T2>
 Pix AVLMap<T1,T2>::first() const
{
  return Pix(leftmost());
}

template<class T1, class T2>
 Pix AVLMap<T1,T2>::last()
{
  return Pix(rightmost());
}

template<class T1, class T2>
 void AVLMap<T1,T2>::next(Pix& i) const
{
  if (i != 0) i = Pix(succ((AVLMNode<T1,T2>*)i));
}

template<class T1, class T2>
 void AVLMap<T1,T2>::prev(Pix& i)
{
  if (i != 0) i = Pix(pred((AVLMNode<T1,T2>*)i));
}

template<class T1, class T2>
 T1& AVLMap<T1,T2>::key(Pix i) const
{
  if (i == 0) this->error("null Pix");
  return ((AVLMNode<T1,T2>*)i)->item;
}

template<class T1, class T2>
 T2& AVLMap<T1,T2>::contents(Pix i) const
{
  if (i == 0) this->error("null Pix");
  return ((AVLMNode<T1,T2>*)i)->cont;
}

template<class T1, class T2>
 void AVLMap<T1,T2>::clear()
{
  _kill(root);
  this->count = 0;
  root = 0;
}

template<class T1, class T2>
 int AVLMap<T1,T2>::contains(T1  key)
{
  return seek(key) != 0;
}




template<class T1, class T2>
AVLMNode<T1,T2>* AVLMap<T1,T2>::leftmost() const
{
  AVLMNode<T1,T2>* t = root;
  if (t != 0) while (t->lt != 0) t = t->lt;
  return t;
}

template<class T1, class T2>
AVLMNode<T1,T2>* AVLMap<T1,T2>::rightmost()
{
  AVLMNode<T1,T2>* t = root;
  if (t != 0) while (t->rt != 0) t = t->rt;
  return t;
}

template<class T1, class T2>
AVLMNode<T1,T2>* AVLMap<T1,T2>::succ(AVLMNode<T1,T2>* t) const
{
  AVLMNode<T1,T2>* r = t->rt;
  if (!rthread(t)) while (!lthread(r)) r = r->lt;
  return r;
}

template<class T1, class T2>
AVLMNode<T1,T2>* AVLMap<T1,T2>::pred(AVLMNode<T1,T2>* t)
{
  AVLMNode<T1,T2>* l = t->lt;
  if (!lthread(t)) while (!rthread(l)) l = l->rt;
  return l;
}


template<class T1, class T2>
Pix AVLMap<T1,T2>::seek(T1  key)
{
  AVLMNode<T1,T2>* t = root;
  if (t == 0)
    return 0;
  for (;;)
  {
		extern int CMP(T1&,T1&);
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



template<class T1, class T2>
void AVLMap<T1,T2>:: _add(AVLMNode<T1,T2>*& t)
{
	extern int CMP(T1&,T1&);
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
      _found_node = new AVLMNode<T1,T2>(*_target_item, this->def);
      ++this->count;
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
        AVLMNode<T1,T2>* l = t->lt;
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
          AVLMNode<T1,T2>* r = l->rt;
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
      _found_node = new AVLMNode<T1,T2>(*_target_item, this->def);
      ++this->count;
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
        AVLMNode<T1,T2>* r = t->rt;
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
          AVLMNode<T1,T2>* l = r->lt;
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

    
template<class T1, class T2>
T2& AVLMap<T1,T2>::operator [] (T1  item)
{
  if (root == 0)
  {
    root = new AVLMNode<T1,T2>(item, this->def);
    ++this->count;
    set_rthread(root, 1);
    set_lthread(root, 1);
    return root->cont;
  }
  else
  {
    _target_item = &item;
    _need_rebalancing = 0;
    _add(root);
    return _found_node->cont;
  }
}


template<class T1, class T2>
void AVLMap<T1,T2>::_del(AVLMNode<T1,T2>* par, AVLMNode<T1,T2>*& t)
{
	extern int CMP(T1&,T1&);
  int comp;
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
      AVLMNode<T1,T2>* s = succ(t);
      if (s != 0 && lthread(s))
        s->lt = t->lt;
      t = t->rt;
      _need_rebalancing = 1;
      return;
    }
    else if (rthread(t))
    {
      _found_node = t;
      AVLMNode<T1,T2>* p = pred(t);
      if (p != 0 && rthread(p))
        p->rt = t->rt;
      t = t->lt;
      _need_rebalancing = 1;
      return;
    }
    else                        // replace item & find someone deletable
    {
      AVLMNode<T1,T2>* p = pred(t);
      t->item = p->item;
      t->cont = p->cont;
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
      AVLMNode<T1,T2>* r = t->rt;
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
        AVLMNode<T1,T2>* l = r->lt;
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
      AVLMNode<T1,T2>* l = t->lt;
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
        AVLMNode<T1,T2>* r = l->rt;
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

        

template<class T1, class T2>
void AVLMap<T1,T2>::del(T1  item)
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

template<class T1, class T2>
void AVLMap<T1,T2>::_kill(AVLMNode<T1,T2>* t)
{
  if (t != 0)
  {
    if (!lthread(t)) _kill(t->lt);
    if (!rthread(t)) _kill(t->rt);
    delete t;
  }
}


template<class T1, class T2>
AVLMap<T1,T2>::AVLMap(const AVLMap<T1,T2>& b) :MFMap<T1,T2>(b.def)
{
  root = 0;
  this->count = 0;
  try {
    for (Pix i = b.first(); i != 0; b.next(i))
      (*this)[b.key(i)] = b.contents(i);
  } catch (...) {
    // A throwing constructor does not call the derived destructor.
    clear();
    throw;
  }
}


template<class T1, class T2>
int AVLMap<T1,T2>::OK()
{
 extern int CMP(T1&,T1&);
 unsigned int v = 1;
  if (root == 0) 
    v = this->count == 0;
  else
  {
   unsigned int n = 1;
    AVLMNode<T1,T2>* trail = leftmost();
    AVLMNode<T1,T2>* t = succ(trail);
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
