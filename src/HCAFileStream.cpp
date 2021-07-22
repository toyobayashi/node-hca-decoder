#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#include <string>
#endif

#include <cstring>
#include "HCAFileStream.h"

FILE* HCAFileStream::OpenFile(const char* filepath, const char* flag, unsigned int windows_cp) {
  FILE *fp;
#ifdef _WIN32
  int len = MultiByteToWideChar(windows_cp, 0, filepath, -1, nullptr, 0);
  if (len == 0) return nullptr;
  std::wstring path;
  path.resize(len - 1);
  MultiByteToWideChar(windows_cp, 0, filepath, -1, &path[0], len);

  len = MultiByteToWideChar(windows_cp, 0, flag, -1, nullptr, 0);
  if (len == 0) return nullptr;
  std::wstring flg;
  flg.resize(len - 1);
  MultiByteToWideChar(windows_cp, 0, flag, -1, &flg[0], len);

  fp = _wfopen(path.c_str(), flg.c_str());
#else
  fp = fopen(filepath, flag);
#endif
  return fp;
}

HCAFileStream::HCAFileStream(FILE* fp)
  :buffer_(nullptr), fp_(fp), size_(0), offset_(0) {}

HCAFileStream::HCAFileStream(void* buffer, size_t size)
  :buffer_(buffer), fp_(nullptr), size_(size), offset_(0) {}

int HCAFileStream::Close() {
  if (fp_) {
    int r = ::fclose(fp_);
    if (r == 0) fp_ = nullptr;
    return r;
  }
  if (!buffer_) return 0;
  
  delete[] buffer_;
  buffer_ = nullptr;
  size_ = 0;
  offset_ = 0;
  return 0;
}

bool HCAFileStream::Ok() const {
  return (fp_ != nullptr) || (buffer_ != nullptr && size_ > 0);
}

int HCAFileStream::Puts(const char* buffer) {
  if (fp_) return ::fputs(buffer, fp_);
  if (!buffer_) return -1;
  
  size_t r = this->Write(buffer, 1, strlen(buffer));
  return r == 0 ? -1 : r;
}

int HCAFileStream::Seek(long offset, int origin) {
  if (fp_) return ::fseek(fp_, offset, origin);
  if (!buffer_) return 1;

  if (origin == SEEK_CUR) {
    if ((offset_ + offset) > size_) {
      return 1;
    }
    offset_ += offset;
    return 0;
  }
  if (origin == SEEK_SET) {
    offset_ = offset < 0 ? 0 : (offset > size_ ? size_ : offset);
    return 0;
  }
  if (origin == SEEK_END) {
    if ((size_ - offset) < 0) {
      return 1;
    }
    offset_ = size_ - offset;
    return 0;
  }
  return 1;
}

long HCAFileStream::Tell() const {
  if (fp_) return ::ftell(fp_);
  return offset_;
}

size_t HCAFileStream::Write(const void *buf, size_t elsize, size_t elcount) {
  if (fp_) return ::fwrite(buf, elsize, elcount, fp_);
  if (!buffer_) return 0;

  size_t byte_length = elsize * elcount;
  if ((offset_ + byte_length) > size_) {
    return 0;
  }
  ::memcpy(buffer_, buf, byte_length);
  offset_ += byte_length;
  return elcount;
}

HCAFileStream HCAFileStream::Open(void* buffer, size_t size) {
  return HCAFileStream(buffer, size);
}

HCAFileStream HCAFileStream::Open(const char* filepath, const char* flag, unsigned int windows_cp) {
  return OpenFile(filepath, flag, windows_cp);
}
