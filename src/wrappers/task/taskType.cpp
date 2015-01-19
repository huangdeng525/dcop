/// -------------------------------------------------
/// taskType.cpp : 任务封装类实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "taskType.h"
#include "taskApi.h"
#include <string.h>


objTask *objTask::CreateInstance(const char *szName,
                        OSTASK_ENTRY pEntry,
                        DWORD dwStackSize,
                        DWORD dwPriority,
                        IPara *pPara,
                        const char *file,
                        int line)
{
    #undef new
    CTaskBase *pTaskBase = new (file, line) CTaskBase();
    #define new new(__FILE__, __LINE__)
    if (pTaskBase)
    {
        pTaskBase->vSetName(szName);
        if (pTaskBase->Create(pEntry, dwStackSize, dwPriority, pPara))
        {
            delete pTaskBase;
            pTaskBase = 0;
        }
    }

    return pTaskBase;
}

objTask::~objTask()
{
}

void objTask::Delay(DWORD delayMilliseconds)
{
    OS_VDFUNC_CALL(Task, Delay)(delayMilliseconds);
}

DWORD objTask::Current()
{
    DWORD dwCurID = OS_FUNC_CALL(Task, Current)();
    if (dwCurID == FAILURE) dwCurID = 0;
    return dwCurID;
}

objTask::IPara::~IPara()
{
}

CTaskBase::CTaskBase()
{
    m_pEntry = 0;
    m_dwStackSize = 0;
    m_dwPriority = 0;
    m_pPara = 0;
    m_dwID = 0;
    *m_szName = 0;
}

CTaskBase::~CTaskBase()
{
    (void)Destroy();

    if (m_pPara)
    {
        delete m_pPara;
        m_pPara = 0;
    }
}

void CTaskBase::vSetName(const char *szName)
{
    if (!szName || !(*szName))
    {
        return;
    }

    (void)snprintf(m_szName, sizeof(m_szName), "%s", szName);
    m_szName[sizeof(m_szName) - 1] = '\0';
}

const char *CTaskBase::szGetName()
{
    return m_szName;
}

void CTaskBase::vAllTaskEntry(void *pPara)
{
    CTaskBase *pThis = (CTaskBase *)pPara;
    if (!pThis)
    {
        return;
    }

    pThis->osBase::vSetID(objTask::Current());

    /// 调用真正的任务入口
    (pThis->pGetEntry())(pThis->pGetPara());

    /// 任务自己结束
    delete pThis;
}

DWORD CTaskBase::Create(OSTASK_ENTRY pEntry,
            DWORD dwStackSize,
            DWORD dwPriority,
            IPara *pPara)
{
    m_pEntry = pEntry;
    m_dwStackSize = dwStackSize;
    m_dwPriority = dwPriority;
    m_pPara = pPara;
    OSHANDLE Handle = 0;

    DWORD dwRc = OS_FUNC_CALL(Task, Create)(&Handle, 
                    m_szName,
                    &m_dwID,
                    vAllTaskEntry, 
                    m_dwStackSize,
                    m_dwPriority,
                    this);
    if (SUCCESS == dwRc)
    {
        osBase::vSetHandle(Handle);
        osBase::vAddToList(OSTYPE_TASK, this, m_szName, m_dwID);
    }
    else
    {
        /// 失败后，参数由外面释放
        m_pPara = 0;
    }

    TRACE_LOG(STR_FORMAT("Task '%s' Create rc:0x%x", m_szName, dwRc));

    return dwRc;
}

DWORD CTaskBase::Destroy()
{
    DWORD dwRc = OS_FUNC_CALL(Task, Destroy)(osBase::hGetHandle(),
                    m_szName,
                    m_dwID);
    if (SUCCESS == dwRc)
    {
        osBase::vSetHandle(0);
    }

    TRACE_LOG(STR_FORMAT("Task '%s' Destroy rc:0x%x", m_szName, dwRc));

    return dwRc;
}

