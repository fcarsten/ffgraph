#ifndef FF_RANDOM
#define FF_RANDOM

#include <globdefs.h>
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFEdgeClass.h>
#include <ffgraph/FFLayoutLabel.h>
#include <String.h>

#define ffRandomType String("FFRandom")


class FFRandom : public FFLayoutLabel
{
 void doLayout();
 
public:
 FFRandom();

 ~FFRandom();
 
 virtual BOOL hasOwnInspector() const {return NO;} 
 
 virtual void init(const FFGraph& g);
 virtual BOOL parse(const String&) {return YES;}
 virtual String unparse() {return String("");}
 virtual FFGraphLabel* proto() const {return new FFRandom;}
 virtual BOOL isInspectable() const {return NO;}
 
 void layout (FFSet <FFNodeClass *>&) {layout();}
 void layout ();
};

#endif
