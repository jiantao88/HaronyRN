#include "CustomNodeComponentInstance.h"
#include "conversions.h"
#include <RNOH/arkui/NativeNodeApi.h>
#include "RNOH/arkui/ArkUINodeRegistry.h"

namespace rnoh {
CustomNodeComponentInstance::CustomNodeComponentInstance(Context context)
    : CppComponentInstance(std::move(context)) {
  getLocalRootArkUINode().setCustomNodeDelegate(this);
}

void CustomNodeComponentInstance::onChildInserted(
    ComponentInstance::Shared const& childComponentInstance, std::size_t index) {
  CppComponentInstance::onChildInserted(childComponentInstance, index);
  if (m_props->removeClippedSubviews && !m_parent.expired()) {
  } else {
    m_customNode.insertChild(
        childComponentInstance->getLocalRootArkUINode(), index);
  }
}

void CustomNodeComponentInstance::onChildRemoved(
    ComponentInstance::Shared const& childComponentInstance)
{
  CppComponentInstance::onChildRemoved(childComponentInstance);
  getLocalRootArkUINode().removeChild(childComponentInstance->getLocalRootArkUINode());
}

void CustomNodeComponentInstance::onHoverIn() {
    if (m_eventEmitter != nullptr) {
      m_eventEmitter->dispatchEvent(
          "pointerEnter", [=](facebook::jsi::Runtime& runtime) {
            auto payload = facebook::jsi::Object(runtime);
            return payload;
          });
    }
}

void CustomNodeComponentInstance::onHoverOut() {
    if (m_eventEmitter != nullptr) {
      m_eventEmitter->dispatchEvent(
          "pointerLeave", [=](facebook::jsi::Runtime& runtime) {
            auto payload = facebook::jsi::Object(runtime);
            return payload;
          });
    }
}

void CustomNodeComponentInstance::onPropsChanged(SharedConcreteProps const& props) {
  CppComponentInstance::onPropsChanged(props);
  if (props->removeClippedSubviews) {
    updateClippedSubviews(true);
  } else if (!m_childrenClippedState.empty()) {
    restoreClippedSubviews();
  }
}

bool CustomNodeComponentInstance::isViewClipped(
    const ComponentInstance::Shared& child,
    facebook::react::Point currentOffset,
    facebook::react::Rect parentBoundingBox) {
  auto scrollRectOrigin =
      facebook::react::Point{currentOffset.x, currentOffset.y};
  auto scrollRect =
      facebook::react::Rect{scrollRectOrigin, parentBoundingBox.size};

  return !rnoh::rectIntersects(scrollRect, child->getLayoutMetrics().frame);
}

void CustomNodeComponentInstance::restoreClippedSubviews() {
  size_t i = 0;
  for (const auto& child : m_children) {
    if (m_childrenClippedState[i]) {
      m_customNode.insertChild(child->getLocalRootArkUINode(), i);
      m_childrenClippedState[i] = false;
    }
    i++;
  }
}

void CustomNodeComponentInstance::updateClippedSubviews(bool childrenChange) {
  auto parent = m_parent.lock();
  if ((m_props && !m_props->removeClippedSubviews) || !parent) {
    return;
  }

  auto currentOffset = parent->getCurrentOffset();

  auto parentBoundingBox = parent->getBoundingBox();

  if (m_previousOffset == currentOffset && !childrenChange) {
    return;
  }

  m_previousOffset = currentOffset;

  bool remakeVector = false;
  if (childrenChange || m_childrenClippedState.empty()) {
    m_childrenClippedState.clear();
    remakeVector = true;
  }

  size_t i = 0;
  size_t nextChildIndex = 0;
  for (const auto& child : m_children) {
    bool childClipped = isViewClipped(child, currentOffset, parentBoundingBox);

    if (remakeVector) {
      m_customNode.removeChild(child->getLocalRootArkUINode());
      m_childrenClippedState.push_back(childClipped);

      if (!childClipped) {
        m_customNode.insertChild(child->getLocalRootArkUINode(), -1);
        child->getLocalRootArkUINode().markDirty();
      }
    }

    if (m_childrenClippedState[i] != childClipped) {
      m_childrenClippedState[i] = childClipped;

      if (childClipped) {
        m_customNode.removeChild(child->getLocalRootArkUINode());
      } else {
        m_customNode.insertChild(child->getLocalRootArkUINode(), nextChildIndex);
        child->getLocalRootArkUINode().markDirty();
        nextChildIndex++;
      }
    } else if (!childClipped) {
      nextChildIndex++;
    }
    i++;
  }
}

void CustomNodeComponentInstance::onFinalizeUpdates() {
  ComponentInstance::onFinalizeUpdates();
  if (m_props->removeClippedSubviews) {
    updateClippedSubviews(true);
  }
}

void CustomNodeComponentInstance::onClick() {
  if (m_eventEmitter != nullptr) {
    m_eventEmitter->dispatchEvent(
        "click", [=](facebook::jsi::Runtime& runtime) {
          auto payload = facebook::jsi::Object(runtime);
          return payload;
        });
  }
}

CustomNode& CustomNodeComponentInstance::getLocalRootArkUINode()
{
  return m_customNode;
}

} // namespace rnoh
