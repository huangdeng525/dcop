/// -------------------------------------------------
/// file.cpp : 文件操作封装实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "fs/file.h"
#include <errno.h>


/*******************************************************
  函 数 名: DCOP_SaveToFile
  描    述: 保存到文件中
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD DCOP_SaveToFile(const char *szFile, const void *cpBuf, DWORD dwCount, DWORD dwOffset)
{
    FILE *pf = 0;
    DWORD dwRc = SUCCESS;

    pf = fopen(szFile, "r+b");
    if (!pf)
    {
        pf = fopen(szFile, "w+b");
        if (!pf)
        {
            dwRc = (DWORD)errno;
            return (dwRc)? dwRc : FAILURE;
        }
    }

    int iRc = fseek(pf, (long)dwOffset, SEEK_SET);
    if (iRc)
    {
        fclose(pf);
        dwRc = (DWORD)errno;
        return (dwRc)? dwRc : FAILURE;
    }

    size_t numwritten = fwrite(cpBuf, sizeof(BYTE), dwCount, pf);
    fclose(pf);

    if (numwritten < dwCount)
    {
        dwRc = (DWORD)errno;
        return (dwRc)? dwRc : FAILURE;
    }

    return SUCCESS;
}

/*******************************************************
  函 数 名: DCOP_RestoreFromFile
  描    述: 从文件中恢复
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD DCOP_RestoreFromFile(const char *szFile, void *cpBuf, DWORD dwCount, DWORD dwOffset)
{
    FILE *pf = 0;
    DWORD dwRc = SUCCESS;

    pf = fopen(szFile, "rb");
    if (!pf)
    {
        return FAILURE;
    }

    int iRc = fseek(pf, (long)dwOffset, SEEK_SET);
    if (iRc)
    {
        fclose(pf);
        dwRc = (DWORD)errno;
        return (dwRc)? dwRc : FAILURE;
    }

    size_t numread = fread(cpBuf, sizeof(BYTE), dwCount, pf);
    fclose(pf);

    if (numread < dwCount)
    {
        dwRc = (DWORD)errno;
        return (dwRc)? dwRc : FAILURE;
    }

    return SUCCESS;
}

/*******************************************************
  函 数 名: DCOP_IsFileExist
  描    述: 文件是否存在
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
BOOL DCOP_IsFileExist(const char *szFile)
{
    FILE *pf = fopen(szFile, "rb");
    if (!pf)
    {
        return FALSE;
    }

    fclose(pf);
    return TRUE;
}

/*******************************************************
  函 数 名: DCOP_GetFileLen
  描    述: 获取文件长度
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD DCOP_GetFileLen(const char *szFile)
{
    FILE *pf = fopen(szFile, "rb");
    if (!pf)
    {
        return 0;
    }

    int iRc = fseek(pf, (long)0, SEEK_END);
    if (iRc)
    {
        fclose(pf);
        return 0;
    }

    long position = ftell(pf);
    fclose(pf);

    if (position < 0)
    {
        return 0;
    }

    return (DWORD)position;
}


