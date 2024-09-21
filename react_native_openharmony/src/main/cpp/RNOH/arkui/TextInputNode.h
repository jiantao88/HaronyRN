#pragma once

#include "TextInputNodeBase.h"
#include "react/renderer/components/textinput/TextInputProps.h"
#include "react/renderer/graphics/Color.h"

namespace rnoh {

class TextInputNodeDelegate {
 public:
  virtual ~TextInputNodeDelegate() = default;
  virtual void onChange(std::string text) {};
  virtual void onBlur() {};
  virtual void onFocus() {};
  virtual void onSubmit() {};
  virtual void onPasteOrCut() {};
  virtual void onContentScroll() {};
  virtual void onContentSizeChange(float width, float height, bool multiline) {};

  virtual void onTextSelectionChange(int32_t location, int32_t length){};
};

class TextInputNode : public TextInputNodeBase {
 private:
  uint32_t m_caretColorValue;
 protected:
  TextInputNodeDelegate* m_textInputNodeDelegate;

 public:
  TextInputNode();
  ~TextInputNode() override;

  facebook::react::Point getTextInputOffset() const;

  facebook::react::Rect getTextContentRect() const override;

  void onNodeEvent(ArkUI_NodeEventType eventType, EventArgs& eventArgs)
      override;

  void onNodeEvent(ArkUI_NodeEventType eventType, std::string_view eventString)
      override;

  void setTextInputNodeDelegate(TextInputNodeDelegate* textInputNodeDelegate);

  void setTextContent(std::string const& textContent);

  void setCaretHidden(bool hidden);

  void setInputType(ArkUI_TextInputType keyboardType);

  void setSelectedBackgroundColor(facebook::react::SharedColor const& color);

  void setPasswordIconVisibility(bool isVisible);

  void setEnterKeyType(ArkUI_EnterKeyType returnKeyType);

  void setCancelButtonMode(
      facebook::react::TextInputAccessoryVisibilityMode mode);

  void setFont(facebook::react::TextAttributes const& textAttributes) override;

  void setCaretColor(facebook::react::SharedColor const& color) override;

  void setMaxLength(int32_t maxLength) override;

  void setPlaceholder(std::string const& placeholder) override;

  void setPlaceholderColor(facebook::react::SharedColor const& color) override;

  void resetSelectedBackgroundColor();

  void setPasswordRules(std::string rules);
  
  void setUnderlineColorAndroid(facebook::react::SharedColor const& underlineColorAndroid);
  
  void SetContextMenuHidden(bool const& hidden);
  
  void setTextContentType(std::string const& textContentType);
  
  void setAutoFill(bool autoFill);
  
  void setBlurOnSubmit(bool blurOnSubmit);

  void setshowSoftInputOnFocus(int32_t enable);

  void setInputFilter(std::string const& inputFilter);

  std::string getTextContent() override;
};

} // namespace rnoh