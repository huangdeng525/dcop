/// -------------------------------------------------
/// ObjResponse_main.cpp : 响应器对象实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "ObjResponse_main.h"
#include "Factory_if.h"
#include "BaseID.h"


/// -------------------------------------------------
/// 实现类厂
/// -------------------------------------------------
DCOP_IMPLEMENT_FACTORY(CResponse, "response")

/// -------------------------------------------------
/// 实现基类
/// -------------------------------------------------
DCOP_IMPLEMENT_INSTANCE(CResponse)
    DCOP_IMPLEMENT_INTERFACE(IResponse)
    DCOP_IMPLEMENT_INTERFACE(IObject)
    DCOP_IMPLEMENT_INTERFACE(Instance)
DCOP_IMPLEMENT_INSTANCE_END

/// -------------------------------------------------
/// 实现对象类
/// -------------------------------------------------
DCOP_IMPLEMENT_IOBJECT(CResponse)
    DCOP_IMPLEMENT_IDENTIFY_DYNAMIC("name", "id")
    DCOP_IMPLEMENT_CONFIG_THREADSAFE("threadsafe")
DCOP_IMPLEMENT_IOBJECT_END

/// -------------------------------------------------
/// 实现消息分发
/// -------------------------------------------------
DCOP_IMPLEMENT_IOBJECT_MSG_HANDLE(CResponse)
    DCOP_IMPLEMENT_IOBJECT_MSG_PROC(DCOP_MSG_MANAGER_START, OnStart)
    DCOP_IMPLEMENT_IOBJECT_MSG_PROC(DCOP_MSG_MANAGER_FINISH, OnFinish)
    DCOP_IMPLEMENT_IOBJECT_MSG_PROC(DCOP_MSG_RESPONSE_TIMER_1S, OnTimer1s)
DCOP_IMPLEMENT_IOBJECT_MSG_HANDLE_END


/*******************************************************
  函 数 名: CResponse::CResponse
  描    述: CResponse构造函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CResponse::CResponse(Instance *piParent, int argc, char **argv)
{
    m_piManager = 0;
    m_piTimer = 0;
    m_hTimer1s = 0;

    DCOP_CONSTRUCT_INSTANCE(piParent);
    DCOP_CONSTRUCT_IOBJECT(argc, argv);
}

/*******************************************************
  函 数 名: CResponse::~CResponse
  描    述: CResponse析构函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CResponse::~CResponse()
{
    Fini();

    DCOP_DESTRUCT_IOBJECT();
    DCOP_DESTRUCT_INSTANCE();
}

/*******************************************************
  函 数 名: CResponse::Init
  描    述: 初始化
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CResponse::Init(IObject *root, int argc, void **argv)
{
    if (!root)
    {
        return FAILURE;
    }

    AutoObjLock(this);

    /// 查询对象
    DCOP_QUERY_OBJECT_START(root)
        DCOP_QUERY_OBJECT_ITEM(IManager,     DCOP_OBJECT_MANAGER,    m_piManager)
        DCOP_QUERY_OBJECT_ITEM(ITimer,       DCOP_OBJECT_TIMER,      m_piTimer)
    DCOP_QUERY_OBJECT_END

    return SUCCESS;
}

/*******************************************************
  函 数 名: CResponse::Fini
  描    述: 完成时入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CResponse::Fini()
{
    DCOP_RELEASE_INSTANCE(m_piTimer);
    DCOP_RELEASE_INSTANCE(m_piManager);
}

/*******************************************************
  函 数 名: CResponse::OnStart
  描    述: 开始运行时
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CResponse::OnStart(objMsg *msg)
{
    if (m_piTimer)
    {
        m_hTimer1s = m_piTimer->Start(ITimer::TYPE_LOOP, DCOP_MSG_RESPONSE_TIMER_1S, 1000, this);
    }
}

/*******************************************************
  函 数 名: CResponse::OnFinish
  描    述: 结束运行时
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CResponse::OnFinish(objMsg *msg)
{
    if (m_piTimer && m_hTimer1s)
    {
        m_piTimer->Stop(m_hTimer1s);
        m_hTimer1s = 0;
    }
}

/*******************************************************
  函 数 名: CResponse::OnTimer1s
  描    述: 1S定时
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CResponse::OnTimer1s(objMsg *msg)
{
    for (IT_RESPONSE it_rsp = m_requests.begin();
        it_rsp != m_requests.end(); ++it_rsp)
    {
        CResponsePool *pPool = (*it_rsp);
        if (!pPool) continue;

        pPool->OnTick();
    }
}

/*******************************************************
  函 数 名: CResponse::CreatePool
  描    述: 创建缓冲区
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
IResponse::IPool *CResponse::CreatePool(IObject *owner, DWORD count)
{
    if (!count)
    {
        return NULL;
    }

    AutoObjLock(this);

    CResponsePool *pPool = new CResponsePool();
    if (!pPool)
    {
        return NULL;
    }

    DWORD dwRc = pPool->Init(m_piManager, owner, count);
    if (dwRc != SUCCESS)
    {
        delete pPool;
        return NULL;
    }

    (void)m_requests.insert(pPool);
    return pPool;
}

/*******************************************************
  函 数 名: CResponse::DestroyPool
  描    述: 删除缓冲区
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CResponse::DestroyPool(IResponse::IPool *pool)
{
    if (!pool)
    {
        return;
    }

    AutoObjLock(this);

    CResponsePool *pPool = (CResponsePool *)pool;
    (void)m_requests.erase(pPool);
    delete pPool;
}

