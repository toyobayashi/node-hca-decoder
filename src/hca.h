#ifndef __HCA_H__
#define __HCA_H__
#include <node_api.h>
#include "./common.h"

#define MAX_PATH 260

class HCADecoder {
public:
  static void init(napi_env env, napi_value exports);
  static void destructor(napi_env env, void* nativeObject, void* finalizeHint);
private:
  explicit HCADecoder(unsigned int key1, unsigned int key2);
  ~HCADecoder();

  unsigned int key1;
  unsigned int key2;
  napi_env env_;
  napi_ref wrapper_;
  static napi_ref constructorRef;
  static napi_value constructor(napi_env env, napi_callback_info info);

  static napi_value decodeToWaveFileSync(napi_env env, napi_callback_info info);
  static napi_value decrypt(napi_env env, napi_callback_info info);
  static napi_value printInfo(napi_env env, napi_callback_info info);
  static napi_value decodeToWaveFile(napi_env env, napi_callback_info info);
};

typedef struct {
  unsigned int key1;
  unsigned int key2;
  char filenameHCA[MAX_PATH];
  char filenameWAV[MAX_PATH];
  double volume;
  int mode;
  int loop;
  bool status;
  int callbackIndex;
  napi_ref _cbref;
  napi_async_work _request;
} AsyncDec;

#endif
