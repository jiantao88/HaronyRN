#pragma once
#include <react/renderer/components/view/ViewShadowNode.h>
#include "RNOH/CppComponentInstance.h"
#include "RNOH/arkui/CustomNode.h"

namespace rnoh {
class CustomNodeComponentInstance
    : public CppComponentInstance<facebook::react::ViewShadowNode>,
      public CustomNodeDelegate {
 private:
  CustomNode m_customNode;
  std::vector<bool> m_childrenClippedState;
  facebook::react::Point m_previousOffset;

  bool isViewClipped(
      const ComponentInstance::Shared& child,
      facebook::react::Point currentOffset,
      facebook::react::Rect parentBoundingBox);
 public:
  CustomNodeComponentInstance(Context context);

  void onChildInserted(
      ComponentInstance::Shared const& childComponentInstance,
      std::size_t index) override;
  void onChildRemoved(
      ComponentInstance::Shared const& childComponentInstance) override;

  void onPropsChanged(SharedConcreteProps const& props) override;

  void updateClippedSubviews(bool childrenChange = false);
  void restoreClippedSubviews();

  void onFinalizeUpdates() override;
  void onClick() override;
  void onHoverIn() override;
  void onHoverOut() override;
  CustomNode& getLocalRootArkUINode() override;
};
} // namespace rnoh