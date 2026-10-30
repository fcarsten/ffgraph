#if __GNUG__ >= 2
#  pragma implementation
#endif

#include <ffgraph/FFSubGraphLabel.h>

//---- newclass ---------------------------------------------------------

BOOL FFSubGraphLabel::parse(const String& s)
{
 fileName=s;
 fileName.gsub(String('\n'), String(""));
 
 if(fileName != "")
  {
   derGraph->addNodeLabel(new FFSubGraphLabel);
   derGraph->createGraphic(NULL);
   cerr << "Loading Graph :" << fileName << ":" << endl;
   
   derGraph->load(fileName, YES);
   hasGraph=YES;
  }
 return YES;
}

String FFSubGraphLabel::unparse() 
{
 return fileName;
}
