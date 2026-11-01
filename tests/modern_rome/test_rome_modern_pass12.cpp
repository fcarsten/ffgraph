#include "rome_records.h"
#include "../test_check.h"
#include <limits>
#include <string>
#include <thread>
#include <vector>

using ffgraph_rome::parse;
using ffgraph_rome::Kind;
int main() {
  const auto result = parse("# comment\n-1 99\n2 0\n#\n10 123 -1 2");
  FF_CHECK(result.ok && result.error.empty() && result.records.size() == 3);
  FF_CHECK(result.records[0].kind == Kind::Node && result.records[0].id == -1);
  FF_CHECK(result.records[0].line == 2 && result.records[1].line == 3);
  FF_CHECK(result.records[2].kind == Kind::Edge && result.records[2].id == 10);
  FF_CHECK(result.records[2].source == -1 && result.records[2].target == 2);
  FF_CHECK(result.records[2].line == 5);
  FF_CHECK(parse("").ok && parse("\n#\n# comment").records.empty());
  FF_CHECK(parse("1 0\r\n2 0\r\n3 0 1 2\r").records.size() == 3);
  // Deliberately retain legacy punctuation and unmatched-quote behavior.
  FF_CHECK(parse("@+1,0;\n\"2 0\n3 0 1 2").records.size() == 3);
  const auto limits = parse("-2147483648 2147483647\n2147483647 -2147483648\n");
  FF_CHECK(limits.ok && limits.records[0].id == std::numeric_limits<int>::min());
  FF_CHECK(limits.records[1].id == std::numeric_limits<int>::max());

  const std::vector<std::string> invalid = {
    "1", "1 2 3", "1 2 3 4 5", "{}", "[]", "# inline\n  # text\n",
    "1 0.5", "1 -1.2", "keyword", "\"string\"", "\"escaped \\\" quote\"",
    "\"multi\nline\"", "1 0\n2 0\n3 0 0\n", "2147483648 0", "-2147483649 0",
    "1 2147483648", "1 0\n99999999999999999999999999999999999999999999 0",
    std::string("1 0\n\0ignored\n", 14)
  };
  for (int repeat = 0; repeat < 40; ++repeat) {
    for (const auto& text : invalid) {
      const auto rejected = parse(text);
      FF_CHECK(!rejected.ok && !rejected.error.empty() && rejected.records.empty());
      const auto recovered = parse("7 0\n");
      FF_CHECK(recovered.ok && recovered.records.size() == 1 && recovered.records[0].id == 7);
    }
  }
  const auto located = parse("1 0\n2 0\n3 0 0\n");
  FF_CHECK(located.error.find("line 3, column 6") != std::string::npos);
  const auto overflow = parse("1 0\n2 2147483648\n");
  FF_CHECK(overflow.error.find("out of range in line 2, column 3") != std::string::npos);
  std::string large;
  for (int i = 0; i < 20000; ++i) large += std::to_string(i) + " 0\n";
  const auto many = parse(large);
  FF_CHECK(many.ok && many.records.size() == 20000 && many.records.back().line == 20000);
  const auto longLine = parse("#" + std::string(65536, 'x') + "\n1 0\n");
  FF_CHECK(longLine.ok && longLine.records.size() == 1);
  // Results own all semantic data and survive subsequent scanner lifetimes.
  FF_CHECK(result.records[2].id == 10 && result.records[2].line == 5);
  std::vector<std::thread> workers;
  for (int worker = 0; worker < 4; ++worker) workers.emplace_back([worker] {
    for (int repeat = 0; repeat < 100; ++repeat) {
      const auto own = parse(std::to_string(worker) + " 0\n");
      FF_CHECK(own.ok && own.records.size() == 1 && own.records[0].id == worker);
      const auto bad = parse("\n\ninvalid");
      FF_CHECK(!bad.ok && bad.records.empty() && bad.error.find("line 3") != std::string::npos);
    }
  });
  for (auto& worker : workers) worker.join();
}
