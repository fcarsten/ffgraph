// Real OS allocation failures; the limit is applied after fixture construction.
#include "test_check.h"
#include "sugiyama_layout_snapshot.h"
#include "flayout/FFsugi_sizes.h"
#include <ffgraph/FFSugi.h>
#include <ffgraph/FFreduce_crossing.h>
#include <ffgraph/FFTclWindow.h>
#include <ffgraph/ffError.h>
#include <tk.h>
#include <cstdio>
#include <cstdlib>
#include <new>
#include <string>
#ifdef __linux__
#include <sys/resource.h>
#include <unistd.h>
#endif

extern kindErrorHandler* userKindErrorHandler;

namespace {
constexpr std::size_t MiB=1024*1024;
struct Trace {
  bool active=false;
  std::size_t attempts=0, accepted=0, failed=0, largestFailure=0;
} trace;
int errors=0;
}
// Observe actual array allocations, without injecting or predicting failure.
// Pair malloc/free on every path, including the nothrow family.
void* operator new[](std::size_t size) {
  if(trace.active)++trace.attempts;
  if(void* p=std::malloc(size?size:1)) {
    if(trace.active)++trace.accepted;
    return p;
  }
  if(trace.active){++trace.failed;trace.largestFailure=std::max(trace.largestFailure,size);}
  throw std::bad_alloc();
}
void operator delete[](void* p) noexcept {std::free(p);}
void operator delete[](void* p,std::size_t) noexcept {std::free(p);}
void* operator new[](std::size_t size,const std::nothrow_t&) noexcept {
  try{return ::operator new[](size);}catch(...){return nullptr;}
}
void operator delete[](void* p,const std::nothrow_t&) noexcept {::operator delete[](p);}

namespace {
struct AddressLimit {
#ifdef __linux__
  struct rlimit original{};
  bool active=false;
  std::size_t baseline=0;
  explicit AddressLimit(std::size_t allowance) {
    // No heap allocation needed to measure virtual memory or change the limit.
    FILE* file=std::fopen("/proc/self/statm","r");FF_CHECK(file);
    unsigned long pages=0;FF_CHECK(std::fscanf(file,"%lu",&pages)==1);
    FF_CHECK(std::fclose(file)==0);
    const long pageSize=sysconf(_SC_PAGESIZE);FF_CHECK(pageSize>0);
    FF_CHECK(pages<=std::numeric_limits<std::size_t>::max()/std::size_t(pageSize));
    baseline=pages*std::size_t(pageSize);
    FF_CHECK(baseline<=std::numeric_limits<rlim_t>::max()-allowance);
    FF_CHECK(getrlimit(RLIMIT_AS,&original)==0);
    struct rlimit bounded=original;
    bounded.rlim_cur=baseline+allowance;
    // An inherited tighter limit is an environment failure, never a test pass.
    FF_CHECK(original.rlim_cur==RLIM_INFINITY || original.rlim_cur>=bounded.rlim_cur);
    FF_CHECK(original.rlim_max==RLIM_INFINITY || original.rlim_max>=bounded.rlim_cur);
    FF_CHECK(setrlimit(RLIMIT_AS,&bounded)==0);active=true;
    struct rlimit actual{};FF_CHECK(getrlimit(RLIMIT_AS,&actual)==0);
    FF_CHECK(actual.rlim_cur==bounded.rlim_cur && actual.rlim_max==original.rlim_max);
  }
  void restore() {
    if(active){FF_CHECK(setrlimit(RLIMIT_AS,&original)==0);active=false;}
  }
  ~AddressLimit(){restore();}
#else
  explicit AddressLimit(std::size_t){FF_CHECK(false);}
  void restore(){}
#endif
};
struct Sugi:FFSugi {void init(const FFGraph& g)override{FFLayoutLabel::init(g);}};
struct Window:FFTclWindow {
  int busy=0;
  void setBusy()override{++busy;FFTclWindow::setBusy();}
  void unsetBusy()override{--busy;FFTclWindow::unsetBusy();}
};
struct Fixture {
  FFGraph graph{YES};
  std::vector<FFNode> nodes;
  std::vector<FFEdge> edges;
  std::vector<int> source,target;
  Sugi* layout=nullptr;
  Fixture(const std::string& shape,bool large) {
    graph->_setDirected(YES);
    graph->setName("resource-control");
    graph->setNodeIndex(graph->addNodeLabel(new FFGraphicNode));
    graph->setEdgeIndex(graph->addEdgeLabel(new FFGraphicEdge));
    auto addNode=[&] {
      int i=int(nodes.size());
      auto n=graph->addNode(FFPoint(i*3,i*7,i%11));
      n->setValue(101+i*3);n->setName("keep-node");nodes.push_back(n);
    };
    auto addEdge=[&](int from,int to) {
      auto e=graph->addEdge(nodes[from],nodes[to]);
      e->setValue(501+int(edges.size())*3);e->setName("keep-edge");e->setWeight(2.5f);
      e->getGraphicEdge()->addPointAt(1,FFPoint(777,888,999));
      edges.push_back(e);source.push_back(from);target.push_back(to);
    };
    if(shape=="routes") {
      const int count=large?2048:512, shortcuts=large?256:64;
      for(int i=0;i<count;++i)addNode();
      for(int i=1;i<count;++i)addEdge(i-1,i);
      for(int i=0;i<shortcuts;++i)addEdge(i,count-1-i);
    } else {
      const int layers=shape=="wide2"?2:3;
      const int width=large?(layers==2?2048:1024):(layers==2?256:128);
      for(int i=0;i<layers*width;++i)addNode();
      for(int layer=1;layer<layers;++layer)
        for(int i=0;i<width;++i)addEdge((layer-1)*width+i,layer*width+i);
    }
    graph->setStartNode(nodes.front());
    layout=new Sugi;graph->addGraphLabel(layout);
    FF_CHECK(layout->setCrossingAlg(shape=="routes"?2:0));
    FF_CHECK(layout->setIteration1(2));
  }
  void topology() {
    FF_CHECK(graph->getDirected() && graph->getName()=="resource-control");
    FF_CHECK(graph->getNumNodes()==int(nodes.size()) && graph->getNumEdges()==int(edges.size()));
    FF_CHECK(graph->getStartNode()==nodes.front());
    for(auto node:nodes)FF_CHECK(graph->contains(node) && node->getName()=="keep-node");
    for(std::size_t i=0;i<edges.size();++i) {
      auto edge=edges[i];FF_CHECK(graph->contains(edge));
      FF_CHECK(edge->source()==nodes[source[i]] && edge->target()==nodes[target[i]]);
      FF_CHECK(edge->getName()=="keep-edge" && edge->getWeight()==2.5f);
    }
  }
};
void matrix(const std::string& shape) {
  const bool partial=shape=="matrix-partial";
  const int levels=shape=="matrix-slabs"?2:1;
  const int stride=partial?4097:(levels==1?40000:20000);
  const auto bytes=std::size_t(ffgraph_sugiyama::reductionEntries(levels,stride))*sizeof(int);
  const std::size_t allowance=(partial?80:8)*MiB;
  AddressLimit limit(allowance);
  // Reuse the SAME cap: the partial case must release its first 64 MiB
  // allocation before another first allocation can succeed on the next call.
  for(int repeat=0;repeat<2;++repeat) {
    bool exhausted=false;trace={};trace.active=true;
    try {
      // Dimensions are valid. Failure must occur in allocation before any
      // input is read; null buffers make unexpected success a hard failure.
      reduce_crossings(nullptr,levels,stride-1,stride-1,0,0,nullptr,2,1);
    } catch(const std::bad_alloc&) {exhausted=true;}
    trace.active=false;
    FF_CHECK(exhausted && trace.failed==1 && trace.largestFailure==bytes);
    if(partial)FF_CHECK(trace.accepted==1);
  }
  limit.restore();
  std::cout << shape << ": request=" << bytes << " allowance=" << allowance
            << " accepted-arrays=" << trace.accepted << " bad_alloc recovered twice under same cap\n";
}
void graphCase(const std::string& shape,bool limited,int orientation,bool gui,bool updates) {
  Fixture f(shape,limited);f.layout->setOrient(orientation);
  Window* window=nullptr;
  if(gui){window=new Window;f.graph->createGraphic(window);window->setUpdate(updates);}
  const auto ids=ffgraph_test::layoutIds(*f.graph.operator->());
  const auto before=ffgraph_test::captureLayout(*f.graph.operator->(),ids);
  std::vector<Pix> pix;std::vector<const FFPoint*> addresses;
  for(auto edge:f.edges) {
    const auto& points=edge->getGraphicEdge()->getEdgePoints();
    pix.push_back(points.last());addresses.push_back(&points(points.last()));
  }
  const int repeats=limited?2:1;
  for(int repeat=0;repeat<repeats;++repeat) {
    errors=0;trace={};
    if(limited) {
      AddressLimit limit(8*MiB);
      trace.active=true;f.layout->layout();trace.active=false;
      limit.restore();
      FF_CHECK(errors==1 && trace.failed==1 && trace.largestFailure>0);
      if(shape=="routes") {
        FF_CHECK(trace.largestFailure==2048*sizeof(FFPoint) && trace.accepted>10);
      } else {
        const int levels=shape=="wide2"?1:2, stride=levels==1?2049:1025;
        FF_CHECK(trace.largestFailure==std::size_t(ffgraph_sugiyama::reductionEntries(levels,stride))*sizeof(int));
      }
      const auto after=ffgraph_test::captureLayout(*f.graph.operator->(),ids);
      FF_CHECK(before.nodes==after.nodes && before.edges==after.edges);
      for(std::size_t i=0;i<f.nodes.size();++i)FF_CHECK(f.nodes[i]->getValue()==101+int(i)*3);
      for(std::size_t i=0;i<f.edges.size();++i) {
        FF_CHECK(f.edges[i]->getValue()==501+int(i)*3);
        FF_CHECK(&f.edges[i]->getGraphicEdge()->getEdgePoints()(pix[i])==addresses[i]);
      }
      std::cout << shape << ": allowance=" << 8*MiB << " failed-array=" << trace.largestFailure
                << " accepted-arrays=" << trace.accepted << " repeat=" << repeat << '\n';
    } else {
      f.layout->layout();FF_CHECK(errors==0);
      const auto result=ffgraph_test::captureLayout(*f.graph.operator->(),ids);
      FF_CHECK(result.nodes.size()==f.nodes.size() && result.edges.size()==f.edges.size());
      FF_CHECK(result.nodes!=before.nodes);
      if(shape=="routes") {
        // Collinear dummy points are deliberately omitted by finish_sgraph.
        FF_CHECK(f.edges.back()->getGraphicEdge()->anzPoints()>=4);
        const int layerAxis=orientation==1?1:0;
        for(std::size_t i=1;i<f.nodes.size();++i)
          FF_CHECK(f.nodes[i]->getGraphicNode()->getCoords().v[layerAxis]>
                   f.nodes[i-1]->getGraphicNode()->getCoords().v[layerAxis]);
      }
    }
    f.topology();
    if(window)FF_CHECK(window->busy==0 && window->getUpdate()==updates);
  }
  // Successful reuse after the OS cap has been lifted, on these same objects.
  // Wide cases use the existing no-reduction option to avoid giant matrices.
  if(limited) {
    FF_CHECK(f.layout->setCrossingAlg(2));errors=0;f.layout->layout();FF_CHECK(errors==0);
    ffgraph_test::captureLayout(*f.graph.operator->(),ids);f.topology();
    if(window)FF_CHECK(window->busy==0 && window->getUpdate()==updates);
  }
  std::cout << shape << ": nodes=" << f.nodes.size() << " edges=" << f.edges.size()
            << " orient=" << orientation << " gui=" << gui << " updates=" << updates
            << " recovery/control passed\n";
  if(window){window->setUpdate(YES);f.graph->removeGraphic();}
}
}
int main(int argc,char** argv) {
  FF_CHECK(argc==6);
  const std::string shape=argv[1];
  const bool limited=std::string(argv[2])=="limited";
  const int orientation=std::stoi(argv[3]);
  const bool gui=std::string(argv[4])=="gui",updates=std::string(argv[5])=="on";
  FF_CHECK(shape=="wide2" || shape=="wide3" || shape=="routes" ||
           shape=="matrix-square" || shape=="matrix-slabs" || shape=="matrix-partial");
  FF_CHECK(orientation==1 || orientation==2);
#if defined(FFGRAPH_RESOURCE_ASAN) || defined(__SANITIZE_ADDRESS__)
  // Do not accidentally apply RLIMIT_AS to ASan's reserved shadow mappings.
  FF_CHECK(!limited);
#endif
  if(gui)Tcl_FindExecutable(argv[0]);
  const int windows=gui?Tk_GetNumMainWindows():0;
  auto old=userKindErrorHandler;
  userKindErrorHandler=[](errGrade grade,errKind kind,const char*,int) {
    FF_CHECK(grade==USER_ERROR && kind==NO_MEMORY);++errors;
  };
  if(shape.rfind("matrix-",0)==0){FF_CHECK(limited && !gui);matrix(shape);}
  else graphCase(shape,limited,orientation,gui,updates);
  userKindErrorHandler=old;
  if(gui){FFWindow::collectPendingInterpreters();FF_CHECK(Tk_GetNumMainWindows()==windows);}
}
