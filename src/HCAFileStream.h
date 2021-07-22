#ifndef HCADECODER_SRC_HCA_FILE_STREAM_H
#define HCADECODER_SRC_HCA_FILE_STREAM_H

#include <cstdio>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <memory>

class HCAFileStream {
public:
  virtual ~HCAFileStream() noexcept;
  HCAFileStream(FILE* fp);
  HCAFileStream(const HCAFileStream&) = delete;
  HCAFileStream(HCAFileStream&&);
  HCAFileStream& operator=(const HCAFileStream&) = delete;
  HCAFileStream& operator=(HCAFileStream&&);

  explicit operator bool() const noexcept;

  int Close();
  bool Ok() const noexcept;
  int Puts(const char* buffer);
  std::vector<uint8_t>* Release();
  int Seek(long offset, int origin);
  long Tell() const;
  size_t Write(const void *buf, size_t elsize, size_t elcount);

  static HCAFileStream Open();
  static HCAFileStream Open(void* buffer, size_t size);
  static HCAFileStream Open(const char* filepath, const char* flag, unsigned int windows_cp = 65001U);
  static FILE* OpenFile(const char* filepath, const char* flag, unsigned int windows_cp = 65001U);
private:
  std::unique_ptr<std::vector<uint8_t>> buffer_;
  FILE* fp_;
  long offset_;

  HCAFileStream();
  HCAFileStream(void* buffer, size_t size);
};

#endif
