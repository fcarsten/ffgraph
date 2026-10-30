#include <iostream>
#ifdef __GNUG__
#pragma implementation
#endif
#include <container/MFStack.h>

template<class Type>
void MFStack<Type>::error(const char* msg)
{
  std::cerr << "Stack: " << msg << std::endl;
}
