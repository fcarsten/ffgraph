// One-shot allocation failures, confined to the actual layout call.
#include "test_check.h"
#include "sugiyama_layout_snapshot.h"
#include <ffgraph/FFSugi.h>
#include <ffgraph/ffError.h>
#include <ffgraph/FFTclWindow.h>
#include <ffgraph/FFTclCompat.h>
#include <tk.h>
#include <cstdlib>
#include <cstdio>
#include <new>
#include <vector>
#include "flayout/FFsugi_publication.h"

namespace {
long failAfter=-1;
long scalarFailAfter=-1;
bool injected=false;
int reported=0;
bool denyScalar=false;
unsigned scalarAttempts=0;
int scalarTclErrors=0;
}
void* operator new(std::size_t size) {
  if(scalarFailAfter==0){scalarFailAfter=-1;injected=true;throw std::bad_alloc();}
  if(scalarFailAfter>0)--scalarFailAfter;
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
extern charErrorHandler* userCharErrorHandler;
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
  std::string canvasCoords(const String& id) {
    FF_CHECK(FF_TclEvalObjv(interp,{canvas.c_str(),"coords",id.c_str()})==TCL_OK);
    return Tcl_GetStringResult(interp);
  }
  std::string canvasOption(const String& id,const char* option) {
    FF_CHECK(FF_TclEvalObjv(interp,{canvas.c_str(),"itemcget",id.c_str(),option})==TCL_OK);
    return Tcl_GetStringResult(interp);
  }
  bool idleCursor() {
    FF_CHECK(FF_TclEvalObjv(interp,{canvas.c_str(),"cget","-cursor"})==TCL_OK);
    return std::string(Tcl_GetStringResult(interp))=="left_ptr";
  }
};

void scalarFailures(bool gui) {
  for(bool updates:gui?std::vector<bool>{false,true}:std::vector<bool>{false})
  for(bool reduction:{false,true})for(bool local:{false,true}) {
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
    auto* layout=new Sugi;graph->addGraphLabel(layout);
    FF_CHECK(layout->setCrossingAlg(reduction?0:2));layout->setOrient(local?2:1);
    RecoveryWindow* window=nullptr;
    if(gui){window=new RecoveryWindow;graph->createGraphic(window);window->setUpdate(updates);}
    if(gui)edges[0]->getGraphicEdge()->toggleSpline();
    FFSet<FFNode> selection;selection.add(nodes[1]);selection.add(nodes[4]);
    const Pix selectedPix=selection.first();const FFNode selectedNode=selection(selectedPix);
    const auto ids=ffgraph_test::layoutIds(*graph.operator->());
    const auto original=ffgraph_test::captureLayout(*graph.operator->(),ids);
    if(window)window->updateGraph(); // Normalize legacy addLabel/moveLabel offsets.
    auto canvasSnapshot=[&]{
      std::vector<std::string> out;
      if(!window)return out;
      for(auto n:nodes) {
        for(const auto& id:{n->getGraphicNode()->getId(),n->getGraphicNode()->getLabelId()}) {
          out.push_back(window->canvasCoords(id));out.push_back(window->canvasOption(id,"-state"));
        }
      }
      for(auto e:edges) {
        auto* g=e->getGraphicEdge();
        for(const auto& id:{g->getId(),g->getLabelId()}) {
          out.push_back(window->canvasCoords(id));out.push_back(window->canvasOption(id,"-state"));
        }
        out.push_back(window->canvasOption(g->getId(),"-smooth"));
      }
      return out;
    };
    const auto originalCanvas=canvasSnapshot();
    auto run=[&]{if(local)layout->layout(selection);else layout->layout();};
    auto restoreIds=[&]{
      for(int i=0;i<8;++i)nodes[i]->setValue(11+i*7);
      for(std::size_t i=0;i<edges.size();++i)edges[i]->setValue(123+int(i));
    };
    // The test guard resets a successful/nothrow-fallback layout so every
    // budget tests the same input. Failed-layout checks run BEFORE that reset.
    ffgraph_test::LayoutSnapshot expected;
    std::vector<std::string> expectedCanvas;
    {
      ffgraph_sugiyama::Publication reset(graph->allNodes(),graph->allEdges());reset.begin();
      reported=0;run();FF_CHECK(reported==0);
      expected=ffgraph_test::captureLayout(*graph.operator->(),ids);
      expectedCanvas=canvasSnapshot();
      reset.rollback();restoreIds();reset.refresh(window);
    }
    int failures=0,fallbacks=0;bool completed=false;
    for(int index=0;index<10000;++index) {
      ffgraph_sugiyama::Publication reset(graph->allNodes(),graph->allEdges());reset.begin();
      std::vector<Pix> pix;std::vector<const FFPoint*> addresses;
      for(auto e:edges){const auto& p=e->getGraphicEdge()->getEdgePoints();pix.push_back(p.last());addresses.push_back(&p(p.last()));}
      injected=false;reported=0;scalarTclErrors=0;scalarFailAfter=index;
      bool propagated=false;try{run();}catch(const std::bad_alloc&){propagated=true;}
      scalarFailAfter=-1;
      if(propagated)std::cerr << "scalar allocation escaped: index=" << index << " local=" << local << '\n';
      FF_CHECK(!propagated);
      if(scalarTclErrors)std::cerr << "Tcl errors after scalar allocation index=" << index << '\n';
      FF_CHECK(scalarTclErrors==0);
      FF_CHECK(!window || (window->busy==0 && window->getUpdate()==updates));
      FF_CHECK(!window || window->idleCursor());
      FF_CHECK(selection.length()==2 && selection(selectedPix)==selectedNode && selection.contains(nodes[1]) && selection.contains(nodes[4]));
      const auto result=ffgraph_test::captureLayout(*graph.operator->(),ids);
      if(reported) {
        ++failures;FF_CHECK(injected && reported==1);
        FF_CHECK(original.nodes==result.nodes && original.edges==result.edges);
        FF_CHECK(originalCanvas==canvasSnapshot());
        for(int i=0;i<8;++i)FF_CHECK(nodes[i]->getValue()==11+i*7);
        for(std::size_t i=0;i<edges.size();++i){
          FF_CHECK(edges[i]->getValue()==123+int(i));
          FF_CHECK(&edges[i]->getGraphicEdge()->getEdgePoints()(pix[i])==addresses[i]);
        }
      } else {
        FF_CHECK(expected.nodes==result.nodes && expected.edges==result.edges);
        FF_CHECK(expectedCanvas==canvasSnapshot());
        if(injected)++fallbacks;else completed=true;
      }
      FF_CHECK(graph->getNumNodes()==8 && graph->getNumEdges()==11 && graph->getStartNode()==nodes[1]);
      reset.rollback();restoreIds();reset.refresh(window);
      if(completed)break;
    }
    FF_CHECK(completed && failures>50);
    std::cout << failures << " scalar failures, " << fallbacks << " successful fallbacks"
              << " (gui=" << gui << ", updates=" << updates << ", reduction=" << reduction
              << ", local=" << local << ")\n";
    if(window){window->setUpdate(YES);graph->removeGraphic();}
  }
}
}
int main(int argc,char**argv) {
  const bool scalar=argc==2 && (std::string(argv[1])=="--scalar" || std::string(argv[1])=="--scalar-gui");
  const bool publication=argc==2 && std::string(argv[1])=="--publication-gui";
  const bool gui=publication || (argc==2 && (std::string(argv[1])=="--gui" || std::string(argv[1])=="--scalar-gui"));
  if(gui)Tcl_FindExecutable(argv[0]);
  const int initial=gui?Tk_GetNumMainWindows():0;
  auto old=userKindErrorHandler;
  userKindErrorHandler=[](errGrade grade,errKind kind,const char*,int){
    FF_CHECK(grade==USER_ERROR && kind==NO_MEMORY);++reported;
  };
  if(scalar){
    auto oldChar=userCharErrorHandler;
    userCharErrorHandler=[](errGrade,const char* message,const char*,int){
      ++scalarTclErrors;std::fprintf(stderr,"unexpected error: %s\n",message);
    };
    scalarFailures(gui);userKindErrorHandler=old;
    userCharErrorHandler=oldChar;
    if(gui){FFWindow::collectPendingInterpreters();FF_CHECK(Tk_GetNumMainWindows()==initial);}
    return 0;
  }
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
