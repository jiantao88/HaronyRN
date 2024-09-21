#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include "NativeVsyncHandle.h"

namespace rnoh {
class VSyncListener : public std::enable_shared_from_this<VSyncListener> {
 public:
  using VSyncCallback = std::function<void(long long timestamp)>;
  void requestFrame(VSyncCallback callback) {
    {
      std::lock_guard<std::mutex> lock(m_callbackMutex);
      m_callback = std::move(callback);
    }
    scheduleNextVsync();
  }

 private:
  void scheduleNextVsync();

  NativeVsyncHandle m_vsyncHandle = NativeVsyncHandle("VSyncListener");
  std::mutex m_callbackMutex;
  VSyncCallback m_callback;
  std::atomic_bool m_scheduled{false};
};
} // namespace rnoh