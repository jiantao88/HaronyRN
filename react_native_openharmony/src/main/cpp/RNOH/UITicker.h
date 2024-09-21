#pragma once

#include <chrono>
#include <functional>
#include <future>
#include <mutex>
#include <unordered_set>
#include "NativeVsyncHandle.h"

namespace rnoh {

class UITicker {
 public:
  static void scheduleNextTick(long long timestamp, void* data) {
    auto self = static_cast<UITicker*>(data);
    self->tick(timestamp);
  }

  UITicker() : m_vsyncHandle("UITicker") {}

  using Shared = std::shared_ptr<UITicker>;

  std::function<void()> subscribe(int id, std::function<void(long long)>&& listener) {
    std::lock_guard lock(listenersMutex);
    auto listenersCount = m_listenerById.size();
    m_listenerById.insert_or_assign(id, std::move(listener));
    if (listenersCount == 0) {
      this->requestNextTick();
    }
    return [id, this]() {
      std::lock_guard lock(listenersMutex);
      this->m_listenerById.erase(id);
    };
  }

  int getVsyncPeriod(long long* period) {
    return m_vsyncHandle.getVsyncPeriod(period);
  }

 private:
   std::unordered_map < int, std::function <void(long long)>> m_listenerById;
   std::mutex listenersMutex;
   NativeVsyncHandle m_vsyncHandle;

  void requestNextTick() {
    m_vsyncHandle.requestFrame(scheduleNextTick, this);
  }

  void tick(long long timestamp) {
    std::lock_guard lock(listenersMutex);
    for (const auto& idAndListener : m_listenerById) {
      idAndListener.second(timestamp);
    }
    this->requestNextTick();
  }
};

} // namespace rnoh