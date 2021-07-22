#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#include <string>
#endif

#include <cstring>
#include <algorithm>
#include <utility>
#include "HCAFileStream.h"

HCAFileStream::~HCAFileStream() noexcept {
  if (fp_) {
    int r = ::fclose(fp_);
    if (r == 0) fp_ = nullptr;
  }
}

HCAFileStream::HCAFileStream()
  :buffer_(new std::vector<uint8_t>()), fp_(nullptr), offset_(0) {}

HCAFileStream::HCAFileStream(FILE* fp)
  :buffer_(nullptr), fp_(fp), offset_(0) {}

HCAFileStream::HCAFileStream(void* buffer, size_t size)
  :buffer_(new std::vector<uint8_t>(static_cast<uint8_t*>(buffer), static_cast<uint8_t*>(buffer) + size)), fp_(nullptr), offset_(0) {}

HCAFileStream::HCAFileStream(HCAFileStream&& other)
  :buffer_(std::move(other.buffer_)), fp_(other.fp_), offset_(other.offset_)
{
  other.buffer_ = nullptr;
  other.fp_ = nullptr;
  other.offset_ = 0;
}

HCAFileStream& HCAFileStream::operator=(HCAFileStream&& other) {
  if (fp_) {
    ::fclose(fp_);
  }
  buffer_ = std::move(other.buffer_);
  other.buffer_ = nullptr;
  fp_ = other.fp_;
  other.fp_ = nullptr;
  offset_ = other.offset_;
  other.offset_ = 0;
  return *this;
}

HCAFileStream::operator bool() const noexcept {
  return Ok();
}

int HCAFileStream::Close() {
  if (fp_) {
    int r = ::fclose(fp_);
    if (r == 0) fp_ = nullptr;
    return r;
  }
  if (!buffer_) return 0;
  
  buffer_.reset(nullptr);
  offset_ = 0;
  return 0;
}

bool HCAFileStream::Ok() const noexcept {
  return (fp_ != nullptr) || (buffer_ != nullptr);
}

int HCAFileStream::Puts(const char* buffer) {
  if (fp_) return ::fputs(buffer, fp_);
  if (!buffer_) return -1;
  
  size_t r = this->Write(buffer, 1, strlen(buffer));
  return r == 0 ? -1 : r;
}

std::vector<uint8_t>* HCAFileStream::Release() {
  if (fp_) return nullptr;
  offset_ = 0;
  return buffer_.release();
}

int HCAFileStream::Seek(long offset, int origin) {
  if (fp_) return ::fseek(fp_, offset, origin);
  if (!buffer_) return 1;

  if (origin == SEEK_CUR) {
    if ((offset_ + offset) > buffer_->size()) {
      return 1;
    }
    offset_ += offset;
    return 0;
  }
  if (origin == SEEK_SET) {
    size_t size = buffer_->size();
    offset_ = offset < 0 ? 0 : (offset > size ? size : offset);
    return 0;
  }
  if (origin == SEEK_END) {
    size_t size = buffer_->size();
    if ((size - offset) < 0) {
      return 1;
    }
    offset_ = size - offset;
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
  size_t target_pos = offset_ + byte_length;
  size_t buffer_size = buffer_->size();
  const uint8_t* buf_start = static_cast<const uint8_t*>(buf);
  if (target_pos > buffer_size) {
    if (offset_ == buffer_size) {
      buffer_->insert(buffer_->end(), buf_start, buf_start + byte_length);
    } else {
      size_t left = target_pos - buffer_size;
      std::copy(buf_start, buf_start + byte_length - left, buffer_->begin() + offset_);
      buffer_->insert(buffer_->end(), buf_start + byte_length - left, buf_start + byte_length);
    }
  } else {
    std::copy(buf_start, buf_start + byte_length, buffer_->begin() + offset_);
  }
  // ::memcpy(buffer_, buf, byte_length);
  offset_ += byte_length;
  return elcount;
}

HCAFileStream HCAFileStream::Open() {
  return HCAFileStream();
}

HCAFileStream HCAFileStream::Open(void* buffer, size_t size) {
  return HCAFileStream(buffer, size);
}

HCAFileStream HCAFileStream::Open(const char* filepath, const char* flag, unsigned int windows_cp) {
  return OpenFile(filepath, flag, windows_cp);
}

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
