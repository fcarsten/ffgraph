#if __GNUG__ >= 2
#  pragma implementation
#endif

#include <ffgraph/FFEdgeClass.h>
#include <ffgraph/FFTIEdgeLabel.h>
#include <sstream>

//---- newclass ---------------------------------------------------------

BOOL FFTIEdgeLabel::parse(const String&)
{
 return YES;
}

FFTIEdgeLabel::FFTIEdgeLabel()
{
 _setType(ffTIEdgeLabel);
 type = TI_NO;
 stFlag = NO;
 
}

String FFTIEdgeLabel::unparse() 
{
 std::ostringstream nase;
 nase << "Kantentyp: ";
 
 switch(type) {
 case TI_TREE:
  nase << "Baumkante";
  break;
 case TI_FORWARD:
  nase << "Forwärtskante";
  break;
 case TI_BACKWARD:
  nase << "Rückwärtskante";
  break;
 case TI_CROSS:
  nase << "Crosskante";
  break;
 case TI_NO:
  nase << "Undefiniert";
 }
 
 return String(nase.str());
}
