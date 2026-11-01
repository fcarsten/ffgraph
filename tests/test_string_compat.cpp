#include "test_check.h"
#include <String.h>

int main() {
  String value("  alpha beta  ");
  FF_CHECK(value.length() == 14);
  FF_CHECK(value.at(2, 5) == String("alpha"));
  FF_CHECK(value.before(' ') == String(""));
  FF_CHECK(value.after(' ') == String(" alpha beta  "));
  FF_CHECK(value.contains("beta"));
  FF_CHECK(value.index("beta") == 8);
  String editable(" x ");
  if (editable.at(0, 1) == String(" ")) editable.erase(0, 1);
  if (editable.at(static_cast<int>(editable.length()) - 1, 1) == String(" "))
    editable.erase(editable.length() - 1, 1);
  FF_CHECK(editable == String("x"));
  String escaped("a\\b\"c");
  escaped.gsub("\\", "\\\\");
  FF_CHECK(escaped == String("a\\\\b\"c"));
  return 0;
}
