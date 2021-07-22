#ifndef HCADECODER_SRC_HCA_FILE_STREAM_H
#define HCADECODER_SRC_HCA_FILE_STREAM_H

#include <cstdio>
#include <cstddef>

class HCAFileStream {
public:
  HCAFileStream(FILE* fp);
  HCAFileStream(const HCAFileStream&) = delete;
  HCAFileStream(HCAFileStream&&) = default;
  HCAFileStream& operator=(const HCAFileStream&) = delete;
  HCAFileStream& operator=(HCAFileStream&&) = default;

  int Close();
  bool Ok() const;
  int Puts(const char* buffer);
  int Seek(long offset, int origin);
  long Tell() const;
  size_t Write(const void *buf, size_t elsize, size_t elcount);

  static HCAFileStream Open(void* buffer, size_t size);
  static HCAFileStream Open(const char* filepath, const char* flag, unsigned int windows_cp = 65001U);
  static FILE* OpenFile(const char* filepath, const char* flag, unsigned int windows_cp = 65001U);
private:
  void* buffer_;
  FILE* fp_;
  size_t size_;
  long offset_;

  HCAFileStream(void* buffer, size_t size);
};

#endif
