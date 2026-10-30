#if __GNUG__ >= 2
#  pragma implementation
#endif

#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFTINodeLabel.h>
#include <sstream>

//---- newclass ---------------------------------------------------------

BOOL FFTINodeLabel::parse(const String&)
{
 return YES;
}

FFTINodeLabel::FFTINodeLabel()
{
 _setType(ffTINodeLabel);
 dfsNum=0;
 compNum=0;
 lowNum=0;
 stNum=0;
 artPkt = NO;
 stFlag = NO;
 
 path= 0;
}

String FFTINodeLabel::unparse() 
{
 std::ostringstream nase;
 nase << "DFS Nummer: " << dfsNum <<
  "\nComp Nummer: " << compNum <<
  "\nLow Nummer: " << lowNum <<
  "\nArtikulationspunkt: " << (artPkt ? "Ja" : "Nein") <<
  "\n\nST Nummer: " << stNum;
 return String(nase.str());
}
