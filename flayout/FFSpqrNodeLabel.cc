#if __GNUG__ >= 2
#  pragma implementation
#endif

#include <assert.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFSpqrNodeLabel.h>

//---- newclass ---------------------------------------------------------

FFSpqrNodeLabel::FFSpqrNodeLabel()
{
 _setType(ffSpqrNodeLabelType);
 spqrType = NONE;
 firstNode = 0;
 secondNode = 0;
}

void FFSpqrNodeLabel::del(FFEdge e)
{
 for(Pix i=order.first(); i!= 0; order.next(i))
  if(order(i)==e)
   {
    del(i);
    return;
   }
}

void FFSpqrNodeLabel::delInterval(Pix eins, Pix zwei)
{
 while(eins != zwei)
  {
   order.del(eins);
  }
}

void FFSpqrNodeLabel::_nameQ()
{
 String nase;
 if(firstNode != NULL)
  nase += String(dec(firstNode->getValue())) + "/";
 if(secondNode != NULL)
  nase += String(dec(secondNode->getValue()));
  
 node->setName(nase);
}

FFSpqrNodeLabel *FFSpqrNodeLabel::spqrLabel(FFNode n)
{
 if(index<0)
  {
   cerr << "Wrong index";
   return NULL;
  }
 return (FFSpqrNodeLabel *)n->getLabel(index);
}

FFNode FFSpqrNodeLabel::getFirst()
{
 if(firstNode != NULL)
  return firstNode;

 const FFSet<FFEdge>& nach=node->transToSucc();
 if((spqrType != Q_NODE) && (nach.length() == 0))
  return NULL;

 switch(spqrType)
  {
  case S_NODE:
   return spqrLabel(order.front()->target())->getFirst();
  case R_NODE:
  case P_NODE:
   return spqrLabel(nach(nach.first())->target())->getFirst();
  case Q_NODE:
  default:
   return NULL;
  }
 return NULL;
}

FFNode FFSpqrNodeLabel::getSecond()
{
 if(secondNode != NULL)
  return secondNode;

 const FFSet<FFEdge>& nach=node->transToSucc();
 if((spqrType != Q_NODE) && (nach.length() == 0))
  return NULL;

 switch(spqrType)
  {
  case S_NODE:
   return spqrLabel(order.rear()->target())->getSecond();
  case R_NODE:
  case P_NODE:
   return spqrLabel(nach(nach.first())->target())->getSecond();
  case Q_NODE:
  default:
   return NULL;
  }
 return NULL;
}

void FFSpqrNodeLabel::insertAfter(FFEdge target, FFEdge neu)
{
 assert(spqrType == S_NODE);
 
 if(target == NULL)
  {
   order.append(neu);
   return;
  }
 
 for(Pix i=order.first(); i!= 0; order.next(i))
  {
   FFEdge tmpE= order(i);
   if(tmpE == target)
    {
     order.ins_after(i, neu);
     return;
    }
  }
}

void FFSpqrNodeLabel::updatePoles()
{
 if(spqrType != Q_NODE)
  {
   firstNode= 0;
   secondNode= 0;
  }
}

