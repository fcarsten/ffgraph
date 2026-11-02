// Regression test: FFGraph is a reference-counted handle, not a graph copy.
// In particular, self-assignment must not destroy the only owning graph.
#include "test_check.h"
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFGraphClass.h>

int main()
{
  FFGraph first(YES);
  FFGraphClass* original = first.operator->();

  first = first;                 // Sole owner: used to free the graph.
  FF_CHECK(first.operator->() == original);

  FFGraph alias(first);
  first = alias;                 // Two handles for the same graph.
  FF_CHECK(first.operator->() == original);
  FF_CHECK(alias.operator->() == original);

  FFGraph second(YES);
  FFGraphClass* other = second.operator->();
  first = second;                // Reassign across distinct graphs.
  FF_CHECK(first.operator->() == other);
  FF_CHECK(second.operator->() == other);
  FF_CHECK(alias.operator->() == original);

  // Assign from a lazily created (as yet empty) graph handle.
  FFGraph lazy(YES);
  first = lazy;
  FF_CHECK(first.operator->() == lazy.operator->());
  FF_CHECK(second.operator->() == other);
}
