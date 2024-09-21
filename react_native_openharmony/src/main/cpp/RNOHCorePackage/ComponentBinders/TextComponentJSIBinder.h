#pragma once
#include "RNOHCorePackage/ComponentBinders/ViewComponentJSIBinder.h"

namespace rnoh {

class TextComponentJSIBinder : public ViewComponentJSIBinder {
  facebook::jsi::Object createNativeProps(facebook::jsi::Runtime& rt) override {
    auto nativeProps = ViewComponentJSIBinder::createNativeProps(rt);
    nativeProps.setProperty(rt, "selectionColor", "Color");
    nativeProps.setProperty(rt, "dataDetectorType", "string");
    nativeProps.setProperty(rt, "textDecorationStyle", "TextDecorationStyle");
    return nativeProps;
  };
};

} // namespace rnoh