/// -------------------------------------------------
/// ObjDispatch_main.cpp : 消息分发器对象实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "ObjDispatch_main.h"
#include "Factory_if.h"
#include "Manager_if.h"
#include "BaseID.h"
#include "sock.h"


/// -------------------------------------------------
/// 实现类厂
/// -------------------------------------------------
DCOP_IMPLEMENT_FACTORY(CDispatch, "dispatch")

/// -------------------------------------------------
/// 实现基类
/// -------------------------------------------------
DCOP_IMPLEMENT_INSTANCE(CDispatch)
    DCOP_IMPLEMENT_INTERFACE(IDispatch)
    DCOP_IMPLEMENT_INTERFACE(IObject)
    DCOP_IMPLEMENT_INTERFACE(Instance)
DCOP_IMPLEMENT_INSTANCE_END

/// -------------------------------------------------
/// 实现对象类
/// -------------------------------------------------
DCOP_IMPLEMENT_IOBJECT(CDispatch)
    DCOP_IMPLEMENT_IDENTIFY_DYNAMIC("name", "id")
    DCOP_IMPLEMENT_CONFIG_THREADSAFE("threadsafe")
DCOP_IMPLEMENT_IOBJECT_END


/*******************************************************
  函 数 名: CDispatch::CDispatch
  描    述: CDispatch构造函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CDispatch::CDispatch(Instance *piParent, int argc, char **argv)
{
    DCOP_CONSTRUCT_INSTANCE(piParent);
    DCOP_CONSTRUCT_IOBJECT(argc, argv);

    m_dwMsgHookFlag = DCOP_CLOSE;
}

/*******************************************************
  函 数 名: CDispatch::~CDispatch
  描    述: CDispatch析构函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CDispatch::~CDispatch()
{
    DCOP_DESTRUCT_IOBJECT();
    DCOP_DESTRUCT_INSTANCE();
}

/*******************************************************
  函 数 名: CDispatch::Init
  描    述: 初始化入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CDispatch::Init(IObject *root, int argc, void **argv)
{
    if (!root)
    {
        return FAILURE;
    }

    AutoObjLock(this);

    ISchedule *piSchedule = 0;
    DCOP_QUERY_OBJECT(ISchedule, DCOP_OBJECT_SCHEDULE, root, piSchedule);
    if (!piSchedule)
    {
        return FAILURE;
    }

    m_pInfLayer = piSchedule;

    return SUCCESS;
}

/*******************************************************
  函 数 名: CDispatch::Fini
  描    述: 完成时入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CDispatch::Fini()
{
}

/*******************************************************
  函 数 名: CDispatch::Dump
  描    述: DUMP入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CDispatch::Dump(LOG_PRINT logPrint, LOG_PARA logPara, int argc, void **argv)
{
}

/*******************************************************
  函 数 名: CDispatch::GetMTU
  描    述: 获取MTU
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CDispatch::GetMTU()
{
    /// 通道的MTU包括数据区和消息头部，这里应减去头部大小
    return OSSOCK_DEFAULT_MTU - OSMsgHeader::GetHeaderSize();
}

/*******************************************************
  函 数 名: CDispatch::Send
  描    述: 发送消息
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CDispatch::Send(objMsg *message)
{
    AutoObjLock(this);

    ISchedule *pSchedule = dynamic_cast<ISchedule *>((IObject *)m_pInfLayer);
    if (!pSchedule || !message)
    {
        return FAILURE;
    }

    return pSchedule->Join(message);
}

/*******************************************************
  函 数 名: CDispatch::SendAndWait
  描    述: 发送后等待回应
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CDispatch::SendAndWait(objMsg *request, objMsg **response, DWORD waittime)
{
    AutoObjLock(this);

    return 0;
}

/*******************************************************
  函 数 名: CDispatch::OpenMsgHook
  描    述: 打开消息钩子
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CDispatch::OpenMsgHook()
{
    AutoObjLock(this);

    m_dwMsgHookFlag = DCOP_OPEN;
}

/*******************************************************
  函 数 名: CDispatch::CloseMsgHook
  描    述: 关闭消息钩子
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CDispatch::CloseMsgHook()
{
    AutoObjLock(this);

    m_dwMsgHookFlag = DCOP_CLOSE;
}

/*******************************************************
  函 数 名: CDispatch::GetMsgHookFlag
  描    述: 获取消息钩子
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CDispatch::GetMsgHookFlag()
{
    AutoObjLock(this);

    return m_dwMsgHookFlag;
}

/*******************************************************
  函 数 名: CDispatch::StackRecvMsg
  描    述: 分布式协议栈接收
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CDispatch::StackRecvMsg(
                    DWORD dwSrcNodeID, 
                    void *pMsgBuf, 
                    DWORD dwBufLength, 
                    void *pUserArg
                    )
{
    return 0;
}

