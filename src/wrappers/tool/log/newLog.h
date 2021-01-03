/// -------------------------------------------------
/// newLog.h : 新日志封装公共头文件
/// -------------------------------------------------
/// Copyright (c) 2021, huangdeng525 <huangdeng525@foxmail.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _TOOL_NEW_LOG_LOG_H_
#define _TOOL_NEW_LOG_LOG_H_

#include "os.h"
#include "sem.h"
#include <memory>

template<typename T>
AutoLocker
{
private:
    T *m_pLock;
public:
    AutoLocker(T &L)
    {
        m_pLock = &L;
        m_pLock->Enter();
    }
    ~AutoLocker()
    {
        if(m_pLock != nullptr)
        {
            m_pLock->Leave();
        }
    }
};


AutoFile
{
private:
    FILE *m_FP;
public:
    AutoFile(FILE *pFP):m_FP(pFP);
    ~AutoFile()
    {
        if(m_FP != nullptr)
        {
            fclose(m_FP);
        }
    }
};


class CNewLog
{
public:
    // 创建一个简单的日志文件，strlogName是日志文件的全路径名
    // 默认大小100K时进行一次备份，文件名加上.bak
    static std::shared_ptr<CNewLog> build(std::string &strlogName)
    {
        return std::make_shared<CNewLog>(strlogName);
    }

public:
    void Write(const char *info);
    void Enter() {if (m_pLock) m_pLock->Enter();}
    void Leave() {if (m_pLock) m_pLock->Leave();}

protected:
    CNewLog(std::string &strlogName);
    ~CNewLog(){};

private:
    void CheckLogSize();
    std::shared_ptr<objLock> m_pLock;
    std:: string m_strLogName;
    int m_maxLogLen;
};


#endif // #ifndef _TOOL_NEW_LOG_LOG_H_

