/// -------------------------------------------------
/// session_main.cpp : 会话管理实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "session_main.h"
#include "Factory_if.h"
#include "Manager_if.h"


/// -------------------------------------------------
/// 实现类厂
/// -------------------------------------------------
DCOP_IMPLEMENT_FACTORY(CSession, "session")

/// -------------------------------------------------
/// 实现基类
/// -------------------------------------------------
DCOP_IMPLEMENT_INSTANCE(CSession)
    DCOP_IMPLEMENT_INTERFACE(ISession)
    DCOP_IMPLEMENT_INTERFACE(IObject)
    DCOP_IMPLEMENT_INTERFACE(Instance)
DCOP_IMPLEMENT_INSTANCE_END

/// -------------------------------------------------
/// 实现对象类
/// -------------------------------------------------
DCOP_IMPLEMENT_IOBJECT(CSession)
    DCOP_IMPLEMENT_IDENTIFY_DYNAMIC("name", "id")
    DCOP_IMPLEMENT_CONFIG_THREADSAFE("threadsafe")
DCOP_IMPLEMENT_IOBJECT_END

/// -------------------------------------------------
/// 实现属性
/// -------------------------------------------------
IMPLEMENT_ATTRIBUTE(CSession, sessions, CSession::SESS_TABLE_ID, IModel::TYPE_DATA)

/// -------------------------------------------------
/// 实现消息分发
/// -------------------------------------------------
DCOP_IMPLEMENT_IOBJECT_MSG_HANDLE(CSession)
    IMPLEMENT_ATTRIBUTE_MSG_PROC(sessionIndex)
    DCOP_IMPLEMENT_IOBJECT_MSG_PROC(DCOP_MSG_MANAGER_START, OnStart)
    DCOP_IMPLEMENT_IOBJECT_MSG_PROC(DCOP_MSG_MANAGER_FINISH, OnFinish)
    DCOP_IMPLEMENT_IOBJECT_MSG_PROC(DCOP_MSG_SESSION_TIMER_1S, OnTimer1s)
DCOP_IMPLEMENT_IOBJECT_MSG_HANDLE_END

/// -------------------------------------------------
/// 会话表名
/// -------------------------------------------------
const char* CSession::SESS_TABLE_NAME = "sessions";

/// -------------------------------------------------
/// 会话字段描述
/// -------------------------------------------------
IModel::Field CSession::SessFields[] = 
{
    {"session", IModel::KEY_YES, CSession::SESS_ID_TYPE, CSession::SESS_ID_SIZE, 0, 0, 0},
    {"user", IModel::KEY_NO, CSession::SESS_USER_TYPE, CSession::SESS_USER_SIZE, 0, 0, 0},
    {"usergroup", IModel::KEY_NO, CSession::SESS_GROUP_TYPE, CSession::SESS_GROUP_SIZE, 0, 0, 0},
    {"tty", IModel::KEY_NO, CSession::SESS_TTY_TYPE, CSession::SESS_TTY_SIZE, 0, 0, 0},
    {"timer", IModel::KEY_NO, CSession::SESS_TIMER_TYPE, CSession::SESS_TIMER_SIZE, 0, 0, 0},
    {"IP", IModel::KEY_NO, CSession::SESS_IP_TYPE, CSession::SESS_IP_SIZE, 0, 0, 0},
    {"port", IModel::KEY_NO, CSession::SESS_PORT_TYPE, CSession::SESS_PORT_SIZE, 0, 0, 0},
    {"info", IModel::KEY_NO, CSession::SESS_INFO_TYPE, CSession::SESS_INFO_SIZE, 0, 0, 0}
};

/// -------------------------------------------------
/// 关联字段描述
/// -------------------------------------------------
IModel::Relation CSession::SessShips[] = 
{
    {CSession::SESS_USER, DCOP_OBJATTR_USER_TABLE, CSession::RELATION_USER},
};

/// -------------------------------------------------
/// 会话参数描述
/// -------------------------------------------------
DCOP_PARA_NODE CSession::SessParas[] = 
{
    {CSession::SESS_ID, 0, CSession::SESS_ID_TYPE, CSession::SESS_ID_SIZE},
    {CSession::SESS_USER, 0, CSession::SESS_USER_TYPE, CSession::SESS_USER_SIZE},
    {CSession::SESS_GROUP, 0, CSession::SESS_GROUP_TYPE, CSession::SESS_GROUP_SIZE},
    {CSession::SESS_TTY, 0, CSession::SESS_TTY_TYPE, CSession::SESS_TTY_SIZE},
    {CSession::SESS_TIMER, 0, CSession::SESS_TIMER_TYPE, CSession::SESS_TIMER_SIZE},
    {CSession::SESS_IP, 0, CSession::SESS_IP_TYPE, CSession::SESS_IP_SIZE},
    {CSession::SESS_PORT, 0, CSession::SESS_PORT_TYPE, CSession::SESS_PORT_SIZE}
};

/// -------------------------------------------------
/// 会话用户描述
/// -------------------------------------------------
DCOP_PARA_NODE CSession::SessUser[] = 
{
    {CSession::SESS_USER, 0, CSession::SESS_USER_TYPE, CSession::SESS_USER_SIZE},
    {CSession::SESS_GROUP, 0, CSession::SESS_GROUP_TYPE, CSession::SESS_GROUP_SIZE}
};

/// -------------------------------------------------
/// 会话定时器描述
/// -------------------------------------------------
DCOP_PARA_NODE CSession::SessTimer[] = 
{
    {CSession::SESS_TIMER, 0, CSession::SESS_TIMER_TYPE, CSession::SESS_TIMER_SIZE}
};

/// -------------------------------------------------
/// 会话信息描述
/// -------------------------------------------------
DCOP_PARA_NODE CSession::SessInfo[] = 
{
    {CSession::SESS_INFO, 0, CSession::SESS_INFO_TYPE, CSession::SESS_INFO_SIZE}
};

/// -------------------------------------------------
/// IP和端口作为关键字索引
/// -------------------------------------------------
DCOP_PARA_NODE CSession::SessKeyIPPort[] = 
{
    {CSession::SESS_IP, 0, CSession::SESS_IP_TYPE, CSession::SESS_IP_SIZE},
    {CSession::SESS_PORT, 0, CSession::SESS_PORT_TYPE, CSession::SESS_PORT_SIZE}
};

/// -------------------------------------------------
/// SessionID作为关键字索引
/// -------------------------------------------------
DCOP_PARA_NODE CSession::SessKeyID[] = 
{
    {CSession::SESS_ID, 0, CSession::SESS_ID_TYPE, CSession::SESS_ID_SIZE}
};


/*******************************************************
  函 数 名: CSession::BytesChangeRecord
  描    述: 转换记录字节序
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CSession::BytesChangeRecord(NODE *pRec)
{
    const BYTES_CHANGE_RULE SessRecBORule[] = 
    {
        {SIZE_OF(ISession::NODE, SessID), OFFSET_OF(ISession::NODE, SessID)},
        {SIZE_OF(ISession::NODE, UserID), OFFSET_OF(ISession::NODE, UserID)},
        {SIZE_OF(ISession::NODE, UserGroup), OFFSET_OF(ISession::NODE, UserGroup)},
        {SIZE_OF(ISession::NODE, TTY), OFFSET_OF(ISession::NODE, TTY)},
        {SIZE_OF(ISession::NODE, Port), OFFSET_OF(ISession::NODE, Port)}
    };
    Bytes_ChangeOrderByRule(SessRecBORule, ARRAY_SIZE(SessRecBORule), pRec, sizeof(NODE));
}

/*******************************************************
  函 数 名: CSession::CSession
  描    述: CSession构造函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CSession::CSession(Instance *piParent, int argc, char **argv) : 
    m_sessions(IData::TYPE_MEM)
{
    m_piModel = 0;
    m_piData = 0;

    m_piDispatch = 0;
    m_piNotify = 0;
    m_pNotifyPool = 0;

    m_piTimer = 0;
    m_hTimer1s = 0;
    m_pTimerWheel = 0;

    m_piSecure = 0;

    DCOP_CONSTRUCT_INSTANCE(piParent);
    DCOP_CONSTRUCT_IOBJECT(argc, argv);
}

/*******************************************************
  函 数 名: CSession::~CSession
  描    述: CSession析构函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CSession::~CSession()
{
    Fini();

    DCOP_DESTRUCT_IOBJECT();
    DCOP_DESTRUCT_INSTANCE();
}

/*******************************************************
  函 数 名: CSession::Init
  描    述: 初始化入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CSession::Init(IObject *root, int argc, void **argv)
{
    if (!root)
    {
        return FAILURE;
    }

    AutoObjLock(this);

    /// 查询对象
    DCOP_QUERY_OBJECT_START(root)
        DCOP_QUERY_OBJECT_ITEM(IModel,       DCOP_OBJECT_MODEL,      m_piModel)
        DCOP_QUERY_OBJECT_ITEM(IData,        DCOP_OBJECT_DATA,       m_piData)
        DCOP_QUERY_OBJECT_ITEM(IDispatch,    DCOP_OBJECT_DISPATCH,   m_piDispatch)
        DCOP_QUERY_OBJECT_ITEM(INotify,      DCOP_OBJECT_NOTIFY,     m_piNotify)
        DCOP_QUERY_OBJECT_ITEM(ITimer,       DCOP_OBJECT_TIMER,      m_piTimer)
        DCOP_QUERY_OBJECT_ITEM(ISecure,      DCOP_OBJECT_SECURE,     m_piSecure)
    DCOP_QUERY_OBJECT_END

    /// 创建缓冲池
    DWORD adwEvents[] = 
    {
        DCOP_OBJATTR_SESSION_TABLE
    };
    m_pNotifyPool = m_piNotify->CreatePool(this, adwEvents, ARRAY_SIZE(adwEvents));
    if (!m_pNotifyPool)
    {
        return FAILURE;
    }

    /// 初始化属性
    INIT_ATTRIBUTE_START(sessionIndex, m_piDispatch, m_pNotifyPool)
        INIT_ATTRIBUTE_MEMBER(sessions)
    INIT_ATTRIBUTE_END

    /// 设置数据对象
    m_sessions.SetDataObj(m_piData);

    /// 启动定时器
    m_pTimerWheel = ITimer::IWheel::CreateInstance(WHEEL_S_SEC_ID, 
                        WHEEL_S_SEC_SLOT_COUNT, 
                        NULL, NULL, 
                        WHEEL_S_HASH_BASE, 
                        OnWheelTimeout, this);
    if (!m_pTimerWheel)
    {
        return FAILURE;
    }

    return SUCCESS;
}

/*******************************************************
  函 数 名: CSession::Fini
  描    述: 完成时入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CSession::Fini()
{
    if (m_pTimerWheel)
    {
        delete m_pTimerWheel;
        m_pTimerWheel = 0;
    }

    if (m_piNotify && m_pNotifyPool)
    {
        m_piNotify->DestroyPool(m_pNotifyPool);
        m_pNotifyPool = 0;
    }

    DCOP_RELEASE_INSTANCE(m_piSecure);
    DCOP_RELEASE_INSTANCE(m_piTimer);
    DCOP_RELEASE_INSTANCE(m_piNotify);
    DCOP_RELEASE_INSTANCE(m_piDispatch);
    DCOP_RELEASE_INSTANCE(m_piData);
    DCOP_RELEASE_INSTANCE(m_piModel);
}

/*******************************************************
  函 数 名: CSession::OnStart
  描    述: 开始运行时
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CSession::OnStart(objMsg *msg)
{
    DWORD dwRc = InitModelData();
    CHECK_ERRCODE(dwRc, "Init Model Data");

    if (m_piTimer)
    {
        m_hTimer1s = m_piTimer->Start(ITimer::TYPE_LOOP, DCOP_MSG_SESSION_TIMER_1S, 1000, this);
    }

    if (m_piSecure)
    {
        ISecure::Node node[] = 
        {
            {m_sessions.GetAttribute()->GetID(), 
                0, 0, 0, 0, 0, 0, 0, 0, 0, 
                DCOP_GROUP_ADMINISTRATOR},
        };
        dwRc = m_piSecure->RegRule(node, ARRAY_SIZE(node));
        CHECK_ERRCODE(dwRc, "Reg Sceure Rule");
    }
}

/*******************************************************
  函 数 名: CSession::OnFinish
  描    述: 结束运行时
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CSession::OnFinish(objMsg *msg)
{
    if (m_piTimer && m_hTimer1s)
    {
        m_piTimer->Stop(m_hTimer1s);
        m_hTimer1s = 0;
    }

    (void)m_sessions.Destroy();
}

/*******************************************************
  函 数 名: CSession::OnTimer1s
  描    述: 1S定时
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CSession::OnTimer1s(objMsg *msg)
{
    if (m_pTimerWheel)
    {
        m_pTimerWheel->OnTick();
    }
}

/*******************************************************
  函 数 名: CSession::CreateSession
  描    述: 创建会话
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CSession::CreateSession(DWORD dwUserID,
                        DWORD dwUserGroup,
                        DWORD dwTTY,
                        DWORD dwRemoteIP,
                        WORD wRemotePort,
                        DWORD &rdwSessionID)
{
    /// 启动定时器
    TimerValue timerValue = {0, dwTTY};
    ITimer::Handle hTimer = ITimer::IWheel::Alloc(&timerValue, sizeof(timerValue));
    if (!hTimer)
    {
        return FAILURE;
    }

    AutoObjLock(this);

    DWORD dwRc = InsertToWheel(SESS_TIMEOUT, hTimer);
    if (dwRc != SUCCESS)
    {
        ITimer::IWheel::Free(hTimer);
        return dwRc;
    }

    /// 添加记录
    NODE addSessData = 
    {
        0, dwUserID, dwUserGroup, dwTTY, *hTimer, dwRemoteIP, wRemotePort
    };
    BytesChangeRecord(&addSessData);

    dwRc = m_sessions.AddRecord(SessParas, ARRAY_SIZE(SessParas), 
                        &addSessData, sizeof(addSessData));
    if (dwRc != SUCCESS)
    {
        return dwRc;
    }

    /// 根据IP和端口查找记录
    dwRc = FindSession(dwRemoteIP, wRemotePort, addSessData);
    if (dwRc != SUCCESS)
    {
        return dwRc;
    }

    timerValue.m_dwSessID = addSessData.SessID;
    ITimer::IWheel::SetValue(hTimer, &timerValue, sizeof(timerValue));

    rdwSessionID = addSessData.SessID;
    return SUCCESS;
}

/*******************************************************
  函 数 名: CSession::DeleteSession
  描    述: 删除会话
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CSession::DeleteSession(DWORD dwSessionID)
{
    AutoObjLock(this);

    NODE rNode;
    DWORD dwRc = GetSession(dwSessionID, rNode);
    if (dwRc != SUCCESS)
    {
        return dwRc;
    }

    BYTE condData[4];
    Bytes_SetDword(condData, dwSessionID);

    ITimer::Handle hTimer = ITimer::IWheel::GetHandle(&rNode.Timer);
    if (hTimer)
    {
        /// 停掉定时器
        DelFromWheel(hTimer);

        /// 为防止定时器句柄在事件中被重用，故先删除定时器
        /// 使用iData原生API，防止删除定时器时触发上报事件
        (void)m_piData->EditRecord(m_sessions, 
                            DCOP_CONDITION_ONE, 
                            SessKeyID, ARRAY_SIZE(SessKeyID), 
                            condData, sizeof(condData), 
                            SessTimer, ARRAY_SIZE(SessTimer), 
                            hTimer, sizeof(*hTimer));

        /// 删除定时器句柄
        ITimer::IWheel::Free(hTimer);
    }

    return m_sessions.DelRecord(DCOP_CONDITION_ONE, 
                        SessKeyID, ARRAY_SIZE(SessKeyID), 
                        condData, sizeof(condData));
}

/*******************************************************
  函 数 名: CSession::UpdateSession
  描    述: 更新会话
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CSession::UpdateSession(DWORD dwSessionID)
{
    NODE rNode;

    AutoObjLock(this);

    DWORD dwRc = GetSession(dwSessionID, rNode);
    if (dwRc != SUCCESS)
    {
        return dwRc;
    }

    BYTE condData[4];
    Bytes_SetDword(condData, dwSessionID);

    ITimer::Handle hTimer = ITimer::IWheel::GetHandle(&rNode.Timer);
    if (!hTimer)
    {
        return FAILURE;
    }

    (void)DelFromWheel(hTimer);
    dwRc = InsertToWheel(SESS_TIMEOUT, hTimer);
    if (dwRc != SUCCESS)
    {
        return dwRc;
    }

    return m_piData->EditRecord(m_sessions, 
                        DCOP_CONDITION_ONE, 
                        SessKeyID, ARRAY_SIZE(SessKeyID), 
                        condData, sizeof(condData), 
                        SessTimer, ARRAY_SIZE(SessTimer), 
                        hTimer, sizeof(*hTimer));
}

/*******************************************************
  函 数 名: CSession::UpdateUserID
  描    述: 更新用户
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CSession::UpdateUserID(DWORD dwSessionID,
                        DWORD dwUserID,
                        DWORD dwUserGroup)
{
    BYTE condData[4];
    Bytes_SetDword(condData, dwSessionID);

    BYTE reqData[8];
    Bytes_SetDword(reqData, dwUserID);
    Bytes_SetDword(reqData + 4, dwUserGroup);

    AutoObjLock(this);

    return m_sessions.EditRecord(DCOP_CONDITION_ONE, 
                        SessKeyID, ARRAY_SIZE(SessKeyID), 
                        condData, sizeof(condData), 
                        SessUser, ARRAY_SIZE(SessUser), 
                        reqData, sizeof(reqData));
}

/*******************************************************
  函 数 名: CSession::FindSession
  描    述: 查找会话
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CSession::FindSession(DWORD dwRemoteIP,
                WORD wRemotePort,
                NODE &rNode)
{
    BYTE condData[6];
    *(DWORD *)condData = dwRemoteIP;
    Bytes_SetWord(condData + 4, wRemotePort);

    DCOP_PARA_NODE *pRspPara = 0;
    DWORD dwRspParaCount = 0;
    CDArray aRspData;

    AutoObjLock(this);

    DWORD dwRc = m_piData->QueryRecord(m_sessions, 
                        DCOP_CONDITION_ONE, 
                        SessKeyIPPort, ARRAY_SIZE(SessKeyIPPort), 
                        condData, sizeof(condData), 
                        SessParas, ARRAY_SIZE(SessParas), 
                        pRspPara, 
                        dwRspParaCount, 
                        aRspData);
    if (dwRc != SUCCESS)
    {
        return dwRc;
    }

    /// 这里不需要响应字段，直接释放
    if (pRspPara) DCOP_Free(pRspPara);

    if (aRspData.Count() == 0)
    {
        return FAILURE;
    }

    /// 只取第一条记录
    NODE *pNode = (NODE *)aRspData.Pos(0);
    if (!pNode)
    {
        return FAILURE;
    }

    BytesChangeRecord(pNode);
    rNode = *pNode;

    return SUCCESS;
}

/*******************************************************
  函 数 名: CSession::GetSession
  描    述: 获取会话
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CSession::GetSession(DWORD dwSessionID, NODE &rNode)
{
    BYTE condData[4];
    Bytes_SetDword(condData, dwSessionID);

    DCOP_PARA_NODE *pRspPara = 0;
    DWORD dwRspParaCount = 0;
    CDArray aRspData;

    AutoObjLock(this);

    DWORD dwRc = m_piData->QueryRecord(m_sessions, 
                        DCOP_CONDITION_ONE, 
                        SessKeyID, ARRAY_SIZE(SessKeyID), 
                        condData, sizeof(condData), 
                        SessParas, ARRAY_SIZE(SessParas), 
                        pRspPara, 
                        dwRspParaCount, 
                        aRspData);
    if (dwRc != SUCCESS)
    {
        return dwRc;
    }

    /// 这里不需要响应字段，直接释放
    if (pRspPara) DCOP_Free(pRspPara);

    if (aRspData.Count() == 0)
    {
        return FAILURE;
    }

    /// 只取第一条记录
    NODE *pNode = (NODE *)aRspData.Pos(0);
    if (!pNode)
    {
        return FAILURE;
    }

    BytesChangeRecord(pNode);
    rNode = *pNode;

    return SUCCESS;
}

/*******************************************************
  函 数 名: CSession::SetSessionInfo
  描    述: 设置会话信息
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CSession::SetSessionInfo(DWORD dwSessionID,
                        char szInfo[INFO_SIZE])
{
    BYTE condData[4];
    Bytes_SetDword(condData, dwSessionID);

    AutoObjLock(this);

    return m_sessions.EditRecord(DCOP_CONDITION_ONE, 
                        SessKeyID, ARRAY_SIZE(SessKeyID), 
                        condData, sizeof(condData), 
                        SessInfo, ARRAY_SIZE(SessInfo), 
                        szInfo, INFO_SIZE);
}

/*******************************************************
  函 数 名: CSession::GetSessionInfo
  描    述: 获取会话信息
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CSession::GetSessionInfo(DWORD dwSessionID,
                        char szInfo[INFO_SIZE])
{
    BYTE condData[4];
    Bytes_SetDword(condData, dwSessionID);

    DCOP_PARA_NODE *pRspPara = 0;
    DWORD dwRspParaCount = 0;
    CDArray aRspData;

    AutoObjLock(this);

    DWORD dwRc = m_piData->QueryRecord(m_sessions, 
                        DCOP_CONDITION_ONE, 
                        SessKeyID, ARRAY_SIZE(SessKeyID), 
                        condData, sizeof(condData), 
                        SessInfo, ARRAY_SIZE(SessInfo), 
                        pRspPara, 
                        dwRspParaCount, 
                        aRspData);
    if (dwRc != SUCCESS)
    {
        return dwRc;
    }

    /// 这里不需要响应字段，直接释放
    if (pRspPara) DCOP_Free(pRspPara);

    if (aRspData.Count() == 0)
    {
        return FAILURE;
    }

    /// 只取第一条记录
    char *pInfo = (char *)aRspData.Pos(0);
    if (!pInfo)
    {
        return FAILURE;
    }

    (void)strncpy(szInfo, pInfo, INFO_SIZE);
    szInfo[INFO_SIZE - 1] = '\0';

    return SUCCESS;
}

/*******************************************************
  函 数 名: CSession::InitModelData
  描    述: 初始化模型和数据
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CSession::InitModelData()
{
    if (!m_piModel || !m_piData)
    {
        return FAILURE;
    }

    /// 注册属性建模
    DWORD dwRc = m_piModel->RegTable((char *)SESS_TABLE_NAME, 
                        ID(), 
                        m_sessions.GetAttribute()->GetID(), 
                        m_sessions.GetModelType(), 
                        SessFields, 
                        ARRAY_SIZE(SessFields), 
                        SESS_REC_DEF_COUNT, 
                        SessShips, 
                        ARRAY_SIZE(SessShips));
    if (dwRc != SUCCESS)
    {
        return dwRc;
    }

    /// 创建数据
    dwRc = m_sessions.Create();
    if (dwRc != SUCCESS)
    {
        return dwRc;
    }

    /// 添加IP和端口作为关键字索引
    dwRc = m_piData->AddKeyIdx(m_sessions, SessKeyIPPort, ARRAY_SIZE(SessKeyIPPort));
    if (dwRc != SUCCESS)
    {
        return dwRc;
    }

    return SUCCESS;
}

/*******************************************************
  函 数 名: CSession::OnWheelTimeout
  描    述: 时间轮超时时
            [只要是轮子上的节点超时时就是该节点保存的会话超时了]
            [然后对该节点句柄里的会话所在的终端模块发送超时消息]
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CSession::OnWheelTimeout(ITimer::Handle handle, void *para)
{
    CSession *pThis = (CSession *)para;
    OSASSERT(pThis != 0);

    IDispatch *pDispatch = pThis->GetDispatch();
    if (!pDispatch)
    {
        return;
    }

    TimerValue *pValue = (TimerValue *)ITimer::IWheel::GetValue(handle);
    if (!pValue)
    {
        return;
    }

    objMsg *pMsg = DCOP_CreateMsg(sizeof(DWORD), DCOP_MSG_SESSION_TIMEOUT, pThis->ID());
    if (!pMsg)
    {
        return;
    }

    DWORD *pdwSessID = (DWORD *)pMsg->GetDataBuf();
    if (!pdwSessID)
    {
        delete pMsg;
        return;
    }

    *pdwSessID = pValue->m_dwSessID;

    pMsg->MsgHead().m_dwDstID = pValue->m_dwTtyID;
    DWORD dwRc = pDispatch->Send(pMsg);
    if (dwRc)
    {
        CHECK_RETCODE(dwRc, STR_FORMAT("Send Timeout Msg To Obj:%d Fail(0x%x)!", pValue->m_dwTtyID, dwRc));
        delete pMsg;
    }

    /// 然后等接入模块主动删除会话
}

/*******************************************************
  函 数 名: CSession::InsertToWheel
  描    述: 插入到定时器轮
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CSession::InsertToWheel(DWORD dwTimeout, ITimer::Handle hTimer)
{
    if (!m_pTimerWheel)
    {
        return FAILURE;
    }

    ITimer::IWheel::SetTimeBase(hTimer, dwTimeout);
    return m_pTimerWheel->Add(hTimer);
}

/*******************************************************
  函 数 名: CSession::DelFromWheel
  描    述: 从定时器轮中删除
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CSession::DelFromWheel(ITimer::Handle hTimer)
{
    if (!m_pTimerWheel)
    {
        return;
    }

    m_pTimerWheel->Del(hTimer);
}

