#include "TimingTurboModule.h"
#include <thread>
#include "RNOH/RNInstance.h"
#include <chrono>
#include <functional>
#include <glog/logging.h>

namespace rnoh {

using namespace facebook;

class TimingArkTSMessageHandler: public ArkTSMessageHandler {
  public:
  TimingArkTSMessageHandler(std::weak_ptr<TimingTurboModule> weakTurboModule)
      : m_weakTurboModule(weakTurboModule) {}
  void handleArkTSMessage(const Context& ctx) override
  {
    if (ctx.messageName == "TimingCreateTimer") {
      if (auto turboModule = m_weakTurboModule.lock(); turboModule!= nullptr) {
        turboModule->createTimer(ctx.messagePayload["id"].asDouble(),
                                ctx.messagePayload["duration"].asDouble(),
                                0,
                                ctx.messagePayload["repeats"].asBool());
      }
    }
  }
  std::weak_ptr<TimingTurboModule> m_weakTurboModule;
};

static jsi::Value __hostFunction_TimingTurboModule_createTimer(
    jsi::Runtime& rt,
    react::TurboModule& turboModule,
    const jsi::Value* args,
    size_t count) {
  auto self = static_cast<TimingTurboModule*>(&turboModule);
  self->runtime = &rt;
  self->createTimer(args[0].getNumber(),
                    args[1].getNumber(),
                    args[2].getNumber(),
                    args[3].getBool()
                    );
  return jsi::Value::undefined();
}

static jsi::Value __hostFunction_TimingTurboModule_deleteTimer(
    jsi::Runtime& rt,
    react::TurboModule& turboModule,
    const jsi::Value* args,
    size_t count) {
  auto self = static_cast<TimingTurboModule*>(&turboModule);
  self->deleteTimer(args[0].getNumber());
  return jsi::Value::undefined();
}

static jsi::Value __hostFunction_TimingTurboModule_setSendIdleEvents(
    jsi::Runtime& rt,
    react::TurboModule& turboModule,
    const jsi::Value* args,
    size_t count) {
  auto self = static_cast<TimingTurboModule*>(&turboModule);
  self->setSendIdleEvents(args[0].getBool());
  return jsi::Value::undefined();
}

TimingTurboModule::TimingTurboModule(
    const ArkTSTurboModule::Context ctx,
    const std::string name)
    : ArkTSTurboModule(ctx, name) {
  initLoop();
  methodMap_ = {
      {"createTimer", {4, __hostFunction_TimingTurboModule_createTimer}},
      {"deleteTimer", {1, __hostFunction_TimingTurboModule_deleteTimer}},
      {"setSendIdleEvents",
       {1, __hostFunction_TimingTurboModule_setSendIdleEvents}},
  };
}

folly::dynamic getObject(double callbackID) {
  auto object1 = folly::dynamic(callbackID);
  folly::dynamic object2 = folly::dynamic::array();
  object2.push_back(object1);
  folly::dynamic object3 = folly::dynamic::array();
  object3.push_back(object2);
  return object3;
}

void TimingTurboModule::initLoop() {
  loop = std::make_shared<uv::EventLoop>();
  std::thread([loop=this->loop]() {
    loop->run();
  }).detach();
}

void TimingTurboModule::createTimer(
    double id,
    double duration,
    double jsSchedulingTime,
    bool repeat) {
  auto wptr = this->weak_from_this();

  if (auto instance = m_ctx.instance.lock(); instance != nullptr) {
    if (messageHandler == nullptr) {
      auto rnInstance = dynamic_cast<RNInstanceCAPI*>(instance.get());
      messageHandler = std::make_shared<TimingArkTSMessageHandler>(wptr);
      rnInstance->addArkTSMessageHandler(messageHandler);
    }
  }

  auto triggerTimer = [wptr, id, duration, repeat](uv::Timer* timer) {
    auto timingPtr = wptr.lock();
    if (timingPtr == nullptr) {
      return;
    }
    if (auto instance = timingPtr->m_ctx.instance.lock(); instance != nullptr) {
      instance->callFunction(
      "JSTimers", "callTimers", std::move(getObject(id)));
    }
    if (!repeat) {
      timingPtr->deleteTimer(id);
    }
    if (timingPtr->call(*timingPtr->runtime, "isPaused", nullptr, 0).getBool()) {
      timingPtr->deleteTimer(id);
      facebook::jsi::Value testargs[4] ={
      facebook::jsi::Value(id),
      facebook::jsi::Value(duration),
      facebook::jsi::Value(repeat)};
      timingPtr->call(*timingPtr->runtime, "createTimerInCpp", testargs, 3);
    }
  };

  loop->runInThisLoopEn([wptr, id, duration, repeat, triggerTimer, jsSchedulingTime]()
  {
    auto timingPtr = wptr.lock();
    if (timingPtr == nullptr) {
      return;
    }
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::chrono::milliseconds milliseconds =
      std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    uint64_t delay = milliseconds.count() - static_cast<uint64_t>(jsSchedulingTime);

    uint64_t timeout = static_cast<uint64_t>(duration)
      > delay ?(static_cast<uint64_t>(duration) - delay) : 0;
    uint64_t interval = repeat? static_cast<uint64_t>(duration) : 0;

    uv::Timer * timer = new uv::Timer(timingPtr->loop.get(), timeout, interval, triggerTimer);
    timingPtr->nativeTimerMap[id] = timer;
    timer->start();
  });
}

void TimingTurboModule::deleteTimer(double id) {
  auto wptr = this->weak_from_this();
  loop->runInThisLoopEn([wptr, id]() {
    auto timingPtr = wptr.lock();
    if (timingPtr == nullptr) {
      return;
    }
    auto it = timingPtr->nativeTimerMap.find(id);
    if (it != timingPtr->nativeTimerMap.end()) {
      auto timer = it->second;
      timer->stop();
      timer->close([](uv::Timer* t)
      {
        delete t;
      });
      timingPtr->nativeTimerMap.erase(it);
    }
  });
}

void TimingTurboModule::setSendIdleEvents(bool enabled) {
  LOG(INFO) << "TimingTurboModule::setSendIdleEvents("
            << enabled
            << "): not implemented";
}

TimingTurboModule::~TimingTurboModule() {
  if (auto instance = m_ctx.instance.lock(); instance != nullptr) {
    auto rnInstance = dynamic_cast<RNInstanceCAPI*>(instance.get());
    rnInstance->removeArkTSMessageHandler(messageHandler);
  }
  loop->runInThisLoopEn([timeMap=this->nativeTimerMap]() {
    for (auto it = timeMap.begin(); it != timeMap.end(); ++it) {
      auto timer = it->second;
      timer->stop();
      timer->close([](uv::Timer* t)
      {
        delete t;
      });
    }
  });
}

} // namespace rnoh