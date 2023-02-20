#include "user_util.h"

/**判断str1是否以str2开头
 * 如果是返回1
 * 不是返回0
 * 出错返回-1
 * */
int is_begin_with(const char *str1, char *str2)
{
    if (str1 == NULL || str2 == NULL)
        return -1;
    int len1 = strlen(str1);
    int len2 = strlen(str2);
    if ((len1 < len2) || (len1 == 0 || len2 == 0))
        return -1;
    char *p = str2;
    int i = 0;
    while (*p != '\0')
    {
        if (*p != str1[i])
            return 0;
        p++;
        i++;
    }
    return 1;
}

/**判断str1是否以str2结尾
 * 如果是返回1
 * 不是返回0
 * 出错返回-1
 * */
int is_end_with(const char *str1, char *str2)
{
    if (str1 == NULL || str2 == NULL)
        return -1;
    int len1 = strlen(str1);
    int len2 = strlen(str2);
    if ((len1 < len2) || (len1 == 0 || len2 == 0))
        return -1;
    while (len2 >= 1)
    {
        if (str2[len2 - 1] != str1[len1 - 1])
            return 0;
        len2--;
        len1--;
    }
    return 1;
}

//搜索 指定目录下的所有文件及其子目录下的文件
int getFilesName(fileParams *params,char files[][256])
{
    DIR *dir = opendir(params->dirPath);
    int count = 0;
    if (dir == NULL)
    {
        // printf("%s\n", strerror(errno));
        return 0;
    }
    chdir(params->dirPath); //进入到当前读取目录
    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL)
    {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
        {
            continue;
        }
        struct stat st;
        stat(ent->d_name, &st);
        if (S_ISDIR(st.st_mode) && 1 == params->recursive)
        {
            //暂时不做
            // fileParams tmpParam = {ent->d_name,params->startStr,params->endStr,5,256,0};
            // getFilesName(ent->d_name,params);//这里暂时用递归调用 注意栈溢出问题
        }
        else if(!S_ISDIR(st.st_mode))
        {
            // printf("%s\n", ent->d_name);
            if (count<params->total)
            {
                if (params->startStr!=NULL && 1 != is_begin_with(ent->d_name,params->startStr))
                {
                    continue;
                }
                if (params->endStr!=NULL && 1 != is_end_with(ent->d_name,params->endStr))
                {
                    continue;
                }
                snprintf(files[count],params->size,"%s/%s",params->dirPath,ent->d_name);
                printf("%s\n ", files[count]);
                count++;
            }
        }
    }
    closedir(dir);
    chdir(".."); //返回当前目录的上一级目录
    return count;
}


int getTypeFilesCount(char *dirPath,char *startStr,char *endStr)
{
    DIR *dir = opendir(dirPath);
    int count = 0;
    if (dir == NULL)
    {
        return 0;
    }
    if(0!=chdir(dirPath))
    {
        return 0;
    }
    struct dirent *ent;
    // printf("getTypeFilesCount[");
    while ((ent = readdir(dir)) != NULL)
    {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
        {
            continue;
        }
        struct stat st;
        stat(ent->d_name, &st);
        if(!S_ISDIR(st.st_mode))
        {
            if (startStr!=NULL && 1 != is_begin_with(ent->d_name,startStr))
            {
                continue;
            }
            if (endStr!=NULL && 1 != is_end_with(ent->d_name,endStr))
            {
                continue;
            }
            // printf("%s ==>", ent->d_name);
            count++;
        }
    }
    closedir(dir);
    // printf("[%d]\n",count);
    return count;
}

