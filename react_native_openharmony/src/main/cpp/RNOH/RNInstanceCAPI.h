#pragma once
#include <js_native_api.h>
#include <js_native_api_types.h>
#include <atomic>
#include <functional>
#include <memory>
#include <thread>
#include <utility>

#include <ace/xcomponent/native_interface_xcomponent.h>
#include <cxxreact/Instance.h>
#include <cxxreact/ModuleRegistry.h>
#include <cxxreact/NativeModule.h>
#include <folly/dynamic.h>
#include <rawfile/raw_file_manager.h>
#include <react/renderer/animations/LayoutAnimationDriver.h>
#include <react/renderer/componentregistry/ComponentDescriptorProviderRegistry.h>
#include <react/renderer/scheduler/Scheduler.h>
#include <react/renderer/uimanager/LayoutAnimationStatusDelegate.h>

#include "ArkTSMessageHub.h"
#include "RNOH/ArkTSChannel.h"
#include "RNOH/EventDispatcher.h"
#include "RNOH/EventEmitRequestHandler.h"
#include "RNOH/GlobalJSIBinder.h"
#include "RNOH/MessageQueueThread.h"
#include "RNOH/RNInstance.h"
#include "RNOH/MountingManager.h"
#include "RNOH/ShadowViewRegistry.h"
#include "RNOH/TaskExecutor/TaskExecutor.h"
#include "RNOH/TurboModuleFactory.h"
#include "RNOH/TurboModuleProvider.h"
#include "RNOH/UITicker.h"

#include "RNOH/ArkTSMessageHandler.h"
#include "RNOH/ComponentInstanceFactory.h"
#include "RNOH/ComponentInstanceRegistry.h"
#include "RNOH/arkui/XComponentSurface.h"

namespace rnoh {
using MutationsListener = std::function<void(
    MutationsToNapiConverter const&,
    facebook::react::ShadowViewMutationList const& mutations)>;

using SharedNativeResourceManager = std::shared_ptr<NativeResourceManager>;

class RNInstanceCAPI : public RNInstanceInternal,
                       public facebook::react::LayoutAnimationStatusDelegate {
 public:
  RNInstanceCAPI(
      int id,
      std::shared_ptr<facebook::react::ContextContainer> contextContainer,
      TurboModuleFactory&& turboModuleFactory,
      std::shared_ptr<TaskExecutor> taskExecutor,
      std::shared_ptr<facebook::react::ComponentDescriptorProviderRegistry>
          componentDescriptorProviderRegistry,
      MutationsToNapiConverter::Shared mutationsToNapiConverter,
      EventEmitRequestHandlers eventEmitRequestHandlers,
      GlobalJSIBinders globalJSIBinders,
      UITicker::Shared uiTicker,
      ShadowViewRegistry::Shared shadowViewRegistry,
      MountingManager::Shared mountingManager,
      std::vector<ArkTSMessageHandler::Shared> arkTSMessageHandlers,
      ArkTSChannel::Shared arkTSChannel,
      ArkTSMessageHub::Shared arkTSMessageHub,
      ComponentInstanceRegistry::Shared componentInstanceRegistry,
      ComponentInstanceFactory::Shared componentInstanceFactory,
      SharedNativeResourceManager nativeResourceManager,
      bool shouldEnableDebugger,
      bool shouldEnableBackgroundExecutor)
      : RNInstanceInternal(),
        m_id(id),
        instance(std::make_shared<facebook::react::Instance>()),
        m_contextContainer(contextContainer),
        scheduler(nullptr),
        taskExecutor(taskExecutor),
        m_shadowViewRegistry(shadowViewRegistry),
        m_turboModuleFactory(std::move(turboModuleFactory)),
        m_componentDescriptorProviderRegistry(
            componentDescriptorProviderRegistry),
        m_mutationsToNapiConverter(mutationsToNapiConverter),
        m_eventEmitRequestHandlers(eventEmitRequestHandlers),
        m_globalJSIBinders(globalJSIBinders),
        m_shouldRelayUITick(false),
        m_uiTicker(uiTicker),
        m_mountingManager(std::move(mountingManager)),
        m_shouldEnableDebugger(shouldEnableDebugger),
        m_shouldEnableBackgroundExecutor(shouldEnableBackgroundExecutor),
        m_arkTSMessageHub(std::move(arkTSMessageHub)),
        m_componentInstanceRegistry(componentInstanceRegistry),
        m_componentInstanceFactory(componentInstanceFactory),
        m_arkTSChannel(std::move(arkTSChannel)),
        m_arkTSMessageHandlers(std::move(arkTSMessageHandlers)),
        m_nativeResourceManager(std::move(nativeResourceManager)) {}

 ~RNInstanceCAPI() noexcept override;

  TaskExecutor::Shared getTaskExecutor() override;

  void start() override;
  void loadScript(
      std::vector<uint8_t>&& bundle,
      std::string const sourceURL,
      std::function<void(const std::string)>&& onFinish) override;
  void createSurface(
      facebook::react::Tag surfaceId,
      std::string const& moduleName) override;
  void updateSurfaceConstraints(
      facebook::react::Tag surfaceId,
      float width,
      float height,
      float viewportOffsetX,
      float viewportOffsetY,
      float pixelRatio,
      bool isRTL) override;
  void startSurface(
      facebook::react::Tag surfaceId,
      float width,
      float height,
      float viewportOffsetX,
      float viewportOffsetY,
      float pixelRatio,
      bool isRTL,
      folly::dynamic&& initialProps) override;
  void setSurfaceProps(facebook::react::Tag surfaceId, folly::dynamic&& props)
      override;
  void stopSurface(facebook::react::Tag surfaceId) override;
  void destroySurface(facebook::react::Tag surfaceId) override;
  void setSurfaceDisplayMode(
      facebook::react::Tag surfaceId,
      facebook::react::DisplayMode displayMode) override;
  void callFunction(
      std::string&& module,
      std::string&& method,
      folly::dynamic&& params) override;
  void emitComponentEvent(
      napi_env env,
      facebook::react::Tag tag,
      std::string eventName,
      napi_value payload) override;
  void onMemoryLevel(size_t memoryLevel) override;
  void updateState(
      napi_env env,
      std::string const& componentName,
      facebook::react::Tag tag,
      napi_value newState) override;

  void synchronouslyUpdateViewOnUIThread(
      facebook::react::Tag tag,
      folly::dynamic props) override;

  facebook::react::ContextContainer const& getContextContainer() const override;

  void registerNativeXComponentHandle(
      OH_NativeXComponent* nativeXComponent,
      facebook::react::Tag surfaceId);

  TurboModule::Shared getTurboModule(const std::string& name) override;

  std::shared_ptr<TaskExecutor> taskExecutor;

  ComponentInstance::Shared findComponentInstanceByTag(
      facebook::react::Tag tag);

  std::optional<facebook::react::Tag> findComponentInstanceTagById(
      const std::string& id);

  void handleArkTSMessage(
      const std::string& name,
      folly::dynamic const& payload) override;

  void postMessageToArkTS(
      const std::string& name,
      folly::dynamic const& payload) override;
  void setBundlePath(std::string const& path) override;
  std::string getBundlePath() override;
  void addArkTSMessageHandler(ArkTSMessageHandler::Shared handler);
  void removeArkTSMessageHandler(ArkTSMessageHandler::Shared handler);
  int getId() override { return m_id; }
  NativeResourceManager const* getNativeResourceManager() const override;
    
  std::optional<Surface::Weak> getSurfaceByRootTag(
      facebook::react::Tag rootTag) override;
    

 protected:
  int m_id;
  facebook::react::ContextContainer::Shared m_contextContainer;
  std::map<
      facebook::react::Tag,
      std::shared_ptr<facebook::react::SurfaceHandler>>
      surfaceHandlers;
  std::shared_ptr<facebook::react::ComponentDescriptorProviderRegistry>
      m_componentDescriptorProviderRegistry;
  ShadowViewRegistry::Shared m_shadowViewRegistry;
  TurboModuleFactory m_turboModuleFactory;
  TurboModuleProvider::Shared m_turboModuleProvider;
  std::shared_ptr<EventDispatcher> m_eventDispatcher;
  MutationsToNapiConverter::Shared m_mutationsToNapiConverter;
  EventEmitRequestHandlers m_eventEmitRequestHandlers;
  GlobalJSIBinders m_globalJSIBinders;
  std::shared_ptr<facebook::react::LayoutAnimationDriver> m_animationDriver;
  UITicker::Shared m_uiTicker;
  std::function<void()> unsubscribeUITickListener = nullptr;
  std::atomic<bool> m_shouldRelayUITick;
  std::shared_ptr<MessageQueueThread> m_jsQueue;
  bool m_shouldEnableDebugger;
  bool m_shouldEnableBackgroundExecutor;
  std::unordered_map<facebook::react::SurfaceId, XComponentSurface::Shared>
      m_surfaceById;
  ComponentInstanceRegistry::Shared m_componentInstanceRegistry;
  ComponentInstanceFactory::Shared m_componentInstanceFactory;
  MountingManager::Shared m_mountingManager;
  std::unique_ptr<facebook::react::SchedulerDelegate> m_schedulerDelegate = nullptr;
  /**
   * NOTE: Order matters. m_scheduler holds indirectly jsi::Values.
   * These values must be destructed before the runtime.
   * The runtime is destructed when `m_reactInstance` is destructed.
   * Therefore, `m_scheduler` must be declared after `m_reactInstance`.
   */
  std::shared_ptr<facebook::react::Instance> instance;
  std::shared_ptr<facebook::react::Scheduler> scheduler = nullptr;
  std::vector<ArkTSMessageHandler::Shared> m_arkTSMessageHandlers;
  ArkTSChannel::Shared m_arkTSChannel;
  SharedNativeResourceManager m_nativeResourceManager;
  std::string m_bundlePath;
  ArkTSMessageHub::Shared m_arkTSMessageHub;

  void initialize();
  void initializeScheduler(
      std::shared_ptr<TurboModuleProvider> turboModuleProvider);
  std::shared_ptr<TurboModuleProvider> createTurboModuleProvider();
  void onUITick(long long timestamp);
  void schedulerTransactionByVsync(long long timestamp, long long period);
  void onAnimationStarted() override; // react::LayoutAnimationStatusDelegate
  void onAllAnimationsComplete()
      override; // react::LayoutAnimationStatusDelegate
};

} // namespace rnoh
