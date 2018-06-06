#include "./hca.h"
#include "./clHCA.h"
#include <stdio.h>
#include <string.h>

#define MAX_PATH 260

HCADecoder::HCADecoder(unsigned int k1, unsigned int k2): key1(k1), key2(k2), env_(nullptr), wrapper_(nullptr) {}

HCADecoder::~HCADecoder() { napi_delete_reference(env_, wrapper_); }

napi_ref HCADecoder::constructorRef;

void HCADecoder::init(napi_env env, napi_value exports) {
  napi_property_descriptor properties[] = {
    DECLARE_NAPI_PROPERTY("decodeToWaveFileSync", decodeToWaveFileSync),
    DECLARE_NAPI_PROPERTY("printInfo", printInfo),
    DECLARE_NAPI_PROPERTY("decrypt", decrypt)
  };
  napi_value jsClassHCADecoder;
  NAPI_CALL_RETURN_VOID(env, napi_define_class(
    env, "HCADecoder", -1, constructor, nullptr, 3, properties, &jsClassHCADecoder));

  NAPI_CALL_RETURN_VOID(env, napi_create_reference(env, jsClassHCADecoder, 1, &constructorRef));

  NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, exports, "HCADecoder", jsClassHCADecoder));
}

napi_value HCADecoder::constructor(napi_env env, napi_callback_info info) {
  napi_value newTarget;
  NAPI_CALL(env, napi_get_new_target(env, info, &newTarget));
  bool isConstructor = (newTarget != nullptr);

  size_t argc = 2;
  napi_value args[2];
  napi_value _this;
  NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, nullptr));

  if (isConstructor) {
    unsigned int ciphKey1 = 0xF27E3B22;
    unsigned int ciphKey2 = 0x00003657;

    napi_valuetype valuetype0, valuetype1;
    NAPI_CALL(env, napi_typeof(env, args[0], &valuetype0));
    NAPI_CALL(env, napi_typeof(env, args[1], &valuetype1));

    if (valuetype0 == napi_number) {
      NAPI_CALL(env, napi_get_value_uint32(env, args[0], &ciphKey1));
    } 
    if (valuetype1 == napi_number) {
      NAPI_CALL(env, napi_get_value_uint32(env, args[1], &ciphKey2));
    }

    HCADecoder* dec = new HCADecoder(ciphKey1, ciphKey2);

    dec->env_ = env;
    NAPI_CALL(env, napi_wrap(env,
                             _this,
                             dec,
                             destructor,
                             nullptr,
                             &dec->wrapper_));

    return _this;
  }

  argc = 2;
  napi_value argv[2] = { args[0], args[1] };

  napi_value cons;
  NAPI_CALL(env, napi_get_reference_value(env, constructorRef, &cons));

  napi_value instance;
  NAPI_CALL(env, napi_new_instance(env, cons, argc, argv, &instance));

  return instance;
}

napi_value HCADecoder::decodeToWaveFileSync(napi_env env, napi_callback_info info) {
  napi_value _this;
  size_t argc = 5;
  napi_value args[5];
  NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, nullptr));

  char filenameHCA[MAX_PATH];
  napi_valuetype valuetype0;
  NAPI_CALL(env, napi_typeof(env, args[0], &valuetype0));
  NAPI_ASSERT(env, valuetype0 == napi_string, "HCA file name must be a string.");
  NAPI_CALL(env, napi_get_value_string_utf8(env, args[0], filenameHCA, MAX_PATH, nullptr));

  char filenameWAV[MAX_PATH];
  napi_valuetype valuetype1;
  NAPI_CALL(env, napi_typeof(env, args[1], &valuetype1));
  if (valuetype1 != napi_string) {
    strcpy(filenameWAV, filenameHCA);
    char *d1 = strrchr(filenameWAV, '\\');
    char *d2 = strrchr(filenameWAV, '/');
    char *e = strrchr(filenameWAV, '.');
    if (e && d1 < e && d2 < e) *e = '\0';
    strcat(filenameWAV, ".wav");
  } else {
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[1], filenameWAV, MAX_PATH, nullptr));
  }

  double volume = 1;
  napi_valuetype valuetype2;
  NAPI_CALL(env, napi_typeof(env, args[2], &valuetype2));
  if (valuetype2 == napi_number) {
    NAPI_CALL(env, napi_get_value_double(env, args[2], &volume));
  }

  int mode = 16;
  napi_valuetype valuetype3;
  NAPI_CALL(env, napi_typeof(env, args[3], &valuetype3));
  if (valuetype3 == napi_number) {
    NAPI_CALL(env, napi_get_value_int32(env, args[3], &mode));
  }

  int loop = 0;
  napi_valuetype valuetype4;
  NAPI_CALL(env, napi_typeof(env, args[4], &valuetype4));
  if (valuetype4 == napi_number) {
    NAPI_CALL(env, napi_get_value_int32(env, args[4], &loop));
  }

  HCADecoder* dec;
  NAPI_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&dec)));

  clHCA hca(dec->key1, dec->key2);
  if (!hca.DecodeToWavefile(filenameHCA, filenameWAV, (float)volume, mode, loop)) {
    napi_value resultFalse;
    NAPI_CALL(env, napi_get_boolean(env, false, &resultFalse));
    return resultFalse;
  }
  
  napi_value resultTrue;
  NAPI_CALL(env, napi_create_string_utf8(env, filenameWAV, -1, &resultTrue));
  return resultTrue;
}

napi_value HCADecoder::printInfo(napi_env env, napi_callback_info info) {
  napi_value _this;
  size_t argc = 1;
  napi_value args[1];
  NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, nullptr));

  char filenameHCA[MAX_PATH];
  napi_valuetype valuetype0;
  NAPI_CALL(env, napi_typeof(env, args[0], &valuetype0));
  NAPI_ASSERT(env, valuetype0 == napi_string, "HCA file name must be a string.");
  NAPI_CALL(env, napi_get_value_string_utf8(env, args[0], filenameHCA, MAX_PATH, nullptr));

  HCADecoder* dec;
  NAPI_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&dec)));

  clHCA hca(dec->key1, dec->key2);
  hca.PrintInfo(filenameHCA);
  return nullptr;
}

napi_value HCADecoder::decrypt(napi_env env, napi_callback_info info) {
  napi_value _this;
  size_t argc = 1;
  napi_value args[1];
  NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, nullptr));

  char filenameHCA[MAX_PATH];
  napi_valuetype valuetype0;
  NAPI_CALL(env, napi_typeof(env, args[0], &valuetype0));
  NAPI_ASSERT(env, valuetype0 == napi_string, "HCA file name must be a string.");
  NAPI_CALL(env, napi_get_value_string_utf8(env, args[0], filenameHCA, MAX_PATH, nullptr));

  HCADecoder* dec;
  NAPI_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&dec)));

  clHCA hca(dec->key1, dec->key2);
  if (!hca.Decrypt(filenameHCA)) {
    napi_value resultFalse;
    NAPI_CALL(env, napi_get_boolean(env, false, &resultFalse));
    return resultFalse;
  }
  napi_value resultTrue;
  NAPI_CALL(env, napi_create_string_utf8(env, filenameHCA, -1, &resultTrue));
  return resultTrue;
}

void HCADecoder::destructor(napi_env env, void* nativeObject, void* finalizeHint) {
  HCADecoder* dec = static_cast<HCADecoder*>(nativeObject);
  delete dec;
}
