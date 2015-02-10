/// -------------------------------------------------
/// taskType.cpp : 任务封装类实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "taskType.h"
#include "taskApi.h"
#include <string.h>


/*******************************************************
  函 数 名: objTask::CreateInstance
  描    述: 创建任务
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
objTask *objTask::CreateInstance(const char *cszName,
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
    if (!pTaskBase)
    {
        return NULL;
    }

    pTaskBase->SetName(cszName);

    if (pEntry)
    {
        if (pTaskBase->Create(pEntry, dwStackSize, dwPriority, pPara))
        {
            delete pTaskBase;
            pTaskBase = NULL;
        }
    }
    else
    {
        /// 没有入口就添加当前任务
        pTaskBase->SetID(CTaskBase::Current());
    }

    return pTaskBase;
}

/*******************************************************
  函 数 名: objTask::~objTask
  描    述: 析构任务
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
objTask::~objTask()
{
}

/*******************************************************
  函 数 名: objTask::Delay
  描    述: 任务延时
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void objTask::Delay(DWORD delayMilliseconds)
{
    OS_VDFUNC_CALL(Task, Delay)(delayMilliseconds);
}

/*******************************************************
  函 数 名: objTask::Current
  描    述: 获取当前任务
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
objTask *objTask::Current()
{
    osBase *pBase = osBase::Find(OSTYPE_TASK, CTaskBase::Current());
    if (!pBase)
    {
        return NULL;
    }

    return (objTask *)pBase->objGetPtr();
}

/*******************************************************
  函 数 名: objTask::IPara::~IPara
  描    述: 析构任务参数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
objTask::IPara::~IPara()
{
}

/*******************************************************
  函 数 名: CTaskBase::CTaskBase
  描    述: CTaskBase构造
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CTaskBase::CTaskBase()
{
    m_pEntry = 0;
    m_dwStackSize = 0;
    m_dwPriority = 0;
    m_pPara = 0;

    osBase::vAddToList(OSTYPE_TASK, this);
}

/*******************************************************
  函 数 名: CTaskBase::~CTaskBase
  描    述: CTaskBase析构
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CTaskBase::~CTaskBase()
{
    (void)Destroy();

    if (m_pPara)
    {
        delete m_pPara;
        m_pPara = 0;
    }
}

/*******************************************************
  函 数 名: CTaskBase::vAllTaskEntry
  描    述: 所有任务入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CTaskBase::vAllTaskEntry(void *pPara)
{
    CTaskBase *pThis = (CTaskBase *)pPara;
    if (!pThis)
    {
        return;
    }

    /// 在入口处获得当前任务ID
    pThis->SetID(CTaskBase::Current());

    /// 调用真正的任务入口
    (pThis->pGetEntry())(pThis->pGetPara());

    /// 任务自己结束
    delete pThis;
}

/*******************************************************
  函 数 名: CTaskBase::Create
  描    述: 创建任务
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CTaskBase::Create(OSTASK_ENTRY pEntry,
                        DWORD dwStackSize,
                        DWORD dwPriority,
                        IPara *pPara)
{
    if (!pEntry)
    {
        return FAILURE;
    }

    m_pEntry = pEntry;
    m_dwStackSize = dwStackSize;
    m_dwPriority = dwPriority;
    m_pPara = pPara;
    OSHANDLE Handle = 0;
    DWORD dwID = 0;

    DWORD dwRc = OS_FUNC_CALL(Task, Create)(&Handle,
                        osBase::cszGetName(),
                        &dwID,
                        vAllTaskEntry,
                        m_dwStackSize,
                        m_dwPriority,
                        this);
    if (SUCCESS == dwRc)
    {
        osBase::vSetHandle(Handle);
    }
    else
    {
        /// 失败后，参数由外面释放
        m_pPara = 0;
    }

    TRACE_LOG(STR_FORMAT("Task '%s'(%d) Create rc:0x%x", osBase::cszGetName(), dwID, dwRc));

    return dwRc;
}

/*******************************************************
  函 数 名: CTaskBase::Destroy
  描    述: 删除任务
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CTaskBase::Destroy()
{
    if (!osBase::hGetHandle())
    {
        return SUCCESS;
    }

    DWORD dwRc = OS_FUNC_CALL(Task, Destroy)(osBase::hGetHandle(),
                    osBase::cszGetName(),
                    osBase::dwGetID());

    osBase::vSetHandle(0);

    TRACE_LOG(STR_FORMAT("Task '%s' Destroy rc:0x%x", osBase::cszGetName(), dwRc));

    return dwRc;
}

/*******************************************************
  函 数 名: CTaskBase::SetLocal
  描    述: 设置本地变量
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CTaskBase::SetLocal(DWORD dwPos, void *pVal, DWORD dwLen)
{
    if (!dwLen)
    {
        return FAILURE;
    }

    DWORD dwCurPos = m_sLocal.GetOffSet();
    if (dwCurPos < dwPos)
    {
        DWORD dwRc = m_sLocal.SetNeedMemLen((dwPos - dwCurPos) + dwLen);
        if (dwRc != SUCCESS) return dwRc;
    }

    if (pVal)
    {
        m_sLocal.SetOffset(dwPos) << CBufferPara(pVal, dwLen);
    }

    return SUCCESS;
}

/*******************************************************
  函 数 名: CTaskBase::GetLocal
  描    述: 获取本地变量
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void *CTaskBase::GetLocal(DWORD dwPos)
{
    return m_sLocal.Buffer(dwPos);
}

/*******************************************************
  函 数 名: CTaskBase::Current
  描    述: 获取当前任务ID
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CTaskBase::Current()
{
    DWORD dwCurID = OS_FUNC_CALL(Task, Current)();
    if (dwCurID == FAILURE) dwCurID = 0;
    return dwCurID;
}

