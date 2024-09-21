#pragma once

#include <react/renderer/scheduler/SchedulerDelegate.h>
#include "RNOH/MountingManager.h"
#include "RNOH/TaskExecutor/TaskExecutor.h"

namespace rnoh {

/**
 * @ThreadSafe
 *
 * Implementation of the react::SchedulerDelegate interface.
 * Schedules operations on the main thread
 * and delegates them to the MountingManager.
 */
class SchedulerDelegate final : public facebook::react::SchedulerDelegate {
  using MountingCoordinator = facebook::react::MountingCoordinator;
  using ShadowNode = facebook::react::ShadowNode;
  using ShadowView = facebook::react::ShadowView;
  using SurfaceId = facebook::react::SurfaceId;

 public:
  SchedulerDelegate(
      MountingManager::Shared mountingManager,
      TaskExecutor::Shared taskExecutor,
       PreAllocationBuffer::Shared preAllocationBuffer)
      : m_mountingManager(mountingManager), m_taskExecutor(taskExecutor),
        m_preAllocationBuffer(std::move(preAllocationBuffer)){};
    
  void schedulerDidRequestPreliminaryViewAllocation(
    SurfaceId surfaceId,
    const ShadowNode& shadowView) override{
        if(m_preAllocationBuffer){
            m_preAllocationBuffer->push(shadowView);
        }
    }

 void schedulerDidViewAllocationByVsync(long long timestamp, long long period) {
         if(m_preAllocationBuffer){
            m_preAllocationBuffer->flushByVsync(timestamp, period);
        }
  }

  void schedulerDidFinishTransaction(
      MountingCoordinator::Shared mountingCoordinator) override;
    

  void schedulerDidDispatchCommand(
      const ShadowView& shadowView,
      std::string const& commandName,
      folly::dynamic const& args) override;

  void schedulerDidSendAccessibilityEvent(
      const ShadowView& shadowView,
      std::string const& eventType) override;

  void schedulerDidSetIsJSResponder(
      ShadowView const& shadowView,
      bool isJSResponder,
      bool blockNativeResponder) override;

 private:
  template <typename Operation>
  void performOnMainThread(Operation operation) {
    m_taskExecutor->runTask(
        TaskThread::MAIN,
        [weakMountingManager = m_mountingManager,
         operation = std::move(operation)] {
          if (auto mountingManager = weakMountingManager.lock()) {
            operation(mountingManager);
          }
        });
  }

  MountingManager::Weak m_mountingManager;
  TaskExecutor::Shared m_taskExecutor;
  PreAllocationBuffer::Shared m_preAllocationBuffer;
};

}; // namespace rnoh