#include "test_check.h"
#include <ffgraph/FFreduce_crossing.h>
#include <algorithm>
#include <vector>

extern FFCoord row_barycenter(int*,int,int,int);
extern FFCoord column_barycenter(int*,int,int,int);
extern void exch_double(FFCoord*,FFCoord*);
extern void exch_rows(int*,int,int,int);
extern void exch_columns(int*,int,int,int);
extern int b_r(int*,int*,int,int,int);
extern int b_c(int*,int*,int,int,int);
namespace {
#include "sugiyama_sort_reference.inc"
}
int main() {
  unsigned cases = 0;
  for (auto [rows,columns] : {std::pair<int,int>{4,3},{3,4},{3,3},{4,2},{2,4},{0,4},{4,0}}) {
    const int stride = std::max(rows,columns)+3;
    for (unsigned bits = 0; bits < (1u << (rows*columns)); ++bits)
      for (bool rowWise : {false,true}) for (int pairing : {0,1,2}) {
        std::vector<int> original(stride*stride), paired(stride*stride);
        for (int i = 0; i < stride*stride; ++i) { original[i] = 50000+i; paired[i] = -10000-i; }
        for (int r = 1; r <= rows; ++r) for (int c = 1; c <= columns; ++c)
          original[r*stride+c] = (bits >> ((r-1)*columns+c-1)) & 1;
        auto actual = original, expected = original, partner = paired, referencePartner = paired;
        int* a = pairing == 0 ? nullptr : pairing == 1 ? partner.data() : actual.data();
        int* b = pairing == 0 ? nullptr : pairing == 1 ? referencePartner.data() : expected.data();
        FF_CHECK((rowWise ? b_r : b_c)(actual.data(),a,rows,columns,stride) == 0);
        FF_CHECK((rowWise ? reference_b_r : reference_b_c)(expected.data(),b,rows,columns,stride) == 0);
        FF_CHECK(actual == expected && partner == referencePartner);
        ++cases;
      }
  }
  std::cout << cases << " full-stride barycenter permutations match the original bubble operations\n";
}
