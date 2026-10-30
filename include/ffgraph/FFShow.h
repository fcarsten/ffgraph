#ifndef FF_SHOW
#define FF_SHOW

#include <globdefs.h>
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFEdgeClass.h>
#include <ffgraph/FFLayoutLabel.h>
#include <String.h>

#define ffShowType String("FFShow")


class FFShow : public FFLayoutLabel
{
 void doLayout();
 
public:
 FFShow();

 ~FFShow();
 
 virtual BOOL hasOwnInspector() const {return NO;} 
 
 virtual void init(const FFGraph& g);
 virtual BOOL parse(const String&) {return YES;}
 virtual String unparse() {return String("");}
 virtual FFGraphLabel* proto() const {return new FFShow;}
 virtual BOOL isInspectable() const {return NO;}
 
 void layout (FFSet <FFNodeClass *>&) {layout();}
 void layout ();
};

#endif
