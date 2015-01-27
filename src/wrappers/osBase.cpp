/// -------------------------------------------------
/// osBase.cpp : 操作系统基类实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "osBase.h"
#include "task.h"
#include "string/tablestring.h"


/// -------------------------------------------------
/// OS基础元素集合
/// -------------------------------------------------
static LIST_HEAD(osBase) g_osBaseHead[OSTYPE_NUM] =
{
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0}
};

/// -------------------------------------------------
/// OS基础元素集合操作自旋锁
/// -------------------------------------------------
objSpinLock g_osBaseLock;
objSpinLock *g_pOsBaseLock = &g_osBaseLock;


/*******************************************************
  函 数 名: osBase::osBase
  描    述: osBase构造
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
osBase::osBase()
{
    m_hHandle = 0;
    m_osType = OSTYPE_NULL;
    m_objPtr = NULL;
    (void)memset(m_szName, 0, sizeof(m_szName));
    m_dwID = 0;
    LIST_NODE_INIT(this, m_field);
}

/*******************************************************
  函 数 名: osBase::~osBase
  描    述: osBase析构
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
osBase::~osBase()
{
    vDelFromList();
}

/*******************************************************
  函 数 名: osBase::vSetName
  描    述: 设置名字
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void osBase::vSetName(const char *cszName)
{
    if (!cszName || !(*cszName))
    {
        return;
    }

    (void)snprintf(m_szName, sizeof(m_szName), "%s", cszName);
    m_szName[sizeof(m_szName) - 1] = '\0';
}

/*******************************************************
  函 数 名: osBase::vAddToList
  描    述: 添加到集合中
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void osBase::vAddToList(DWORD osType, void *objPtr)
{
    if (osType >= OSTYPE_NUM)
    {
        return;
    }

    m_osType = osType;
    m_objPtr = objPtr;

    AutoSpinLock(g_pOsBaseLock);
    LIST_INSERT_HEAD(&g_osBaseHead[osType], this, m_field);
}

/*******************************************************
  函 数 名: osBase::vAddToList
  描    述: 从集合中删除
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void osBase::vDelFromList()
{
    if (m_osType >= OSTYPE_NUM)
    {
        return;
    }

    AutoSpinLock(g_pOsBaseLock);
    LIST_REMOVE(&g_osBaseHead[m_osType], this, m_field);
}

/*******************************************************
  函 数 名: osBase::First
  描    述: 开始元素
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
osBase *osBase::First(DWORD osType)
{
    if (osType >= OSTYPE_NUM)
    {
        return NULL;
    }

    AutoSpinLock(g_pOsBaseLock);
    return LIST_FIRST(&g_osBaseHead[osType]);
}

/*******************************************************
  函 数 名: osBase::Next
  描    述: 下一个元素
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
osBase *osBase::Next()
{
    AutoSpinLock(g_pOsBaseLock);
    return LIST_NEXT(this, m_field);
}

/*******************************************************
  函 数 名: osBase::Find
  描    述: 通过名字查找元素
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
osBase *osBase::Find(DWORD osType, const char *cszName)
{
    if (osType >= OSTYPE_NUM)
    {
        return NULL;
    }

    AutoSpinLock(g_pOsBaseLock);
    osBase *pBase = LIST_FIRST(&g_osBaseHead[osType]);
    while (pBase)
    {
        if (!strcmp(pBase->cszGetName(), cszName))
        {
            break;
        }

        pBase = LIST_NEXT(pBase, m_field);
    }

    return pBase;
}

/*******************************************************
  函 数 名: osBase::Find
  描    述: 通过ID查找元素
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
osBase *osBase::Find(DWORD osType, DWORD dwID)
{
    if (osType >= OSTYPE_NUM)
    {
        return NULL;
    }

    AutoSpinLock(g_pOsBaseLock);
    osBase *pBase = LIST_FIRST(&g_osBaseHead[osType]);
    while (pBase)
    {
        if (pBase->dwGetID() == dwID)
        {
            break;
        }

        pBase = LIST_NEXT(pBase, m_field);
    }

    return pBase;
}

/*******************************************************
  函 数 名: osBase::Dump
  描    述: Dump
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void osBase::Dump(LOG_PRINT logPrint, LOG_PARA logPara, int argc, void **argv)
{
    DWORD osType = (argc)? *(DWORD *)(argv[0]) : OSTYPE_NULL;

    objTask *pTask = objTask::Current();
    PrintLog(STR_FORMAT("Current Task: '%s'(%d)", 
                        (pTask)? pTask->Name() : "Null", 
                        (pTask)? pTask->ID() : 0), 
                        PrintToConsole, 0);
    ShowCallStack(0, 0, 0);

    AutoSpinLock(g_pOsBaseLock);

    for (DWORD i = 0; i < OSTYPE_NUM; ++i)
    {
        if ((osType < OSTYPE_NUM) && (osType != i))
        {
            continue;
        }

        osBase *pBase = LIST_FIRST(&g_osBaseHead[i]);
        if (!pBase)
        {
            continue;
        }

        DWORD dwCount = (DWORD)LIST_COUNT(&g_osBaseHead[i]);
        CTableString tableStr(5, dwCount + 1, "  ");
        tableStr << "Type";
        tableStr << "Name";
        tableStr << "ID";
        tableStr << "Handle";
        tableStr << "Ptr";

        DCOP_START_TIME();

        while (pBase)
        {
            DWORD dwType = pBase->osGetType();
            tableStr << ((dwType >= OSTYPE_NUM)? "OSTYPE_NULL" : OSTYPE_INFO[dwType]);
            tableStr << pBase->cszGetName();
            tableStr << STR_FORMAT("%d", pBase->dwGetID());
            tableStr << STR_FORMAT("%p", pBase->hGetHandle());
            tableStr << STR_FORMAT("%p", pBase->objGetPtr());
            pBase = LIST_NEXT(pBase, m_field);
        }

        DCOP_END_TIME();

        logPrint(STR_FORMAT("%s Dump: (Count: %d) \r\n", OSTYPE_INFO[i], dwCount), logPara);
        tableStr.Show(logPrint, logPara, "=", "-");
        logPrint(STR_FORMAT("[cost time: %d ms] \r\n", DCOP_COST_TIME()), logPara);
    }
}

