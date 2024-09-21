#pragma once

#include "RNOH/ArkTSTurboModule.h"
#include <uv.h>
#include <thread>
#include <map>
#include "uv/EventLoop.h"
#include "uv/Timer.h"
#include <functional>
#include "RNOH/RNInstanceCAPI.h"
namespace rnoh {
class TimingArkTSMessageHandler;
class JSI_EXPORT TimingTurboModule : public ArkTSTurboModule, public std::enable_shared_from_this<TimingTurboModule> {
 public:
  TimingTurboModule(
      const ArkTSTurboModule::Context ctx,
      const std::string name);
  ~TimingTurboModule() override;

  void createTimer(
    double id,
    double duration,
    double jsSchedulingTime,
    bool repeat);
  void deleteTimer(double id);
  void setSendIdleEvents(bool enabled);
  facebook::jsi::Runtime* runtime;

 private:
  void initLoop();
  std::map<double, uv::Timer*> nativeTimerMap;
  std::shared_ptr<uv::EventLoop> loop;
  std::shared_ptr<TimingArkTSMessageHandler> messageHandler;
};

} // namespace rnoh