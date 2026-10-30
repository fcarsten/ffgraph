#ifndef FF_KAESE
#define FF_KAESE

#include <globdefs.h>
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFEdgeClass.h>
#include <container/AVLMap.h>
#include <ffgraph/FFLayoutLabel.h>
#include <String.h>

#define ffKaeseType String("FFKaese")

struct Cuts 
{
 int inner;
 int outer;
};

class FFKaese : public FFLayoutLabel
{
 FFSet<FFNode> setA;
 FFSet<FFNode> setB;
 
 AVLMap<FFNode, Cuts*> cutMap;
 FFNode * nucleus;
 int *clusterSize;
 int numClusters;
 
 void doLayout();
 void updateCuts();
 void thinGraph();
 void placeNodes();
 void mergeClusters(int, int);
 int mostCommonNeighbours(FFNode, int);
 void doLocalOptimize();
 
public:
 FFKaese();
 ~FFKaese();
 
 virtual BOOL hasOwnInspector() const {return NO;} 
 
 virtual void init(const FFGraph& g);
 virtual BOOL parse(const String&) {return YES;}
 virtual String unparse() {return String("");}
 virtual FFGraphLabel* proto() const {return new FFKaese;}
 virtual BOOL isInspectable() const {return NO;}
 
 void layout (FFSet <FFNodeClass *>&) {layout();}
 void layout ();
};

#endif
