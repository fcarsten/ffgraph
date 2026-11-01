// Pure comparator unit tests. Does not construct real FFGraphClass objects.
#include "graph_snapshot.h"
#include <sstream>
#include <stdexcept>
#include <iostream>

using ffgraph_test::GraphSnapshot;
using ffgraph_test::difference;

void require(bool value, const char* reason) {
  if (!value) throw std::runtime_error(reason);
}

int main() {
  try {
    const char* oracle =
      "#ffgraph-corpus-oracle-v1\n"
      "graph\texample\n"
      "node\t2\tbeta\t-1.250\t5.125\n"
      "node\t1\talpha\t12.375\t-3.500\n"
      "edge\t8\t1\t2\tfirst\n"
      "edge\t9\t1\t2\tsecond\n"
      "edge\t10\t2\t2\tself\n";
    std::istringstream in(oracle);
    GraphSnapshot ref = ffgraph_test::readOracle(in);
    require(ref.nodes.size() == 2 && ref.edges.size() == 3, "oracle counts");
    require(difference(ref, ref).empty(), "reflexivity");
    auto copy = ref;
    copy.name = "other";
    require(!difference(ref, copy).empty(), "graph label regression");
    copy = ref;
    std::get<2>(copy.nodes[0]) += 1;
    require(!difference(ref, copy).empty(), "coordinate regression");
    copy = ref;
    std::get<1>(copy.nodes[0]) = "changed";
    require(!difference(ref, copy).empty(), "node label regression");
    copy = ref;
    copy.edges.erase(copy.edges.begin());
    require(!difference(ref, copy).empty(), "parallel-edge multiplicity regression");
    copy = ref;
    std::get<0>(copy.edges[0]) = 2;
    require(!difference(ref, copy).empty(), "direction regression");
    copy = ref;
    std::get<3>(copy.edges[0]) = "changed";
    require(!difference(ref, copy).empty(), "edge label regression");
    copy = ref;
    std::get<2>(copy.edges[0]) = 100;
    require(!difference(ref, copy).empty(), "edge ID regression");
    bool caught = false;
    try { std::istringstream empty("node\t1\tx\t0\t0\n");
          (void)ffgraph_test::readOracle(empty); }
    catch (const std::runtime_error&) { caught = true; }
    require(caught, "invalid oracle must be rejected");
    std::cout << "graph snapshot comparator unit checks passed\n";
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
