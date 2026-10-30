#ifdef __GNUG__
#pragma implementation
#endif
#include "container/SLStack.h"

template<class T>
void SLStack<T>::clear() 
{
  p.clear();
}

template<class T>
void SLStack<T>::del(T item)
{
 for(Pix i= p.first(); i!= 0; p.next(i))
  {
   if(p(i) == item)
    {
     p.del(i);
     break;
    }
  }
}
