#include "NetworkingTurboModule.h"

namespace rnoh {

using namespace facebook;

NetworkingTurboModule::NetworkingTurboModule(
    const ArkTSTurboModule::Context ctx,
    const std::string name)
    : ArkTSTurboModule(ctx, name) {
  methodMap_ = {
      ARK_ASYNC_METHOD_METADATA(sendRequest, 2),
      ARK_METHOD_METADATA(abortRequest, 1),
      ARK_ASYNC_METHOD_METADATA(clearCookies, 1),
  };
}

} // namespace rnoh
