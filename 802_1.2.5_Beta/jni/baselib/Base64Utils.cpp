
#include "Base64Utils.h"

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
using namespace std;

Base64Utils::Base64Utils(){
}

Base64Utils* Base64Utils::getInstance(){
	static Base64Utils mInstance;
	return &mInstance;
}
string Base64Utils::base64_encode(string str){
	string base64_table="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	int str_len = str.length();
	string res="";
	//注意这里str_len/3*3
	//通过字符的ascii二进制位运算组合成base64编码二进制
	for (int strp=0; strp<str_len/3*3; strp+=3){
		res+=base64_table[str[strp]>>2];
		res+=base64_table[(str[strp]&0x3)<<4 | (str[strp+1])>>4];
		res+=base64_table[(str[strp+1]&0xf)<<2 | (str[strp+2])>>6];
		res+=base64_table[(str[strp+2])&0x3f];
		//cout<<res<<endl;
	}
	if (str_len%3==1){
		int pos=str_len/3 * 3;
		res += base64_table[str[pos]>>2];
		res += base64_table[(str[pos]&0x3)<<4];
		res += "=";	res += "=";
	}else if (str_len%3==2){
		int pos=str_len/3 * 3;
		res += base64_table[str[pos]>>2];
		res += base64_table[(str[pos]&0x3)<<4 | (str[pos+1])>>4];
		res += base64_table[(str[pos+1]&0xf)<<2];
		res += "=";
	}
	return res;
}

char Base64Utils::a_to_c(int x){
//int转char
	char ch=x;
	return ch;
}

int Base64Utils::find_int(char ch){
//找base64表中字符对应的int值
//string base64_table="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	if (ch>='A' && ch<='Z')		return ch-'A'+0;
	else if (ch>='a' && ch<='z')	return ch-'a'+26;
	else if (ch>='0' && ch<='9')	return ch-'0'+52;
	else if (ch=='+')	return 62;
	else if (ch=='/')	return 63;
	else return 0;
}

string Base64Utils::base64_decode(string str){
	int str_len = str.length();
	string res="";
	int flag;
	if (str[str_len-1]=='=' && str[str_len-2]=='=')		flag=2;
	else if (str[str_len-1]=='=' && str[str_len-2]!='=')	flag = 1;
	else	flag = 0;
	//cout<<flag<<" "<<str;
	for (int strp=0; strp<str_len-4; strp+=4){
		res += a_to_c(find_int(str[strp])<<2 | find_int(str[strp+1])>>4);
		res += a_to_c(find_int(str[strp+1])<<4 | find_int(str[strp+2])>>2);
		res += a_to_c(find_int(str[strp+2])<<6 | find_int(str[strp+3]));
	}
	int pos = str_len-4;
	if (flag==1){	//原字符串模3余2
		res += a_to_c(find_int(str[pos])<<2 | find_int(str[pos+1])>>4);
		res += a_to_c(find_int(str[pos+1])<<4 | find_int(str[pos+2])>>2);
	}else if (flag==2){	//原字符串模3余1
		res += a_to_c(find_int(str[pos])<<2 | find_int(str[pos+1])>>4);
	}else if (flag==0){
		res += a_to_c(find_int(str[pos])<<2 | find_int(str[pos+1])>>4);
		res += a_to_c(find_int(str[pos+1])<<4 | find_int(str[pos+2])>>2);
		res += a_to_c(find_int(str[pos+2])<<6 | find_int(str[pos+3]));
	}
	return res;
}
