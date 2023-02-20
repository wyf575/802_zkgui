#include "ioutil.h"
#include <unistd.h>
#include <string>

namespace ioutil {

bool WriteFile(const char* filename, const void* data, int len) {
  Writer w;
  if (!w.Open(filename, false)) {
    return false;
  }
  return len == w.Write(data, len);
}

bool AppendFile(const char* filename, const void* data, int len) {
  Writer w;
  if (!w.Open(filename, true)) {
    return false;
  }
  return len == w.Write(data, len);
}

string ReadFile(const char* filename) {
  Reader r;
  if (!r.Open(filename)) {
    return "";
  }
  string raw;
  char buf[1024] = {0};
  while (true) {
    int n = r.Read(buf, (int)sizeof(buf));
    if (n > 0) {
      raw.append(buf, (string::size_type)n);
    } else if (n == 0) {
      break;
    } else {
      //出错
      return "";
    }
  }
  return raw;
}



bool Reader::Open(const char* filename) {
  file_ = fopen(filename, "rb");
  return file_ != NULL;
}

int Reader::Read(void* buffer, int len) {
  if (file_ == NULL) {
    return -1;
  }
  return fread(buffer, 1, len, file_);
}

void Reader::Close() {
  if (file_ != NULL) {
    fflush(file_);
    fclose(file_);
    file_ = NULL;
  }
}

Reader::Reader():file_(NULL) {
}

Reader::~Reader() {
  Close();
}

bool Writer::Open(const char* filename, bool append) {
  file_ = fopen(filename, append ? "ab" : "wb");
  return file_ != NULL;
}

int Writer::Write(const void* buffer, int len) {
  if (file_ == NULL) {
    return -1;
  }
  return fwrite(buffer, 1, len, file_);
}

void Writer::Close() {
  if (file_ != NULL) {
    fflush(file_);
    fclose(file_);
    sync();
    file_ = NULL;
  }
}

Writer::Writer():file_(NULL) {
}

Writer::~Writer() {
  Close();
}

} /* namespace ioutil */


