/*
 * NetUtils.h
 *
 *  Created on: 2020年12月23日
 *      Author: Administrator
 */

#ifndef JNI_MODEL_NETUTILS_H_
#define JNI_MODEL_NETUTILS_H_

#include "JsonParse.h"

namespace NetUtils{

	void sendConfigSuccessMsg(int id);
	void http_post_video_list_result(std::vector<std::string> newVideoId, std::vector<std::string> oldVideoId);
}


#endif /* JNI_MODEL_NETUTILS_H_ */
