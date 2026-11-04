// One-shot array allocation failures, confined to the actual layout call.
#include "test_check.h"
#include "sugiyama_layout_snapshot.h"
#include <ffgraph/FFSugi.h>
#include <ffgraph/ffError.h>
#include <ffgraph/FFTclWindow.h>
#include <tk.h>
#include <cstdlib>
#include <new>
#include <vector>

namespace {
long failAfter=-1;
bool injected=false;
int reported=0;
}
void* operator new[](std::size_t size) {
  if(failAfter==0){failAfter=-1;injected=true;throw std::bad_alloc();}
  if(failAfter>0)--failAfter;
  if(void* p=std::malloc(size?size:1))return p;
  throw std::bad_alloc();
}
void operator delete[](void* p) noexcept {std::free(p);}
void operator delete[](void* p,std::size_t) noexcept {std::free(p);}
extern kindErrorHandler* userKindErrorHandler;
namespace {
struct Sugi:FFSugi{void init(const FFGraph& g)override{FFLayoutLabel::init(g);}};
struct RecoveryWindow:FFTclWindow {
  int busy=0;
  void setBusy()override{++busy;FFTclWindow::setBusy();}
  void unsetBusy()override{--busy;FFTclWindow::unsetBusy();}
};
}
int main(int argc,char**argv) {
  const bool gui=argc==2 && std::string(argv[1])=="--gui";
  if(gui)Tcl_FindExecutable(argv[0]);
  const int initial=gui?Tk_GetNumMainWindows():0;
  auto old=userKindErrorHandler;
  userKindErrorHandler=[](errGrade grade,errKind kind,const char*,int){
    FF_CHECK(grade==USER_ERROR && kind==NO_MEMORY);++reported;
  };
  for(bool reduction:{false,true}) {
    FFGraph graph(YES);graph->_setDirected(YES);
    graph->setNodeIndex(graph->addNodeLabel(new FFGraphicNode));
    graph->setEdgeIndex(graph->addEdgeLabel(new FFGraphicEdge));
    std::vector<FFNode> nodes;std::vector<FFEdge> edges;
    for(int i=0;i<8;++i){auto n=graph->addNode(FFPoint(i*71,i*53,i*11));n->setValue(11+i*7);nodes.push_back(n);}
    for(int i=1;i<8;++i)edges.push_back(graph->addEdge(nodes[i-1],nodes[i]));
    edges.push_back(graph->addEdge(nodes[0],nodes[7]));edges.push_back(graph->addEdge(nodes[7],nodes[1]));
    edges.push_back(graph->addEdge(nodes[0],nodes[7]));edges.push_back(graph->addEdge(nodes[2],nodes[2]));
    for(std::size_t i=0;i<edges.size();++i){edges[i]->setValue(123+int(i));edges[i]->getGraphicEdge()->addPointAt(1,FFPoint(777,888,99));}
    graph->setStartNode(nodes[1]);
    const auto ids=ffgraph_test::layoutIds(*graph.operator->());
    auto* layout=new Sugi;graph->addGraphLabel(layout);FF_CHECK(layout->setCrossingAlg(reduction?0:2));
    RecoveryWindow* window=nullptr;
    if(gui){window=new RecoveryWindow;graph->createGraphic(window);window->setUpdate(NO);}
    const auto original=ffgraph_test::captureLayout(*graph.operator->(),ids);
    int failures=0;
    for(int index=0;index<256;++index) {
      injected=false;reported=0;failAfter=index;
      bool propagated=false;try{layout->layout();}catch(const std::bad_alloc&){propagated=true;}
      failAfter=-1;
      if(window)FF_CHECK(window->busy==0 && window->getUpdate()==NO);
      if(!injected){FF_CHECK(!propagated && reported==0);break;}
      ++failures;FF_CHECK(propagated || reported==1);
      const auto after=ffgraph_test::captureLayout(*graph.operator->(),ids);
      FF_CHECK(original.nodes==after.nodes && original.edges==after.edges);
      for(int i=0;i<8;++i)FF_CHECK(nodes[i]->getValue()==11+i*7);
      for(std::size_t i=0;i<edges.size();++i)FF_CHECK(edges[i]->getValue()==123+int(i));
      FF_CHECK(graph->getStartNode()==nodes[1] && graph->getNumNodes()==8 && graph->getNumEdges()==11);
      FF_CHECK(index<255);
    }
    FF_CHECK(failures>5);
    std::cout << failures << " allocation failures preserved IDs, positions and bent routes (reduction=" << reduction << ")\n";
    if(window){window->setUpdate(YES);graph->removeGraphic();}
  }
  userKindErrorHandler=old;
  if(gui){FFWindow::collectPendingInterpreters();FF_CHECK(Tk_GetNumMainWindows()==initial);}
}
