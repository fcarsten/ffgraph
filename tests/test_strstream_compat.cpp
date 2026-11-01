#include "test_check.h"
#include <cstring>
#include <string>
#include <strstream.h>

int main() {
  strstream out;
  out << "value=" << 42 << ends;
  char* first = out.str();
  FF_CHECK(std::strcmp(first, "value=42") == 0);
  out.freeze(0);
  out << "next" << ends;
  FF_CHECK(std::strcmp(out.str(), "next") == 0);

  const char input[] = "alpha beta";
  istrstream in(input, 5);
  std::string word;
  in >> word;
  FF_CHECK(word == "alpha");
  return 0;
}
