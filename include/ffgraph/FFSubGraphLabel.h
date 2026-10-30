//-*-Mode: C++;-*-
#ifndef FF_SUBGRAPHLABEL
#define FF_SUBGRAPHLABEL

#if __GNUG__ >= 2
#  pragma interface
#endif

#include <globdefs.h>
#include <String.h>
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFNodeLabel.h>

//---- newclass -----------------------------------------------------------
#define ffSubGraphLabel String("FFSubGraphLabel")

class FFSubGraphLabel: public FFNodeLabel
{
protected:

 String fileName;
 BOOL hasGraph;
 FFGraph derGraph;

public:

 void setGraph(FFGraph g) {derGraph = g; }
 FFGraph& getGraph() {return derGraph;}

 virtual FFNodeLabel *proto() const 
   {return (FFNodeLabel *)new FFSubGraphLabel;}
  
 virtual BOOL parse(const String& );
 virtual String unparse();
 virtual BOOL isInspectable() const {return YES;}
 
 FFSubGraphLabel() 
   { _setType(ffSubGraphLabel); hasGraph = NO;}
 
 virtual BOOL isSaveable() const {return YES;}
  //	virtual const String& type() const {return ffSubGraphLabel;}
};

#endif
