#include "./hca.h"

NAPI_MODULE_INIT() {
  HCADecoder::init(env, exports);
  return exports;
}
