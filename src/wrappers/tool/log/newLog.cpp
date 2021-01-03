/// -------------------------------------------------
/// newLog.cpp : 新日志封装实现文件
/// -------------------------------------------------
/// Copyright (c) 2021, huangdeng525 <huangdeng525@foxmail.com>
/// All rights reserved.
/// -------------------------------------------------

#include "newLog.h"
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#if DCOP_OS == DCOP_OS_WINDOWS
#elif DCOP_OS == DCOP_OS_LINUX
#include <signal.h>
#endif


CNewLog::CNewLog(std::string &strlogName)
:m_strLogName(strlogName)
,m_maxLogLen(100000)
{
    m_pLock = std::make_shared<objLock> (objLock::CreateInstance(szFileName, __LINE__));
}

// 记录信息到日志，自动检查文件大小进行备份
void CNewLog::Write(const char *info)
{
    if (!info || !(*info)) return;

    AutoLocker<CNewLog> autoLocker(*this);
    CheckLogSize();

    FILE *fp = fopen(m_strLogName.c_str() , "a+b");
    if (fp == nullptr)
    {
        return;
    }
    AutoFile autofile(fp);

    bool newline = true;

    if (info[strlen(info) - 1] == '\n')
    {
        newline = false;
    }

    fprintf(fp, ((newline)? "%s\r\n" : "%s"), info);
}

// 检查日志文件大小，超大则重命名为.bak的一个日志文件，当前大小暂定为100k
// 内部接口，不保护，由调用方负责
void CNewLog::CheckLogSize()
{
    {
        fp = fopen(m_strLogName.c_str() , "a+b");
        if (fp == nullptr)
        {
            return;
        }
        AutoFile autofile(fp);

        fseek(fp, 0, SEEK_END); //定位到文件末
        int nFileLen = ftell(fp); //文件长度
        if (nFileLen < m_maxLogLen)
        {
            return;
        }
    }

    std::string bak_log_name(m_strLogName + '.bak');

    (void)remove(bak_log_name.c_str());
    (void)rename(m_strLogName.c_str(), bak_log_name.c_str());        
}
