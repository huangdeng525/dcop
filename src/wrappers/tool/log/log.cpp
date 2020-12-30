/// -------------------------------------------------
/// log.cpp : 日志封装实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "log.h"
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

/// -------------------------------------------------
/// 自动锁
/// -------------------------------------------------
#define AutoLogLock(x) AutoLogLockEx __tmp_##x(x)

/// -------------------------------------------------
/// 日志类型
/// -------------------------------------------------
static CLog *sg_pErrLog = 0;                        // 错误日志
static CLog *sg_pTraceLog = 0;                      // 跟踪信息
static CLog *sg_pCheckLog = 0;                      // 检查日志


/*******************************************************
  函 数 名: DebugLogStatus
  描    述: 调试开关状态
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void DebugLogStatus(int status)
{
    if (status)
    {
        SetSysErrHandler();
    }

    g_debug_switch = status;
}

/*******************************************************
  函 数 名: GetLogTime
  描    述: 获取日志时间
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
int GetLogTime(char *szStr, int strLen)
{
    struct tm *newtime;
    time_t ltime;

    time(&ltime);
    newtime = localtime(&ltime);

    return snprintf(szStr, strLen, "[%02d-%02d %02d:%02d:%02d.%03d] ", 
        newtime->tm_mon + 1, newtime->tm_mday, 
        newtime->tm_hour, newtime->tm_min, newtime->tm_sec, 
        int(clock()/(CLOCKS_PER_SEC/1000)) % 1000);
}

/*******************************************************
  函 数 名: GetLogFileNameAndLine
  描    述: 获取文件名和行号
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
int GetLogFileNameAndLine(char *szStr, int strLen, const char *file, int line)
{
    if (!file) return 0;

    const char *pFileName = strrchr(file, '\\');
    if (!pFileName) pFileName = strrchr(file, '/');
    if (!pFileName) pFileName = file;
    else ++pFileName;

    return snprintf(szStr, strLen, " <%s:%d>", pFileName, line);
}

/*******************************************************
  函 数 名: PrintLogEx
  描    述: 打印日志
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void PrintLogEx(const char *info, LOG_PRINT print, LOG_PARA para, const char *file, int line)
{
    if (!info || !(*info) || !print) return;

    char szStr[STR_FORMAT_LEN_MAX];
    int pos = GetLogTime(szStr, sizeof(szStr));
    int lprint = snprintf(szStr+pos, sizeof(szStr)-pos, "%s", info);
    szStr[sizeof(szStr)-1] = '\0';
    if (lprint < 0) pos = (int)strlen(szStr);
    else pos += lprint;

    while (pos && ((szStr[pos-1] == '\n') || 
                   (szStr[pos-1] == '\r') || 
                   (szStr[pos-1] == ' ')))
    {
        pos--;
    }

    pos += GetLogFileNameAndLine(szStr+pos, sizeof(szStr)-pos, file, line);

    (void)snprintf(szStr+pos, sizeof(szStr)-pos, "\r\n");
    szStr[sizeof(szStr)-1] = '\0';

    print(szStr, para);
}

/*******************************************************
  函 数 名: PrintBufferEx
  描    述: 打印内存
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void PrintBufferEx(const char *info, const void *buf, size_t len, LOG_PRINT print, LOG_PARA para, const char *file, int line)
{
    if (!buf || !len || !print) return;

    char szLine[84]; // 80个字符刚好满足一行16个字节的打印
    char szByte[20]; // 16个字节的字符显示
    size_t offset = 0;

    PrintLogEx(info, print, para, file, line);

    for (size_t i = 0; i < len; ++i)
    {
        /// 开头的处理
        if ((i % 16) == 0)
        {
            offset = (size_t)snprintf(szLine, sizeof(szLine), "0x%08x: ", (unsigned int)i);
        }

        /// 中间的处理
        BYTE *pbyTmp = (BYTE *)buf;
        offset += (size_t)snprintf(szLine + offset, sizeof(szLine) - offset, "%02x ", pbyTmp[i]);
        if (isprint(pbyTmp[i]))
        {
            szByte[i % 16] = pbyTmp[i];
        }
        else
        {
            szByte[i % 16] = '.';
        }

        /// 最后的处理
        if (((i % 16) == 15) || (i == (len - 1)))
        {
            size_t differ = 15 - (i % 16);
            for (size_t j = 0; j < differ; ++j)
            {
                offset += (size_t)snprintf(szLine + offset, sizeof(szLine) - offset, "   ");
                szByte[(i % 16) + j + 1] = ' ';
            }
            szByte[16] = '\0';
            offset += (size_t)snprintf(szLine + offset, sizeof(szLine) - offset, "; %s\r\n", szByte);
            print(szLine, para);
        }

    }
}

/*******************************************************
  函 数 名: PrintToConsole
  描    述: 向控制台打印
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void PrintToConsole(const char *info, void *)
{
    if (!info) return;

    printf("%s", info);
}

/*******************************************************
  函 数 名: TraceLogEx
  描    述: 记录信息到日志
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void TraceLogEx(const char *info, const char *file, int line)
{
    /// info为空，也打印文件名和行号

    if (!g_debug_switch)
    {
        return;
    }

    /// 合并输入信息和日期
    char szStr[STR_FORMAT_LEN_MAX];
    int pos = GetLogTime(szStr, sizeof(szStr));
    pos += snprintf(szStr+pos, sizeof(szStr)-pos, "%s", info);
    pos += GetLogFileNameAndLine(szStr+pos, sizeof(szStr)-pos, file, line);

    /// 写入trace.log文件
    if (g_debug_switch & LOG_OUT_FILE)
    {
        if (!sg_pTraceLog)
        {
            sg_pTraceLog = CLog::Init(LOG_ID_TRACELOG, "trace.log");
            if (!sg_pTraceLog) return;

            sg_pTraceLog->Write("\r\n========================= TRACE START =========================");
        }

        sg_pTraceLog->Write(szStr);
    }

    /// 以printf方式输出
    if (g_debug_switch & LOG_OUT_PRINT)
    {
        printf("%s\r\n", szStr);
    }
}

/*******************************************************
  函 数 名: TraceBufEx
  描    述: 记录内存到日志
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void TraceBufEx(const char *info, const void *buf, size_t len, const char *file, int line)
{
    if (!g_debug_switch)
    {
        return;
    }

    /// 写入trace.log文件
    if (g_debug_switch & LOG_OUT_FILE)
    {
        if (!sg_pTraceLog)
        {
            sg_pTraceLog = CLog::Init(LOG_ID_TRACELOG, "trace.log");
            if (!sg_pTraceLog) return;

            sg_pTraceLog->Write("\r\n========================= TRACE START =========================");
        }

        sg_pTraceLog->Write(info, buf, len, file, line);
    }

    /// 以printf方式输出
    if (g_debug_switch & LOG_OUT_PRINT)
    {
        PrintBufferEx(info, buf, len, PrintToConsole, 0, file, line);
    }
}

/*******************************************************
  函 数 名: CheckRetCodeEx
  描    述: 检查返回值
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CheckRetCodeEx(int rc, int expect, const char *info, const char *file, int line)
{
    if (rc == expect)
    {
        return;
    }

    char szStr[STR_FORMAT_LEN_MAX];
    int pos = GetLogTime(szStr, sizeof(szStr));
    pos += snprintf(szStr+pos, sizeof(szStr)-pos, "%s('%d'!='%d')", info, rc, expect);
    pos += GetLogFileNameAndLine(szStr+pos, sizeof(szStr)-pos, file, line);

    if (!sg_pCheckLog)
    {
        sg_pCheckLog = CLog::Init(LOG_ID_CHECKLOG, "check.log");
        if (!sg_pCheckLog) return;

        sg_pCheckLog->Write("\r\n========================= CHECK START =========================");
    }

    sg_pCheckLog->Write(szStr);

    /// 以printf方式输出
    if (g_debug_switch & LOG_OUT_PRINT)
    {
        printf("%s\r\n", szStr);
    }
}

/*******************************************************
  函 数 名: CheckErrCodeEx
  描    述: 检查错误值
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CheckErrCodeEx(int rc, int expect, const char *info, const char *file, int line)
{
    if (rc == expect)
    {
        return;
    }

    char szStr[STR_FORMAT_LEN_MAX];
    int pos = GetLogTime(szStr, sizeof(szStr));
    pos += snprintf(szStr+pos, sizeof(szStr)-pos, "%s('%d'!='%d')", info, rc, expect);
    pos += GetLogFileNameAndLine(szStr+pos, sizeof(szStr)-pos, file, line);

    if (!sg_pErrLog)
    {
        sg_pErrLog = CLog::Init(LOG_ID_ERRLOG, "err.log");
        if (sg_pErrLog)
        {
            sg_pErrLog->Write("\r\n========================= SYSTEM START =========================");
        }
    }

    if (sg_pErrLog)
    {
        sg_pErrLog->Write(szStr);
    }

    /// 以printf方式输出
    if (g_debug_switch & LOG_OUT_PRINT)
    {
        printf("%s\r\n", szStr);
    }
    DCOP_Reset(-1, file, line);
}

/*******************************************************
  函 数 名: HandleSysErrLog
  描    述: 处理系统错误记录
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void HandleSysErrLog(int error)
{
#if DCOP_OS == DCOP_OS_WINDOWS
#elif DCOP_OS == DCOP_OS_LINUX

    signal(error, SIG_DFL); // 还原默认的信号处理handler

#endif

    PrintLog(STR_FORMAT("!!! System Error(%d) !!!", error), CLog::PrintCallBack, sg_pErrLog);
    ShowCallStack(CLog::PrintCallBack, sg_pErrLog, 0);
}

/*******************************************************
  函 数 名: SetSysErrHandler
  描    述: 设置系统错误处理
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void SetSysErrHandler()
{
#if DCOP_OS == DCOP_OS_WINDOWS
#elif DCOP_OS == DCOP_OS_LINUX

    /// 设置信号的处理函数
    signal(SIGSEGV, HandleSysErrLog); // SIGSEGV    11       Core Invalid memory reference
    signal(SIGABRT, HandleSysErrLog); // SIGABRT     6       Core Abort signal from

#endif
}


/// -------------------------------------------------
/// 全局日志容器
/// -------------------------------------------------
CLog::MAP_LOGS CLog::sm_logs;


/*******************************************************
  函 数 名: CLog::CLog
  描    述: CLog构造
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CLog::CLog(const char *szFileName)
{
    m_pLock = 0;
    (void)strncpy(m_szFileName, szFileName, LOG_FILENAME_LEN_MAX);
    m_szFileName[LOG_FILENAME_LEN_MAX - 1] = '\0';
    m_pLock = objLock::CreateInstance(szFileName, __LINE__);
    m_bOutputToConsole = false;
}

/*******************************************************
  函 数 名: CLog::~CLog
  描    述: CLog析构
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CLog::~CLog()
{
    if (m_pLock)
    {
        delete m_pLock;
        m_pLock = 0;
    }
}

/*******************************************************
  函 数 名: CLog::Init
  描    述: 初始化日志
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CLog *CLog::Init(DWORD dwID, const char *szFileName)
{
    IT_LOGS it = sm_logs.find(dwID);
    if (it != sm_logs.end())
    {
        return 0;
    }

    #undef new
    CLog *pLog = new (szFileName, __LINE__) CLog(szFileName);
    #define new new(__FILE__, __LINE__)
    if (!pLog)
    {
        return 0;
    }

    sm_logs.insert(MAP_LOGS::value_type(dwID, pLog));

    return pLog;
}

/*******************************************************
  函 数 名: CLog::Get
  描    述: 获取日志
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CLog *CLog::Get(DWORD dwID)
{
    IT_LOGS it = sm_logs.find(dwID);
    if (it == sm_logs.end())
    {
        return 0;
    }

    CLog *pLog = (*it).second;

    return pLog;
}

/*******************************************************
  函 数 名: CLog::Write
  描    述: 记录信息到日志
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CLog::Write(const char *info, bool newline)
{
    if (!info || !(*info)) return;

    AutoLogLock(this);
    FILE *fp;

    fp = fopen(m_szFileName , "a+b");
    if (!fp)
    {
        return;
    }

    if (newline && (info[strlen(info) - 1] == '\n'))
    {
        newline = false;
    }

    fprintf(fp, ((newline)? "%s\r\n" : "%s"), info);
    fclose(fp);

    if (m_bOutputToConsole)
    {
        PrintToConsole(info, 0);
    }
}

/*******************************************************
  函 数 名: CLog::Write
  描    述: 记录内存到日志
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CLog::Write(const char *info, const void *buf, size_t len, const char *file, int line)
{
    AutoLogLock(this);

    PrintBufferEx(info, buf, len, PrintCallBack, this, file, line);
}

/*******************************************************
  函 数 名: CLog::PrintCallBack
  描    述: 内存打印回调
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CLog::PrintCallBack(const char *info, void *para)
{
    if (!info || !(*info)) return;

    CLog *pLog = (CLog *)para;
    if (pLog)
    {
        pLog->Write(info, false);
    }
}

