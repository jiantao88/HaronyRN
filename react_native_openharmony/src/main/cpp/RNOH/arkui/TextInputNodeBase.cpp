#include "TextInputNodeBase.h"
#include "NativeNodeApi.h"
#include "RNOH/arkui/conversions.h"

namespace rnoh {

TextInputNodeBase::TextInputNodeBase(ArkUI_NodeType nodeType)
    : ArkUINode(NativeNodeApi::getInstance()->createNode(nodeType)) {}

void TextInputNodeBase::setPadding(
    facebook::react::RectangleEdges<facebook::react::Float> padding) {
  std::array<ArkUI_NumberValue, 4> value = {
      static_cast<float>(padding.top),
      static_cast<float>(padding.right),
      static_cast<float>(padding.bottom),
      static_cast<float>(padding.left)};
  ArkUI_AttributeItem item = {value.data(), value.size()};
  maybeThrow(NativeNodeApi::getInstance()->setAttribute(
      m_nodeHandle, NODE_PADDING, &item));
}

void TextInputNodeBase::setFocusable(bool const& focusable) {
  int32_t focusableValue = 1;
  if (!focusable) {
    focusableValue = 0;
  }
  ArkUI_NumberValue value[] = {{.i32 = focusableValue}};
  ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(ArkUI_NumberValue)};
  maybeThrow(NativeNodeApi::getInstance()->setAttribute(
      m_nodeHandle, NODE_FOCUSABLE, &item));
}

void TextInputNodeBase::setAutoFocus(bool autoFocus) {
  ArkUI_NumberValue value = {.i32 = static_cast<int32_t>(autoFocus)};
  ArkUI_AttributeItem item = {&value, 1};
  maybeThrow(NativeNodeApi::getInstance()->setAttribute(
      m_nodeHandle, NODE_DEFAULT_FOCUS, &item));
  maybeThrow(NativeNodeApi::getInstance()->setAttribute(
      m_nodeHandle, NODE_FOCUS_STATUS, &item));
}

void TextInputNodeBase::setResponseRegion(
    facebook::react::Point const& position,
    facebook::react::Size const& size) {
  ArkUI_NumberValue value[] = {
      0.0f, 0.0f, (float)size.width, (float)size.height};
  ArkUI_AttributeItem item = {value, sizeof(value) / sizeof(ArkUI_NumberValue)};
  maybeThrow(NativeNodeApi::getInstance()->setAttribute(
      m_nodeHandle, NODE_RESPONSE_REGION, &item));
}

void TextInputNodeBase::setFontColor(
    facebook::react::SharedColor const& color) {
  uint32_t colorValue = *color;
  ArkUI_NumberValue preparedColorValue[] = {{.u32 = colorValue}};
  ArkUI_AttributeItem colorItem = {
      preparedColorValue,
      sizeof(preparedColorValue) / sizeof(ArkUI_NumberValue)};
  maybeThrow(NativeNodeApi::getInstance()->setAttribute(
      m_nodeHandle, NODE_FONT_COLOR, &colorItem));
}

void TextInputNodeBase::setTextInputLineHeight(
      facebook::react::TextAttributes const& textAttributes) {
  bool allowFontScaling = true;
  if (textAttributes.allowFontScaling.has_value()) {
        allowFontScaling = textAttributes.allowFontScaling.value();
  }
  float lineHeight = static_cast<float>(textAttributes.lineHeight);
  if (!allowFontScaling) {
      float scale = ArkTSBridge::getInstance()
                            ->getFontSizeScale();
      lineHeight /= scale;
  }
   ArkUI_NumberValue value[] = {{.f32 = lineHeight}};
   ArkUI_AttributeItem item = {.value = value, .size = 1};
   maybeThrow(NativeNodeApi::getInstance()->setAttribute(
     m_nodeHandle, NODE_TEXT_LINE_HEIGHT, &item));
 }

void TextInputNodeBase::setCommonFontAttributes(
    facebook::react::TextAttributes const& textAttributes) {
  if (textAttributes.fontFamily.empty()) {
    maybeThrow(NativeNodeApi::getInstance()->resetAttribute(
        m_nodeHandle, NODE_FONT_FAMILY));
  } else {
    ArkUI_AttributeItem item = {.string = textAttributes.fontFamily.c_str()};
    maybeThrow(NativeNodeApi::getInstance()->setAttribute(
        m_nodeHandle, NODE_FONT_FAMILY, &item));
  }

  if (std::isnan(textAttributes.fontSize)) {
    maybeThrow(NativeNodeApi::getInstance()->resetAttribute(
        m_nodeHandle, NODE_FONT_SIZE));
  } else {
    bool allowFontScaling = true;
    if (textAttributes.allowFontScaling.has_value()) {
        allowFontScaling = textAttributes.allowFontScaling.value();
    }

    float fontSize = static_cast<float>(textAttributes.fontSize);
    if (!allowFontScaling) {
        float scale = ArkTSBridge::getInstance()
                            ->getFontSizeScale();
        fontSize /= scale;
    }
    std::array<ArkUI_NumberValue, 1> value = {
        {{.f32 = fontSize}}};
    ArkUI_AttributeItem item = {value.data(), value.size()};
    maybeThrow(NativeNodeApi::getInstance()->setAttribute(
        m_nodeHandle, NODE_FONT_SIZE, &item));
  }

  if (textAttributes.fontWeight.has_value()) {
    std::array<ArkUI_NumberValue, 1> value = {
        {{.i32 = static_cast<int32_t>(
              rnoh::convertFontWeight(textAttributes.fontWeight.value()))}}};
    ArkUI_AttributeItem item = {value.data(), value.size()};
    maybeThrow(NativeNodeApi::getInstance()->setAttribute(
        m_nodeHandle, NODE_FONT_WEIGHT, &item));
  } else {
    maybeThrow(NativeNodeApi::getInstance()->resetAttribute(
        m_nodeHandle, NODE_FONT_WEIGHT));
  }

  if (textAttributes.fontStyle.has_value() &&
      textAttributes.fontStyle.value() == facebook::react::FontStyle::Italic) {
    std::array<ArkUI_NumberValue, 1> value = {
        {{.i32 = static_cast<int32_t>(ARKUI_FONT_STYLE_ITALIC)}}};
    ArkUI_AttributeItem item = {value.data(), value.size()};
    maybeThrow(NativeNodeApi::getInstance()->setAttribute(
        m_nodeHandle, NODE_FONT_STYLE, &item));
  } else {
    maybeThrow(NativeNodeApi::getInstance()->resetAttribute(
        m_nodeHandle, NODE_FONT_STYLE));
  }
}

void TextInputNodeBase::setTextAlign(
    std::optional<facebook::react::TextAlignment> const& textAlign) {
  if (textAlign.has_value()) {
    std::array<ArkUI_NumberValue, 1> value = {
        {{.i32 = static_cast<int32_t>(
              rnoh::convertTextAlign(textAlign.value()))}}};
    ArkUI_AttributeItem item = {value.data(), value.size()};
    maybeThrow(NativeNodeApi::getInstance()->setAttribute(
        m_nodeHandle, NODE_TEXT_ALIGN, &item));
  } else {
    maybeThrow(NativeNodeApi::getInstance()->resetAttribute(
        m_nodeHandle, NODE_TEXT_ALIGN));
  }
}

void TextInputNodeBase::setTextSelection(int32_t start, int32_t end) {
  std::array<ArkUI_NumberValue, 2> value = {{{.i32 = start}, {.i32 = end}}};
  ArkUI_AttributeItem item = {.value = value.data(), .size = 2};
  maybeThrow(NativeNodeApi::getInstance()->setAttribute(
      m_nodeHandle, NODE_TEXT_INPUT_TEXT_SELECTION, &item));
}
} // namespace rnoh
