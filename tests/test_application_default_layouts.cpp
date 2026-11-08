// Exercise the real application registrations without initializing Tk/inspectors.
#include "misc/FFDefaultLayouts.h"
#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFGraphLabel.h>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>

extern FFSet<FFGraphLabel*> defGraphLabels;
namespace {
void checkRegistration() {
  ffgraph_app::registerDefaultLayouts();
  const std::set<std::string> expected = {
    "FFSugiyama", "FFSpringembedder", "FFKaese", "FFRandom", "FFShow", "FFOrtho"};
  std::set<std::string> actual;
  for (Pix p = defGraphLabels.first(); p; defGraphLabels.next(p)) {
    auto* prototype = defGraphLabels(p);
    actual.insert(prototype->baseType().c_str());
    std::unique_ptr<FFGraphLabel> clone(prototype->proto());
    if (!clone || clone.get() == prototype || clone->type() != prototype->type())
      throw std::runtime_error("default prototype did not create an independent clone");
  }
  if (actual != expected || defGraphLabels.length() != 6)
    throw std::runtime_error("application default layouts changed");
}
}
int main(int argc, char** argv) {
  try {
    checkRegistration();
    std::cout << "application defaults: six prototypes and independent clones verified\n";
    if (argc == 2 && std::string(argv[1]) == "--exit") std::exit(0);
    if (argc != 1) throw std::runtime_error("usage: test_application_default_layouts [--exit]");
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
