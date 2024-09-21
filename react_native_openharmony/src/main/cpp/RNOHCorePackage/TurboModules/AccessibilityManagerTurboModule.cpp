#include "RNOHCorePackage/TurboModules/AccessibilityManagerTurboModule.h"
#include "RNOH/RNInstance.h"
#include "RNOH/RNInstanceCAPI.h"
#include "RNOH/CppComponentInstance.h"
#include <typeinfo>

namespace rnoh {
using namespace facebook;

// use class String;

static jsi::Value __hostFunction_AccessibilityManagerTurboModule_setAccessibilityFocus(
    jsi::Runtime& rt,
    react::TurboModule& turboModule,
    const jsi::Value* args,
    size_t count) {
  ArkTSTurboModule& module = static_cast<ArkTSTurboModule&>(turboModule);
  std::shared_ptr<RNInstance> sharedInstance = module.getContext().instance.lock();
  if (!sharedInstance) {
    return nullptr;
  }
  RNInstance* instance = sharedInstance.get();
  if (!instance) {
    return nullptr;
  }
    
  // CAPI not support get component element Id ,use customId instead
  RNInstanceCAPI* capiInstance = dynamic_cast<RNInstanceCAPI*>(instance);
  facebook::react::Tag tag = static_cast<facebook::react::Tag>(args[0].asNumber());
  auto comp = capiInstance->findComponentInstanceByTag(tag);
  if (!comp){
    return nullptr;
  }
  jsi::Value newArgs = jsi::Value(jsi::String::createFromUtf8(rt, comp->getCompId()));
  return static_cast<ArkTSTurboModule&>(turboModule)
      .call(rt, "setAccessibilityFocus", &newArgs, count);
}

AccessibilityManagerTurboModule::AccessibilityManagerTurboModule(
    const ArkTSTurboModule::Context ctx,
    const std::string name)
    : ArkTSTurboModule(ctx, name) {
  methodMap_ = {
      ARK_METHOD_METADATA(isTouchExplorationEnabled, 1),
      ARK_METHOD_METADATA(isAccessibilityServiceEnabled, 1),
      ARK_METHOD_METADATA(getCurrentBoldTextState, 1),
      ARK_METHOD_METADATA(announceForAccessibility, 1)};
  methodMap_["setAccessibilityFocus"] =
      MethodMetadata{1, __hostFunction_AccessibilityManagerTurboModule_setAccessibilityFocus};  
}

} // namespace rnoh
