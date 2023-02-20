/*
 * Base64Utils.h
 *
 *  Created on: 2020年12月31日
 *      Author: Administrator
 */

#ifndef JNI_BASELIB_BASE64UTILS_H_
#define JNI_BASELIB_BASE64UTILS_H_

#include <string>

/**
 * Base64 编码/解码
 * @author liruixing
 */
class Base64Utils{

private:
    Base64Utils();
    int find_int(char ch);
    char a_to_c(int x);
public:
    /**
     * 这里必须是unsigned类型，否则编码中文的时候出错
     */
    std::string base64_encode(std::string str);
    std::string base64_decode(std::string str);
    static Base64Utils* getInstance();


};

#define BASE64UTILS 	Base64Utils::getInstance()
#endif /* JNI_BASELIB_BASE64UTILS_H_ */
