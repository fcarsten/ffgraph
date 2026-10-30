#include <cassert>
#include <String.h>

int main() {
  String value("  alpha beta  ");
  assert(value.length() == 14);
  assert(value.at(2, 5) == String("alpha"));
  assert(value.before(' ') == String(""));
  assert(value.after(' ') == String(" alpha beta  "));
  assert(value.contains("beta"));
  assert(value.index("beta") == 8);
  String editable(" x ");
  if (editable.at(0, 1) == String(" ")) editable.erase(0, 1);
  if (editable.at(static_cast<int>(editable.length()) - 1, 1) == String(" "))
    editable.erase(editable.length() - 1, 1);
  assert(editable == String("x"));
  String escaped("a\\b\"c");
  escaped.gsub("\\", "\\\\");
  assert(escaped == String("a\\\\b\"c"));
  return 0;
}
