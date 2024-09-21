#pragma once
#include <react/renderer/components/view/ViewShadowNode.h>
#include "RNOH/CppComponentInstance.h"
#include "RNOH/arkui/ArkUINode.h"
#include "arkui/ArkUINode.h"

namespace rnoh {
/**
 * This ComponentInstance is created when no other ComponentInstance was found.
 * It is used for backward compatibility reasons with ArkTS-based architecture.
 */
class FallbackComponentInstance
    : public CppComponentInstance<facebook::react::ViewShadowNode>,
      public ArkUINodeDelegate {
 private:
  std::unique_ptr<ArkUINode> m_arkUINode;
  std::function<void()> m_arkUIBuilderNodeDestroyer;

 public:
  FallbackComponentInstance(
      Context ctx,
      std::unique_ptr<ArkUINode> arkUINode,
      std::function<void()>&& arkUIBuilderNodeDestroyer)
      : CppComponentInstance(ctx),
        m_arkUINode(std::move(arkUINode)),
        m_arkUIBuilderNodeDestroyer(std::move(arkUIBuilderNodeDestroyer)) {
    m_arkUINode->setArkUINodeDelegate(this);
  };

  ArkUINode& getLocalRootArkUINode() override {
    return *m_arkUINode;
  };

  void onArkUINodeDestroy(ArkUINode* node) override {
    m_arkUIBuilderNodeDestroyer();
  }

  void setLayout(facebook::react::LayoutMetrics layoutMetrics) override {
    // Attributes are not set on the C++ side to prevent conflicts with ArkUI
    // front-end state management.
  }

 protected:
  void onPropsChanged(const SharedConcreteProps& props) override {
    // NOOP: Props are set on ArkTS side.
  }

  void onChildInserted(
      ComponentInstance::Shared const& childComponentInstance,
      std::size_t index) override {
    // The child node is added on the ArkTS side.
  }
};
} // namespace rnoh
