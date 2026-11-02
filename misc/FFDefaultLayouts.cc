#include "FFDefaultLayouts.h"
#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFSugi.h>
#include <ffgraph/FFSpring.h>
#include <ffgraph/FFKaese.h>
#include <ffgraph/FFRandom.h>
#include <ffgraph/FFShow.h>
#include <ffgraph/FFOrtho.h>
#include <memory>

void ffgraph_app::registerDefaultLayouts()
{
  // Registration borrows these prototypes; graphs own independent proto()
  // clones. Function-static owners survive all graph construction and run
  // their destructors on std::exit(), used by the existing GUI Quit command.
  // Keep allocation/registration order and the enabled layout list unchanged.
  static const auto sugi = std::make_unique<FFSugi>();
  addDefaultGraphLabel(sugi.get());
  static const auto spring = std::make_unique<FFSpring>();
  addDefaultGraphLabel(spring.get());
  static const auto kaese = std::make_unique<FFKaese>();
  addDefaultGraphLabel(kaese.get());
  static const auto random = std::make_unique<FFRandom>();
  addDefaultGraphLabel(random.get());
  static const auto show = std::make_unique<FFShow>();
  addDefaultGraphLabel(show.get());
  static const auto ortho = std::make_unique<FFOrtho>();
  addDefaultGraphLabel(ortho.get());
}
