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

/// -------------------------------------------------
/// 实现配置项
/// -------------------------------------------------
IMPLEMENT_CONFIG_ITEM(CDispatch, hookFlag, "k", "hookflag", "Msg Hook Flag", CArgCfgType::TYPE_VALUE, false)
IMPLEMENT_CONFIG_ITEM(CDispatch, lenMax, "l", "lenmax", "Output Len Max", CArgCfgType::TYPE_VALUE, false)
IMPLEMENT_CONFIG_ITEM(CDispatch, srcID, "s", "srcid", "Msg Src ID", CArgCfgType::TYPE_VALUE, false)
IMPLEMENT_CONFIG_ITEM(CDispatch, dstID, "d", "dstid", "Msg Dst ID", CArgCfgType::TYPE_VALUE, false)
IMPLEMENT_CONFIG_ITEM(CDispatch, logParaLen, "lpl", "logparalen", "Log Para Len", CArgCfgType::TYPE_VALUE, false)


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

    INIT_CONFIG_START(m_cfgTable)
        INIT_CONFIG_ITEM_VAL(hookFlag, DCOP_CLOSE)
        INIT_CONFIG_ITEM_VAL(lenMax, 0)
        INIT_CONFIG_ITEM_VAL(srcID, 0)
        INIT_CONFIG_ITEM_VAL(dstID, 0)
        INIT_CONFIG_ITEM_VAL(logParaLen, 0)
    INIT_CONFIG_END

    m_logPrint = 0;
    m_logPara = 0;
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

    if (m_logPara)
    {
        DCOP_Free(m_logPara);
        m_logPara = 0;
    }

    m_logParaLen = 0;
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
    AutoObjLock(this);

    if (m_logPara)
    {
        DCOP_Free(m_logPara);
        m_logPara = 0;
    }

    m_logParaLen = 0;

    m_cfgTable.Cfg(argc, (char **)argv);
    m_logPrint = logPrint;
    if (!m_logPrint)
    {
        return;
    }

    if (!m_logParaLen)
    {
        return;
    }

    m_logPara = (LOG_PARA)DCOP_Malloc(m_logParaLen);
    if (!m_logPara)
    {
        return;
    }

    (void)memcpy(m_logPara, logPara, m_logParaLen);

    m_logPrint(STR_FORMAT("  Hook Flag: %d;\r\n  Len Max: %d;\r\n  Src ID: %d;\r\n  Dst ID: %d!\r\n", 
                        (DWORD)m_hookFlag, (DWORD)m_lenMax, (DWORD)m_srcID, (DWORD)m_dstID), logPara);
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

    if (m_hookFlag && m_logPrint)
    {
        Hook(message);
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
  函 数 名: CDispatch::Hook
  描    述: 消息监控
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CDispatch::Hook(objMsg *message)
{
    if (!message || !m_hookFlag || !m_logPrint)
    {
        return;
    }

    DWORD dwSrcID = message->GetSrcID();
    DWORD dwDstID = message->GetDstID();

    if (m_srcID && (dwSrcID != m_srcID))
    {
        return;
    }

    if (m_dstID && (dwDstID != m_dstID))
    {
        return;
    }

    const char *pcszSrcName = "Null";
    IObject *piSrcObj = 0;
    DCOP_QUERY_OBJECT(IObject, dwSrcID, Parent(), piSrcObj);
    if (piSrcObj) pcszSrcName = piSrcObj->Name();

    const char *pcszDstName = "Null";
    IObject *piDstObj = 0;
    DCOP_QUERY_OBJECT(IObject, dwDstID, Parent(), piDstObj);
    if (piDstObj) pcszDstName = piDstObj->Name();

    DWORD dwDispLen = message->GetDataLen();
    if (m_lenMax && (dwDispLen > m_lenMax)) dwDispLen = m_lenMax;

    PrintBuffer(STR_FORMAT("[MSG HOOK] '%s'(%d) -> '%s'(%d) Len:%d", 
                    pcszSrcName, dwSrcID, pcszDstName, dwDstID, message->GetDataLen()), 
                    message->GetDataBuf(), dwDispLen, m_logPrint, m_logPara);
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

