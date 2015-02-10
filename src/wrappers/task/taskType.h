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
#include "stream/dstream.h"


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

    const char *Name() {return osBase::cszGetName();}
    DWORD ID() {return osBase::dwGetID();}

    void SetName(const char *cszName) {osBase::vSetName(cszName);}
    void SetID(DWORD dwID) {osBase::vSetID(dwID);}

    DWORD SetLocal(DWORD dwPos, void *pVal, DWORD dwLen);
    void *GetLocal(DWORD dwPos);

    static DWORD Current();

private:
    static void vAllTaskEntry(void *pPara);
    OSTASK_ENTRY pGetEntry() const {return m_pEntry;}
    IPara *pGetPara() const {return m_pPara;}

private:
    OSTASK_ENTRY    m_pEntry;
    DWORD           m_dwStackSize;
    DWORD           m_dwPriority;
    IPara *         m_pPara;
    CDStream        m_sLocal;
};


#endif // #ifndef _TASKTYPE_H_
