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

class CNewLog
{
public:
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

    std::shared_ptr<objLock> m_pLock;
    std:: string m_strLogName;
};


#endif // #ifndef _TOOL_NEW_LOG_LOG_H_

