/// -------------------------------------------------
/// log.h : 日志封装公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _TOOL_LOG_LOG_H_
#define _TOOL_LOG_LOG_H_

#define INC_MAP

#include "os.h"
#include "sem.h"


/// 日志ID定义
enum LOG_ID
{
    LOG_ID_ERRLOG = 1,
    LOG_ID_TRACELOG,
    LOG_ID_CHECKLOG,
    LOG_ID_MEMLOG
};

/// 日志输出方式
#define LOG_OUT_NULL    0   // 不进行输出
#define LOG_OUT_FILE    1   // 输出到文件中
#define LOG_OUT_PRINT   2   // 以printf方式输出

/// 文件名长度
#define LOG_FILENAME_LEN_MAX    32


class CLog
{
public:
    class AutoLogLockEx
    {
    public:
        AutoLogLockEx() {m_pLog = 0;}
        AutoLogLockEx(CLog *pLog) {m_pLog = pLog; if (m_pLog) m_pLog->Enter();}
        ~AutoLogLockEx() {if (m_pLog) m_pLog->Leave(); m_pLog = 0;}
    private:
        CLog *m_pLog;
    };

public:
    typedef std::map<DWORD, CLog *> MAP_LOGS;
    typedef MAP_LOGS::iterator IT_LOGS;

public:
    CLog(const char *szFileName);
    ~CLog();

    static CLog *Init(DWORD dwID, const char *szFileName);

    static CLog *Get(DWORD dwID);

    void Write(const char *info, bool newline = true);
    void Write(const char *info, const void *puf, size_t len, const char *file = 0, int line = 0);

    void Enter() {if (m_pLock) m_pLock->Enter();}
    void Leave() {if (m_pLock) m_pLock->Leave();}

    void OutputToConsole(bool bEnable) {m_bOutputToConsole = bEnable;}

    static void PrintCallBack(const char *info, void *para);

private:
    static MAP_LOGS sm_logs;
    objLock *m_pLock;
    char m_szFileName[LOG_FILENAME_LEN_MAX];
    bool m_bOutputToConsole;
};


extern "C" void SetSysErrHandler();


#endif // #ifndef _TOOL_LOG_LOG_H_

