#include <ffgraph/FFLabelClass.h>

void FFLabelClass::_setType(const String& s)
{
  if(_type != "")
    _type += String(":");
    
  _type += s;
}

String FFLabelClass::baseType() const
{
  String tmpS(_type);
  
  if(tmpS.contains(":"))
    return String(tmpS.after(tmpS.index(":",-1)));
    
  return tmpS;
}
