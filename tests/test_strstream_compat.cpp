#include <cassert>
#include <cstring>
#include <string>
#include <strstream.h>

int main() {
  strstream out;
  out << "value=" << 42 << ends;
  char* first = out.str();
  assert(std::strcmp(first, "value=42") == 0);
  out.freeze(0);
  out << "next" << ends;
  assert(std::strcmp(out.str(), "next") == 0);

  const char input[] = "alpha beta";
  istrstream in(input, 5);
  std::string word;
  in >> word;
  assert(word == "alpha");
  return 0;
}
