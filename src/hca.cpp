#include "./hca.h"
#include "./clHCA.h"
#include <stdio.h>
#include <string.h>

HCADecoder::HCADecoder(unsigned int k1, unsigned int k2): key1(k1), key2(k2), env_(nullptr), wrapper_(nullptr) {}

HCADecoder::~HCADecoder() { napi_delete_reference(env_, wrapper_); }

napi_ref HCADecoder::constructorRef;

void HCADecoder::init(napi_env env, napi_value exports) {
  napi_property_descriptor properties[] = {
    DECLARE_NAPI_PROPERTY("decodeToWaveFileSync", decodeToWaveFileSync),
    DECLARE_NAPI_PROPERTY("decodeToWaveFile", decodeToWaveFile),
    DECLARE_NAPI_PROPERTY("printInfo", printInfo),
    DECLARE_NAPI_PROPERTY("decrypt", decrypt)
  };
  napi_value jsClassHCADecoder;
  NAPI_CALL_RETURN_VOID(env, napi_define_class(
    env, "HCADecoder", -1, constructor, nullptr, 4, properties, &jsClassHCADecoder));

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
    napi_value resultUndefined;
    NAPI_CALL(env, napi_get_undefined(env, &resultUndefined));
    return resultUndefined;
  }

  napi_value wav;
  NAPI_CALL(env, napi_create_string_utf8(env, filenameWAV, -1, &wav));
  return wav;
}

napi_value HCADecoder::decodeToWaveFile(napi_env env, napi_callback_info info) {
  // int n = -1;
  napi_value _this;
  size_t argc = 6;
  napi_value args[6];
  NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, nullptr));

  AsyncDec* req = new AsyncDec;
  req->callbackIndex = -1;

  napi_valuetype valuetype0;
  NAPI_CALL(env, napi_typeof(env, args[0], &valuetype0));
  NAPI_ASSERT(env, valuetype0 == napi_string, "HCA file name must be a string.");
  NAPI_CALL(env, napi_get_value_string_utf8(env, args[0], req->filenameHCA, MAX_PATH, nullptr));

  for (int i = 1; i < 6; i++) {
    napi_valuetype t;
    NAPI_CALL(env, napi_typeof(env, args[i], &t));
    if (t == napi_function) {
      req->callbackIndex = i;
      break;
    }
  }

  req->volume = 1;
  req->mode = 16;
  req->loop = 0;
  napi_valuetype valuetype;

  for (int i = 1; i < (req->callbackIndex == -1 ? 5 : req->callbackIndex + 1); i++) {
    switch (i) {
      case 1:
        NAPI_CALL(env, napi_typeof(env, args[i], &valuetype));
        if (valuetype != napi_string) {
          strcpy(req->filenameWAV, req->filenameHCA);
          char *d1 = strrchr(req->filenameWAV, '\\');
          char *d2 = strrchr(req->filenameWAV, '/');
          char *e = strrchr(req->filenameWAV, '.');
          if (e && d1 < e && d2 < e) *e = '\0';
          strcat(req->filenameWAV, ".wav");
        } else {
          NAPI_CALL(env, napi_get_value_string_utf8(env, args[i], req->filenameWAV, MAX_PATH, nullptr));
        }
        break;
      case 2:
        NAPI_CALL(env, napi_typeof(env, args[i], &valuetype));
        if (valuetype == napi_number) {
          NAPI_CALL(env, napi_get_value_double(env, args[i], &req->volume));
        }
        break;
      case 3:
        NAPI_CALL(env, napi_typeof(env, args[i], &valuetype));
        if (valuetype == napi_number) {
          NAPI_CALL(env, napi_get_value_int32(env, args[i], &req->mode));
        }
        break;
      case 4:
        NAPI_CALL(env, napi_typeof(env, args[i], &valuetype));
        if (valuetype == napi_number) {
          NAPI_CALL(env, napi_get_value_int32(env, args[i], &req->loop));
        }
        break;
      default:
        break;
    }
  }
  
  if (req->callbackIndex != -1) {
    NAPI_CALL(env, napi_create_reference(env, args[req->callbackIndex], 1, &req->_cbref));
  }

  HCADecoder* dec;
  NAPI_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&dec)));
  req->key1 = dec->key1;
  req->key2 = dec->key2;

  napi_value resourceName;
  NAPI_CALL(env, napi_create_string_utf8(env, "decodeToWaveFile", -1, &resourceName));
  NAPI_CALL(env, napi_create_async_work(env, nullptr, resourceName, [](napi_env env, void* data) {
    AsyncDec* r = static_cast<AsyncDec*>(data);
    clHCA hca(r->key1, r->key2);
    r->status = hca.DecodeToWavefile(r->filenameHCA, r->filenameWAV, (float)r->volume, r->mode, r->loop);
  }, [](napi_env env, napi_status status, void* data) {
    AsyncDec* r = static_cast<AsyncDec*>(data);
    if (status != napi_ok) {
      printf("Decode failed.");
      return;
    }

    if (r->callbackIndex != -1) {
      napi_value callback;
      NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, r->_cbref, &callback));

      napi_value argv[2];
      if (r->status) {
        NAPI_CALL_RETURN_VOID(env, napi_get_null(env, &argv[0]));
        NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, r->filenameWAV, -1, &argv[1]));
      } else {
        char msg[MAX_PATH];
        strcpy(msg, r->filenameHCA);
        strcat(msg, " decode failed.");
        NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, msg, -1, &argv[0]));
        NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &argv[1]));
      }
      napi_value global;
      NAPI_CALL_RETURN_VOID(env, napi_get_global(env, &global));
      NAPI_CALL_RETURN_VOID(env, napi_call_function(env, global, callback, 2, argv, nullptr));
      NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, r->_cbref));
    }
    
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, r->_request));
    delete r;
  }, req, &req->_request));
  
  NAPI_CALL(env, napi_queue_async_work(env, req->_request));
  return nullptr;
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
    napi_value resultUndefined;
    NAPI_CALL(env, napi_get_undefined(env, &resultUndefined));
    return resultUndefined;
  }
  napi_value hcaPath;
  NAPI_CALL(env, napi_create_string_utf8(env, filenameHCA, -1, &hcaPath));
  return hcaPath;
}

void HCADecoder::destructor(napi_env env, void* nativeObject, void* finalizeHint) {
  HCADecoder* dec = static_cast<HCADecoder*>(nativeObject);
  delete dec;
}
