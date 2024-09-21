#include <ReactCommon/CallbackWrapper.h>
#include <ReactCommon/TurboModuleUtils.h>
#include <glog/logging.h>
#include <jsi/JSIDynamic.h>
#include <react/renderer/debug/SystraceSection.h>
#include <exception>
#include <optional>
#include <variant>

#include "ArkTSTurboModule.h"
#include "RNOH/ArkTSTurboModule.h"
#include "RNOH/JsiConversions.h"
#include "RNOH/TaskExecutor/TaskExecutor.h"

using namespace rnoh;
using namespace facebook;

using IntermediaryArg = ArkJS::IntermediaryArg;
using IntermediaryCallback = ArkJS::IntermediaryCallback;

IntermediaryCallback createIntermediaryCallback(
    std::shared_ptr<react::CallbackWrapper>);
const std::vector<facebook::jsi::Value> convertDynamicsToJSIValues(
    facebook::jsi::Runtime& rt,
    const std::vector<folly::dynamic>& dynamics);
facebook::jsi::Value preparePromiseResolverResult(
    facebook::jsi::Runtime& rt,
    const std::vector<folly::dynamic> args);
std::string preparePromiseRejectionResult(
    const std::vector<folly::dynamic> args);

ArkTSTurboModule::ArkTSTurboModule(Context ctx, std::string name)
    : m_ctx(ctx), TurboModule(ctx, name) {}

// calls a TurboModule method and blocks until it returns, returning its result
jsi::Value ArkTSTurboModule::call(
    jsi::Runtime& runtime,
    const std::string& methodName,
    const jsi::Value* jsiArgs,
    size_t argsCount) {
  auto args = convertJSIValuesToIntermediaryValues(
      runtime, m_ctx.jsInvoker, jsiArgs, argsCount);
  return jsi::valueFromDynamic(runtime, callSync(methodName, args));
}

// the cpp side calls a ArkTs TurboModule method and blocks until it returns,
// returning its result
folly::dynamic ArkTSTurboModule::callSync(
    const std::string& methodName,
    std::vector<IntermediaryArg> args) {
  facebook::react::SystraceSection s(
    "ArkTSTurboModule::callSync moduleName: ", name_, " methodName: ", methodName);
  if (!m_ctx.arkTSTurboModuleInstanceRef) {
    auto errorMsg = "Couldn't find turbo module '" + name_ +
        "' on ArkUI side. Did you link RNPackage that provides this turbo module?";
    LOG(FATAL) << errorMsg;
    throw std::runtime_error(errorMsg);
  }
  folly::dynamic result;
  m_ctx.taskExecutor->runSyncTask(
      TaskThread::MAIN, [ctx = m_ctx, &methodName, &args, &result]() {
        ArkJS arkJs(ctx.env);
        auto napiArgs = arkJs.convertIntermediaryValuesToNapiValues(args);
        auto napiTurboModuleObject =
            arkJs.getObject(ctx.arkTSTurboModuleInstanceRef);
        auto napiResult = napiTurboModuleObject.call(methodName, napiArgs);
        result = arkJs.getDynamic(napiResult);
      });
  return result;
}

// calls a TurboModule method without blocking and ignores its result
void rnoh::ArkTSTurboModule::scheduleCall(
    facebook::jsi::Runtime& runtime,
    const std::string& methodName,
    const facebook::jsi::Value* jsiArgs,
    size_t argsCount) {
  facebook::react::SystraceSection s(
    "ArkTSTurboModule::scheduleCall moduleName: ", name_, " methodName: ", methodName);
  if (!m_ctx.arkTSTurboModuleInstanceRef) {
    auto errorMsg = "Couldn't find turbo module '" + name_ +
        "' on ArkUI side. Did you link RNPackage that provides this turbo module?";
    LOG(FATAL) << errorMsg;
    throw std::runtime_error(errorMsg);
  }
  auto args = convertJSIValuesToIntermediaryValues(
      runtime, m_ctx.jsInvoker, jsiArgs, argsCount);
  m_ctx.taskExecutor->runTask(
      TaskThread::MAIN,
      [ctx = m_ctx,
       name = name_,
       methodName,
       args = std::move(args),
       &runtime]() {
        try {
          ArkJS arkJs(ctx.env);
          auto napiArgs = arkJs.convertIntermediaryValuesToNapiValues(args);
          auto napiTurboModuleObject =
              arkJs.getObject(ctx.arkTSTurboModuleInstanceRef);
          napiTurboModuleObject.call(methodName, napiArgs);
        } catch (const std::exception& e) {
          LOG(ERROR) << "Exception thrown while calling " << name
                     << " TurboModule method " << methodName << ": "
                     << e.what();
        }
      });
}

// calls an async TurboModule method and returns a Promise
jsi::Value ArkTSTurboModule::callAsync(
    jsi::Runtime& runtime,
    const std::string& methodName,
    const jsi::Value* jsiArgs,
    size_t argsCount) {
  facebook::react::SystraceSection s(
      "ArkTSTurboModule::callAsync moduleName: ", name_, " methodName: ", methodName);
  if (!m_ctx.arkTSTurboModuleInstanceRef) {
    auto errorMsg = "Couldn't find turbo module '" + name_ +
        "' on ArkUI side. Did you link RNPackage that provides this turbo module?";
    LOG(FATAL) << errorMsg;
    throw std::runtime_error(errorMsg);
  }
  auto args = convertJSIValuesToIntermediaryValues(
      runtime, m_ctx.jsInvoker, jsiArgs, argsCount);

  return react::createPromiseAsJSIValue(
      runtime,
      [ctx = m_ctx, args = args, methodName = methodName](
          jsi::Runtime& rt2, std::shared_ptr<react::Promise> jsiPromise) {
        try {
          ctx.taskExecutor->runTask(
              TaskThread::MAIN, [ctx, args = args, methodName = methodName, &rt2, jsiPromise]() {
                facebook::react::SystraceSection s(
                    "ArkTSTurboModule::callAsync Promise methodName name: ", methodName);
                ArkJS arkJs(ctx.env);
                auto napiArgs =
                    arkJs.convertIntermediaryValuesToNapiValues(args);
                auto napiTurboModuleObject =
                    arkJs.getObject(ctx.arkTSTurboModuleInstanceRef);

                auto napiResult =
                    napiTurboModuleObject.call(methodName, napiArgs);
                auto napiResultRef = arkJs.createReference(napiResult);

                Promise(ctx.env, napiResult)
                    .then([&rt2, jsiPromise, ctx, napiResultRef](auto args) {
                      facebook::react::SystraceSection s(
                          "ArkTSTurboModule::callAsync Promise then");
                      ctx.jsInvoker->invokeAsync(
                          [&rt2, jsiPromise, args = std::move(args)]() {
                            jsiPromise->resolve(
                                preparePromiseResolverResult(rt2, args));
                            jsiPromise->allowRelease();
                          });
                      ArkJS arkJs(ctx.env);
                      arkJs.deleteReference(napiResultRef);
                    })
                    .catch_([&rt2, jsiPromise, ctx, napiResultRef](auto args) {
                      ctx.jsInvoker->invokeAsync([&rt2, jsiPromise, args]() {
                        jsiPromise->reject(preparePromiseRejectionResult(args));
                        jsiPromise->allowRelease();
                      });
                      ArkJS arkJs(ctx.env);
                      arkJs.deleteReference(napiResultRef);
                    });
              });
        }catch (const std::exception& e) {}
      });
}

std::vector<IntermediaryArg>
ArkTSTurboModule::convertJSIValuesToIntermediaryValues(
    jsi::Runtime& runtime,
    std::shared_ptr<react::CallInvoker> jsInvoker,
    const jsi::Value* jsiArgs,
    size_t argsCount) {
  std::vector<IntermediaryArg> args(argsCount);
  for (int argIdx = 0; argIdx < argsCount; argIdx++) {
    if (jsiArgs[argIdx].isObject()) {
      auto obj = jsiArgs[argIdx].getObject(runtime);
      if (obj.isFunction(runtime)) {
        args[argIdx] = createIntermediaryCallback(
            react::CallbackWrapper::createWeak(
                std::move(obj.getFunction(runtime)), runtime, jsInvoker)
                .lock());
        continue;
      }
    }
    args[argIdx] = jsi::dynamicFromValue(runtime, jsiArgs[argIdx]);
  }
  return args;
}

IntermediaryCallback createIntermediaryCallback(
    std::shared_ptr<react::CallbackWrapper> cbCtx) {
  return std::function([cbCtx](std::vector<folly::dynamic> cbArgs) -> void {
    cbCtx->jsInvoker().invokeAsync([cbCtx, callbackArgs = std::move(cbArgs)]() {
      const auto jsArgs =
          convertDynamicsToJSIValues(cbCtx->runtime(), callbackArgs);
      cbCtx->callback().call(cbCtx->runtime(), jsArgs.data(), jsArgs.size());
      cbCtx->allowRelease();
    });
  });
}

const std::vector<jsi::Value> convertDynamicsToJSIValues(
    jsi::Runtime& rt,
    const std::vector<folly::dynamic>& dynamics) {
  std::vector<jsi::Value> values;
  for (auto dynamic : dynamics) {
    values.push_back(jsi::valueFromDynamic(rt, dynamic));
  }
  return values;
}

jsi::Value preparePromiseResolverResult(
    jsi::Runtime& rt,
    const std::vector<folly::dynamic> args) {
  if (args.size() == 0) {
    return jsi::Value::undefined();
  }
  if (args.size() > 1) {
    throw std::invalid_argument("`resolve` accepts only one argument");
  }
  return jsi::valueFromDynamic(rt, args[0]);
}

std::string preparePromiseRejectionResult(
    const std::vector<folly::dynamic> args) {
  if (args.size() == 0) {
    return "";
  }
  if (args.size() > 1) {
    throw std::invalid_argument("`reject` accepts only one argument");
  }

  auto error = args[0];
  if (error.isObject()) {
    auto message = error["message"];
    if (message.isString()) {
      return message.getString();
    }
  } else if (error.isString()) {
    return error.getString();
  }
  throw std::invalid_argument(
      "The type of argument provided `reject` must be string or contain a string 'message' "
      "field. It's going to be used as an error message");
}