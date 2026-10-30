#ifndef FFGRAPH_COMPAT_STRSTREAM_H
#define FFGRAPH_COMPAT_STRSTREAM_H

// Compatibility facade for the obsolete libg++/std::strstream API.
// Storage is owned by standard C++ string streams; no manually managed
// strstream buffer or freeze/unfreeze protocol is used.
#include <sstream>
#include <string>
#include <ostream>
#include <istream>

class strstream : public std::ostringstream {
  mutable std::basic_string<char> legacyBuffer_;
public:
  using std::ostringstream::ostringstream;
  strstream() = default;
  strstream(const strstream&) = delete;
  strstream& operator=(const strstream&) = delete;

  // Legacy callers expect a NUL-terminated mutable pointer. The pointer stays
  // valid until the next str() call or stream reset, matching the old usage
  // pattern where it was consumed immediately by Tcl/C APIs.
  char* str() {
    legacyBuffer_ = std::ostringstream::str();
    return legacyBuffer_.data();
  }

  // Legacy freeze(0) released the strstream buffer. Here it safely resets the
  // owned std::basic_string<char> buffer so static stream objects can be reused.
  void freeze(bool frozen = true) {
    if (!frozen) {
      std::ostringstream::str(std::basic_string<char>());
      std::ostringstream::clear();
      std::ostringstream::seekp(0);
      legacyBuffer_.clear();
    }
  }
};

class ostrstream : public strstream {
public:
  using strstream::strstream;
  ostrstream() = default;
};

class istrstream : public std::istringstream {
public:
  explicit istrstream(const char* s)
    : std::istringstream(s ? s : "") {}
  istrstream(const char* s, std::streamsize n)
    : std::istringstream(s ? std::basic_string<char>(s, static_cast<std::size_t>(n)) : std::basic_string<char>()) {}
  explicit istrstream(const std::basic_string<char>& s) : std::istringstream(s) {}
};

using std::ends;
using std::flush;

#endif
