#ifndef FF_NAIV
#define FF_NAIV

#include <globdefs.h>
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFEdgeClass.h>

#include <ffgraph/FFLayoutLabel.h>
#include <String.h>

#define ffNaivType String("FFNaiv")
#define NAIV_X_OFFSET 100
#define NAIV_Y_OFFSET 200

class FFNaiv : public FFLayoutLabel
{
 FFCoord aktuX;
 FFCoord aktuY;
 FFCoord xOffSet;
 FFCoord yOffSet;
 
 // void _layout(FFSet<FFNode>&);
 FFCoord _layout(FFNode);
 int tiIndex;
 
public:
 FFNaiv ();
 virtual ~FFNaiv ();

 void setxoff(FFCoord x) {xOffSet = x;}
 void setyoff(FFCoord y) {yOffSet = y;}
 
 FFCoord getxoff() {return xOffSet;}
 FFCoord getyoff() {return yOffSet;}
 
 void event(labelEvent, labelEventStruct *);
 virtual BOOL hasOwnInspector() const {return YES;} 
 
 virtual void init(const FFGraph& g);
 virtual BOOL parse(const String&) {return YES;}
 virtual String unparse() {return String("");}
 virtual FFGraphLabel* proto() const {return new FFNaiv;}
 virtual BOOL isInspectable() const {return YES;}
 
 void    layout (FFSet <FFNodeClass *>&) {layout();}
  void    layout ();
};

#endif
