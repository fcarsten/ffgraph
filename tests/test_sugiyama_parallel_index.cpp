#include "test_check.h"
#include <ffgraph/FFsugiyama.h>
#include <vector>

extern void rem_par_edges(SNode*,SEdge*,int,int&,int,DLList<int>&);
extern void change_edges(SNode*,SEdge*,DLList<int>&);
namespace {
#include "sugiyama_parallel_reference.inc"
std::vector<int> values(const DLList<int>& list) {
  std::vector<int> result;
  for (Pix p = list.first(); p; list.next(p)) result.push_back(list(p));
  return result;
}
void equal(const std::vector<SNode>& a, const std::vector<SNode>& b,
           const std::vector<SEdge>& x, const std::vector<SEdge>& y) {
  for (std::size_t i = 1; i < a.size(); ++i) {
    FF_CHECK(a[i].indeg == b[i].indeg && a[i].outdeg == b[i].outdeg);
    FF_CHECK(values(a[i].pred) == values(b[i].pred) && values(a[i].succ) == values(b[i].succ));
  }
  for (std::size_t i = 1; i < x.size(); ++i)
    FF_CHECK(x[i].type == y[i].type && x[i].source == y[i].source && x[i].target == y[i].target);
}
}
int main() {
  for (int seed = 0; seed < 200; ++seed) {
    constexpr int count = 10, edgeCount = 60;
    std::vector<SNode> a(count+1), b(count+1);
    std::vector<SEdge> x(edgeCount+1), y(edgeCount+1);
    for (int i = 1; i <= edgeCount; ++i) {
      const int source = (i*7+seed)%count+1, target = (i*3+seed/3)%count+1;
      x[i].source = y[i].source = source; x[i].target = y[i].target = target;
      x[i].type = y[i].type = i%11 == 0 ? 1 : 0;
      if (x[i].type == 0) for (auto* nodes : {a.data(),b.data()}) {
        nodes[source].succ.append(target); ++nodes[source].outdeg;
        nodes[target].pred.append(source); ++nodes[target].indeg;
      }
    }
    DLList<int> actual, reference; actual.append(999); reference.append(999);
    for (int type : {2,3}) {
      int n = -1, m = -1;
      rem_par_edges(a.data(),x.data(),edgeCount,n,type,actual);
      reference_rem_par_edges(b.data(),y.data(),edgeCount,m,type,reference);
      FF_CHECK(n == m && values(actual) == values(reference)); equal(a,b,x,y);
      DLList<int> reverse;
      for (int i = 1; i <= edgeCount; ++i) if (x[i].type == 0 && x[i].source > x[i].target) reverse.append(i);
      change_edges(a.data(),x.data(),reverse); change_edges(b.data(),y.data(),reverse);
    }
  }
  std::cout << "200 multigraphs match original preprocessing before/after reversal\n";
}
