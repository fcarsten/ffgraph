#ifndef FF_SUGI
#define FF_SUGI

#include <globdefs.h>
#include <ffgraph/FFGraph.h>
#include <container/AVLSet.h>
#include <container/VOHSet.h>
#include <container/VHSet.h>
#include <container/AVLMap.h>
// #include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFsugiyama.h>
#include <ffgraph/FFLayoutLabel.h>
#include <String.h>

class FFNodeClass;

#define ffSugiType String("FFSugiyama")

class FFSugi : public FFLayoutLabel
{
  FSSugiInfo      info;

  public:
  FFSugi ();
  virtual ~FFSugi ();
  virtual void init(const FFGraph& g);   

 virtual BOOL hasOwnInspector() const {return YES;} 
 virtual BOOL parse(const String&) {return YES;}
 virtual String unparse() {return String("");}
 virtual BOOL isInspectable() const {return YES;}
 virtual FFGraphLabel* proto() const {return new FFSugi;}
 
 virtual FFGraphWindow *getWindow() const {return graph->getWindow();}
 
  
  BOOL    getUseStartNode ();      
  void    setUseStartNode (BOOL);   

  BOOL    getUseEndNodes ();
  void    setUseEndNodes (BOOL);

  BOOL    getKeepEdgePoints ();      
  void    setKeepEdgePoints (BOOL);   
 
  int     getSameSize ();      
  void    setSameSize (int);    

  int     getOrient ();      
  void    setOrient (int);    

  BOOL    getUseGrid ();      
  void    setUseGrid (BOOL);   
 
  BOOL    getLeavesDown ();      
  void    setLeavesDown (BOOL);   
  
  BOOL    getDeterministic ();      
  void    setDeterministic (BOOL);   
  
  int     getCrossingAlg ();      
  BOOL    setCrossingAlg (int);  
  
  int     getIteration1 ();      
  BOOL    setIteration1 (int);

  FFCoord getxoff ();
  BOOL    setxoff (FFCoord);
  FFCoord getyoff ();
  BOOL    setyoff (FFCoord);
  FFCoord getzoff ();
  BOOL    setzoff (FFCoord);
  
  FFCoord getxDistance ();
  BOOL    setxDistance (FFCoord);
  FFCoord getyDistance ();
  BOOL    setyDistance (FFCoord);
  FFCoord getzDistance ();
  BOOL    setzDistance (FFCoord);
  
  FFCoord getDummyOffset ();
  void    setDummyOffset (FFCoord);
  
  int     getDimension ();      
  void    setDimension (int);  

  void    layout (FFSet <FFNodeClass *>&);
  void    layout ();
};

#endif


