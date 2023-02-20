/*
 * FileUtils.cpp
 *
 *  Created on: 2021年1月4日
 *      Author: Administrator
 */

#include "FileUtils.h"

#include <sys/stat.h>
#include <dirent.h>
#include <iostream>
#include <unistd.h>

using namespace std;

FileUtils::FileUtils(){

}

FileUtils* FileUtils::getInstance(){
	static FileUtils mInstance;
	return &mInstance;
}

std::string FileUtils::getDefPath(){
	string dir = "/dev/mmcblk0p1";
	bool result = isFileExists_stat(dir);
	if(!result){
		dir = "/mnt/download";
	}else{
		dir = "/mnt/sdcard/download";
	}
	result = isFileExists_stat(dir);
	if(!result){
	  int ret = mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	  if(ret == 0){
		  printf("创建 %s 文件夹成功\n", dir);
	  }else{
		  printf("创建 %s 文件夹失败\n", dir);
	  }
	}
	return dir;
}

string FileUtils::getAppUpgardePath(){
	return "../data/SStarOta.bin.gz";
}

string FileUtils::getMCUUpgardePath(){
	return "./ba407SS.bin";
}

bool FileUtils::isFileExists_stat(string& name) {
  struct stat buffer;
  bool result = (stat(name.c_str(), &buffer) == 0);
  return result;
}

int FileUtils::getFileSize(string filePath){
	struct stat buffer;
	stat(filePath.c_str(), &buffer);
	return buffer.st_size;
}

