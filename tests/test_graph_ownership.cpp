// Regression test: FFGraph is a reference-counted handle, not a graph copy.
// In particular, self-assignment must not destroy the only owning graph.
#include <cassert>
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFGraphClass.h>

int main()
{
  FFGraph first(YES);
  FFGraphClass* original = first.operator->();

  first = first;                 // Sole owner: used to free the graph.
  assert(first.operator->() == original);

  FFGraph alias(first);
  first = alias;                 // Two handles for the same graph.
  assert(first.operator->() == original);
  assert(alias.operator->() == original);

  FFGraph second(YES);
  FFGraphClass* other = second.operator->();
  first = second;                // Reassign across distinct graphs.
  assert(first.operator->() == other);
  assert(second.operator->() == other);
  assert(alias.operator->() == original);

  // Assign from a lazily created (as yet empty) graph handle.
  FFGraph lazy(YES);
  first = lazy;
  assert(first.operator->() == lazy.operator->());
  assert(second.operator->() == other);
}
