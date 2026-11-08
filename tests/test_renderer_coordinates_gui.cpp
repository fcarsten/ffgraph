#include "test_check.h"
#include "sugiyama_layout_snapshot.h"
#include "fgraphic/FFCanvasCoordinates.h"
#include <ffgraph/FFSugi.h>
#include <ffgraph/FFTclWindow.h>
#include <ffgraph/FFTcl3DWindow.h>
#include <ffgraph/FFTclCompat.h>
#include <ffgraph/ffError.h>
#include <tk.h>
#include <limits>
#include <climits>

extern kindErrorHandler* userKindErrorHandler;
extern charErrorHandler* userCharErrorHandler;
namespace {
struct Sugi:FFSugi {void init(const FFGraph& g)override{FFLayoutLabel::init(g);}};
template<class Base> struct Window:Base {
  int busy=0;
  bool forced=false;
  bool failNodeUpdate=false;
  FFPoint projection;
  Tcl_Interp* interpreter(){return this->interp;}
  const String& path(){return this->canvas;}
  FFCoord labelX(){return this->labelXoff;}
  FFCoord labelY(){return this->labelYoff;}
  FFPoint proj(const FFPoint& p,const FFPoint& range=FFPoint())override {
    return forced?projection:Base::proj(p,range);
  }
  using Base::updateGraph;
  void updateGraph(FFGraphicNode* node)override {
    Base::updateGraph(node);
    if(failNodeUpdate){failNodeUpdate=false;throw std::runtime_error("renderer rollback control");}
  }
  void setBusy()override{++busy;Base::setBusy();}
  void unsetBusy()override{--busy;Base::unsetBusy();}
};
template<class W> std::string command(W* w,std::initializer_list<const char*> args) {
  int code=FF_TclEvalObjv(w->interpreter(),args);
  if(code!=TCL_OK)std::cerr << Tcl_GetStringResult(w->interpreter()) << '\n';
  FF_CHECK(code==TCL_OK);return Tcl_GetStringResult(w->interpreter());
}
template<class W> std::string state(W* w,const String& id) {
  FF_CHECK(id.length());return command(w,{w->path().c_str(),"itemcget",id.c_str(),"-state"});
}
template<class W> void setState(W* w,const String& id,const char* value) {
  command(w,{w->path().c_str(),"itemconfigure",id.c_str(),"-state",value});
}
template<class W> std::vector<double> coords(W* w,const String& id) {
  command(w,{w->path().c_str(),"coords",id.c_str()});
  int count=0;Tcl_Obj** objects=nullptr;
  FF_CHECK(Tcl_ListObjGetElements(w->interpreter(),Tcl_GetObjResult(w->interpreter()),&count,&objects)==TCL_OK);
  std::vector<double> out;
  for(int i=0;i<count;++i){double x;FF_CHECK(Tcl_GetDoubleFromObj(w->interpreter(),objects[i],&x)==TCL_OK);out.push_back(x);}
  return out;
}
template<class Base> void exercise(int orientation,bool updates) {
  FFGraph graph(YES);graph->_setDirected(YES);
  graph->setNodeIndex(graph->addNodeLabel(new FFGraphicNode));
  graph->setEdgeIndex(graph->addEdgeLabel(new FFGraphicEdge));
  std::vector<FFNode> nodes;std::vector<FFEdge> edges;
  for(int i=0;i<4;++i){auto n=graph->addNode(FFPoint(i*10,i*20,0));n->setValue(i+1);n->setName("node");nodes.push_back(n);}
  for(int i=1;i<4;++i)edges.push_back(graph->addEdge(nodes[i-1],nodes[i]));
  edges.push_back(graph->addEdge(nodes[0],nodes[3]));
  for(std::size_t i=0;i<edges.size();++i){edges[i]->setValue(int(i)+1);edges[i]->setName("edge");}
  auto* layout=new Sugi;graph->addGraphLabel(layout);layout->setOrient(orientation);
  FF_CHECK(layout->setxoff(1e9f) && layout->setyoff(1e9f));
  const auto ids=ffgraph_test::layoutIds(*graph.operator->());
  layout->layout(); // Headless world-geometry oracle, on the same objects.
  const auto expected=ffgraph_test::captureLayout(*graph.operator->(),ids);
  auto* w=new Window<Base>;graph->createGraphic(w);w->setUpdate(updates);
  std::vector<String> items;
  for(auto n:nodes){items.push_back(n->getGraphicNode()->getId());items.push_back(n->getGraphicNode()->getLabelId());}
  for(auto e:edges){items.push_back(e->getGraphicEdge()->getId());items.push_back(e->getGraphicEdge()->getLabelId());}
  setState(w,items[0],"disabled");setState(w,items[1],"hidden");
  w->scaleXYZ(4,4,1);
  layout->layout();
  FF_CHECK(w->busy==0 && w->getUpdate()==updates);
  auto actual=ffgraph_test::captureLayout(*graph.operator->(),ids);
  FF_CHECK(expected.nodes==actual.nodes && expected.edges==actual.edges);
  FF_CHECK(!ffgraph_canvas::supported(w->proj(nodes[0]->getGraphicNode()->getCoords()),true));
  for(const auto& item:items)FF_CHECK(state(w,item)=="hidden");
  w->translate(-2e9f,-2e9f,0);
  for(const auto& item:items)FF_CHECK(state(w,item)=="hidden");
  w->translate(2e9f,2e9f,0);w->scaleXYZ(0.25f,0.25f,1);
  FF_CHECK(state(w,items[0])=="disabled" && state(w,items[1])=="hidden");
  for(std::size_t i=2;i<items.size();++i)FF_CHECK(state(w,items[i])!="hidden");
  for(const auto& item:items)setState(w,item,"normal");
  actual=ffgraph_test::captureLayout(*graph.operator->(),ids);
  FF_CHECK(expected.nodes==actual.nodes && expected.edges==actual.edges);

  // Exercise ordinary positive/negative fractional truncation at the actual
  // Tcl boundary, independently of projection arithmetic and world geometry.
  w->forced=true;w->projection=FFPoint(17.75f,-21.25f,9.75f);w->updateGraph();
  FF_CHECK(coords(w,nodes[0]->getGraphicNode()->getId())==std::vector<double>({8,-30,26,-12}));
  FF_CHECK(coords(w,edges[0]->getGraphicEdge()->getId())==std::vector<double>({17,-21,17,-21}));
  FF_CHECK(coords(w,nodes[0]->getGraphicNode()->getLabelId())==
    std::vector<double>({double(int(17.75f+9.75f*w->labelX())+5),double(int(-21.25f+9.75f*w->labelY()))}));
  w->projection=FFPoint(17.75f,-21.25f,20);
  w->setLabelOffset(INT_MAX,INT_MAX);
  FF_CHECK(state(w,nodes[0]->getGraphicNode()->getId())!="hidden");
  FF_CHECK(state(w,nodes[0]->getGraphicNode()->getLabelId())=="hidden");
  w->setLabelOffset(0,0);
  FF_CHECK(state(w,nodes[0]->getGraphicNode()->getLabelId())!="hidden");

  for(const auto& bad:std::vector<FFPoint>{FFPoint(4e9f,0,10),FFPoint(-4e9f,0,10),
      FFPoint(0,std::numeric_limits<float>::infinity(),10),
      FFPoint(std::numeric_limits<float>::quiet_NaN(),0,10),
      FFPoint(0,0,std::numeric_limits<float>::quiet_NaN())}) {
    w->projection=bad;w->updateGraph();
    for(const auto& item:items)FF_CHECK(state(w,item)=="hidden");
    auto node=graph->addNode(FFPoint(20,30,0));node->setName("new hidden node");
    auto edge=graph->addEdge(nodes[0],node);edge->setName("new hidden edge");
    FF_CHECK(state(w,node->getGraphicNode()->getId())=="hidden");
    FF_CHECK(state(w,node->getGraphicNode()->getLabelId())=="hidden");
    FF_CHECK(state(w,edge->getGraphicEdge()->getId())=="hidden");
    FF_CHECK(state(w,edge->getGraphicEdge()->getLabelId())=="hidden");
    node->getGraphicNode()->changeToFigure(FF_RECTANGLE);
    FF_CHECK(state(w,node->getGraphicNode()->getId())=="hidden");
    node->getGraphicNode()->changeToIcon("default.xbm");
    FF_CHECK(state(w,node->getGraphicNode()->getId())=="hidden");
    graph->removeEdge(edge);graph->removeNode(node);
  }
  w->projection=FFPoint(17.75f,-21.25f,9.75f);w->updateGraph();
  for(const auto& item:items)FF_CHECK(state(w,item)!="hidden");
  w->forced=false;w->updateGraph();
  // Large finite camera changes previously also overflowed the integer UI
  // slider arguments. The existing reset restores a usable view afterward.
  w->scaleXYZ(1e10f,1e10f,1);w->scaleXYZ(1e10f,1e10f,1);
  w->scaleXYZ(1,1,1);
  w->rotateX(1e30f);w->rotateY(1e30f);w->rotateZ(1e30f);
  const char* reset[]={"recall","resetScales"};
  FF_CHECK(w->recall(w->interpreter(),2,reset)==TCL_OK);
  for(const auto& item:items)FF_CHECK(state(w,item)!="hidden");
  FF_CHECK(edges[0]->getGraphicEdge()->addPointAt(1,FFPoint(4e9f,4e9f,0)));
  w->updateGraph();
  FF_CHECK(state(w,edges[0]->getGraphicEdge()->getId())=="hidden");
  FF_CHECK(state(w,nodes[0]->getGraphicNode()->getId())!="hidden");
  FF_CHECK(edges[0]->getGraphicEdge()->deletePointAt(1));
  w->updateGraph();
  FF_CHECK(state(w,edges[0]->getGraphicEdge()->getId())!="hidden");
  FF_CHECK(w->getUpdate()==updates && w->busy==0);
  FF_CHECK(graph->getNumNodes()==4 && graph->getNumEdges()==4 && graph->getDirected());
  actual=ffgraph_test::captureLayout(*graph.operator->(),ids);
  FF_CHECK(expected.nodes==actual.nodes && expected.edges==actual.edges);

  // A real renderer exception after hiding the first newly moved node must
  // still restore the previous supported drawing through publication rollback.
  FF_CHECK(layout->setxoff(0) && layout->setyoff(0));layout->layout();
  w->scaleXYZ(4,4,1);
  const auto original=ffgraph_test::captureLayout(*graph.operator->(),ids);
  FF_CHECK(layout->setxoff(1e9f) && layout->setyoff(1e9f));
  w->failNodeUpdate=true;bool failed=false;
  try{layout->layout();}catch(const std::runtime_error& error){failed=std::string(error.what())=="renderer rollback control";}
  FF_CHECK(failed && !w->failNodeUpdate && w->busy==0 && w->getUpdate()==updates);
  const auto restored=ffgraph_test::captureLayout(*graph.operator->(),ids);
  FF_CHECK(original.nodes==restored.nodes && original.edges==restored.edges);
  for(const auto& item:items)FF_CHECK(state(w,item)!="hidden");
  FF_CHECK(layout->setxoff(0) && layout->setyoff(0));layout->layout();
  graph->removeGraphic();
}
}
int main(int argc,char** argv) {
  Tcl_FindExecutable(argc?argv[0]:"test_renderer_coordinates_gui");
  const int initial=Tk_GetNumMainWindows();
  auto oldKind=userKindErrorHandler;auto oldChar=userCharErrorHandler;
  userKindErrorHandler=[](errGrade,errKind kind,const char*,int){std::cerr << "unexpected kind " << kind << '\n';FF_CHECK(false);};
  userCharErrorHandler=[](errGrade,const char* message,const char*,int){std::cerr << message << '\n';FF_CHECK(false);};
  for(int orientation:{1,2})for(bool updates:{false,true}) {
    exercise<FFTclWindow>(orientation,updates);exercise<FFTcl3DWindow>(orientation,updates);
    FFWindow::collectPendingInterpreters();FF_CHECK(Tk_GetNumMainWindows()==initial);
  }
  userKindErrorHandler=oldKind;userCharErrorHandler=oldChar;
  std::cout << "8 actual 2D/3D large-offset/4x zoom cases: model, canvas visibility, truncation and recovery passed\n";
}
