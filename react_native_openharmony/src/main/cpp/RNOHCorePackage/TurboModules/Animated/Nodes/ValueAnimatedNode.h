#pragma once

#include "AnimatedNode.h"

#include <folly/dynamic.h>
#include "RNOH/Assert.h"

namespace rnoh {

class ValueAnimatedNode : public AnimatedNode {
 public:
  using AnimatedNodeValueListener = std::function<void(double)>;

  ValueAnimatedNode() {}

  ValueAnimatedNode(folly::dynamic const& config) {
    RNOH_ASSERT(config.count("value") > 0);
    m_value = config["value"];
    RNOH_ASSERT(config["offset"].isDouble());
    m_offset = config["offset"].getDouble();
  }

  folly::dynamic getOutput() override {
    if (m_value.isDouble()) {
      folly::dynamic result = this->getOutputAsDouble();
      return result;
    }
    return m_value;
  }

  double getOutputAsDouble() {
    return this->getValueAsDouble() + m_offset;
  }

  void setValue(folly::dynamic value) {
    m_value = value;
  }

  void setValue(double value) {
    m_value = value;
  }

  void setOffset(double offset) {
    m_offset = offset;
  }

  void flattenOffset() {
    m_value = this->getOutputAsDouble();
    m_offset = 0;
  }

  void extractOffset() {
    m_offset += this->getValueAsDouble();
    m_value = 0;
  }

  void onValueUpdate() {
    if (m_valueListener.has_value()) {
      /**
       * NOTE: ValueListenerCallback on JS side expects only numbers.
       */
      m_valueListener.value()(getOutputAsDouble());
    }
  }

  void setValueListener(AnimatedNodeValueListener&& listener) {
    if (m_valueListener.has_value()) {
      throw std::runtime_error("AnimatedNode already has a value listener");
    }
    m_valueListener = std::move(listener);
  }

  void removeValueListener() {
    m_valueListener = std::nullopt;
  }

  double getValueAsDouble() {
    if (m_value.isNull()) {
      // Special case for diffClamp
      return 0;
    }
    if (!m_value.isDouble()) {
      LOG(ERROR) << "Unexpected value type: " << m_value.typeName();
      RNOH_ASSERT(false);
      return 0;
    }
    return m_value.getDouble();
  }

 protected:
  folly::dynamic m_value;
  double m_offset = 0.0;
  std::optional<AnimatedNodeValueListener> m_valueListener;
};

} // namespace rnoh