#include <napi.h>
#include "clHCA.h"
#include <stdio.h>

class HCADecoder : public Napi::ObjectWrap<HCADecoder> {
public:
  static Napi::Object init(Napi::Env env, Napi::Object exports);
  static Napi::Value _getInfo(const Napi::CallbackInfo &info);
  HCADecoder(const Napi::CallbackInfo &info);
  virtual ~HCADecoder();
private:
  clHCA* _hca;

  unsigned int ciphKey1;
  unsigned int ciphKey2;

  Napi::Value _decodeToWaveFileSync(const Napi::CallbackInfo &info);
  // Napi::Value _decrypt(const Napi::CallbackInfo &info);
  Napi::Value _printInfo(const Napi::CallbackInfo &info);
  Napi::Value _decodeToWaveFile(const Napi::CallbackInfo &info);
  Napi::Value _decodeToMemory(const Napi::CallbackInfo &info);
  Napi::Value _decodeToMemorySync(const Napi::CallbackInfo &info);
};

class HCAAsyncWorker : public Napi::AsyncWorker {
public:
  HCAAsyncWorker(unsigned int, unsigned int, const std::string&, const std::string&, double, int, int, const Napi::Function&);
  void Execute();
  void OnOK();
private:
  clHCA _hca;
  std::string _hcaFile;
  std::string _wavFile;
  double _volumn;
  int _mode;
  int _loop;
  bool _res;
  std::vector<uint8_t>* _buf;
};

void BufferFinalizer(Napi::Env env, void* data, void* hint) {
  std::vector<uint8_t>* buf = static_cast<std::vector<uint8_t>*>(hint);
  delete buf;
}

HCAAsyncWorker::HCAAsyncWorker(unsigned int ciphKey1, unsigned int ciphKey2, const std::string& hcaFile, const std::string& wav, double volumn, int mode, int loop, const Napi::Function& callback)
  : Napi::AsyncWorker(callback),
    _hca(ciphKey1, ciphKey2),
    _hcaFile(hcaFile),
    _wavFile(wav),
    _volumn(volumn),
    _mode(mode),
    _loop(loop),
    _res(false),
    _buf(nullptr) {}

void HCAAsyncWorker::Execute() {
  if (_wavFile != "") {
    _res = _hca.DecodeToWavefile(_hcaFile.c_str(), _wavFile.c_str(), _volumn, _mode, _loop);
  } else {
    HCAFileStream stream = HCAFileStream::Open();
    _res = _hca.DecodeToMemory(_hcaFile.c_str(), stream, _volumn, _mode, _loop);
    if (_res) {
      _buf = stream.Release();
    }
  }
}

void HCAAsyncWorker::OnOK() {
  Napi::Env env = Env();
  if (_res) {
    if (!Callback().IsEmpty()) {
      if (_wavFile != "") {
        Callback().Call({ env.Null(), Napi::String::New(env, _wavFile) });
      } else {
        if (_buf) {
          Callback().Call({ env.Null(), Napi::Buffer<uint8_t>::New(env, _buf->data(), _buf->size(), BufferFinalizer, _buf) });
        } else {
          Callback().Call({ Napi::Error::New(env, _hcaFile + " decode failed.").Value(), env.Null() });
        }
      }
    }
  } else {
    if (!Callback().IsEmpty()) {
      Callback().Call({ Napi::Error::New(env, _hcaFile + " decode failed.").Value(), Napi::String::New(env, "") });
    }
  }
}

Napi::Object HCADecoder::init(Napi::Env env, Napi::Object exports) {
  // This method is used to hook the accessor and method callbacks
  Napi::Function classConstructor = DefineClass(env, "HCADecoder", {
    StaticMethod("getInfo", HCADecoder::_getInfo),
    InstanceMethod("printInfo", &HCADecoder::_printInfo),
    // InstanceMethod("decrypt", &HCADecoder::_decrypt),
    InstanceMethod("decodeToWaveFile", &HCADecoder::_decodeToWaveFile),
    InstanceMethod("decodeToWaveFileSync", &HCADecoder::_decodeToWaveFileSync),
    InstanceMethod("decodeToMemory", &HCADecoder::_decodeToMemory),
    InstanceMethod("decodeToMemorySync", &HCADecoder::_decodeToMemorySync)
  });

  Napi::FunctionReference* constructor = new Napi::FunctionReference();

  *constructor = Napi::Persistent(classConstructor);
  exports.Set("HCADecoder", classConstructor);
  env.SetInstanceData<Napi::FunctionReference>(constructor);
  return exports;
}

HCADecoder::HCADecoder(const Napi::CallbackInfo &info)
  : Napi::ObjectWrap<HCADecoder>(info), _hca(nullptr), ciphKey1(0xF27E3B22), ciphKey2(0x00003657)
{
  Napi::Env env = info.Env();
  size_t argc = info.Length();

  if (argc < 1) {
    _hca = new clHCA(ciphKey1, ciphKey2);
  } else if (argc < 2) {
    if (info[0].IsNumber()) {
      ciphKey1 = info[0].As<Napi::Number>().Uint32Value();
    }
    _hca = new clHCA(ciphKey1, ciphKey2);
  } else {
    if (info[0].IsNumber()) {
      ciphKey1 = info[0].As<Napi::Number>().Uint32Value();
    }

    if (info[1].IsNumber()) {
      ciphKey2 = info[1].As<Napi::Number>().Uint32Value();
    }
    _hca = new clHCA(ciphKey1, ciphKey2);
  }
}

HCADecoder::~HCADecoder() {
  if (_hca != nullptr) {
    delete _hca;
    _hca = nullptr;
  }
}

Napi::Value HCADecoder::_printInfo(const Napi::CallbackInfo &info){
  Napi::Env env = info.Env();
  if (info.Length() < 1) {
    Napi::Error::New(env, "HCADecoder::printInfo(): arguments.length < 1").ThrowAsJavaScriptException();
    return Napi::Boolean::New(env, false);
  }

  if (info[0].IsString() && _hca) {
    return Napi::Boolean::New(env, _hca->PrintInfo(info[0].As<Napi::String>().Utf8Value().c_str()));
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

  HCAAsyncWorker* worker = new HCAAsyncWorker(ciphKey1, ciphKey2, hca, wav, volumn, mode, loop, callback);
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

  bool res = _hca->DecodeToWavefile(hca.c_str(), wav.c_str(), volumn, mode, loop);
  if (!res) {
    Napi::Error::New(env, hca + " decode failed.").ThrowAsJavaScriptException();
    return Napi::String::New(env, "");
  }

  return Napi::String::New(env, wav);
}

Napi::Value HCADecoder::_decodeToMemory(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  size_t argc = info.Length();
  if (argc < 1) {
    Napi::Error::New(env, "HCADecoder::decodeToMemory(): arguments.length < 1").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  if (!info[0].IsString()) {
    Napi::Error::New(env, "HCADecoder::decodeToMemory(): typeof arguments[0] !== 'string'").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  std::string hca = "";
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
          break;
        case 1:
          if (info[i].IsNumber()) volumn = info[i].As<Napi::Number>().DoubleValue();
          break;
        case 2:
          if (info[i].IsNumber()) mode = info[i].As<Napi::Number>().Int32Value();
          break;
        case 3:
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

  HCAAsyncWorker* worker = new HCAAsyncWorker(ciphKey1, ciphKey2, hca, "", volumn, mode, loop, callback);
  worker->Queue();

  return env.Undefined();
}

Napi::Value HCADecoder::_decodeToMemorySync(const Napi::CallbackInfo &info){
  Napi::Env env = info.Env();
  size_t argc = info.Length();
  if (argc < 1) {
    Napi::Error::New(env, "HCADecoder::decodeToMemorySync(): arguments.length < 1").ThrowAsJavaScriptException();
    return Napi::String::New(env, "");
  }

  if (!info[0].IsString()) {
    Napi::Error::New(env, "HCADecoder::decodeToMemorySync(): typeof arguments[0] !== 'string'").ThrowAsJavaScriptException();
    return Napi::String::New(env, "");
  }

  std::string hca = "";
  double volumn = 1;
  int mode = 16;
  int loop = 0;

  int i = 0;

  for (i = 0; i < argc; i++) {
    switch (i) {
      case 0:
        hca = info[i].As<Napi::String>().Utf8Value();
        break;
      case 1:
        if (info[i].IsNumber()) volumn = info[i].As<Napi::Number>().DoubleValue();
        break;
      case 2:
        if (info[i].IsNumber()) mode = info[i].As<Napi::Number>().Int32Value();
        break;
      case 3:
        if (info[i].IsNumber()) loop = info[i].As<Napi::Number>().Int32Value();
        break;
      default:
        break;
    }
  }
  HCAFileStream stream = HCAFileStream::Open();
  bool res = _hca->DecodeToMemory(hca.c_str(), stream, volumn, mode, loop);
  if (!res) {
    Napi::Error::New(env, hca + " decode failed.").ThrowAsJavaScriptException();
    return Napi::String::New(env, "");
  }

  std::vector<uint8_t>* buf = stream.Release();

  return Napi::Buffer<uint8_t>::New(env, buf->data(), buf->size(), BufferFinalizer, buf);
}

Napi::Value HCADecoder::_getInfo(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  if (info.Length() < 1) {
    Napi::Error::New(env, "HCADecoder::getInfo(): arguments.length < 1").ThrowAsJavaScriptException();
    return Napi::Value();
  }

  if (!info[0].IsString()) {
    Napi::Error::New(env, "HCADecoder::getInfo(): typeof arguments[0] !== 'string'").ThrowAsJavaScriptException();
    return Napi::Value();
  }

  std::string hcafile = info[0].As<Napi::String>().Utf8Value();
  HCAInfo hcainfo;
  int r = clHCA::GetInfo(hcafile.c_str(), &hcainfo);
  if (r != 0) {
    Napi::Error::New(env, getInfoErrorMessage(r)).ThrowAsJavaScriptException();
    return Napi::Value();
  }

  Napi::Object ret = Napi::Object::New(env);
  std::string ver = std::to_string(hcainfo.versionMajor);
  ver += ".";
  ver += std::to_string(hcainfo.versionMinor);
  ret["version"] = Napi::String::New(env, ver);
  ret["channelCount"] = Napi::Number::New(env, hcainfo.channelCount);
  ret["samplingRate"] = Napi::Number::New(env, hcainfo.samplingRate);
  ret["blockCount"] = Napi::Number::New(env, hcainfo.blockCount);
  ret["muteHeader"] = Napi::Number::New(env, hcainfo.muteHeader);
  ret["muteFooter"] = Napi::Number::New(env, hcainfo.muteFooter);

  if (hcainfo.comp) {
    Napi::Object comp = Napi::Object::New(env);
    ret["bitRate"] = Napi::Number::New(env, hcainfo.bitRate);
    ret["blockSize"] = Napi::Number::New(env, hcainfo.blockSize);
    comp["comp1"] = Napi::Number::New(env, hcainfo.comp1);
    comp["comp2"] = Napi::Number::New(env, hcainfo.comp2);
    comp["comp3"] = Napi::Number::New(env, hcainfo.comp3);
    comp["comp4"] = Napi::Number::New(env, hcainfo.comp4);
    comp["comp5"] = Napi::Number::New(env, hcainfo.comp5);
    comp["comp6"] = Napi::Number::New(env, hcainfo.comp6);
    comp["comp7"] = Napi::Number::New(env, hcainfo.comp7);
    comp["comp8"] = Napi::Number::New(env, hcainfo.comp8);
    ret["comp"] = comp;
  }
  if (hcainfo.dec) {
    Napi::Object dec = Napi::Object::New(env);
    ret["bitRate"] = Napi::Number::New(env, hcainfo.bitRate);
    ret["blockSize"] = Napi::Number::New(env, hcainfo.blockSize);
    dec["dec1"] = Napi::Number::New(env, hcainfo.dec1);
    dec["dec2"] = Napi::Number::New(env, hcainfo.dec2);
    dec["dec3"] = Napi::Number::New(env, hcainfo.dec3);
    dec["dec4"] = Napi::Number::New(env, hcainfo.dec4);
    dec["dec5"] = Napi::Number::New(env, hcainfo.dec5);
    dec["dec6"] = Napi::Number::New(env, hcainfo.dec6);
    dec["dec7"] = Napi::Number::New(env, hcainfo.dec7);
    ret["dec"] = dec;
  }
  if (hcainfo.vbr) {
    Napi::Object vbr = Napi::Object::New(env);
    vbr["vbr1"] = Napi::Number::New(env, hcainfo.vbr1);
    vbr["vbr2"] = Napi::Number::New(env, hcainfo.vbr2);
    ret["vbr"] = vbr;
  }
  ret["athType"] = Napi::Number::New(env, hcainfo.athType);
  if (hcainfo.loop) {
    Napi::Object loop = Napi::Object::New(env);
    loop["start"] = Napi::Number::New(env, hcainfo.loopStart);
    loop["end"] = Napi::Number::New(env, hcainfo.loopEnd);
    loop["count"] = Napi::Number::New(env, hcainfo.loopCount);
    loop["loop1"] = Napi::Number::New(env, hcainfo.loop1);
    loop["infinite"] = Napi::Boolean::New(env, hcainfo.loopCount == 0x80);
    ret["loop"] = loop;
  }
  ret["ciphType"] = Napi::Number::New(env, hcainfo.ciphType);
  ret["rvaVolume"] = Napi::Number::New(env, hcainfo.rvaVolume);

  return ret;
}

static Napi::Object _index (Napi::Env env, Napi::Object exports) {
  HCADecoder::init(env, exports);
  return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, _index)
