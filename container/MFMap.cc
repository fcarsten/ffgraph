#if __GNUG__ >= 2
#  pragma implementation
#endif

#include <container/MFMap.h>
#include <iostream>

//---- newclass ---------------------------------------------------------


template<class T1, class T2>
MFMap<T1,T2>::~MFMap() {}

template<class T1, class T2>
 int MFMap<T1,T2>::length() const
{
  return count;
}

template<class T1, class T2>
 int MFMap<T1,T2>::empty() const
{
  return count == 0;
}

template<class T1, class T2>
 T2& MFMap<T1,T2>::dflt()
{
  return def;
}

template<class T1, class T2>
 MFMap<T1,T2>::MFMap(T2  dflt) :def(dflt)
{
  count = 0;
}



template<class T1, class T2>
Pix MFMap<T1,T2>::seek(T1  item)
{
	extern int EQ(T1&,T1&);
	Pix i;
  for (i = first(); i != 0 && !(EQ(key(i), item)); next(i));
  return i;
}

template<class T1, class T2>
int MFMap<T1,T2>::owns(Pix idx)
{
  if (idx == 0) return 0;
  for (Pix i = first(); i; next(i)) if (i == idx) return 1;
  return 0;
}

template<class T1, class T2>
void MFMap<T1,T2>::clear()
{
  Pix i = first(); 
  while (i != 0)
  {
    del(key(i));
    i = first();
  }
}

template<class T1, class T2>
int MFMap<T1,T2>::contains (T1  item)
{
  return seek(item) != 0;
}


template<class T1, class T2>
void MFMap<T1,T2>::error(const char* msg) const
{
	std::cerr << msg << std::endl;
  // (*lib_error_handler)("Map", msg);
}
