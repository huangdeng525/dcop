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


/// -------------------------------------------------
/// 跟踪信息开关
/// -------------------------------------------------
int g_debug_switch =
#ifdef DEBUG
    LOG_OUT_FILE;
#else
    LOG_OUT_NULL;
#endif

/*******************************************************
  函 数 名: CLog::CLog
  描    述: CLog构造
  输    入:
  输    出:
  返    回:
  修改记录:
 *******************************************************/
CNewLog::CNewLog(std::string &strlogName)
:m_strLogName(strlogName)
{
    m_pLock = std::make_shared<objLock> (objLock::CreateInstance(szFileName, __LINE__));
}

/*******************************************************
  函 数 名: CLog::Write
  描    述: 记录信息到日志
  输    入:
  输    出:
  返    回:
  修改记录:
 *******************************************************/
void CNewLog::Write(const char *info)
{
    if (!info || !(*info)) return;

    AutoLocker<CNewLog> autoLocker(*this);

    FILE *fp;

    fp = fopen(m_strLogName.c_str() , "a+b");
    if (!fp)
    {
        return;
    }

    bool newline = true;

    if (info[strlen(info) - 1] == '\n')
    {
        newline = false;
    }

    fprintf(fp, ((newline)? "%s\r\n" : "%s"), info);
    fclose(fp);
}
