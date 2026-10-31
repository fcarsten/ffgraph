/*
// Copyright 1994 Carsten Friedrich, Universitaet Passau
//
// $Author: friedric $
// $Date: 1996/05/09 09:42:31 $
// $RCSfile: FFGraphicNode.cc,v $
// $Revision: 1.3 $
// $State: Exp $
//
*/

#ifdef RCSID
#undef RCSID
#endif
#define RCSID "$Author: friedric $ $RCSfile: FFGraphicNode.cc,v $ $Date: 1996/05/09 09:42:31 $ $Revision: 1.3 $ $State: Exp $"

#include <globdefs.h>
#include <container/AVLSet.h>
#include <container/VOHSet.h>
#include <String.h>
#include <ffgraph/FFGraphClass.h>

#include <ffgraph/FFGraphicNode.h>
#include <ffgraph/FFEdgeClass.h>
#include <ffgraph/FFGraphicEdge.h>

#include <ffgraph/FFGraphWindow.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/ffError.h>


// void FFGraphicNode::setShape(ffShape s)
// {
//  shape = s;
// }

String shape2Str(ffShape shape)
{
 if(shape == FF_CIRCLE)
  return String("Circle");
 else if(shape == FF_OVAL)
  return String("Oval");
 else if(shape == FF_SQUARE)
  return String("Square");
 else if(shape == FF_RECTANGLE)
  return String("Rectangle");
 else if(shape == FF_RAUTE)
  return String("Raute");
 else if(shape == FF_IMAGE)
  return String("Icon");
 else
  ffError(INTERNAL_ERROR, String("Unknown shape"));
 return String("Custom");
 
}

ffShape str2Shape(const String& shapeStr)
{
 if(shapeStr == "Circle")
  return FF_CIRCLE;
 else if(shapeStr == "Oval")
  return FF_OVAL;
 else if(shapeStr == "Square")
  return FF_SQUARE;
 else if(shapeStr == "Rectangle")
  return FF_RECTANGLE;
 else if(shapeStr == "Raute")
  return FF_RAUTE;
 else if(shapeStr == "Icon")
  return FF_IMAGE;
 else
  ffError(INTERNAL_ERROR, String("Unknown shape ")+ shapeStr);
 
 return FF_CUSTOM_SHAPE;
}


String FFGraphicNode::getColor()
{
  return farbe;
}

BOOL FFGraphicNode::isOval()
{
 if((shape == FF_CIRCLE) ||
    (shape == FF_OVAL))
  return YES;
 return NO;
}

void FFGraphicNode::setColor(const String& s)
{
 farbe=s;
 
 if(fenster && tkNode.length())
  {
   fenster->changeColor(this, farbe);
  }
 return;
}

void FFGraphicNode::setName(const String& s)
{
	if(fenster && tkLabel.length())   
		fenster->changeLabel(this, s);
}


void FFGraphicNode::changeToIcon(const String& s, const String& s2)
{
 if((s == "") || (s == "__NoIcon__"))
  return;
		
 window = "";
 icon = s;
 selIcon = s2;

 if(fenster && tkNode.length())
  {
   String tmpS =fenster->changeToIcon(this, icon, selIcon);
   if(tmpS.length() != 0) // War vorher kein Icon
    {
     tkNode=tmpS;
     shape = FF_IMAGE;
 
     if(!isOval())
      setRange(fenster->getRange(this));
     
     fenster->updateGraph(this);
     _wallNodeChanged();
    }
   else // War schon vorher ein Icon
    {
     // cerr << "Got empty tk Id while changing to Icon" << endl;
    }
  }
 return;
}

void FFGraphicNode::changeToFigure(ffShape s)
{
 // cerr << "void FFGraphicNode::changeToFigure(ffShape s)" << endl;
 
//  if (shape == s)
//   return;
 
 shape=s;
 
 switch(s)
  {
  case FF_WINDOW:
   {
    changeToWindow();
    break;
   }
  case FF_IMAGE:
   {
    changeToIcon(icon, selIcon);
    setMask(mask, selMask);
    break;
   }
  default:
   {
    figure=s;
    
    window = "";
    icon = "__NoIcon__";
    
    if(fenster && tkNode.length()) // tkNode notwendig, da neues item den tag
     {                             // des alten bekommen soll
      String tmpS =fenster->changeToFigure(this, shape);
      tkNode=tmpS;
      if(tmpS.length() != 0)
       {
        if(shape < FF_FIGURE)
         setRange(fenster->getRange(this));
        else
         fenster->changeColor(this, farbe);

        fenster->updateGraph(this);
        _wallNodeChanged();
       }
     }
   }
  }
 
 return;
}

void FFGraphicNode::changeToWindow()
{
 if((window == "") && fenster && tkNode.length())
  {
   //  icon = "";
   String tmpS = fenster->changeToWindow(this);
   //    cerr << "tmpS " << tmpS << endl;
   if(tmpS.length() != 0)
    {
     tkNode= tmpS.after(tmpS.index(RXwhite, -1));
     window= tmpS.before(tmpS.index(RXwhite, -1));
     shape= FF_WINDOW;
    }	
   if(shape < FF_FIGURE)
    setRange(fenster->getRange(this));
   //    _wallNodeChanged();  setRange does this already
   //    fenster->updateGraph(this); setRange should do this
  }
 //  cerr << "window is " << window << endl;
 //  cerr << "tkNode ist " << tkNode << endl;
}

void FFGraphicNode::setMask(const String& m1, const String& m2)
{
 mask = m1;
 selMask = m2;
 
 if(fenster && tkNode.length())
  {
   fenster->changeMask(this, mask, selMask);
  }
 return;
}

// String& FFGraphicNode::getIcon()
// {
//  return icon;
// }

// String& FFGraphicNode::getMask()
// {
//  return mask;
// }

void FFGraphicNode::setWidth(FFCoord i)
{
  range.v[0] = i;
  return;
}

void FFGraphicNode::setHeight(FFCoord i)
{
  range.v[1] = i;
  return;
}

void FFGraphicNode::setDepth(FFCoord i)
{
  range.v[2] = i;
  return;
}

void FFGraphicNode::assign(const FFLabelClass *l)
{
 // cerr << "void FFGraphicNode::assign(const FFLabelClass *l)" << endl;
 
 if(l->type() != type())
  return;
 FFGraphicNode *nl = (FFGraphicNode *)l;
 
 icon = nl->getIcon();
 selIcon =  nl->getSelIcon();
 mask = nl->getMask();
 selMask = nl->getSelMask();

 coords = nl->getCoords();
 range = nl->getRange();
 farbe = nl->getColor();
 figure = nl->getFigure();

 changeToFigure(nl->getShape());
}

void FFGraphicNode::event(labelEvent e, labelEventStruct *theEvent)
{
  switch(e)
   {
   case NAME_CHANGE:
    {
     setName(node->getName());
     break;
    }
   case ADD_GRAPHLABEL:
    {	
     if(!fenster)
      {
       // cerr << "New Window arrived ? " << endl;
       fenster= node->getGraph()->getWindow();
       if(fenster != NULL)
        {
         //            cerr << "Joining new Window" << endl;
         
         tkNode = fenster->addNode(this);
         tkLabel = fenster->addLabel(this);
         if(getIcon() != "__NoIcon__")
          changeToIcon(getIcon(), getSelIcon());
         if(getMask() != "__NoMask__")
          setMask(getMask(), getSelMask());
        }  
      }
     break;
    }
   case DEL_GRAPHLABEL:
    {
     if (theEvent->index == node->getGraph()->getGraphWindowIndex())
      {
       if(fenster )
        {
         if(getNodeWindow() != "__NoIcon__")
          {
           if(getIcon() != "__NoIcon__")
            changeToIcon(getIcon(), getSelIcon());
           else
            changeToFigure(FF_CIRCLE);
          }
         
         fenster->remove(this);
         fenster = 0;
        }
       window = "";
       tkNode = "";
       tkLabel = "";
      }
     break;
    }
   default:
    {
     break;
    }
   }
}

	
FFGraphicNode::FFGraphicNode() : farbe("black"), icon("__NoIcon__"), 
 selIcon("__NoIcon__"), mask("__NoMask__"), selMask("__NoMask__"), 
 coords(100,100,0), range(RADIUS, RADIUS, RADIUS)
{
  fenster= 0;
  shape= FF_CIRCLE;
  figure= FF_CIRCLE;
  _setType(ffNodeWindowType);
}

void FFGraphicNode::init(FFNodeClass *n)
{  
 // cerr << "void FFGraphicNode::init(FFNodeClass *n)" << endl;
 
  FFNodeLabel::init(n);

  fenster= node->getGraph()->getWindow();
  if(node->getGraph()->getGraphicNodeIndex() <= 0)
   node->getGraph()->setNodeIndex(node->getGraph()->getNodeLabelIndex(type()));
  
   if(fenster != NULL)
    {
     tkNode = fenster->addNode(this);
     tkLabel = fenster->addLabel(this);
     if(shape < FF_FIGURE)
      setRange(fenster->getRange(this));
     if(fenster->getUpdate())
      fenster->updateGraph(this);
    }  
  return;
}

FFGraphicNode::~FFGraphicNode()
{
 if(fenster)
  fenster->remove(this);
}

void FFGraphicNode::setWindow(FFGraphWindow *i)
{ 
 if(fenster == i)
  return; 
 
 if(fenster != NULL)
  fenster->remove(this);      
 
 fenster=i;  
 
 if(fenster != NULL)
  {
   tkNode = fenster->addNode(this);
   tkLabel = fenster->addLabel(this);
   changeToFigure(shape);
  }
 else
  {
   tkNode="";
   tkLabel="";
  }
 return;
}

const String& FFGraphicNode::getId() const
{
  return tkNode;
}

// BOOL FFGraphicNode::isScalable()
// {
//   if( (icon != "__NoIcon__") || (window != ""))
//     return NO;
  
//   return YES;
// }

// BOOL FFGraphicNode::isCircle()
// {
//   if( (icon != "__NoIcon__") || (window != ""))
//     return NO;
  
//   return YES;
// }

const String& FFGraphicNode::getLabelId() const
{
  return tkLabel;
}

//void FFGraphicNode::removeGraphic()
//{
//  if(fenster)
//    {
//      fenster->remove(this);
//      tkNode = "";
//      tkLabel = "";
//      
//      fenster= 0;
//    }
//  return;
//}

void FFGraphicNode::updateGraphic()
{
  moveTo(coords);
  return;
}

void FFGraphicNode::_wallNodeChanged()
{
  const FFSet<FFEdge>& nodeSet= node->transFromPred();

  FFEdge tmpNi;
  
  Pix i;
  for(i=nodeSet.first(); i!=0; nodeSet.next(i))
    {
     tmpNi=nodeSet(i);
     if((*tmpNi)() != NULL)
      (*tmpNi)()->targetChanged();
    }
  
  const FFSet<FFEdge>& nodeSet2 = node->transToSucc();
  
  for(i=nodeSet2.first(); i!=0; nodeSet2.next(i))
   {
    tmpNi=nodeSet2(i);
    if((*tmpNi)() != NULL)
     (*tmpNi)()->sourceChanged();
   }
  
  if((fenster != NULL) && (fenster->getUpdate()))
   fenster->update();
}

void FFGraphicNode::moveTo(const FFPoint& point)
{
 coords=point;
 
 if(fenster!= NULL)
  fenster->updateGraph(this);
 
 _wallNodeChanged();
  
 return;
}

void FFGraphicNode::moveTo(const char *x1, const char *y1, const char *z1)
{
  moveTo(FFPoint(atof(x1), atof(y1), atof(z1)));
}

void FFGraphicNode::rmoveTo(const FFPoint& point)
{
//  BOOL tmpDate= NO;
	
//  if(fenster != NULL)
//   {
//    tmpDate=fenster->getUpdate();
//    fenster->setUpdate(NO);
   
//    const FFSet<FFEdgeClass *>& transTo = node->transToSucc();
//    for(Pix i=transTo.first(); i!=0; transTo.next(i))
//     {
//      if(fenster->isCurrent((*transTo(i)->target())()))
//       (*transTo(i))()->rmoveTo(point);
//     }			
//   }
 
 moveTo(coords+point);
 
//  if(fenster != NULL)
//   {
//    fenster->setUpdate(tmpDate);
//    if(tmpDate)
//     fenster->update();	
//   }
 
//  return;
}

void FFGraphicNode::rmoveTo(const char *x1, const char *y1, const char *z1)
{
  rmoveTo(FFPoint(atof(x1), atof(y1), atof(z1)));
  return;
}

