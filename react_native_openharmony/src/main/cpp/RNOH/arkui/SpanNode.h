#pragma once

#include "ArkUINode.h"

namespace rnoh {

class ImageSpanNode : public ArkUINode {
 public:
  ImageSpanNode();
};

class SpanNode : public ArkUINode {
 public:
  SpanNode();

  SpanNode& setSpanContent(const std::string& text);
  SpanNode& setFontColor(uint32_t fontColor);
  SpanNode& setFontSize(float fontSize);
  SpanNode& setFontStyle(int32_t fontStyle);
  SpanNode& setFontFamily(const std::string& fontFamily);
  SpanNode& setFontWeight(int32_t fontWeight);
  SpanNode& setTextDecoration(
      int32_t decorationType,
      uint32_t decorationColor = 0xFFFF0000,
      int32_t decorationStyle = 0);
  SpanNode& setTextLetterSpacing(float textLetterSpacing);
  SpanNode& setTextShadow(
      float textShadowRadius,
      int32_t textShadowType,
      uint32_t textShadowColor,
      float textShadowOffsetX,
      float textShadowOffsetY);
  SpanNode& setTextLineHeight(float textLineHeight);
  SpanNode& setTextCase(int32_t textCase);
  SpanNode& setBackgroundStyle(uint32_t color);
  SpanNode& setLengthMetricUnit(ArkUI_LengthMetricUnit unit);
  SpanNode& resetBackgroundStyle();
};

} // namespace rnoh