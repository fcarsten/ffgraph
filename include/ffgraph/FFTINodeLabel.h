//-*-Mode: C++;-*-
#ifndef FF_TI_LABEL
#define FF_TI_LABEL

#if __GNUG__ >= 2
#  pragma interface
#endif

#include <globdefs.h>
#include <String.h>
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFNodeLabel.h>

//---- newclass -----------------------------------------------------------
#define ffTINodeLabel String("FFTINodeLabel")

class FFTINodeLabel: public FFNodeLabel
{
 int dfsNum;
 int compNum;
 int lowNum;
 int stNum;
 BOOL artPkt;
 BOOL stFlag;
 
 FFNode path;
 
public:
 int getDfsNum() { return dfsNum; }
 int getCompNum() { return compNum; }
 int getLowNum() { return lowNum; }
 int getSTNum() { return stNum; }

 BOOL getSTFlag() {return stFlag;}
 void setSTFlag(BOOL f) { stFlag = f; }
  
 BOOL getArtPkt() {return artPkt;}
 void setArtPkt(BOOL f) {artPkt = f;}
 
 FFNode getNext() {return path; }
 
 virtual FFNodeLabel *proto() const 
   {return (FFNodeLabel *)new FFTINodeLabel;}
  
 virtual BOOL parse(const String& );
 virtual String unparse();
 virtual BOOL isInspectable() const {return YES;}
 
 FFTINodeLabel() ;

 virtual BOOL isSaveable() const {return NO;}

 friend class FFTIGraphLabel; 
};

#endif
