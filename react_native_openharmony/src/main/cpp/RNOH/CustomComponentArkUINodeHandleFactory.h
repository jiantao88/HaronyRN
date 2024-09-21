#pragma once
#include <glog/logging.h>
#include <react/renderer/core/ReactPrimitives.h>
#include "RNOH/ArkJS.h"
#include "RNOH/TaskExecutor/TaskExecutor.h"
#include "RNOH/ThreadGuard.h"

#ifdef C_API_ARCH
#include <arkui/native_node.h>
#include <arkui/native_node_napi.h>
#endif

namespace rnoh {

/**
 * @thread: MAIN
 * Used by the ComponentRegistry to create ComponentInstances written in ArkTS.
 */
class CustomComponentArkUINodeHandleFactory final {
 public:
  using Shared = std::shared_ptr<CustomComponentArkUINodeHandleFactory>;

  CustomComponentArkUINodeHandleFactory(
      napi_env env,
      napi_ref customRNComponentFrameNodeFactoryRef,
      TaskExecutor::Shared taskExecutor)
      : m_env(env),
        m_taskExecutor(taskExecutor),
        m_customRNComponentFrameNodeFactoryRef(
            customRNComponentFrameNodeFactoryRef) {}

  std::pair<ArkUI_NodeHandle, std::function<void()>> create(
      facebook::react::Tag tag,
      std::string componentName) {
    m_threadGuard.assertThread();
#ifdef C_API_ARCH
    ArkJS arkJs(m_env);
    auto frameNodeFactory =
        arkJs.getObject(m_customRNComponentFrameNodeFactoryRef)
            .getProperty("frameNodeFactory");
    auto n_result =
        arkJs.getObject(frameNodeFactory)
            .call(
                "create",
                {arkJs.createInt(tag), arkJs.createString(componentName)});
    auto n_arkTsNodeHandle = arkJs.getObjectProperty(n_result, "frameNode");
    auto n_destroyBuilderNode =
        arkJs.getObjectProperty(n_result, "destroyBuilderNode");
    auto n_destroyBuilderNodeRef = arkJs.createReference(n_destroyBuilderNode);
    ArkUI_NodeHandle arkTsNodeHandle = nullptr;
    auto errorCode = OH_ArkUI_GetNodeHandleFromNapiValue(
        m_env, n_arkTsNodeHandle, &arkTsNodeHandle);
    if (errorCode != 0) {
      LOG(ERROR) << "Couldn't get node handle. Error code: " << errorCode;
      return std::make_pair(nullptr, [] {});
    }
    return std::make_pair(
        arkTsNodeHandle, [env = m_env, n_destroyBuilderNodeRef] {
          ArkJS arkJs(env);
          auto n_destroy = arkJs.getReferenceValue(n_destroyBuilderNodeRef);
          arkJs.call(n_destroy, {});
          arkJs.deleteReference(n_destroyBuilderNodeRef);
        });
#else
    return std::make_pair(nullptr, [] {});
#endif
  }

 private:
  TaskExecutor::Shared m_taskExecutor;
  napi_env m_env;
  napi_ref m_customRNComponentFrameNodeFactoryRef;
  ThreadGuard m_threadGuard{};
};
} // namespace rnoh