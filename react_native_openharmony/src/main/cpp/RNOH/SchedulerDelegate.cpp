#include "SchedulerDelegate.h"

namespace rnoh {

void SchedulerDelegate::schedulerDidFinishTransaction(
    MountingCoordinator::Shared mountingCoordinator) {
  mountingCoordinator->getTelemetryController().pullTransaction(
      [this](auto const& transaction, auto const& surfaceTelemetry) {
        performOnMainThread(
            [transaction](MountingManager::Shared const& mountingManager) {
              mountingManager->willMount(transaction.getMutations());
            });
      },
      [this](auto const& transaction, auto const& surfaceTelemetry) {
        performOnMainThread(
            [transaction](MountingManager::Shared const& mountingManager) {
              mountingManager->doMount(transaction.getMutations());
            });
      },
      [this](auto const& transaction, auto const& surfaceTelemetry) {
        auto mutationVecs = transaction.getMutations();
        facebook::react::ShadowViewMutationList mutationVec;
        facebook::react::ShadowViewMutationList otherMutation;
        // The reason for using 70 here is that the average time for each mutation 
        // creation is about 60 microseconds,and the time for 70 is 4.2 milliseconds, 
        // which corresponds to the length of half a frame
        size_t fragmentSize = 70;
        std::vector<facebook::react::ShadowViewMutationList> destVec;
        auto it = mutationVecs.begin();
        while (it != mutationVecs.end() ) {
            if( it->type == facebook::react::ShadowViewMutation::Create ){
                    mutationVec.push_back(*it);
               }else {
                    otherMutation.push_back(*it);
               }
            ++it;
        }
        size_t mutationSize = mutationVec.size();
        if( mutationSize > 0 ){
            destVec.reserve((mutationSize + fragmentSize - 1) / fragmentSize);
            for (size_t i = 0; i < mutationSize; i += fragmentSize) {
                    destVec.emplace_back(
                        mutationVec.begin() + i,
                        mutationVec.begin() +
                            std::min(mutationSize, i + fragmentSize));
            }
            for (const auto& mutations : destVec) {
                    performOnMainThread(
                        [mutations](
                            MountingManager::Shared const& mountingManager) {
                          mountingManager->didMount(mutations);
                        });
            }
        }
        performOnMainThread(
            [otherMutation](MountingManager::Shared 
        const& mountingManager) {
              mountingManager->didMount(otherMutation);
            });
       });
}

void SchedulerDelegate::schedulerDidDispatchCommand(
    const ShadowView& shadowView,
    std::string const& commandName,
    folly::dynamic const& args) {
  performOnMainThread([shadowView, commandName, args](
                          MountingManager::Shared const& mountingManager) {
    mountingManager->dispatchCommand(shadowView, commandName, args);
  });
}

void SchedulerDelegate::schedulerDidSendAccessibilityEvent(
    const ShadowView& shadowView,
    std::string const& eventType) {}

void SchedulerDelegate::schedulerDidSetIsJSResponder(
    ShadowView const& shadowView,
    bool isJSResponder,
    bool blockNativeResponder) {
  performOnMainThread([shadowView, isJSResponder, blockNativeResponder](
                          MountingManager::Shared const& mountingManager) {
    mountingManager->setIsJsResponder(
        shadowView, isJSResponder, blockNativeResponder);
  });
}

} // namespace rnoh