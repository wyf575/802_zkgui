/*
 * FileUtils.h
 *
 *  Created on: 2021年1月4日
 *      Author: Administrator
 */

#ifndef JNI_BASELIB_FILEUTILS_H_
#define JNI_BASELIB_FILEUTILS_H_

#include <string>

class FileUtils{
public:
	static FileUtils* getInstance();
	std::string getDefPath();
	bool isFileExists_stat(std::string& name);
	bool isDirExists(std::string folderPath);
	int getFileSize(std::string filePath);
	std::string getAppUpgardePath();
	std::string getMCUUpgardePath();
private:
	FileUtils();
};


#define FILEUTILS 	FileUtils::getInstance()
#endif /* JNI_BASELIB_FILEUTILS_H_ */
