/// -------------------------------------------------
/// file.h : 文件操作封装公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _TOOL_FS_FILE_H_
#define _TOOL_FS_FILE_H_

#include "dcop.h"


/////////////////////////////////////////////////////
/// 'r'  只读方式打开，将文件指针指向文件头，如果文件不存在，则File返回空。
/// 'r+' 读写方式打开，将文件指针指向文件头，如果文件不存在，则File返回空。 
/// 'w'  写入方式打开，将文件指针指向文件头并将文件大小截为零。如果文件不存在则尝试创建之。 
/// 'w+' 读写方式打开，将文件指针指向文件头并将文件大小截为零。如果文件不存在则尝试创建之。 
/// 'a'  写入方式打开，将文件指针指向文件末尾。如果文件不存在则尝试创建之。 
/// 'a+' 读写方式打开，将文件指针指向文件末尾。如果文件不存在则尝试创建之。 
/// 'x'  创建并以写入方式打开，将文件指针指向文件头。如果文件已存在，则 fopen() 调用失败并返回 FALSE。 
/// 'x'  创建并以写入方式打开，将文件指针指向文件头。如果文件已存在，则 fopen() 调用失败并返回 FALSE。
/////////////////////////////////////////////////////


/// 文件名长度(含路径)
#define DCOP_FILE_NAME_LEN                  256


#ifdef __cplusplus
    extern "C" {
#endif

DWORD DCOP_SaveToFile(const char *szFile, const void *cpBuf, DWORD dwCount, DWORD dwOffset);
DWORD DCOP_RestoreFromFile(const char *szFile, void *cpBuf, DWORD dwCount, DWORD dwOffset);
BOOL  DCOP_IsFileExist(const char *szFile);
DWORD DCOP_GetFileLen(const char *szFile);

#ifdef __cplusplus
    }
#endif


#endif // #ifndef _TOOL_FS_FILE_H_

