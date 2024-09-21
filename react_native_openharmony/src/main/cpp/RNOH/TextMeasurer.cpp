#include "TextMeasurer.h"
#include <native_drawing/drawing_register_font.h>
#include <filesystem>
#include <fstream>
#include "RNInstance.h"
#include "RNInstanceCAPI.h"
#include "RNOH/ArkJS.h"
#include "RNOH/ArkTSBridge.h"
#include "RNOH/ArkUITypography.h"
#include "RNOHCorePackage/ComponentInstances/TextConversions.h"
#include "RNOHCorePackage/TurboModules/DeviceInfoTurboModule.h"

namespace rnoh {

using TextMeasurement = facebook::react::TextMeasurement;
using AttributedString = facebook::react::AttributedString;
using ParagraphAttributes = facebook::react::ParagraphAttributes;
using LayoutConstraints = facebook::react::LayoutConstraints;
UniqueFontCollection TextMeasurer::m_fontCollection(OH_Drawing_CreateSharedFontCollection(), OH_Drawing_DestroyFontCollection);

TextMeasurement TextMeasurer::measure(
    AttributedString attributedString,
    ParagraphAttributes paragraphAttributes,
    LayoutConstraints layoutConstraints) {
  auto const& fragments = attributedString.getFragments();
  auto canUseOHOSTextMeasurer = fragments.size() == 1 &&
      !fragments[0].isAttachment() &&
      isnan(fragments[0].textAttributes.letterSpacing) &&
      isnan(fragments[0].textAttributes.lineHeight);
  auto isNDKTextMeasuringEnabled =
      this->m_featureFlagRegistry->getFeatureFlagStatus(
          "ENABLE_NDK_TEXT_MEASURING");
  if (canUseOHOSTextMeasurer || isNDKTextMeasuringEnabled) {
    // first deal textCase
    float fontMultiplier = 1.0;
    if (paragraphAttributes.allowFontScaling) {
      fontMultiplier = m_fontScale;
      if (!isnan(paragraphAttributes.maxFontSizeMultiplier)) {
        fontMultiplier = std::min(m_fontScale, (float)paragraphAttributes.maxFontSizeMultiplier);
      }
    }
    for (auto& fragment : attributedString.getFragments()) {
      if (fragment.textAttributes.textTransform.has_value()) {
        textCaseTransform(
            fragment.string, fragment.textAttributes.textTransform.value());
      }
      fragment.textAttributes.fontSize *= fontMultiplier;
    }
    // calc typograph
    facebook::react::TextMeasureCacheKey cacheKey{attributedString, paragraphAttributes, layoutConstraints};
    std::optional<std::shared_ptr<TextMeasureInfo>> measureInfo = TextMeasureRegistry::getTextMeasureRegistry().getTextMeasureInfo(cacheKey);
    if (measureInfo.has_value()) {
      if (!attributedString.getFragments().empty()) {
        std::string key = std::to_string(m_rnInstanceId) + "_" + 
          std::to_string(attributedString.getFragments()[0].parentShadowView.tag) + "_" + 
          std::to_string(attributedString.getFragments()[0].parentShadowView.surfaceId);
        TextMeasureRegistry::getTextMeasureRegistry().setTextMeasureInfo(key, measureInfo.value(), cacheKey);
      }
      const auto& typography = measureInfo.value()->typography;
      auto height = typography.getHeight();
      auto longestLineWidth = typography.getLongestLineWidth();
      auto attachments = typography.getAttachments();
      return {{.width = longestLineWidth + 0.5, .height = height}, attachments};
    }
    if (paragraphAttributes.adjustsFontSizeToFit) {
      int maxFontSize = 0;
      for (const auto& fragment : attributedString.getFragments()) {
        maxFontSize = std::max(maxFontSize, (int)fragment.textAttributes.fontSize);
      }
      std::pair<ArkUITypographyBuilder, ArkUITypography> measureRes = findFitFontSize(maxFontSize, attributedString, paragraphAttributes, layoutConstraints);
      auto typographyBuilder = std::move(measureRes.first);
      auto typography = std::move(measureRes.second);
      auto height = typography.getHeight();
      auto longestLineWidth = typography.getLongestLineWidth();
      auto attachments = typography.getAttachments();
      if (!attributedString.getFragments().empty()) {
        std::string key = std::to_string(m_rnInstanceId) + "_" + 
          std::to_string(attributedString.getFragments()[0].parentShadowView.tag) + "_" + 
          std::to_string(attributedString.getFragments()[0].parentShadowView.surfaceId);
        std::shared_ptr<TextMeasureInfo> textMeasureInfo = std::make_shared<TextMeasureInfo>(std::move(typographyBuilder), std::move(typography));
        TextMeasureRegistry::getTextMeasureRegistry().setTextMeasureInfo(key, textMeasureInfo, cacheKey);
      }
      return {{.width = longestLineWidth + 0.5, .height = height}, attachments};
    } else {
      auto typographyBuilder = measureTypography(attributedString, paragraphAttributes, layoutConstraints);
      auto typography = typographyBuilder.build();
      auto height = typography.getHeight();
      auto longestLineWidth = typography.getLongestLineWidth();
      if (longestLineWidth < layoutConstraints.maximumSize.width) {
          layoutConstraints.maximumSize.width = longestLineWidth;
          typographyBuilder = measureTypography(attributedString, paragraphAttributes, layoutConstraints);
          typography = typographyBuilder.build();
          height = typography.getHeight();
          longestLineWidth = typography.getLongestLineWidth();
      }
      auto attachments = typography.getAttachments();
      if (!attributedString.getFragments().empty()) {
        std::string key = std::to_string(m_rnInstanceId) + "_" + 
          std::to_string(attributedString.getFragments()[0].parentShadowView.tag) + "_" + 
          std::to_string(attributedString.getFragments()[0].parentShadowView.surfaceId);
        std::shared_ptr<TextMeasureInfo> textMeasureInfo = std::make_shared<TextMeasureInfo>(std::move(typographyBuilder), std::move(typography));
        TextMeasureRegistry::getTextMeasureRegistry().setTextMeasureInfo(key, textMeasureInfo, cacheKey);
      }
      return {{.width = longestLineWidth + 0.5, .height = height}, attachments};
    }
  } else {
    TextMeasurement result = {{0, 0}, {}};
    m_taskExecutor->runSyncTask(
        TaskThread::MAIN,
        [&result,
         measureTextRef = m_measureTextFnRef,
         env = m_env,
         &attributedString,
         &paragraphAttributes,
         &layoutConstraints]() {
          ArkJS arkJs(env);
          auto napiMeasureText = arkJs.getReferenceValue(measureTextRef);
          auto napiAttributedStringBuilder = arkJs.createObjectBuilder();
          napiAttributedStringBuilder.addProperty(
              "string", attributedString.getString());
          std::vector<napi_value> napiFragments = {};
          for (auto fragment : attributedString.getFragments()) {
            auto textAttributesBuilder = arkJs.createObjectBuilder();
            textAttributesBuilder.addProperty(
                "fontSize", fragment.textAttributes.fontSize);
            textAttributesBuilder.addProperty(
                "lineHeight", fragment.textAttributes.lineHeight);
            if (!fragment.textAttributes.fontFamily.empty()) {
              textAttributesBuilder.addProperty(
                  "fontFamily", fragment.textAttributes.fontFamily);
            }
            textAttributesBuilder.addProperty(
                "letterSpacing", fragment.textAttributes.letterSpacing);
            if (fragment.textAttributes.fontWeight.has_value()) {
              textAttributesBuilder.addProperty(
                  "fontWeight",
                  int(fragment.textAttributes.fontWeight.value()));
            }

            auto napiFragmentBuilder = arkJs.createObjectBuilder();
            napiFragmentBuilder.addProperty("string", fragment.string)
                .addProperty("textAttributes", textAttributesBuilder.build());
            if (fragment.isAttachment()) {
              napiFragmentBuilder.addProperty(
                  "parentShadowView",
                  arkJs.createObjectBuilder()
                      .addProperty("tag", fragment.parentShadowView.tag)
                      .addProperty(
                          "layoutMetrics",
                          arkJs.createObjectBuilder()
                              .addProperty(
                                  "frame",
                                  arkJs.createObjectBuilder()
                                      .addProperty(
                                          "size",
                                          arkJs.createObjectBuilder()
                                              .addProperty(
                                                  "width",
                                                  fragment.parentShadowView
                                                      .layoutMetrics.frame.size
                                                      .width)
                                              .addProperty(
                                                  "height",
                                                  fragment.parentShadowView
                                                      .layoutMetrics.frame.size
                                                      .height)
                                              .build())
                                      .build())
                              .build())
                      .build());
            }

            napiFragments.push_back(napiFragmentBuilder.build());
          }
          napiAttributedStringBuilder.addProperty(
              "fragments", arkJs.createArray(napiFragments));

          auto napiParagraphAttributesBuilder = arkJs.createObjectBuilder();
          napiParagraphAttributesBuilder.addProperty(
              "maximumNumberOfLines", paragraphAttributes.maximumNumberOfLines);

          auto napiLayoutConstraintsBuilder = arkJs.createObjectBuilder();
          napiLayoutConstraintsBuilder.addProperty(
              "maximumSize",
              arkJs.createObjectBuilder()
                  .addProperty("width", layoutConstraints.maximumSize.width)
                  .addProperty("height", layoutConstraints.maximumSize.height)
                  .build());

          auto resultNapiValue = arkJs.call(
              napiMeasureText,
              {napiAttributedStringBuilder.build(),
               napiParagraphAttributesBuilder.build(),
               napiLayoutConstraintsBuilder.build()});

          result.size.width = arkJs.getDouble(arkJs.getObjectProperty(
              arkJs.getObjectProperty(resultNapiValue, "size"), "width"));
          result.size.height = arkJs.getDouble(arkJs.getObjectProperty(
              arkJs.getObjectProperty(resultNapiValue, "size"), "height"));
          auto napiAttachments =
              arkJs.getObjectProperty(resultNapiValue, "attachmentLayouts");
          for (auto i = 0; i < arkJs.getArrayLength(napiAttachments); i++) {
            auto napiAttachment = arkJs.getArrayElement(napiAttachments, i);
            auto napiPositionRelativeToContainer = arkJs.getObjectProperty(
                napiAttachment, "positionRelativeToContainer");
            auto napiSize = arkJs.getObjectProperty(napiAttachment, "size");
            TextMeasurement::Attachment attachment;
            attachment.frame.origin.x = arkJs.getDouble(
                arkJs.getObjectProperty(napiPositionRelativeToContainer, "x"));
            attachment.frame.origin.y = arkJs.getDouble(
                arkJs.getObjectProperty(napiPositionRelativeToContainer, "y"));
            attachment.frame.size.width =
                arkJs.getDouble(arkJs.getObjectProperty(napiSize, "width"));
            attachment.frame.size.height =
                arkJs.getDouble(arkJs.getObjectProperty(napiSize, "height"));
            result.attachments.push_back(attachment);
          }
        });
    return result;
  }
}

ArkUITypographyBuilder TextMeasurer::measureTypography(
    AttributedString const& attributedString,
    ParagraphAttributes const& paragraphAttributes,
    LayoutConstraints const& layoutConstraints) {
  //  UniqueTypographyStyle typographyStyle(
  //      OH_Drawing_CreateTypographyStyle(), OH_Drawing_DestroyTypographyStyle);
  //  Revert this after API rectification

  std::shared_ptr<OH_Drawing_TypographyStyle> typographyStyle(
      OH_Drawing_CreateTypographyStyle(), OH_Drawing_DestroyTypographyStyle);

  if (paragraphAttributes.ellipsizeMode == facebook::react::EllipsizeMode::Head) {
    OH_Drawing_SetTypographyTextEllipsis(typographyStyle.get(), "...");
    OH_Drawing_SetTypographyTextEllipsisModal(typographyStyle.get(), ELLIPSIS_MODAL_HEAD);
  } else if (paragraphAttributes.ellipsizeMode == facebook::react::EllipsizeMode::Middle) {
    OH_Drawing_SetTypographyTextEllipsis(typographyStyle.get(), "...");
    OH_Drawing_SetTypographyTextEllipsisModal(typographyStyle.get(), ELLIPSIS_MODAL_MIDDLE);
  } else if (paragraphAttributes.ellipsizeMode == facebook::react::EllipsizeMode::Tail) {
    OH_Drawing_SetTypographyTextEllipsis(typographyStyle.get(), "...");
    OH_Drawing_SetTypographyTextEllipsisModal(typographyStyle.get(), ELLIPSIS_MODAL_TAIL);
  }
  
  if (paragraphAttributes.maximumNumberOfLines > 0) {
    OH_Drawing_SetTypographyTextMaxLines(
        typographyStyle.get(), paragraphAttributes.maximumNumberOfLines);
  }

  OH_Drawing_SetTypographyTextWordBreakType(
      typographyStyle.get(),
      TextConversions::getArkUIWordBreakStrategy(
          paragraphAttributes.textBreakStrategy));

  if (paragraphAttributes.writingDirection == facebook::react::WritingDirection::LeftToRight) {
    OH_Drawing_SetTypographyTextDirection(typographyStyle.get(), TEXT_DIRECTION_LTR);
  } else if (paragraphAttributes.writingDirection == facebook::react::WritingDirection::RightToLeft) {
    OH_Drawing_SetTypographyTextDirection(typographyStyle.get(), TEXT_DIRECTION_RTL);
  }
  
  if (!attributedString.getFragments().empty()) {
    auto textAlign =
        attributedString.getFragments()[0].textAttributes.alignment;
    if (textAlign.has_value()) {
      OH_Drawing_SetTypographyTextAlign(
          typographyStyle.get(), getOHDrawingTextAlign(textAlign.value()));
    }
  }
  ArkUITypographyBuilder typographyBuilder(
      typographyStyle.get(), m_fontCollection.get(), m_scale, m_halfleading, m_defaultFontFamilyName, typographyStyle); //  Revert this after API rectification
  for (auto const& fragment : attributedString.getFragments()) {
    typographyBuilder.addFragment(fragment);
  }
  typographyBuilder.setMaximumWidth(layoutConstraints.maximumSize.width * m_scale);
  return typographyBuilder;
}

int32_t TextMeasurer::getOHDrawingTextAlign(
    const facebook::react::TextAlignment& textAlign) {
  int32_t align = OH_Drawing_TextAlign::TEXT_ALIGN_START;
  switch (textAlign) {
    case facebook::react::TextAlignment::Natural:
    case facebook::react::TextAlignment::Left:
      align = OH_Drawing_TextAlign::TEXT_ALIGN_START;
      break;
    case facebook::react::TextAlignment::Right:
      align = OH_Drawing_TextAlign::TEXT_ALIGN_END;
      break;
    case facebook::react::TextAlignment::Center:
      align = OH_Drawing_TextAlign::TEXT_ALIGN_CENTER;
      break;
    case facebook::react::TextAlignment::Justified:
      align = OH_Drawing_TextAlign::TEXT_ALIGN_JUSTIFY;
      break;
    default:
      break;
  }
  return align;
}

std::vector<OH_Drawing_LineMetrics> TextMeasurer::getLineMetrics(
    AttributedString const& attributedString,
    ParagraphAttributes const& paragraphAttributes,
    LayoutConstraints const& layoutConstraints) {
    std::vector<OH_Drawing_LineMetrics> data;
  if (!attributedString.getFragments().empty()) {
    std::string key =
      std::to_string(m_rnInstanceId) + "_" + std::to_string(attributedString.getFragments()[0].parentShadowView.tag) +
      "_" + std::to_string(attributedString.getFragments()[0].parentShadowView.surfaceId);
    auto measureInfo = TextMeasureRegistry::getTextMeasureRegistry().getTextMeasureInfoByKey(key);
    if (measureInfo.has_value()) {
      measureInfo.value()->typography.getLineMetrics(data);
    }
  } else {
    auto typographyBuilder = measureTypography(attributedString, paragraphAttributes, layoutConstraints);
    auto typography = typographyBuilder.build();
    typography.getLineMetrics(data);
  }
  for (int i = 0; i < data.size(); i++) {
    data[i].width = static_cast<Float>(data[i].width) / m_scale;
    data[i].height = static_cast<Float>(data[i].height) / m_scale;
  }
  return data;
}

std::pair<ArkUITypographyBuilder, ArkUITypography> TextMeasurer::findFitFontSize(int maxFontSize, 
  facebook::react::AttributedString& attributedString,
  facebook::react::ParagraphAttributes& paragraphAttributes,
  facebook::react::LayoutConstraints& layoutConstraints) {
  // init params
  int minFontSize = 1;
  if (!isnan(paragraphAttributes.minimumFontScale)) {
    minFontSize = ceil(paragraphAttributes.minimumFontScale * maxFontSize);
  }
  int finalFontSize = minFontSize;
  int initFontSize = maxFontSize;
  auto backupAttributedString = attributedString;
  // check if already fit
  auto finalTypographyBuilder = measureTypography(attributedString, paragraphAttributes, layoutConstraints);
  auto finalTypography = finalTypographyBuilder.build();
  if (finalTypography.getHeight() <= layoutConstraints.maximumSize.height &&
     (paragraphAttributes.maximumNumberOfLines == 0 || !finalTypography.getExceedMaxLines())) {
    return std::make_pair(std::move(finalTypographyBuilder), std::move(finalTypography));
  }
  // find fit fontSize
  while(minFontSize <= maxFontSize) {
    int curFontSize = ceil((minFontSize + maxFontSize) * 1.0 / 2);
    float ratio = 1.0 * curFontSize / initFontSize;
    for (int i = 0; i < attributedString.getFragments().size(); ++i) {
      int newFontSize = ceil(backupAttributedString.getFragments()[i].textAttributes.fontSize * ratio);
      attributedString.getFragments()[i].textAttributes.fontSize = newFontSize;
    }
    auto typographyBuilder = measureTypography(attributedString, paragraphAttributes, layoutConstraints);
    auto typography = typographyBuilder.build();
    if (typography.getHeight() <= layoutConstraints.maximumSize.height &&
      (paragraphAttributes.maximumNumberOfLines == 0 || !typography.getExceedMaxLines())) {
      finalFontSize = curFontSize;
      finalTypography = std::move(typography);
      finalTypographyBuilder = std::move(typographyBuilder);
      minFontSize = curFontSize + 1;
    } else {
      maxFontSize = curFontSize - 1;
    }
  }
  float ratio = 1.0 * finalFontSize / initFontSize;
  for (int i = 0; i < attributedString.getFragments().size(); ++i) {
    int newFontSize = ceil(
        backupAttributedString.getFragments()[i].textAttributes.fontSize *
        ratio);
    attributedString.getFragments()[i].textAttributes.fontSize = newFontSize;
  }
  return std::make_pair(std::move(finalTypographyBuilder), std::move(finalTypography));
}

std::string TextMeasurer::stringCapitalize(const std::string& strInput) {
  if (strInput.empty()) {
    return strInput;
  }

  std::string strRes;
  std::string split = " ";
  std::vector<std::string> subStringVector;
  subStringVector.clear();

  std::string strSrc = strInput + split;
  auto pos = strSrc.find(split);
  auto step = split.size();

  while (pos != std::string::npos) {
    std::string strTemp = strSrc.substr(0, pos);
    subStringVector.push_back(strTemp);

    strSrc = strSrc.substr(pos + step, strSrc.size());
    pos = strSrc.find(split);
  }

  for (auto subString : subStringVector) {
    if (std::isalpha(subString[0]) != 0) {
      std::transform(
          subString.begin(),
          subString.end(),
          subString.begin(),
          [](unsigned char c) { return std::tolower(c); });
      subString[0] = std::toupper(static_cast<unsigned char>(subString[0]));
    }
    if (!strRes.empty()) {
      strRes += split;
    }
    strRes += subString;
  }
  return strRes;
}

void TextMeasurer::textCaseTransform(std::string& textContent, facebook::react::TextTransform type) {
  switch (type) {
    case facebook::react::TextTransform::Uppercase: {
      transform(textContent.begin(), textContent.end(), textContent.begin(), ::toupper);
      break;
    }
    case facebook::react::TextTransform::Lowercase: {
      transform(textContent.begin(), textContent.end(), textContent.begin(), ::tolower);
      break;
    }
    case facebook::react::TextTransform::Capitalize: {
      textContent = stringCapitalize(textContent);
      break;
    }
    default:
      break;
  }
}

void TextMeasurer::setTextMeasureParams(float fontScale, float scale, bool halfleading) {
  m_fontScale = fontScale;
  m_scale = scale;
  m_halfleading = halfleading;
  updateDefaultFont();
}

void TextMeasurer::registerFont(std::weak_ptr<NativeResourceManager> weakResourceManager, const std::string name, const std::string fontFilePathRelativeToRawfileDir) {
  auto resourceManager = weakResourceManager.lock();
  if (resourceManager == nullptr) {
    LOG(ERROR) << "Couldn't register font " << name
               << " — resourceManager is nullptr";
    return;
  };
  auto fontFile =
      std::unique_ptr<RawFile, decltype(&OH_ResourceManager_CloseRawFile)>(
          OH_ResourceManager_OpenRawFile(
              resourceManager.get(), fontFilePathRelativeToRawfileDir.c_str()),
          OH_ResourceManager_CloseRawFile);
  if (!fontFile) {
    throw RNOHError(
        "Failed to open font fontFile: " + fontFilePathRelativeToRawfileDir);
  }
  auto length = OH_ResourceManager_GetRawFileSize(fontFile.get());
  std::vector<uint8_t> buffer(length);
  if (OH_ResourceManager_ReadRawFile(fontFile.get(), buffer.data(), length) !=
      length) {
    throw RNOHError(
        "Failed to read fontFile: " + fontFilePathRelativeToRawfileDir);
  }
  OH_Drawing_RegisterFontBuffer(
    m_fontCollection.get(),
    name.c_str(),
    buffer.data(),
    length
  );
}

void TextMeasurer::updateDefaultFont() {
  m_defaultFontFamilyName.clear();
  std::string path = "/data/themes/a/app";
  if (!existDefaultFont(path)) {
    path = "/data/themes/b/app";
    if (!existDefaultFont(path)) {
      return;
    }
  }
  path = path.append("/fonts/");
  for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
    auto entryPath = entry.path();
    if (entry.is_regular_file() && entryPath.has_extension() && entryPath.extension() == ".ttf") {
      m_defaultFontFamilyName = entryPath.stem();
      path = entryPath;
      break;
    }
  }
  if (m_defaultFontFamilyName.empty()) {
    return;
  }
  std::ifstream ifs(path, std::ios_base::in);
  ifs.seekg(0, ifs.end);
  auto size = ifs.tellg();
  ifs.seekg(ifs.beg);
  std::unique_ptr<char[]> buffer = std::make_unique<char[]>(size);
  ifs.read(buffer.get(), size);
  bool isGood = ifs.good();
  ifs.close();
  if (isGood) {
    OH_Drawing_RegisterFontBuffer(
        m_fontCollection.get(),
        m_defaultFontFamilyName.c_str(),
        (uint8_t*)buffer.get(),
        size
    );
  }
}


bool TextMeasurer::existDefaultFont(std::string path) {
  bool isFlagFileExist = false;
  bool isFontDirExist = false;
  try {
    for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
      auto entryPath = entry.path();
      isFlagFileExist = isFlagFileExist || (entry.is_regular_file() && entryPath.filename() == "flag");
      isFontDirExist = isFontDirExist || (entry.is_directory() && entryPath.filename() == "fonts");
      if (isFlagFileExist && isFontDirExist) {
        break;
      }
    }
  } catch (...) {
    return false;
  }
  return isFlagFileExist && isFontDirExist;
}
} // namespace rnoh