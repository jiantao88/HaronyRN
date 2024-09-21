#include "ColumnNode.h"
#include "ArkUINode.h"

#include <glog/logging.h>
#include <memory>
#include "NativeNodeApi.h"

namespace rnoh {

ColumnNode::ColumnNode()
    : ArkUINode(NativeNodeApi::getInstance()->createNode(
          ArkUI_NodeType::ARKUI_NODE_COLUMN)),
      m_columnNodeDelegate(nullptr) {
  maybeThrow(NativeNodeApi::getInstance()->registerNodeEvent(
      m_nodeHandle, NODE_ON_CLICK, 0, this));
}

void ColumnNode::insertChild(ArkUINode& child, std::size_t index) {
  maybeThrow(NativeNodeApi::getInstance()->insertChildAt(
      m_nodeHandle, child.getArkUINodeHandle(), static_cast<int32_t>(index)));
}

void ColumnNode::removeChild(ArkUINode& child) {
  maybeThrow(NativeNodeApi::getInstance()->removeChild(
      m_nodeHandle, child.getArkUINodeHandle()));
}

ArkUI_NodeHandle ColumnNode::getChildAt(std::size_t index) {
  return NativeNodeApi::getInstance()->getChildAt(
      m_nodeHandle, static_cast<int32_t>(index));
}

void ColumnNode::setColumnNodeDelegate(ColumnNodeDelegate* columnNodeDelegate) {
  m_columnNodeDelegate = columnNodeDelegate;
}

void ColumnNode::onNodeEvent(
    ArkUI_NodeEventType eventType,
    EventArgs& eventArgs) {
  if (eventType == ArkUI_NodeEventType::NODE_ON_CLICK &&
      eventArgs[3].i32 != 2) {
    onClick();
  }
}

void ColumnNode::onClick() {
  if (m_columnNodeDelegate != nullptr) {
    m_columnNodeDelegate->onClick();
  }
}

ColumnNode::~ColumnNode() {
  NativeNodeApi::getInstance()->unregisterNodeEvent(
      m_nodeHandle, NODE_ON_CLICK);
}
} // namespace rnoh