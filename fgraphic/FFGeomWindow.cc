#if __GNUG__ >= 2
#  pragma implementation
#endif

#include <ffgraph/FFGeomWindow.h>
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFGraphicNode.h>

//---- newclass ---------------------------------------------------------

FFGeomWindow::FFGeomWindow()
{
	updateFlag=YES;
	alreadyInitialisized=NO;
}

void FFGeomWindow::init(const FFGraph& c)
{
  FFGraphLabel::init(c);
  if(c->getGraphWindowIndex() <= 0)
    c->setGraphIndex(c->getGraphLabelIndex(type()));
  
  if(!alreadyInitialisized)
  {
		alreadyInitialisized=YES;
//		if(!fork())
//		{
//			system("geomview -c -");
//			exit(0);
//		}
		cout << "(geometry graph { : foo })" << endl;
	}
}

void FFGeomWindow::updateGraph()
{
 const FFSet<FFNode>& knoten= graph->allNodes();
 //	int anz = knoten.length();
 cout << "(read geometry { define foo \n LIST\n";
 for(Pix i = knoten.first(); i!=0; knoten.next(i))
  {
   FFNode tmpN= knoten(i);
   if((*tmpN)())
    {
     cout << "SPHERE\n10\n" << (int)(*tmpN)->getx()
          << " " <<(int)(*tmpN)->gety() << " " << 
      (int)(*tmpN)->getz()<<endl;
    }
  }
 cout << "})"<<endl;
}
