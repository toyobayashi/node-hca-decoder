#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#endif

#include <napi.h>
#include "HCADecodeService.h"
#include <stdio.h>

class HCADecoder : public Napi::ObjectWrap<HCADecoder> {
public:
  static Napi::Object init(Napi::Env env, Napi::Object exports);
  HCADecoder(const Napi::CallbackInfo &info);
  virtual ~HCADecoder();

  HCADecodeService* hca();

private:
  static Napi::FunctionReference _constructor;
  HCADecodeService* _hca;
  
  unsigned int ciphKey1;
  unsigned int ciphKey2;

  Napi::Value _decodeToWaveFileSync(const Napi::CallbackInfo &info);
  // Napi::Value _decrypt(const Napi::CallbackInfo &info);
  Napi::Value _printInfo(const Napi::CallbackInfo &info);
  Napi::Value _decodeToWaveFile(const Napi::CallbackInfo &info);
};

class HCAAsyncWorker : public Napi::AsyncWorker {
public:
  HCAAsyncWorker(
    HCADecodeService* hca,
    const std::string& hcaFile,
    const std::string& wav,
    unsigned int ciphKey1,
    unsigned int ciphKey2,
    double volume,
    int mode,
    int loop,
    const Napi::Function& callback
  );
  void Execute();
  void OnOK();
private:
  HCADecodeService* _hca;
  std::string _hcaFile;
  std::string _wavFile;
  unsigned int _ciphKey1;
  unsigned int _ciphKey2;
  double _volume;
  int _mode;
  int _loop;
  bool _res;
};

HCAAsyncWorker::HCAAsyncWorker(
  HCADecodeService* hca,
  const std::string& hcaFile,
  const std::string& wav,
  unsigned int ciphKey1,
  unsigned int ciphKey2,
  double volume,
  int mode,
  int loop,
  const Napi::Function& callback
): Napi::AsyncWorker(callback) {
  _hca = hca;
  _hcaFile = hcaFile;
  _wavFile = wav;
  _ciphKey1 = ciphKey1;
  _ciphKey2 = ciphKey2;
  _volume = volume;
  _mode = mode;
  _loop = loop;
  _res = false;
}

void HCAAsyncWorker::Execute() {
  auto r = _hca->decode(_hcaFile.c_str(), 0, _ciphKey1, _ciphKey2, 0, _volume, _mode, _loop);
  if (!r.first) return;
  FILE* w;

#ifdef _WIN32
  wchar_t strUnicode[260];
  MultiByteToWideChar(CP_UTF8, 0, _wavFile.c_str(), -1, strUnicode, 260);
  w = _wfopen(strUnicode, L"wb");
#else
  w = fopen(_wavFile.c_str(), "wb");
#endif
  if (!w) {
    delete r.first;
    return;
  } 
  fwrite(r.first, 1, r.second, w);
  fclose(w);
  delete r.first;
  _res = true;
}

void HCAAsyncWorker::OnOK() {
  Napi::Env env = Env();
  if (_res) {
    if (!Callback().IsEmpty()) {
      Callback().Call({ env.Null(), Napi::String::New(env, _wavFile) });
    }
  } else {
    if (!Callback().IsEmpty()) {
      Callback().Call({ Napi::Error::New(env, _hcaFile + " decode failed.").Value(), Napi::String::New(env, "") });
    }
  }
}

Napi::FunctionReference HCADecoder::_constructor;

Napi::Object HCADecoder::init(Napi::Env env, Napi::Object exports) {
  // This method is used to hook the accessor and method callbacks
  Napi::Function classConstructor = DefineClass(env, "HCADecoder", {
    InstanceMethod("printInfo", &HCADecoder::_printInfo),
    // InstanceMethod("decrypt", &HCADecoder::_decrypt),
    InstanceMethod("decodeToWaveFile", &HCADecoder::_decodeToWaveFile),
    InstanceMethod("decodeToWaveFileSync", &HCADecoder::_decodeToWaveFileSync)
  });

  _constructor = Napi::Persistent(classConstructor);

  _constructor.SuppressDestruct();
  exports.Set("HCADecoder", classConstructor);
  return exports;
}

HCADecoder::HCADecoder(const Napi::CallbackInfo &info) : Napi::ObjectWrap<HCADecoder>(info) {
  _hca = nullptr;
  Napi::Env env = info.Env();
  ciphKey1 = 0xF27E3B22;
  ciphKey2 = 0x00003657;
  size_t argc = info.Length();

  if (argc < 1) {
    _hca = new HCADecodeService(0);
  } else if (argc < 2) {
    if (info[0].IsNumber()) {
      ciphKey1 = info[0].As<Napi::Number>().Uint32Value();
    }
    _hca = new HCADecodeService(0);
  } else {
    if (info[0].IsNumber()) {
      ciphKey1 = info[0].As<Napi::Number>().Uint32Value();
    }

    if (info[1].IsNumber()) {
      ciphKey2 = info[1].As<Napi::Number>().Uint32Value();
    }
    _hca = new HCADecodeService(0);
  }
}

HCADecoder::~HCADecoder() {
  if (_hca) {
    delete _hca;
    _hca = nullptr;
  }
}

HCADecodeService* HCADecoder::hca() {
  return _hca;
}

Napi::Value HCADecoder::_printInfo(const Napi::CallbackInfo &info){
  Napi::Env env = info.Env();
  if (info.Length() < 1) {
    Napi::Error::New(env, "HCADecoder::printInfo(): arguments.length < 1").ThrowAsJavaScriptException();
    return Napi::Boolean::New(env, false);
  }

  if (info[0].IsString()) {
    return Napi::Boolean::New(env, HCADecodeService::print_info(info[0].As<Napi::String>().Utf8Value().c_str(), ciphKey1, ciphKey2));
  }

  return Napi::Boolean::New(env, false);
}

// Napi::Value HCADecoder::_decrypt(const Napi::CallbackInfo &info){
//   Napi::Env env = info.Env();
//   if (info.Length() < 1) {
//     Napi::Error::New(env, "HCADecoder::decrypt(): arguments.length < 1").ThrowAsJavaScriptException();
//     return Napi::Boolean::New(env, false);
//   }

//   if (info[0].IsString() && _hca) {
//     const char* hcaPath = info[0].As<Napi::String>().Utf8Value().c_str();
//     bool res = clHCA(ciphKey1, ciphKey2).Decrypt(hcaPath);
//     return Napi::Boolean::New(env, res);
//   }

//   return Napi::Boolean::New(env, false);
// }

static Napi::Value noop(const Napi::CallbackInfo &info) { return info.Env().Undefined(); }

Napi::Value HCADecoder::_decodeToWaveFile(const Napi::CallbackInfo &info){
  Napi::Env env = info.Env();
  size_t argc = info.Length();
  if (argc < 1) {
    Napi::Error::New(env, "HCADecoder::decodeToWaveFile(): arguments.length < 1").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  if (!info[0].IsString()) {
    Napi::Error::New(env, "HCADecoder::decodeToWaveFile(): typeof arguments[0] !== 'string'").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  std::string hca = "";
  std::string wav = "";
  double volumn = 1;
  int mode = 16;
  int loop = 0;
  Napi::Function callback;

  int i = 0;

  for (i = 0; i < argc; i++) {
    if (info[i].IsFunction()) {
      callback = info[i].As<Napi::Function>();
      break;
    } else {
      switch (i) {
        case 0:
          hca = info[i].As<Napi::String>().Utf8Value();
          wav = hca.find_last_of(".") == std::string::npos ? (hca + ".wav") : (hca.substr(0, hca.find_last_of(".")) + ".wav");
          break;
        case 1:
          if (info[i].IsString()) wav = info[i].As<Napi::String>().Utf8Value();
          break;
        case 2:
          if (info[i].IsNumber()) volumn = info[i].As<Napi::Number>().DoubleValue();
          break;
        case 3:
          if (info[i].IsNumber()) mode = info[i].As<Napi::Number>().Int32Value();
          break;
        case 4:
          if (info[i].IsNumber()) loop = info[i].As<Napi::Number>().Int32Value();
          break;
        default:
          break;
      }
    }
  }
  if (callback.IsEmpty()) {
    callback = Napi::Function::New(env, noop);
  }

  HCAAsyncWorker* worker = new HCAAsyncWorker(_hca, hca, wav, ciphKey1, ciphKey2, volumn, mode, loop, callback);
  worker->Queue();

  return env.Undefined();
}

Napi::Value HCADecoder::_decodeToWaveFileSync(const Napi::CallbackInfo &info){
  Napi::Env env = info.Env();
  size_t argc = info.Length();
  if (argc < 1) {
    Napi::Error::New(env, "HCADecoder::decodeToWaveFileSync(): arguments.length < 1").ThrowAsJavaScriptException();
    return Napi::String::New(env, "");
  }

  if (!info[0].IsString()) {
    Napi::Error::New(env, "HCADecoder::decodeToWaveFileSync(): typeof arguments[0] !== 'string'").ThrowAsJavaScriptException();
    return Napi::String::New(env, "");
  }

  std::string hca = "";
  std::string wav = "";
  double volumn = 1;
  int mode = 16;
  int loop = 0;

  int i = 0;

  for (i = 0; i < argc; i++) {
    switch (i) {
      case 0:
        hca = info[i].As<Napi::String>().Utf8Value();
        wav = hca.find_last_of(".") == std::string::npos ? (hca + ".wav") : (hca.substr(0, hca.find_last_of(".")) + ".wav");
        break;
      case 1:
        if (info[i].IsString()) wav = info[i].As<Napi::String>().Utf8Value();
        break;
      case 2:
        if (info[i].IsNumber()) volumn = info[i].As<Napi::Number>().DoubleValue();
        break;
      case 3:
        if (info[i].IsNumber()) mode = info[i].As<Napi::Number>().Int32Value();
        break;
      case 4:
        if (info[i].IsNumber()) loop = info[i].As<Napi::Number>().Int32Value();
        break;
      default:
        break;
    }
  }

  auto res = _hca->decode(hca.c_str(), 0, ciphKey1, ciphKey2, 0, volumn, mode, loop);
  if (!res.first) {
    Napi::Error::New(env, hca + " decode failed.").ThrowAsJavaScriptException();
    return Napi::String::New(env, "");
  }
  FILE* w;
#ifdef _WIN32
  wchar_t strUnicode[260];
  MultiByteToWideChar(CP_UTF8, 0, wav.c_str(), -1, strUnicode, 260);
  w = _wfopen(strUnicode, L"wb");
#else
  w = fopen(wav.c_str(), "wb");
#endif
  if (!w) {
    delete res.first;
    return Napi::String::New(env, "");
  } 
  fwrite(res.first, 1, res.second, w);
  fclose(w);
  delete res.first;
  return Napi::String::New(env, wav);
}

static Napi::Object _index (Napi::Env env, Napi::Object exports) {
  HCADecoder::init(env, exports);
  return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, _index)
