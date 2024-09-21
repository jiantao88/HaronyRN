/**
 * Used only in C-API based Architecture.
 */
#pragma once

#include <queue>
#include "react/renderer/core/ReactPrimitives.h"
#include "react/renderer/core/ShadowNode.h"
namespace rnoh
{
  class PreAllocationBuffer
  {
  private:
    using PreAllocInfo = struct
    {
      facebook::react::Tag tag;
      facebook::react::ComponentHandle componentHandle;
      std::string componentName;
    };
    std::queue<PreAllocInfo> m_preAllocQueue;
    std::mutex m_mtx;
    std::function<void(
        facebook::react::Tag tag,
        facebook::react::ComponentHandle componentHandle,
        std::string componentName)>
        m_preAllocDelegate;

  public:
    using Shared = std::shared_ptr<PreAllocationBuffer>;
    static const int MAX_CAPACITY = 12;

    ~PreAllocationBuffer()
    {
      DLOG(INFO) << "~PreAllocationBuffer";
    }

    void push(const facebook::react::ShadowNode &shadowNode)
    {
      std::lock_guard<std::mutex> lock(m_mtx);
      m_preAllocQueue.push(
          {shadowNode.getTag(),
           shadowNode.getComponentHandle(),
           shadowNode.getComponentName()});
    }

    std::shared_ptr<PreAllocInfo> pop() {
      std::lock_guard<std::mutex> lock(m_mtx);
      if (!m_preAllocQueue.empty()) {
        auto allocInfo = m_preAllocQueue.front();
        m_preAllocQueue.pop();
        return std::make_shared<PreAllocInfo>(allocInfo);
      }
      return nullptr;
    }

    bool isPreAllocBufferEmpty() {
      std::lock_guard<std::mutex> lock(m_mtx);
      return m_preAllocQueue.empty();
    }
    
    bool haveExceededNonBatchedFrameTime(long long timestamp) {
      struct timespec tp;
      // 获取实时时间
      if (clock_gettime(CLOCK_MONOTONIC, &tp) == -1) {
        return false;
      }
      long long nanoseconds = (long long)tp.tv_sec * 1000000000 + tp.tv_nsec;
      long timeLeftInFrame = 8 - ((nanoseconds - timestamp) / 1000000);
      return timeLeftInFrame < 4;
    }

    void setPreAllocDelegate(std::function<void(
                                 facebook::react::Tag,
                                 facebook::react::ComponentHandle,
                                 std::string componentName)>
                                 func)
    {
      std::lock_guard<std::mutex> lock(m_mtx);
      m_preAllocDelegate = func;
    }

    void flushByVsync(long long timestamp, long long period)
    {
      if (isPreAllocBufferEmpty()) {
        return;
      }
      while (true)
      {
        if (haveExceededNonBatchedFrameTime(timestamp))
        {
          break;
        }
        auto allocInfo = pop();
        if (allocInfo == nullptr)
        {
          break;
        }
        if (m_preAllocDelegate != nullptr)
        {
          m_preAllocDelegate(allocInfo->tag, allocInfo->componentHandle, allocInfo->componentName);
        }
      }
    }
  };
} // namespace rnoh