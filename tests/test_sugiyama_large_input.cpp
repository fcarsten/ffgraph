#include "test_check.h"
#include "flayout/FFsugi_sizes.h"
#include <ffgraph/FFreduce_crossing.h>
#include <ffgraph/FFSugi.h>
#include <climits>
#include <cstdint>
#include <sys/resource.h>
#include <vector>
#include <string>

extern FFCoord row_barycenter(int*,int,int,int);
extern FFCoord column_barycenter(int*,int,int,int);
extern int upper_barycenter(int,SNode*,int);
extern int lower_barycenter(int,SNode*,int);
extern void sdfsvisit(int,SNode*,int&,int&);
extern void slevelvisit(int,SNode*);
namespace {
// Frozen recursive helpers, Falk Schreiber 1994; compare visit/finish order,
// including repeated successors and paths whose rank is increased again.
void referenceDfs(int k,SNode* nodes,int& dfs,int& comp) {
  for (Pix p=nodes[k].succ.first();p;nodes[k].succ.next(p)) {
    int child=nodes[k].succ(p);
    if (!nodes[child].dfs) {
      nodes[child].dfs=++dfs;referenceDfs(child,nodes,dfs,comp);nodes[child].comp=++comp;
    }
  }
}
void referenceLevel(int k,SNode* nodes) {
  for (Pix p=nodes[k].succ.first();p;nodes[k].succ.next(p)) {
    int child=nodes[k].succ(p);
    if (nodes[child].level<nodes[k].level+1) {
      nodes[child].level=nodes[k].level+1;nodes[child].attrib=1;referenceLevel(child,nodes);
    }
  }
}
template<class F> void rejected(F f) {
  bool failed=false;try { f(); } catch (const ffgraph_sugiyama::SizeError&) {failed=true;}
  FF_CHECK(failed);
}
void numeric() {
  using namespace ffgraph_sugiyama;
  FF_CHECK(checkedAdd(INT_MAX-1,1)==INT_MAX && checkedProduct(INT_MAX,0)==0);
  FF_CHECK(matrixEntries(46340)==2147395600 && levelEntries(0,100)==202);
  FF_CHECK(scratchNodes(2,3)==12);
  FF_CHECK(reductionEntries(1,40000)==1600000000);
  rejected([]{checkedAdd(INT_MAX,1);});rejected([]{checkedAdd(-1,1);});
  rejected([]{checkedProduct(INT_MAX,2);});rejected([]{checkedProduct(1,-1);});
  rejected([]{scratchNodes(2,INT_MAX/3);});rejected([]{scratchNodes(INT_MAX,0);});
  rejected([]{matrixEntries(46341);});rejected([]{matrixArrayEntries(2,30000);});
  rejected([]{levelEntries(50000,50000);});rejected([]{levelEntries(INT_MAX,0);});
  FF_CHECK(reduce_crossings(nullptr,2,50000,1,1,0,nullptr,8,3)==1);
  FF_CHECK(reduce_crossings(nullptr,INT_MAX,1,1,1,0,nullptr,8,3)==1);
  // A disabled reducer does not need a square matrix, including a wide one.
  FF_CHECK(reduce_crossings(nullptr,1,INT_MAX,1,1,2,nullptr,8,3)==0);
  FF_CHECK(reduce_crossings(nullptr,0,INT_MAX,1,1,0,nullptr,8,3)==0);
  constexpr int n=70000,stride=n+1;
  std::vector<int> row(2*stride,1),column(2*(n+1),1);
  FF_CHECK(row_barycenter(row.data(),1,n,stride)==35000.5f);
  FF_CHECK(column_barycenter(column.data(),1,n,2)==35000.5f);
  std::vector<SNode> nodes(5);
  for(int i=2;i<=4;++i) {
    nodes[i].point.v[0]=1000000000;
    nodes[1].pred.append(i);nodes[1].succ.append(i);
  }
  nodes[1].indeg=nodes[1].outdeg=3;
  FF_CHECK(upper_barycenter(1,nodes.data(),0)==1000000000);
  FF_CHECK(lower_barycenter(1,nodes.data(),0)==1000000000);
  std::cout << "checked int-index boundaries and wide matrix/coordinate barycenters\n";
}
void referenceComparisons() {
  std::uint32_t random=20261104;
  for(int seed=0;seed<200;++seed) {
    const int n=2+seed%31;
    std::vector<SNode> nodes(n+1);
    for(auto& node:nodes){node.node=nullptr;node.posList=nullptr;node.attrib=0;}
    for(int i=1;i<=n;++i) for(int j=1;j<=n;++j) {
      random=random*1664525u+1013904223u;
      if((random>>16)%9==0)nodes[i].succ.append(j);
    }
    auto expected=nodes;
    int a=1,b=0,c=1,d=0;nodes[1].dfs=expected[1].dfs=1;
    sdfsvisit(1,nodes.data(),a,b);referenceDfs(1,expected.data(),c,d);
    FF_CHECK(a==c && b==d);
    for(int i=1;i<=n;++i)FF_CHECK(nodes[i].dfs==expected[i].dfs && nodes[i].comp==expected[i].comp);
    for(int i=1;i<=n;++i){nodes[i].succ.clear();nodes[i].level=0;nodes[i].attrib=0;}
    for(int i=1;i<n;++i)for(int j=i+1;j<=n;++j){
      random=random*1664525u+1013904223u;
      if(j==i+1 || (random>>16)%5==0)nodes[i].succ.append(j);
    }
    expected=nodes;
    slevelvisit(1,nodes.data());referenceLevel(1,expected.data());
    for(int i=1;i<=n;++i)FF_CHECK(nodes[i].level==expected[i].level && nodes[i].attrib==expected[i].attrib);
  }
  std::cout << "200 cyclic DFS and 200 DAG rank cases match recursive reference order\n";
}
void deep() {
  struct rlimit limit;FF_CHECK(getrlimit(RLIMIT_STACK,&limit)==0);
  limit.rlim_cur=std::min<rlim_t>(limit.rlim_cur,512*1024);
  FF_CHECK(setrlimit(RLIMIT_STACK,&limit)==0);
  constexpr int n=20000;
  std::vector<SNode> nodes(n+1);
  for(int i=1;i<n;++i)nodes[i].succ.append(i+1);
  int dfs=1,comp=0;nodes[1].dfs=1;sdfsvisit(1,nodes.data(),dfs,comp);
  FF_CHECK(dfs==n && comp==n-1 && nodes[1].comp==0);
  for(int i=2;i<=n;++i)FF_CHECK(nodes[i].dfs==i && nodes[i].comp==n-i+1);
  slevelvisit(1,nodes.data());
  for(int i=1;i<=n;++i)FF_CHECK(nodes[i].level==i-1);
  std::cout << "20,000-deep DFS and rank propagation passed with at most 512 KiB stack\n";
}
void fullLayout() {
  struct Sugi:FFSugi{void init(const FFGraph& graph)override{FFLayoutLabel::init(graph);}};
  FFGraph graph(YES);graph->_setDirected(YES);
  graph->setNodeIndex(graph->addNodeLabel(new FFGraphicNode));
  graph->setEdgeIndex(graph->addEdgeLabel(new FFGraphicEdge));
  constexpr int count=1024;
  std::vector<FFNode> nodes;
  for(int i=0;i<count;++i){auto* node=graph->addNode(FFPoint(i,i,0));node->setValue(i+1);nodes.push_back(node);}
  for(int i=1;i<count;++i){auto* edge=graph->addEdge(nodes[i-1],nodes[i]);edge->setValue(i);}
  auto* longEdge=graph->addEdge(nodes.front(),nodes.back());longEdge->setValue(count);
  auto* layout=new Sugi;graph->addGraphLabel(layout);layout->layout();
  FF_CHECK(graph->getNumNodes()==count && graph->getNumEdges()==count);
  FF_CHECK(longEdge->source()==nodes.front() && longEdge->target()==nodes.back());
  FF_CHECK(longEdge->getGraphicEdge()->anzPoints()>=4);
  for(auto* node:nodes)for(float x:node->getGraphicNode()->getCoords().v)FF_CHECK(std::isfinite(x));
  for(int i=0;i<longEdge->getGraphicEdge()->anzPoints();++i)
    for(float x:longEdge->getGraphicEdge()->getPointAt(i).v)FF_CHECK(std::isfinite(x));
  std::cout << "1,024-level public layout and long-edge reconstruction passed\n";
}
}
int main(int argc,char**argv) {
  if(argc==2 && std::string(argv[1])=="--deep")deep();
  else {numeric();referenceComparisons();fullLayout();}
}
