#include <cstdint>

template<class T>
static inline unsigned int ffgraph_pointer_hash(T* pointer)
{
  std::uintptr_t bits = reinterpret_cast<std::uintptr_t>(pointer);
  if constexpr (sizeof(std::uintptr_t) > sizeof(unsigned int))
    bits ^= bits >> (sizeof(unsigned int) * 8);
  return static_cast<unsigned int>(bits);
}
#include <String.h>
#include <builtin.h>

// int
// EQ (String& a, String& b)
// {
//   return (a == b);
// }

// int
// CMP (String& a, String& b)
// {
//   if (a <= b)
//     {
//       if (a == b)
// 	return (0);
//       else
// 	return (-1);
//     }
//   else
//     return (1);
// }

// int
// EQ (int& a, int& b)
// {
//   return (a == b);
// }

// int
// CMP (int& a, int& b)
// {
//    return  ( (a <= b)? ((a == b)? 0 : -1) : 1 );
// }

/*
 * hashing method: double hash based on high bits of hash fct,
 * followed by linear probe. Can't do too much better if table
 * sizes not constrained to be prime.
*/



static
inline
unsigned int doublehashinc(unsigned int h, unsigned int s)
{
  unsigned int dh =  ((h / s) % s);
  return (dh > 1)? dh : 1;
}

class FFNodeClass;
class FFEdgeClass;

class FFGraphLabel;
class FFNodeLabel;
class FFEdgeLabel;

class FFGraphicNode;
class FFGraphicEdge;

class FFLayoutLabel;

// Strings

inline int EQ(String& s1, String& s2)
{
  return(s1 == s2);
}

inline int CMP(String& a, String& b)
{
  return ( ((a) <= (b))? (((a) == (b))? 0 : -1) : 1 );
}

inline unsigned int HASH(String& s)
{
 return hashpjw(s);
}

// Integer

inline int EQ(int& s1, int& s2)
{
  return(s1 == s2);
}

inline int CMP(int& a, int& b)
{
  return ( ((a) <= (b))? (((a) == (b))? 0 : -1) : 1 );
}

inline unsigned int HASH(int& i)
{
 return static_cast<unsigned int>(i);
}

// FFLayoutLabel *

inline int EQ(FFLayoutLabel *&n1, FFLayoutLabel *&n2)
{
 return n1==n2;
}

inline int CMP(FFLayoutLabel *&a, FFLayoutLabel *&b)
{
 return ( ((a) <= (b))? (((a) == (b))? 0 : -1) : 1 );
}

inline unsigned int HASH(FFLayoutLabel *&a)
{
 return ffgraph_pointer_hash(a);
}

// FFNodeLabel *

inline int EQ(FFNodeLabel *&n1, FFNodeLabel *&n2)
{
 return n1==n2;
}

inline int CMP(FFNodeLabel *&a, FFNodeLabel *&b)
{
 return ( ((a) <= (b))? (((a) == (b))? 0 : -1) : 1 );
}

inline unsigned int HASH(FFNodeLabel *&a)
{
 return ffgraph_pointer_hash(a);
}

// EdgeLabel *

inline int EQ(FFEdgeLabel *&n1, FFEdgeLabel *&n2)
{
 return n1==n2;
}

inline int CMP(FFEdgeLabel *&a, FFEdgeLabel *&b)
{
 return ( ((a) <= (b))? (((a) == (b))? 0 : -1) : 1 );
}

inline unsigned int HASH(FFEdgeLabel *&a)
{
 return ffgraph_pointer_hash(a);
}

// FFEdgeClass *

inline int EQ(FFEdgeClass *&n1, FFEdgeClass *&n2)
{
	return n1==n2;
}

inline int CMP(FFEdgeClass *& a, FFEdgeClass *& b)
{
	return ( ((a) <= (b))? (((a) == (b))? 0 : -1) : 1 );
}

inline unsigned int HASH(FFEdgeClass *&n)
{
 return ffgraph_pointer_hash(n);
}

// FFGraphLabel *

inline int EQ(FFGraphLabel *&n1, FFGraphLabel *&n2)
{
 return n1==n2;
}

inline int CMP(FFGraphLabel *&a, FFGraphLabel *&b)
{
 return ( ((a) <= (b))? (((a) == (b))? 0 : -1) : 1 );
}

inline unsigned int HASH(FFGraphLabel *&a)
{
 return ffgraph_pointer_hash(a);
}

// FFNodeClass *

inline int EQ(FFNodeClass *&n1, FFNodeClass *&n2)
{
 return n1==n2;
}

inline int CMP(FFNodeClass *& a, FFNodeClass *& b)
{
 return ( ((a) <= (b))? (((a) == (b))? 0 : -1) : 1 );
}

inline unsigned int HASH(FFNodeClass *&n)
{
 return ffgraph_pointer_hash(n);
}

// FFGraphicEdge *

inline int EQ(FFGraphicEdge *&n1, FFGraphicEdge *&n2)
{
 return n1==n2;
}

inline int CMP(FFGraphicEdge *&a, FFGraphicEdge *&b)
{
 return ( ((a) <= (b))? (((a) == (b))? 0 : -1) : 1 );
}

inline unsigned int HASH(FFGraphicEdge *&a)
{
 return ffgraph_pointer_hash(a);
}

// FFGraphicNode *

inline int EQ(FFGraphicNode *&n1, FFGraphicNode *&n2)
{
 return n1==n2;
}

inline int CMP(FFGraphicNode *&a, FFGraphicNode *&b)
{
 return ( ((a) <= (b))? (((a) == (b))? 0 : -1) : 1 );
}

inline unsigned int HASH(FFGraphicNode *&a)
{
 return ffgraph_pointer_hash(a);
}

inline unsigned int HASH(class parse_type *&a)
{
 return ffgraph_pointer_hash(a);
}

inline int EQ(class parse_type *&n1, class parse_type *&n2)
{
 return n1==n2;
}

inline unsigned int HASH(class parse_result *&a)
{
 return ffgraph_pointer_hash(a);
}

inline int EQ(class parse_result *&n1, class parse_result *&n2)
{
 return n1==n2;
}

// in IAbs.cc
extern unsigned int HASH(class IAbs *&a);

extern int EQ(class IAbs *&n1, class IAbs *&n2);

// longs

inline int EQ(long& s1, long& s2)
{
  return(s1 == s2);
}

inline int CMP(long& a, long& b)
{
  return ( ((a) <= (b))? (((a) == (b))? 0 : -1) : 1 );
}
