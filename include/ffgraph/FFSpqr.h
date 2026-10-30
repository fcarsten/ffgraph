#ifndef FF_SPQR
#define FF_SPQR

#include <globdefs.h>
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFEdgeClass.h>

#include <ffgraph/FFLayoutLabel.h>
#include <ffgraph/FFSpqrNodeLabel.h>
#include <String.h>

class FFTINodeLabel;
class FFTIEdgeLabel;

#define ffSpqrType String("FFSpqr")

class FFSpqr : public FFLayoutLabel
{
 FFGraph spqr;
 FFNode *propAllocNode;
 
 // void _layout(FFSet<FFNode>&);
 int tiNodeIndex;
 int tiEdgeIndex;
 int spqrNodeIndex;
 
 
 inline FFTINodeLabel *tiLabel(FFNode n)
  {return (FFTINodeLabel *)n->getLabel(tiNodeIndex);}
 
 inline FFTIEdgeLabel *tiLabel(FFEdge n)
  {return (FFTIEdgeLabel *)n->getLabel(tiEdgeIndex);}

 inline FFSpqrNodeLabel *spqrLabel(FFNode n)
  {return (FFSpqrNodeLabel *)n->getLabel(spqrNodeIndex);}

 FFNode _markPath(FFNode [], FFSet<FFEdge>&, FFNode, SLList<FFNode>&);
 
 BOOL testEdge(FFNode, FFNode){return YES;}

 FFNode insertEdge(FFNode, FFNode);
 FFNode insertNode(FFNode, FFNode);
 
 FFEdge getFatherEdge(FFNode);
 FFNode leastCommonAncestor(FFNode, FFNode, FFNode&, FFNode&);

 FFNode _preliminaryTransformationS(FFNode, FFNode);
 
/*  FFNode _preliminaryTransformationP(FFNode, FFEdge, FFEdge); */
 void _collectSuccessors(FFNode, FFSet<FFNode>&);
 
 FFNode addSNode();
 FFNode addPNode();
 FFNode addQNode(FFNode, FFNode);
 FFNode addRNode();
 
 void insertSP(FFNode, FFNode);
 FFEdge transform(FFNode, FFNode, FFNode, FFNode);
 FFEdge transformSS(FFNode, FFNode, FFNode, FFNode);
 FFEdge transformAnyP(FFNode, FFNode, FFNode, FFNode);
 /* BOOL tryPoleChain(FFNode, FFNode, FFNode, FFNode, FFNode, BOOL); */
 FFNode findLowestChi(FFNode, FFNode, FFNode);
 void pathCondensation(FFNode, FFNode);
 
 void finalTransformation(FFNode, FFNode);
 void finalTransformation(FFNode, FFNode, FFNode);
 void finalTransformation(FFNode, FFNode, FFNode, FFNode);
 
 void showOrder();

 FFEdge addEdge(FFNode, FFNode);
 void setSource(FFEdge, FFNode);
 void setTarget(FFEdge, FFNode);

 void _insertEdgeToSNode(FFEdge, FFNode);
 
public:
 FFSpqr ();
 virtual ~FFSpqr ();

 // void event(labelEvent, labelEventStruct *);
 virtual BOOL hasOwnInspector() const {return NO;} 
 
 virtual void init(const FFGraph& g);
 virtual BOOL parse(const String&) {return YES;}
 virtual String unparse() {return String("");}
 virtual FFGraphLabel* proto() const {return new FFSpqr;}
 virtual BOOL isInspectable() const {return NO;}
 
 void    layout (FFSet <FFNodeClass *>&) {layout();}
 void    layout ();
};

#endif
