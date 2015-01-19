/// -------------------------------------------------
/// taskType.h : 任务封装类公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _TASKTYPE_H_
#define _TASKTYPE_H_

#include "../osBase.h"
#include "task.h"


class CTaskBase : public objTask, private osBase
{
public:
    CTaskBase();
    ~CTaskBase();

    DWORD Create(OSTASK_ENTRY pEntry,
            DWORD dwStackSize,
            DWORD dwPriority,
            IPara *pPara);

    DWORD Destroy();

public:
    OSTASK_ENTRY pGetEntry() const {return m_pEntry;}
    DWORD dwGetStackSize() const {return m_dwStackSize;}
    IPara *pGetPara() const {return m_pPara;}
    DWORD dwGetPriority() const {return m_dwPriority;}

    void vSetName(const char *szName);
    const char *szGetName();

private:
    static void     vAllTaskEntry(void *pPara);
    OSTASK_ENTRY    m_pEntry;
    DWORD           m_dwStackSize;
    DWORD           m_dwPriority;
    IPara *         m_pPara;
    DWORD           m_dwID;
    char            m_szName[OSNAME_LENGTH];
};


#endif // #ifndef _TASKTYPE_H_
