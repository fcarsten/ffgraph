//-*-Mode: C++;-*-
#ifndef FF_SPQR_NODE_LABEL
#define FF_SPQR_NODE_LABEL

#if __GNUG__ >= 2
#  pragma interface
#endif

#include <globdefs.h>
#include <String.h>
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFEdgeClass.h>
#include <ffgraph/FFNodeLabel.h>
#include <DLList.h>

//---- newclass -----------------------------------------------------------
#define ffSpqrNodeLabelType String("FFSPQRNodeLabel")

typedef enum 
{
 NONE,
 S_NODE,
 P_NODE,
 Q_NODE,
 R_NODE
} SpqrType;

class FFSpqrNodeLabel: public FFNodeLabel
{
 void *skeleton;
 FFNode firstNode;
 FFNode secondNode;
 
 SpqrType spqrType;
 DLList<FFEdge> order;

 void _nameQ();
 
 FFSpqrNodeLabel *spqrLabel(FFNode);
 
public:

 void updatePoles();
 const DLList<FFEdge>& edgeToSucc() {return order;}
 void delInterval(Pix, Pix);
 void del(Pix& i) {order.del(i);}
 void del(FFEdge n);

 void insertAfter(FFEdge, FFEdge);
 void insertBefore(Pix i, FFEdge e) {order.ins_before(i,e);}

 void append(FFEdge e) {order.append(e); }
 
 FFNode getFirst();
 void setFirst(FFNode n) {firstNode=n; _nameQ();}

 FFNode getSecond();
 void setSecond(FFNode n) {secondNode=n; _nameQ();}

 BOOL contains(FFNode n) 
  {if((n == firstNode) || (n == secondNode)) return YES; return NO;}

 SpqrType getSpqrType() {return spqrType;}
 void setSpqrType(SpqrType t) {spqrType = t;}
 
 virtual FFNodeLabel *proto() const 
   {return (FFNodeLabel *)new FFSpqrNodeLabel;}
  
 virtual BOOL parse(const String& ) {return YES;}
 virtual String unparse() {return String(dec(order.length()));}

 virtual BOOL isInspectable() const {return YES;}
 
 FFSpqrNodeLabel() ;

 virtual BOOL isSaveable() const {return NO;}
};

#endif
