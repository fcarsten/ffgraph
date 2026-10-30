#if __GNUG__ >= 2
#  pragma implementation
#endif

#include <container/AVL.h>

//---- newclass ---------------------------------------------------------

template<class T>
 int bf(T* t)
{
  return t->stat & AVLBALANCEMASK;
}

template<class T>
 void set_bf(T* t, int b)
{
  t->stat = (t->stat & ~AVLBALANCEMASK) | (b & AVLBALANCEMASK);
}


template<class T>
 int rthread(T* t)
{
  return t->stat & RTHREADBIT;
}

template<class T>
void set_rthread(T* t, int b)
{
  if (b)
    t->stat |= RTHREADBIT;
  else
    t->stat &= ~RTHREADBIT;
}

template<class T>
 int lthread(T* t)
{
  return t->stat & LTHREADBIT;
}

template<class T>
 void set_lthread(T* t, int b)
{
  if (b)
    t->stat |= LTHREADBIT;
  else
    t->stat &= ~LTHREADBIT;
}

/*
 traversal primitives
*/
