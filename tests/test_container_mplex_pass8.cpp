#include <container/MPlex.h>
#include "../container/MFPlex.cc"
#include "test_check.h"
#include <climits>
#include <map>
#include <random>
#include <stdexcept>

struct ThrowOnCopy {
  int value = 0;
  static int copies_until_throw;
  ThrowOnCopy() = default;
  explicit ThrowOnCopy(int v) : value(v) {}
  ThrowOnCopy(const ThrowOnCopy& other) : value(other.value) {
    if (copies_until_throw == 0) throw std::runtime_error("copy failed");
    if (copies_until_throw > 0) --copies_until_throw;
  }
  ThrowOnCopy& operator=(const ThrowOnCopy& other) {
    if (copies_until_throw == 0) throw std::runtime_error("assignment failed");
    if (copies_until_throw > 0) --copies_until_throw;
    value = other.value;
    return *this;
  }
};
int ThrowOnCopy::copies_until_throw = -1;

template<class F> bool out_of_range(F f) {
  try { f(); } catch (const std::out_of_range&) { return true; }
  return false;
}

int main() {
  // Check sparse indices and existing Pix before and after logical deletion.
  MPlex<int> a;
  a.add_high(10); a.add_high(20); a.add_high(30);
  Pix middle = a.index_to_Pix(1);
  FF_CHECK(a.del_Pix(middle) == 1 && !a.owns(middle));
  FF_CHECK(a.Pix_to_index(middle) == 1); // deleted slots still have an index
  a.undel_Pix(middle);
  FF_CHECK(a.owns(middle) && a(middle) == 20);
  a.del_index(0); a.del_index(2);
  FF_CHECK(a.low() == 1 && a.high() == 1 && a.count() == 1 && a.OK());
  a.adjust_bounds();
  FF_CHECK(a.length() == 1 && a.low() == 1 && a.high() == 1 && a.OK());
  a.del_low(); FF_CHECK(a.empty() && a.length() == 0 && a.OK());
  FF_CHECK(out_of_range([&] { a.del_low(); }));
  FF_CHECK(out_of_range([&] { a.del_high(); }));

  // Exercise repeated sparse mutations against an independent reference model.
  std::mt19937 rng(0xBADC0DE);
  MPlex<int> p;
  std::map<int, int> values;
  int lo = 0, fence = 0;
  for (int step = 0; step < 3000; ++step) {
    int choice = static_cast<int>(rng() % 8);
    int value = static_cast<int>(rng() % 10000);
    if (choice == 0) {
      FF_CHECK(p.add_high(value) == fence);
      values[fence++] = value;
    } else if (choice == 1) {
      FF_CHECK(p.add_low(value) == lo - 1);
      values[--lo] = value;
    } else if (choice == 2 && !values.empty()) {
      auto it = values.begin(); std::advance(it, rng() % values.size());
      const int idx = it->first;
      FF_CHECK(p.del_index(idx) == 1);
      values.erase(it);
    } else if (choice == 3 && !values.empty()) {
      auto it = values.begin(); std::advance(it, rng() % values.size());
      const int idx = it->first;
      FF_CHECK(p[idx] == it->second);
      FF_CHECK(p.index_to_Pix(idx) && p.owns(p.index_to_Pix(idx)));
    } else if (choice == 4) {
      p.adjust_bounds();
      if (values.empty()) { lo = fence; }
      else { lo = values.begin()->first; fence = values.rbegin()->first + 1; }
    } else if (choice == 5 && !values.empty()) {
      p.del_high();
      int top = values.rbegin()->first;
      values.erase(top);
      fence = top;
    } else if (choice == 6 && !values.empty()) {
      p.del_low();
      int bottom = values.begin()->first;
      values.erase(bottom);
      lo = bottom + 1;
    } else if (choice == 7 && (rng() % 13 == 0)) {
      p.clear(); values.clear(); fence = lo;
    }
    FF_CHECK(p.OK());
    FF_CHECK(p.count() == static_cast<int>(values.size()));
    FF_CHECK(p.length() == fence - lo);
    FF_CHECK(p.low() == (values.empty() ? fence : values.begin()->first));
    FF_CHECK(p.high() == (values.empty() ? lo - 1 : values.rbegin()->first));
    for (const auto& entry : values) FF_CHECK(p[entry.first] == entry.second);
    int seen = 0;
    for (Pix pix = p.first(); pix; p.next(pix)) {
      int idx = p.Pix_to_index(pix);
      FF_CHECK(values.count(idx) && p(pix) == values.at(idx));
      ++seen;
    }
    FF_CHECK(seen == p.count());
  }

  // Failed construction of a new element must not corrupt indices or count.
  MPlex<ThrowOnCopy> throwing;
  throwing.add_high(ThrowOnCopy(1));
  ThrowOnCopy element(2);
  ThrowOnCopy::copies_until_throw = 1; // parameter copy succeeds, slot copy fails
  try { throwing.add_high(element); FF_CHECK(false); } catch (const std::runtime_error&) {}
  FF_CHECK(throwing.OK() && throwing.length() == 1 && throwing.count() == 1);
  ThrowOnCopy::copies_until_throw = 1;
  try { throwing.add_low(element); FF_CHECK(false); } catch (const std::runtime_error&) {}
  FF_CHECK(throwing.OK() && throwing.length() == 1 && throwing.count() == 1);
  ThrowOnCopy::copies_until_throw = -1;

  // Copy assignment must also leave the destination unchanged on failure.
  MPlex<ThrowOnCopy> src, dst;
  src.add_high(ThrowOnCopy(7)); src.add_high(ThrowOnCopy(8));
  dst.add_high(ThrowOnCopy(99));
  ThrowOnCopy::copies_until_throw = 0;
  try { dst = src; FF_CHECK(false); } catch (const std::runtime_error&) {}
  ThrowOnCopy::copies_until_throw = -1;
  FF_CHECK(dst.OK() && dst.length() == 1 && dst[0].value == 99);

  // Overflow must fail without modifying the object (UBSan should stay silent).
  MPlex<int> top(INT_MAX - 1, 1);
  FF_CHECK(top.add_high(1) == INT_MAX - 1);
  FF_CHECK(out_of_range([&] { top.add_high(2); }));
  FF_CHECK(top.OK() && top.length() == 1 && top.high() == INT_MAX - 1);
  FF_CHECK(out_of_range([&] { top.reset_low(INT_MAX); }));
  FF_CHECK(top.OK() && top.low() == INT_MAX - 1);
  MPlex<int> bottom(INT_MIN + 1, 1);
  FF_CHECK(bottom.add_low(1) == INT_MIN);
  FF_CHECK(out_of_range([&] { bottom.add_low(2); }));
  FF_CHECK(bottom.OK() && bottom.low() == INT_MIN);
  FF_CHECK(out_of_range([&] { (void)bottom.ecnef(); }));
  bottom.del_index(INT_MIN);
  FF_CHECK(out_of_range([&] { (void)bottom.high(); }));
  // The MFPlex base also supports an empty, chunkless state.
  MPlex<int> chunkless;
  FF_CHECK(chunkless.MFPlex<int>::reset_low(7) == 0);
  FF_CHECK(chunkless.empty() && chunkless.low() == 7 && chunkless.OK());
  FF_CHECK(chunkless.MFPlex<int>::reset_low(INT_MAX) == 7);
  FF_CHECK(chunkless.empty() && chunkless.low() == INT_MAX && chunkless.OK());
  int data[3] = {1, 2, 3};
  MFIChunk<int> chunk(data, 0, 0, 2, 3);
  FF_CHECK(out_of_range([&] { chunk.re_index(INT_MAX - 2); }));
  FF_CHECK(chunk.low_index() == 0 && chunk.top_index() == 3 && chunk.OK());
  chunk.re_index(10);
  FF_CHECK(chunk.low_index() == 10 && chunk.top_index() == 13 && chunk.OK());

  MPlex<int> shifted;
  shifted.add_high(3); shifted.add_high(4);
  FF_CHECK(out_of_range([&] { shifted.reset_low(INT_MAX); }));
  FF_CHECK(shifted.OK() && shifted[0] == 3 && shifted[1] == 4);
  Pix before_shift = shifted.index_to_Pix(1);
  FF_CHECK(shifted.reset_low(1000) == 0);
  FF_CHECK(shifted.OK() && shifted.owns(before_shift));
  FF_CHECK(shifted.Pix_to_index(before_shift) == 1001 && shifted(before_shift) == 4);
  shifted.append(shifted);
  FF_CHECK(shifted.OK() && shifted.length() == 4);
  shifted.prepend(shifted);
  FF_CHECK(shifted.OK() && shifted.length() == 8);
}
