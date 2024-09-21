#pragma once
#include <thread>
#include "RNOH/Assert.h"

namespace rnoh {

/**
 * Utility class for asserting that an object is called and destroyed on the
 * same thread that created the object.
 */
class ThreadGuard final {
 public:
  void assertThread() const {
    RNOH_ASSERT_MSG(
        m_threadId == std::this_thread::get_id(),
        "This method must be called on the same thread that created the object.");
  }

  ThreadGuard() = default;
  ~ThreadGuard() {
    assertThread();
  }

  // rule-of-five constructors and assignment operators
  ThreadGuard(ThreadGuard const&) = default;
  ThreadGuard& operator=(ThreadGuard const&) = default;

  ThreadGuard(ThreadGuard&&) = default;
  ThreadGuard& operator=(ThreadGuard&&) = default;

 private:
  std::thread::id m_threadId = std::this_thread::get_id();
};
} // namespace rnoh