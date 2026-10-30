// -*- C++ -*-
//
// author:      Andreas Classen
//
// file:        Tuple.hh
//
// $Id: Tuple.hh,v 0.5.0.1 1995/10/20 12:12:56 friedric Exp $
//

#ifndef _Tuple_hh
#define _Tuple_hh

#ifdef __GNUG__
#pragma interface
#endif // __GNUG__

#include <String.h>
#include <iostream.h>

template<class T1, class T2>
class Tuple
{
public:
  Tuple() {};
  Tuple(const T1& t1, const T2& t2)
    : firstEntry(t1), secondEntry(t2) {};
  virtual ~Tuple() {};

  void setFirst(const T1& t1) { firstEntry = t1; }
  void getFirst(T1& t1) const { t1 = firstEntry; }
  T1 first() const { return firstEntry; }
  void setSecond(const T2& t2) { secondEntry = t2; }
  void getSecond(T2& t2) const { t2 = secondEntry; }
  T2 second() const { return secondEntry; }

  int operator == (const Tuple& m) const { return (firstEntry == m.firstEntry &&
						   secondEntry == m.secondEntry); }
  int operator <= (const Tuple& m) const { return (firstEntry <= m.firstEntry); }

protected:

  T1 firstEntry;
  T2 secondEntry;
    
};

template<class T1, class T2, class T3>
class Triple
{
public:
  Triple() {};
  Triple(const T1& t1, const T2& t2, const T3& t3)
    : firstEntry(t1), secondEntry(t2), thirdEntry(t3) {};
  virtual ~Triple() {};

  void setFirst(const T1& t1) { firstEntry = t1; }
  void getFirst(T1& t1) const { t1 = firstEntry; }
  T1 first() const { return firstEntry; }
  T1& firstR() { return firstEntry; }

  void setSecond(const T2& t2) { secondEntry = t2; }
  void getSecond(T2& t2) const { t2 = secondEntry; }
  T2 second() const { return secondEntry; }
  T2& secondR() { return secondEntry; }

  void setThird(const T3& t3) { thirdEntry = t3; }
  void getThird(T3& t3) const { t3 = thirdEntry; }
  T3 third() const { return thirdEntry; }
  T3& thirdR() { return thirdEntry; }

  int operator == (const Triple& m) const { return (firstEntry == m.firstEntry &&
                                                    secondEntry == m.secondEntry &&
                                                    thirdEntry == m.thirdEntry); }
  int operator <= (const Triple& m) const { return (firstEntry <= m.firstEntry); }

protected:

  T1 firstEntry;
  T2 secondEntry;
  T3 thirdEntry;
    
};

#endif // _Tuple_hh
// EOF
