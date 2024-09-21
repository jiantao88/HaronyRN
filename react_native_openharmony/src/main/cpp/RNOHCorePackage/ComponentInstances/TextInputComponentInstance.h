#pragma once

#include "RNOH/CppComponentInstance.h"
#include "RNOH/arkui/TextAreaNode.h"
#include "RNOH/arkui/TextInputNode.h"
#include "ScrollViewComponentInstance.h"
#include "react/renderer/components/textinput/TextInputEventEmitter.h"
#include "react/renderer/components/textinput/TextInputShadowNode.h"

namespace rnoh {
class TextInputComponentInstance
    : public CppComponentInstance<facebook::react::TextInputShadowNode>,
      public TextInputNodeDelegate,
      public TextAreaNodeDelegate,
      public ArkTSMessageHub::Observer,
      public KeyboardAvoider {
 private:
  TextInputNode m_textInputNode;
  TextAreaNode m_textAreaNode;
  bool m_multiline{false};

  int32_t m_nativeEventCount = 0;

  facebook::react::TextInputMetrics getTextInputMetrics();
  facebook::react::TextInputMetrics getTextInputMetrics(std::string text);
  facebook::react::OnChangeMetrics getOnChangeMetrics();
  facebook::react::Size getOnContentSizeChangeMetrics();

  bool m_secureInput{false};

  bool m_clearTextOnFocus{false};

  std::string m_content;

  int32_t m_selectionLocation = 0;

  int32_t m_selectionLength = 0;

  float m_contentSizeWidth = 0;
  float m_contentSizeHeight = 0;

  bool m_textWasPastedOrCut = false;
  bool m_valueChanged = false;

  bool m_focused{false};
  bool m_shouldIgnoreNextChangeEvent = false;
  
  int32_t m_selectionStart = -1;
  int32_t m_selectionEnd = -1;

  void focus();
  void blur();

 public:
  TextInputComponentInstance(Context context);

  void onPropsChanged(SharedConcreteProps const& props) override;

  void onStateChanged(SharedConcreteState const& state) override;

  void onLayoutChanged(
      facebook::react::LayoutMetrics const& layoutMetrics) override;

  void onCommandReceived(
      std::string const& commandName,
      folly::dynamic const& args) override;

  void onChange(std::string text) override;

  void onBlur() override;

  void onSubmit() override;

  void onFocus() override;

  void onPasteOrCut() override;

  void onTextSelectionChange(int32_t location, int32_t length) override;
  
  void onContentScroll() override;

  void onContentSizeChange(float width, float height, bool multiline) override;

  ArkUINode& getLocalRootArkUINode() override;

  void onMessageReceived(ArkTSMessage const& message) override;

  // KeyboardAvoider
  facebook::react::Float getBottomEdgeOffsetRelativeToScrollView(
      std::shared_ptr<ScrollViewComponentInstance> scrollView) override;
};
} // namespace rnoh