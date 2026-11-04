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
#include "flayout/FFsugi_publication.h"

namespace {
long failAfter=-1;
bool injected=false;
int reported=0;
bool denyScalar=false;
unsigned scalarAttempts=0;
}
void* operator new(std::size_t size) {
  if(denyScalar){++scalarAttempts;throw std::bad_alloc();}
  if(void* p=std::malloc(size?size:1))return p;
  throw std::bad_alloc();
}
void operator delete(void* p) noexcept {std::free(p);}
void operator delete(void* p,std::size_t) noexcept {std::free(p);}
void* operator new(std::size_t size,const std::nothrow_t&) noexcept {
  try{return ::operator new(size);}catch(...){return nullptr;}
}
void operator delete(void* p,const std::nothrow_t&) noexcept {::operator delete(p);}
void* operator new[](std::size_t size) {
  if(failAfter==0){failAfter=-1;injected=true;throw std::bad_alloc();}
  if(failAfter>0)--failAfter;
  if(void* p=std::malloc(size?size:1))return p;
  throw std::bad_alloc();
}
void operator delete[](void* p) noexcept {std::free(p);}
void operator delete[](void* p,std::size_t) noexcept {std::free(p);}
void* operator new[](std::size_t size,const std::nothrow_t&) noexcept {
  try{return ::operator new[](size);}catch(...){return nullptr;}
}
void operator delete[](void* p,const std::nothrow_t&) noexcept {::operator delete[](p);}
extern kindErrorHandler* userKindErrorHandler;
namespace {
struct Sugi:FFSugi{void init(const FFGraph& g)override{FFLayoutLabel::init(g);}};
struct Graphic:FFGraphicEdge {
  FFEdgeLabel* proto()const override{return new Graphic;}
  BOOL smooth()const{return spline;}
};
struct RecoveryWindow:FFTclWindow {
  int busy=0;
  long failCallback=-1;
  bool callbackInjected=false, genericFailure=false;
  std::map<FFGraphicEdge*,BOOL> smoothState;
  void setSpline(FFGraphicEdge* edge,BOOL value)override{
    callback();FFTclWindow::setSpline(edge,value);smoothState[edge]=value;
  }
  void callback() {
    if(failCallback==0){failCallback=-1;callbackInjected=injected=true;
      if(genericFailure)throw std::runtime_error("injected renderer failure");
      throw std::bad_alloc();}
    if(failCallback>0)--failCallback;
  }
  void updateGraph(FFGraphicNode* node)override{callback();FFTclWindow::updateGraph(node);}
  void updateGraph(FFGraphicEdge* edge)override{callback();FFTclWindow::updateGraph(edge);}
  void updateGraph()override{callback();FFTclWindow::updateGraph();}
  void adjustWindowSize()override{callback();FFTclWindow::adjustWindowSize();}
  void setBusy()override{++busy;FFTclWindow::setBusy();}
  void unsetBusy()override{--busy;FFTclWindow::unsetBusy();}
};
}
int main(int argc,char**argv) {
  const bool publication=argc==2 && std::string(argv[1])=="--publication-gui";
  const bool gui=publication || (argc==2 && std::string(argv[1])=="--gui");
  if(gui)Tcl_FindExecutable(argv[0]);
  const int initial=gui?Tk_GetNumMainWindows():0;
  auto old=userKindErrorHandler;
  userKindErrorHandler=[](errGrade grade,errKind kind,const char*,int){
    FF_CHECK(grade==USER_ERROR && kind==NO_MEMORY);++reported;
  };
  for(bool generic:publication?std::vector<bool>{false,true}:std::vector<bool>{false})
  for(bool updatesEnabled:publication?std::vector<bool>{false,true}:std::vector<bool>{false})
  for(bool reduction:{false,true}) {
    FFGraph graph(YES);graph->_setDirected(YES);
    graph->setNodeIndex(graph->addNodeLabel(new FFGraphicNode));
    graph->setEdgeIndex(graph->addEdgeLabel(new Graphic));
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
    if(gui){window=new RecoveryWindow;graph->createGraphic(window);window->setUpdate(updatesEnabled);window->genericFailure=generic;}
    if(publication)edges[0]->getGraphicEdge()->toggleSpline();
    std::vector<Pix> oldPix;std::vector<const FFPoint*> oldAddresses;std::vector<BOOL> smooth;
    for(auto edge:edges){
      const auto& points=edge->getGraphicEdge()->getEdgePoints();const auto p=points.last();
      oldPix.push_back(p);oldAddresses.push_back(&points(p));
      smooth.push_back(static_cast<Graphic*>(edge->getGraphicEdge())->smooth());
    }
    const auto original=ffgraph_test::captureLayout(*graph.operator->(),ids);
    int failures=0;
    for(int index=0;index<256;++index) {
      injected=false;reported=0;failAfter=publication?-1:index;
      if(publication){window->failCallback=index;window->callbackInjected=false;}
      bool propagated=false;try{layout->layout();}catch(const std::bad_alloc&){propagated=true;}
      catch(const std::runtime_error&){FF_CHECK(publication && window->callbackInjected);propagated=true;}
      failAfter=-1;
      if(publication)window->failCallback=-1;
      if(window)FF_CHECK(window->busy==0 && window->getUpdate()==updatesEnabled);
      if(!injected){FF_CHECK(!propagated && reported==0);break;}
      ++failures;FF_CHECK(propagated || reported==1);
      const auto after=ffgraph_test::captureLayout(*graph.operator->(),ids);
      FF_CHECK(original.nodes==after.nodes && original.edges==after.edges);
      for(std::size_t i=0;i<edges.size();++i){
        auto* graphic=static_cast<Graphic*>(edges[i]->getGraphicEdge());
        FF_CHECK(&graphic->getEdgePoints()(oldPix[i])==oldAddresses[i] && graphic->smooth()==smooth[i]);
      }
      if(publication)FF_CHECK(window->smoothState.at(edges[0]->getGraphicEdge())==YES);
      for(int i=0;i<8;++i)FF_CHECK(nodes[i]->getValue()==11+i*7);
      for(std::size_t i=0;i<edges.size();++i)FF_CHECK(edges[i]->getValue()==123+int(i));
      FF_CHECK(graph->getStartNode()==nodes[1] && graph->getNumNodes()==8 && graph->getNumEdges()==11);
      FF_CHECK(index<255);
    }
    FF_CHECK(failures>5);
    std::cout << failures << (publication?" renderer":" array allocation")
              << " failures preserved IDs, geometry and original route storage (reduction=" << reduction
              << ", updates=" << updatesEnabled << ", generic=" << generic << ")\n";
    if(!gui){
      const auto expected=ffgraph_test::captureLayout(*graph.operator->(),ids);
      ffgraph_sugiyama::Publication saved(graph->allNodes(),graph->allEdges());
      saved.begin();
      nodes[0]->getGraphicNode()->moveTo(FFPoint(123,456,789));
      edges[0]->getGraphicEdge()->addPointAt(1,FFPoint(987,654,321));
      injected=false;scalarAttempts=0;failAfter=0;denyScalar=true;
      saved.rollback();
      denyScalar=false;failAfter=-1;
      FF_CHECK(!injected && scalarAttempts==0);
      const auto restored=ffgraph_test::captureLayout(*graph.operator->(),ids);
      FF_CHECK(expected.nodes==restored.nodes && expected.edges==restored.edges);
    }
    if(window){window->setUpdate(YES);graph->removeGraphic();}
  }
  userKindErrorHandler=old;
  if(gui){FFWindow::collectPendingInterpreters();FF_CHECK(Tk_GetNumMainWindows()==initial);}
}
