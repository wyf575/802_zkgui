
#ifndef GHTTP_CLIENT_H
#define GHTTP_CLIENT_H
/**
 * @brief 下载文件到本地
 *  
 * @param url 只支持HTTP不支持HTTPS
 * @param save_path 必须是文件路径不能是文件夹
 * @return int 成功为0
 */
void *ghttp_malloc(void);
void ghttp_get_free(void *request);
void ghttp_post_free(void *request);

int ghttp_get_json(void *request,const char *url,char **result);
int ghttp_post_json(void *request,const char *url,const char * post_str,char **result);

int http_download(char *url, char *save_path);

#endif/*GHTTP_CLIENT_H*/
