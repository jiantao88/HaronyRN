#pragma once
#include <folly/dynamic.h>
#include <jsi/jsi.h>
#include <napi/native_api.h>

namespace rnoh {

using namespace facebook;

napi_value jsiToNapi(napi_env env, jsi::Runtime& rt, const jsi::Value& value);

jsi::Value napiToJsi(napi_env env, jsi::Runtime& rt, napi_value value);

} // namespace rnoh
