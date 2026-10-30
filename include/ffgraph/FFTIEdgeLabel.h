//-*-Mode: C++;-*-
#ifndef FF_TI_EDGE_LABEL
#define FF_TI_EDGE_LABEL

#if __GNUG__ >= 2
#  pragma interface
#endif

#include <globdefs.h>
#include <String.h>
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFEdgeClass.h>
#include <ffgraph/FFEdgeLabel.h>

typedef enum 
{
 TI_NO,
 TI_TREE,
 TI_FORWARD,
 TI_BACKWARD,
 TI_CROSS
} tiEdgeType;


//---- newclass -----------------------------------------------------------
#define ffTIEdgeLabel String("FFTIEdgeLabel")

class FFTIEdgeLabel: public FFEdgeLabel
{
 tiEdgeType type;
 BOOL stFlag;
 
public:
 tiEdgeType getTiType() { return type; }
 void setTiType(tiEdgeType t) {type= t;}

 BOOL getSTFlag() { return stFlag; }
 void setSTFlag(BOOL t) {stFlag = t;}

 virtual FFEdgeLabel *proto() const 
   {return (FFEdgeLabel *)new FFTIEdgeLabel;}
  
 virtual BOOL parse(const String& );
 virtual String unparse();
 virtual BOOL isInspectable() const {return YES;}
 
 FFTIEdgeLabel() ;

 virtual BOOL isSaveable() const {return NO;}

 friend class FFTIGraphLabel; 
};

#endif
