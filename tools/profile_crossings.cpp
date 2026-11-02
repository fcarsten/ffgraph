// Profile the real Sugiyama counter against a tool-only prefix-count prototype.
// This executable is opt-in; the application never links the prototype.
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#if __has_include(<valgrind/callgrind.h>)
#include <valgrind/callgrind.h>
#else
#define CALLGRIND_START_INSTRUMENTATION ((void)0)
#define CALLGRIND_STOP_INSTRUMENTATION ((void)0)
#endif

// Actual production function in FFreduce_crossing.cc; no public API is added.
extern int number_of_crossings(int*, int, int, int);
namespace {
void require(bool okay, const char* message) {
  if (!okay) throw std::runtime_error(message);
}
struct Matrix {
  int rows, cols, stride;
  std::vector<int> cells;
  Matrix(int r, int c, int pitch = 0) : rows(r), cols(c),
      stride(pitch ? pitch : std::max(r, c) + 3), cells((r + 2) * stride, 1) {
    // Active cells are zero; dummy rows/columns and padding are sentinel ones.
    for (int i = 1; i <= rows; ++i)
      for (int j = 1; j <= cols; ++j) at(i, j) = 0;
  }
  int& at(int i, int j) { return cells[i * stride + j]; }
  int at(int i, int j) const { return cells[i * stride + j]; }
};
std::uint64_t production(const Matrix& m) {
  const int result = number_of_crossings(const_cast<int*>(m.cells.data()), m.rows, m.cols, m.stride);
  require(result >= 0, "production count overflowed");
  return static_cast<std::uint64_t>(result);
}
std::uint64_t prefixPrototype(const Matrix& m) {
  std::vector<std::uint64_t> columns(m.cols + 1, 0);
  std::uint64_t previousEdges = 0, result = 0;
  for (int row = 1; row <= m.rows; ++row) {
    auto right = previousEdges;
    for (int col = 1; col <= m.cols; ++col) {
      right -= columns[col];
      if (m.at(row, col) == 1) result += right;
    }
    // Update after querying the entire row: shared-source edges do not cross.
    for (int col = 1; col <= m.cols; ++col) {
      if (m.at(row, col) == 1) { ++columns[col]; ++previousEdges; }
    }
  }
  return result;
}
using Edge = std::pair<int, int>;
std::vector<Edge> edges(const Matrix& m) {
  std::vector<Edge> result;
  for (int row = 1; row <= m.rows; ++row)
    for (int col = 1; col <= m.cols; ++col)
      if (m.at(row, col) == 1) result.emplace_back(row, col);
  return result;
}
// Independent geometric oracle: distinct edges cross only with opposite order
// at their two ends. Shared rows/columns are excluded by strict inequalities.
std::uint64_t pairOracle(const Matrix& m) {
  const auto list = edges(m);
  std::uint64_t result = 0;
  for (std::size_t i = 0; i < list.size(); ++i)
    for (std::size_t j = i + 1; j < list.size(); ++j)
      if ((list[i].first < list[j].first && list[i].second > list[j].second) ||
          (list[j].first < list[i].first && list[j].second > list[i].second)) ++result;
  return result;
}
void verify(const Matrix& m, std::uint64_t expected) {
  require(expected <= std::numeric_limits<int>::max(), "oracle exceeds production int range");
  const auto before = m.cells;
  require(production(m) == expected, "production differs from oracle");
  require(prefixPrototype(m) == expected, "prefix prototype differs from oracle");
  require(m.cells == before, "counter modified its input");
}
void exhaustive() {
  Matrix m(4, 4);
  for (std::uint32_t mask = 0; mask < (1u << 16); ++mask) {
    for (int bit = 0; bit < 16; ++bit) m.at(bit / 4 + 1, bit % 4 + 1) = (mask >> bit) & 1;
    verify(m, pairOracle(m));
  }
  std::uint32_t state = 20261102;
  for (int sample = 0; sample < 128; ++sample) {
    Matrix rectangular(sample % 17, (sample * 7) % 19);
    for (int row = 1; row <= rectangular.rows; ++row)
      for (int col = 1; col <= rectangular.cols; ++col) {
        state = state * 1664525u + 1013904223u;
        rectangular.at(row, col) = static_cast<int>((state >> 16) % 4) - 1;
      }
    verify(rectangular, pairOracle(rectangular));
  }
  std::cout << "{\"kind\":\"verification\",\"exhaustive_4x4\":65536,"
               "\"rectangular_nonbinary\":128}\n";
}
Matrix fixture(const std::string& pattern, int rows, int cols) {
  Matrix m(rows, cols);
  std::uint32_t state = 20261102;
  for (int row = 1; row <= rows; ++row) {
    if (pattern == "complete")
      for (int col = 1; col <= cols; ++col) m.at(row, col) = 1;
    else if (pattern == "diagonal") m.at(row, row) = 1;
    else if (pattern == "reverse") m.at(row, cols - row + 1) = 1;
    else {
      for (int k = 0; k < 4; ++k) {
        state = state * 1664525u + 1013904223u;
        m.at(row, 1 + ((state >> 16) % cols)) = 1;
      }
    }
  }
  return m;
}
void measure(const Matrix& m, const std::string& pattern, const std::string& implementation,
             int repeats, std::uint64_t expected) {
  // A volatile function pointer prevents hoisting a tool-only pure call out of
  // repeated samples. Allocation of the prototype's column buffer is timed.
  using Counter = std::uint64_t (*)(const Matrix&);
  Counter volatile counter = implementation == "production" ? production : prefixPrototype;
  require(counter(m) == expected, "warmup count mismatch");
  std::vector<long long> samples;
  for (int sample = 0; sample < repeats; ++sample) {
    CALLGRIND_START_INSTRUMENTATION;
    const auto start = std::chrono::steady_clock::now();
    const auto count = counter(m);
    const auto end = std::chrono::steady_clock::now();
    CALLGRIND_STOP_INSTRUMENTATION;
    require(count == expected, "measured count mismatch");
    samples.push_back(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
  }
  auto sorted = samples;
  std::sort(sorted.begin(), sorted.end());
  const double median = (static_cast<double>(sorted[(sorted.size() - 1) / 2]) +
                         sorted[sorted.size() / 2]) / 2;
  std::cout << "{\"kind\":\"measurement\",\"pattern\":\"" << pattern
            << "\",\"implementation\":\"" << implementation << "\",\"rows\":" << m.rows
            << ",\"cols\":" << m.cols << ",\"stride\":" << m.stride
            << ",\"edges\":" << edges(m).size() << ",\"crossings\":" << expected
            << ",\"median_ns\":" << std::setprecision(12) << median << ",\"samples_ns\":[";
  for (std::size_t i = 0; i < samples.size(); ++i) std::cout << (i ? "," : "") << samples[i];
  std::cout << "]}\n";
}
}
int main(int argc, char** argv) {
  try {
    int maxWidth = 128, repeats = 5;
    bool checkOnly = false;
    std::string implementation = "both", matrixPath;
    for (int i = 1; i < argc; ++i) {
      const std::string option = argv[i];
      if (option == "--check-only") checkOnly = true;
      else if (option == "--help") {
        std::cout << "profile_crossings [--check-only] [--max-width 8..256] "
                     "[--repeat 1..100] [--implementation production|prefix|both] [--matrix FILE]\n";
        return 0;
      } else {
        require(i + 1 < argc, "missing option value");
        const std::string value = argv[++i];
        if (option == "--max-width") maxWidth = std::stoi(value);
        else if (option == "--repeat") repeats = std::stoi(value);
        else if (option == "--implementation") implementation = value;
        else if (option == "--matrix") matrixPath = value;
        else throw std::runtime_error("unknown option");
      }
    }
    require(maxWidth >= 8 && maxWidth <= 256 && repeats > 0 && repeats <= 100, "invalid limits");
    require(implementation == "production" || implementation == "prefix" || implementation == "both",
            "invalid implementation");
    exhaustive();
    if (checkOnly) return 0;
    if (!matrixPath.empty()) {
      std::ifstream input(matrixPath);
      int rows, cols, stride;
      require(bool(input >> rows >> cols >> stride) && rows > 0 && cols > 0 &&
              stride > std::max(rows, cols) && stride <= 4096,
              "invalid matrix dimensions");
      Matrix m(rows, cols, stride);
      for (int row = 1; row <= rows; ++row)
        for (int col = 1; col <= cols; ++col)
          require(bool(input >> m.at(row, col)), "missing matrix cell");
      std::string trailing;
      require(!(input >> trailing), "trailing matrix data");
      require(edges(m).size() <= 10000, "pair-oracle matrix edge limit exceeded");
      const auto expected = pairOracle(m);
      verify(m, expected);
      if (implementation != "prefix") measure(m, "captured", "production", repeats, expected);
      if (implementation != "production") measure(m, "captured", "prefix", repeats, expected);
      return 0;
    }
    for (int width = 8; width <= maxWidth; width *= 2) {
      for (const std::string pattern : {"diagonal", "reverse", "sparse", "complete"}) {
        for (int orientation = 0; orientation < (pattern == "sparse" || pattern == "complete" ? 3 : 1);
             ++orientation) {
          const auto m = fixture(pattern, orientation == 1 ? width / 4 : width,
                                orientation == 2 ? width / 4 : width);
          const std::uint64_t expected = pattern == "complete"
            ? std::uint64_t(m.rows) * (m.rows - 1) / 2 * m.cols * (m.cols - 1) / 2
            : pairOracle(m);
          verify(m, expected);
          if (implementation != "prefix") measure(m, pattern, "production", repeats, expected);
          if (implementation != "production") measure(m, pattern, "prefix", repeats, expected);
        }
      }
    }
  } catch (const std::exception& error) {
    std::cerr << "profile_crossings: " << error.what() << '\n';
    return 1;
  }
}
