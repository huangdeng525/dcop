/// -------------------------------------------------
/// secure_main.cpp : 安全管理实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "secure_main.h"
#include "Factory_if.h"
#include "Manager_if.h"


/// -------------------------------------------------
/// 实现类厂
/// -------------------------------------------------
DCOP_IMPLEMENT_FACTORY(CSecure, "secure")

/// -------------------------------------------------
/// 实现基类
/// -------------------------------------------------
DCOP_IMPLEMENT_INSTANCE(CSecure)
    DCOP_IMPLEMENT_INTERFACE(ISecure)
    DCOP_IMPLEMENT_INTERFACE(IObject)
    DCOP_IMPLEMENT_INTERFACE(Instance)
DCOP_IMPLEMENT_INSTANCE_END

/// -------------------------------------------------
/// 实现对象类
/// -------------------------------------------------
DCOP_IMPLEMENT_IOBJECT(CSecure)
    DCOP_IMPLEMENT_IDENTIFY_DYNAMIC("name", "id")
    DCOP_IMPLEMENT_CONFIG_THREADSAFE("threadsafe")
DCOP_IMPLEMENT_IOBJECT_END

/// -------------------------------------------------
/// 实现消息分发
/// -------------------------------------------------
DCOP_IMPLEMENT_IOBJECT_MSG_HANDLE(CSecure)
    DCOP_IMPLEMENT_IOBJECT_MSG_PROC(DCOP_MSG_MANAGER_START, OnStart)
    DCOP_IMPLEMENT_IOBJECT_MSG_PROC(DCOP_MSG_MANAGER_FINISH, OnFinish)
DCOP_IMPLEMENT_IOBJECT_MSG_HANDLE_END


/*******************************************************
  函 数 名: CSecure::CSecure
  描    述: CSecure构造函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CSecure::CSecure(Instance *piParent, int argc, char **argv)
{
    m_piControl = 0;

    DCOP_CONSTRUCT_INSTANCE(piParent);
    DCOP_CONSTRUCT_IOBJECT(argc, argv);
}

/*******************************************************
  函 数 名: CSecure::~CSecure
  描    述: CSecure析构函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CSecure::~CSecure()
{
    Fini();

    DCOP_DESTRUCT_IOBJECT();
    DCOP_DESTRUCT_INSTANCE();
}

/*******************************************************
  函 数 名: CSecure::Init
  描    述: 初始化入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CSecure::Init(IObject *root, int argc, void **argv)
{
    if (!root)
    {
        return FAILURE;
    }

    AutoObjLock(this);

    /// 查询对象
    DCOP_QUERY_OBJECT_START(root)
        DCOP_QUERY_OBJECT_ITEM(IControl,     DCOP_OBJECT_CONTROL,    m_piControl)
    DCOP_QUERY_OBJECT_END

    return SUCCESS;
}

/*******************************************************
  函 数 名: CSecure::Fini
  描    述: 完成时入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CSecure::Fini()
{
    DCOP_RELEASE_INSTANCE(m_piControl);
}

/*******************************************************
  函 数 名: CSecure::Dump
  描    述: Dump
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CSecure::Dump(LOG_PRINT logPrint, LOG_PARA logPara, int argc, void **argv)
{
}

/*******************************************************
  函 数 名: CSecure::OnStart
  描    述: 开始运行时
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CSecure::OnStart(objMsg *msg)
{
    /// 注册安全控制
    if (m_piControl)
    {
        IControl::Node node[] = 
        {
            {0, DCOP_CTRL_NULL, DCOP_REQ, DCOP_GROUP_VISITOR, 0, InputCtrl},
        };
        DWORD dwRc = m_piControl->RegCtrlNode(this, 
                        DCOP_OBJECT_ACCESS, 
                        node, 
                        ARRAY_SIZE(node));
        CHECK_ERRCODE(dwRc, "Reg Ctrl Node To Access");
    }
}

/*******************************************************
  函 数 名: CSecure::OnFinish
  描    述: 结束运行时
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CSecure::OnFinish(objMsg *msg)
{
}

/*******************************************************
  函 数 名: CSecure::InputCtrl
  描    述: 输入控制
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CSecure::InputCtrl(objMsg *pInput,
                        objMsg *&pOutput,
                        bool &bContinue,
                        IObject *piCtrler)
{
    /// 获取消息头
    CDArray aSessHeads;
    IObjectMember::GetMsgHead(pInput->GetDataBuf(), pInput->GetDataLen(), &aSessHeads, 0, 0, 0, 0);
    if (!aSessHeads.Count())
    {
        return FAILURE;
    }

    /// 获取会话头(只获取第一个会话头)
    DCOP_SESSION_HEAD *pSessionHead = (DCOP_SESSION_HEAD *)aSessHeads.Pos(0);
    if (!pSessionHead)
    {
        return FAILURE;
    }

    PrintLog(STR_FORMAT("group:%d\r\n session:%d\r\n user:%d\r\n tty:%d\r\n attribute:0x%x\r\n",
                        pSessionHead->m_group,
                        pSessionHead->m_session,
                        pSessionHead->m_user,
                        pSessionHead->m_tty,
                        pSessionHead->m_attribute), 
                        PrintToConsole, 0);

    return SUCCESS;
}

