#ifndef JNI_IO_IOUTIL_H_
#define JNI_IO_IOUTIL_H_

#include <string>

using std::string;

namespace ioutil {

/**
 * 写文件，如果文件存在，将会覆盖，如果文件不存在，则新建文件，再写入内容
 * 成功返回true
 */
bool WriteFile(const char* filename, const void* data, int len);

/**
 * 在文件末尾追加内容,如果文件不存在，则先新建文件，再写入内容
 * 成功返回true
 */
bool AppendFile(const char* filename, const void* data, int len);

/**
 * 读文件
 * 成功，将文件以二进制的形式保存在string的data中，以 string.data()读取二进制内容
 * 失败返回空串
 */
string ReadFile(const char* filename);

class Reader {
public:
  Reader();
  //打开文件，成功返回true
  bool Open(const char* filename);
  //读取文件, 成功，则返回读取的字节数; 失败，则返回负数; 读取结束返回0
  int Read(void* buffer, int len);
  //关闭文件
  void Close();
  virtual ~Reader();
private:
  FILE* file_;
};

class Writer {
public:
  Writer();
  /**
   * 打开文件,如果文件不存在，会自动创建
   * append:  是否是追加写文件，如果不是则会覆盖原文件
   * 操作成功返回true
   */
  bool Open(const char* filename, bool append);
  //写文件, 成功，则返回写的字节数; 失败，则返回负数
  int Write(const void* buffer, int len);
  //关闭文件
  void Close();
  virtual ~Writer();
private:
  FILE* file_;
};



} /* namespace ioutil */

#endif /* JNI_IO_IOUTIL_H_ */
