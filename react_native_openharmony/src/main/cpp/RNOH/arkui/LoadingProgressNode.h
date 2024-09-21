#pragma once

#include "ArkUINode.h"

namespace rnoh {

class LoadingProgressNode : public ArkUINode {
 public:
  LoadingProgressNode();
  LoadingProgressNode& setColor(facebook::react::SharedColor const& color);
  LoadingProgressNode& setAnimating(const bool& enable);
};

} // namespace rnoh