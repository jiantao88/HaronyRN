#pragma once

#include <react/renderer/components/text/ParagraphShadowNode.h>
#include "RNOH/CppComponentInstance.h"
#include "RNOH/arkui/StackNode.h"
#include "RNOH/arkui/TextNode.h"
#include "../../RNOH/TextMeasureRegistry.h"

namespace rnoh {
class TextComponentInstance
    : public CppComponentInstance<facebook::react::ParagraphShadowNode>,
      public TextNodeDelegate {
 private:
  using FragmentTouchTargetByTag =
      std::unordered_map<facebook::react::Tag, std::shared_ptr<TouchTarget>>;

  TextNode m_textNode{};
  StackNode* m_stackNodePtr = nullptr;
  std::vector<std::shared_ptr<ArkUINode>> m_childNodes{};
  FragmentTouchTargetByTag m_fragmentTouchTargetByTag{};
  bool m_touchTargetChildrenNeedUpdate = false;
  bool m_hasCheckNesting = false;
  std::string m_key;
 public:
  TextComponentInstance(Context context);
  ~TextComponentInstance() override;
  ArkUINode& getLocalRootArkUINode() override;
  std::vector<TouchTarget::Shared> getTouchTargetChildren() override;
  bool checkUpdateBaseNode() override;
  void setShadowView(facebook::react::ShadowView const& shadowView) override;
 protected:
  void onChildInserted(
      ComponentInstance::Shared const& childComponentInstance,
      std::size_t index) override;
  void onChildRemoved(
      ComponentInstance::Shared const& childComponentInstance) override;
  void onPropsChanged(SharedConcreteProps const& props) override;
  void onStateChanged(SharedConcreteState const& textState) override;

 private:
  void setTextAttributes(const facebook::react::TextAttributes& textAttributes);
  void updateFragmentTouchTargets(
      facebook::react::ParagraphState const& newState);
};
} // namespace rnoh