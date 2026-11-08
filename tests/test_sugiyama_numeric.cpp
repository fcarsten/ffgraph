#include "test_check.h"
#include "sugiyama_layout_snapshot.h"
#include "flayout/FFsugi_numeric.h"
#include "flayout/FFsugi_sizes.h"
#include <ffgraph/FFSugi.h>
#include <ffgraph/ffError.h>
#include <climits>
#include <limits>
#include <string>
#include <vector>

extern charErrorHandler* userCharErrorHandler;
extern void correct_lines(SNode*,int*,int,int,int,FFCoord,int,int);
extern int upper_barycenter(int,SNode*,int);
extern void make_node_list_down(int*,SNode*,int,int*,int*,int*,int,int);
namespace {
int rejectedLayouts=0;
struct Sugi:FFSugi{void init(const FFGraph& g)override{FFLayoutLabel::init(g);}};
template<class E,class F> void rejects(F f) {
  bool rejected=false;try{f();}catch(const E&){rejected=true;}FF_CHECK(rejected);
}
void helpers() {
  using namespace ffgraph_sugiyama;
  FF_CHECK(coordinateInt(17.9)==17 && coordinateInt(-17.9)==-17);
  FF_CHECK(coordinateInt(double(INT_MAX)+0.5)==INT_MAX);
  FF_CHECK(coordinateInt(double(INT_MIN)-0.5)==INT_MIN);
  rejects<NumericError>([]{coordinateInt(2147483648.0f);});
  rejects<NumericError>([]{coordinateInt(std::numeric_limits<float>::infinity());});
  rejects<NumericError>([]{coordinateInt(std::numeric_limits<double>::quiet_NaN());});
  rejects<NumericError>([]{signedInt(std::int64_t(INT_MAX)+1);});
  rejects<NumericError>([]{signedInt(std::int64_t(INT_MIN)-1);});
  rejects<NumericError>([]{finitePoint(FFPoint(0,0,1e30f));});
  SNode nodes[3];nodes[1].point.v[0]=16777216;nodes[2].point.v[0]=16777218;
  rejects<NumericError>([&]{correct_lines(nodes,nullptr,1,2,2,16777216,0,0);});
  nodes[1].point.v[0]=1e30f;
  rejects<NumericError>([&]{upper_barycenter(1,nodes,0);});
  nodes[1].outdeg=INT_MAX;int order[]={1,1},number[2],priority[2],done[2];
  rejects<SizeError>([&]{make_node_list_down(order,nodes,0,number,priority,done,0,1);});
}
void settings() {
  Sugi s;
  using Setter=BOOL(FFSugi::*)(FFCoord);
  const Setter setters[]={&FFSugi::setxoff,&FFSugi::setyoff,&FFSugi::setzoff,
                          &FFSugi::setxDistance,&FFSugi::setyDistance,&FFSugi::setzDistance};
  for(auto setter:setters) {
    FF_CHECK((s.*setter)(17));
    for(float bad:{std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::infinity(),
                   -std::numeric_limits<float>::infinity()})FF_CHECK(!(s.*setter)(bad));
  }
  FF_CHECK(s.getxoff()==17 && s.getyoff()==17 && s.getzoff()==17 &&
           s.getxDistance()==17 && s.getyDistance()==17 && s.getzDistance()==17);
}
void layout(int mode,int orientation) {
  FFGraph graph(YES);graph->_setDirected(YES);
  graph->setNodeIndex(graph->addNodeLabel(new FFGraphicNode));
  graph->setEdgeIndex(graph->addEdgeLabel(new FFGraphicEdge));
  std::vector<FFNode> nodes;
  for(int i=0;i<6;++i){auto* n=graph->addNode(FFPoint(i*11,i*17,0));n->setValue(100+i);nodes.push_back(n);}
  std::vector<FFEdge> edges;
  for(int i=1;i<6;++i)edges.push_back(graph->addEdge(nodes[i-1],nodes[i]));
  edges.push_back(graph->addEdge(nodes[0],nodes[5]));
  for(std::size_t i=0;i<edges.size();++i){edges[i]->setValue(200+int(i));edges[i]->getGraphicEdge()->addPointAt(1,FFPoint(77,88,99));}
  const auto identities=ffgraph_test::layoutIds(*graph.operator->());
  auto* s=new Sugi;graph->addGraphLabel(s);s->setOrient(orientation);
  if(mode==0){s->setUseGrid(NO);FF_CHECK(s->setxDistance(1000000000));nodes[0]->getGraphicNode()->setWidth(2000000000);}
  if(mode==1)nodes[0]->getGraphicNode()->setHeight(std::numeric_limits<float>::quiet_NaN());
  if(mode==2)FF_CHECK(s->setxoff(3000000000.0f));
  if(mode==3){s->setUseGrid(NO);FF_CHECK(s->setxDistance(1000000));}
  const auto original=ffgraph_test::captureLayout(*graph.operator->(),identities);
  const int before=rejectedLayouts;s->layout();
  if(mode==3){FF_CHECK(rejectedLayouts==before);return;}
  FF_CHECK(rejectedLayouts==before+1);
  const auto after=ffgraph_test::captureLayout(*graph.operator->(),identities);
  FF_CHECK(original.nodes==after.nodes && original.edges==after.edges);
  for(int i=0;i<6;++i)FF_CHECK(nodes[i]->getValue()==100+i);
  for(std::size_t i=0;i<edges.size();++i)FF_CHECK(edges[i]->getValue()==200+int(i));
}
}
int main() {
  helpers();settings();
  auto old=userCharErrorHandler;
  userCharErrorHandler=[](errGrade grade,const char* message,const char*,int){
    FF_CHECK(grade==USER_ERROR && std::string(message).find("Sugiyama coordinates")!=std::string::npos);++rejectedLayouts;
  };
  for(int orient:{1,2})for(int mode=0;mode<4;++mode)layout(mode,orient);
  userCharErrorHandler=old;
  std::cout << "numeric boundary/progress checks, nonfinite settings and six preserved rejected layouts passed\n";
}
