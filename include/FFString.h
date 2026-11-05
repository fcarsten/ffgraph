#ifndef FFGRAPH_COMPAT_STRING_H
#define FFGRAPH_COMPAT_STRING_H

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <ostream>
#include <string>

// Compatibility facade for the GNU libg++ String API. The representation and
// ownership are provided by std::string; legacy convenience methods are kept
// only to make the first port incremental. New code should prefer std::string.
class String : public std::string {
  using Base = std::string;
  static std::string number(long long v) { return std::to_string(v); }
public:
  using Base::operator=;
  String() = default;
  String(const Base& s) : Base(s) {}
  String(Base&& s) noexcept : Base(std::move(s)) {}
  String(const char* s) : Base(s ? s : "") {}
  String(const char* s, size_type n) : Base(s, n) {}
  String(char c) : Base(1, c) {}
  String(int v) : Base(number(v)) {}
  String(unsigned int v) : Base(number(v)) {}
  String(long v) : Base(number(v)) {}
  String(unsigned long v) : Base(number(v)) {}
  String(long long v) : Base(number(v)) {}
  String(unsigned long long v) : Base(number(static_cast<long long>(v))) {}
  // GNU libg++ String exposed at(position, length); retain this read-only
  // convenience while keeping mutation explicit via replace()/erase().
  String at(int pos, int count) const {
    if (pos < 0 || count <= 0 || static_cast<size_type>(pos) >= size()) return String();
    return String(substr(static_cast<size_type>(pos), static_cast<size_type>(count)));
  }
  String at(int pos, int count) {
    return static_cast<const String&>(*this).at(pos, count);
  }

  operator const char*() const { return c_str(); }
  friend String operator+(const String& a, const String& b) { return String(static_cast<const Base&>(a) + static_cast<const Base&>(b)); }
  friend String operator+(const String& a, const char* b) { return String(static_cast<const Base&>(a) + Base(b ? b : "")); }
  friend String operator+(const char* a, const String& b) { return String(Base(a ? a : "") + static_cast<const Base&>(b)); }
  friend String operator+(const String& a, char b) { return String(static_cast<const Base&>(a) + b); }
  friend String operator+(char a, const String& b) { return String(a + static_cast<const Base&>(b)); }

  int contains(const String& needle) const { return find(needle) != npos; }
  int contains(char needle) const { return find(needle) != npos; }
  int index(const String& needle, int start = 0) const {
    if (start < 0) {
      if (empty()) return -1;
      auto p = rfind(needle);
      return p == npos ? -1 : static_cast<int>(p);
    }
    auto p = find(needle, static_cast<size_type>(start));
    return p == npos ? -1 : static_cast<int>(p);
  }
  int index(char needle, int start = 0) const {
    if (start < 0) { auto p = rfind(needle); return p == npos ? -1 : static_cast<int>(p); }
    auto p = find(needle, static_cast<size_type>(start));
    return p == npos ? -1 : static_cast<int>(p);
  }
  String before(int pos) const {
    if (pos < 0) return String();
    return String(substr(0, static_cast<size_type>(pos)));
  }
  String before(char needle) const { auto p=find(needle); return String(substr(0,p)); }
  String before(const String& needle) const { auto p=find(needle); return String(substr(0,p)); }
  String after(int pos) const {
    if (pos < 0) return *this;
    if (static_cast<size_type>(pos) >= size()) return String();
    return String(substr(static_cast<size_type>(pos)+1));
  }
  String after(char needle) const { auto p=find(needle); return p==npos ? String() : String(substr(p+1)); }
  String after(const String& needle) const { auto p=find(needle); return p==npos ? String() : String(substr(p+needle.size())); }
  String from(int pos) const { if(pos<0) pos=0; return pos>=static_cast<int>(size()) ? String() : String(substr(static_cast<size_type>(pos))); }
  int freq(const String& needle) const {
    if (needle.empty()) return 0;
    int count=0; size_type p=0;
    while ((p=find(needle,p)) != npos) { ++count; p += needle.size(); }
    return count;
  }
  String& gsub(const String& from, const String& to) {
    if (from.empty()) return *this;
    size_type p=0;
    while ((p=find(from,p)) != npos) { replace(p,from.size(),to); p += to.size(); }
    return *this;
  }
};

template<class T> inline String dec(T value) { return String(static_cast<long long>(value)); }

inline unsigned int hashpjw(const String& value) {
  unsigned int h = 0, g = 0;
  for (unsigned char c : value) {
    h = (h << 4) + c;
    if ((g = h & 0xF0000000u) != 0) { h ^= g >> 24; h ^= g; }
  }
  return h;
}

// Legacy GNU libg++ whitespace regular-expression marker used by split/index.
// It is intentionally represented as a simple marker; call sites are migrated
// to explicit whitespace handling in the portability pass.
#ifndef RXwhite
#define RXwhite " "
#endif

#endif
