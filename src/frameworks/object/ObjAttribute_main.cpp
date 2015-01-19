/// -------------------------------------------------
/// ObjAttribute_main.cpp : 对象属性实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "ObjAttribute_main.h"
#include "cpu/bytes.h"
#include "sock.h"


/*******************************************************
  函 数 名: IObjectMember::Action
  描    述: Action
  输    入: pMsg            - 原始消息
            sessionHead     - 从原始消息中解析的会话消息头
            aCondHeads      - 从原始消息中解析的条件消息头
            pReqPara        - 从原始消息中解析的请求参数列表
            dwReqParaCount  - 从原始消息中解析的请求参数个数
            pReqData        - 从原始消息中解析的请求数据
            dwReqDataLen    - 从原始消息中解析的请求数据长度
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void IObjectMember::Action(objMsg *pMsg, 
                        const DCOP_SESSION_HEAD &sessionHead, 
                        const CDArray &aCondHeads, 
                        DCOP_PARA_NODE *pReqPara, 
                        DWORD dwReqParaCount, 
                        void *pReqData, 
                        DWORD dwReqDataLen)
{
    if (!pMsg)
    {
        CHECK_RETCODE(FAILURE, "Session Msg Error!");
        return;
    }

    CAttribute *pAttr = GetAttribute();
    if (!pAttr)
    {
        CHECK_RETCODE(FAILURE, "Attr Type Error!");
        return;
    }

    /// 准备响应数据
    DWORD dwRc = SUCCESS;
    CAttribute::PACK_RSP_HEAD rspHead;
    CDArray aRspData;

    /// 根据会话控制类型进行操作
    switch (sessionHead.m_ctrl)
    {
        case DCOP_CTRL_SET:
            dwRc = Set(pReqData, dwReqDataLen);
            break;
        case DCOP_CTRL_GET:
        {
            DWORD dwRspDataLen = 0;
            void *pRspData = Get(dwRspDataLen);
            if (!pRspData)
            {
                dwRc = FAILURE;
                break;
            }
            aRspData.SetNodeSize(dwRspDataLen);
            dwRc = aRspData.Append(pRspData);
            if (dwRc != SUCCESS)
            {
                break;
            }
            rspHead.m_paraLen = (WORD)aRspData.Size();
        }
            break;
        default:
            dwRc = FAILURE;
            break;
    }

    /// 响应处理结果
    rspHead.m_retCode = dwRc;
    rspHead.m_recordCount = 1;
    Notify((DCOP_MSG_HEAD *)&rspHead, aRspData, (DCOP_SESSION_HEAD *)&sessionHead, pMsg->GetSrcID());
}

/*******************************************************
  函 数 名: IObjectMember::Notify
  描    述: Notify
  输    入: pHead           - 发送的消息头
            aData           - 发送的数据列表
            pSessionHead    - 之前的会话信息(事件无此头)
            dwDstID         - 发送的目的信息(事件无此头)
            pPara           - 发送的参数列表(可无)
            dwParaCount     - 发送的参数个数(可无)
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void IObjectMember::Notify(DCOP_MSG_HEAD *pHead, 
                        const CDArray &aData, 
                        DCOP_SESSION_HEAD *pSessionHead, 
                        DWORD dwDstID, 
                        DCOP_PARA_NODE *pPara, 
                        DWORD dwParaCount)
{
    if (!pHead)
    {
        CHECK_RETCODE(FAILURE, "Notify Head Error!");
        return;
    }

    IObject *pOwner = GetOwner();
    CAttribute *pAttr = GetAttribute();
    IDispatch *pDispatch = GetDispatch();
    if (!pOwner || !pAttr || !pDispatch)
    {
        CHECK_RETCODE(FAILURE, "Obj Attr Error!");
        return;
    }

    CAttribute::PACK_SESS_HEAD sessHead(pSessionHead);
    sessHead.m_attribute = pAttr->GetID();
    DWORD dwMsgType;

    if (pHead->m_headType == DCOP_MSG_HEAD_EVENT)
    {
        /// 是事件的话，需要先判断是否有人订阅
        INotify::IPool *pPool = GetNotifyPool();

        if (!pPool || (pPool->OnPublish(sessHead.m_attribute) != SUCCESS))
            return;

        /// 事件的固定目的地是notify组件
        dwDstID = DCOP_OBJECT_NOTIFY;

        /// 设置事件标识
        if (!pSessionHead) sessHead.m_ctrl = DCOP_CTRL_EVENT;
        sessHead.m_ack = DCOP_EVT;
        dwMsgType = DCOP_MSG_OBJECT_EVENT;
    }
    else
    {
        /// 设置响应标识
        sessHead.m_ack = DCOP_RSP_END;
        dwMsgType = DCOP_MSG_OBJECT_RESPONSE;
    }

    /// 组装消息并发往目的地
    DWORD dwRc = PackMsg(pDispatch, pOwner->ID(), dwDstID, dwMsgType, 
                        &sessHead, pHead, 
                        pPara, dwParaCount, 
                        aData);

    CHECK_RETCODE(dwRc, "Send Notify Msg Error!");
}

/*******************************************************
  函 数 名: CObjectMember<IData*>::Action
  描    述: Action
  输    入: pMsg            - 原始消息
            sessionHead     - 从原始消息中解析的会话消息头
            aCondHeads      - 从原始消息中解析的条件消息头
            pReqPara        - 从原始消息中解析的请求参数列表
            dwReqParaCount  - 从原始消息中解析的请求参数个数
            pReqData        - 从原始消息中解析的请求数据
            dwReqDataLen    - 从原始消息中解析的请求数据长度
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CObjectMember<IData*>::Action(objMsg *pMsg, 
                        const DCOP_SESSION_HEAD &sessionHead, 
                        const CDArray &aCondHeads, 
                        DCOP_PARA_NODE *pReqPara, 
                        DWORD dwReqParaCount, 
                        void *pReqData, 
                        DWORD dwReqDataLen)
{
    if (!pMsg)
    {
        CHECK_RETCODE(FAILURE, "Session Msg Error!");
        return;
    }

    CAttribute *pAttr = GetAttribute();
    if (!pAttr || (pAttr->GetType() != IModel::TYPE_DATA))
    {
        CHECK_RETCODE(FAILURE, "Attr Type Error!");
        return;
    }

    IData *pData = GetData();
    if (!pData)
    {
        CHECK_RETCODE(FAILURE, "Get Data Error!");
        return;
    }

    /// 准备输入条件
    DWORD dwRc = SUCCESS;
    DCOP_CONDITION_HEAD *pCondHead = (DCOP_CONDITION_HEAD *)aCondHeads.Pos(0);
    void *pCondParaData = (pCondHead)? *(void **)(pCondHead + 1) : NULL;
    DWORD dwCondParaCount = (pCondHead)? pCondHead->m_paraCount : 0;
    DWORD dwCondDataLen = (pCondHead)? pCondHead->m_paraLen : 0;
    CDStream sCondPara(dwCondParaCount * sizeof(DCOP_PARA_NODE));
    DCOP_PARA_NODE *pCondPara = (DCOP_PARA_NODE *)sCondPara.Buffer();
    void *pCondData = NULL;

    /// 准备输出响应
    CAttribute::PACK_RSP_HEAD rspHead;
    DCOP_PARA_NODE *pRspPara = NULL;
    DWORD dwRspParaCount = 0;
    CDArray aRspData;

    /// 调用数据对象进行处理
    switch (sessionHead.m_ctrl)
    {
        case DCOP_CTRL_CREATE:
            dwRc = Create(&sessionHead);
            break;
        case DCOP_CTRL_DESTROY:
            dwRc = Destroy(&sessionHead);
            break;
        case DCOP_CTRL_ADD:
            dwRc = AddRecord(pReqPara, dwReqParaCount, 
                        pReqData, dwReqDataLen, 
                        &sessionHead);
            break;
        case DCOP_CTRL_DEL:
            pCondData = GetMsgParaData(pCondParaData, 
                        dwCondParaCount, dwCondDataLen, 
                        pCondPara);
            dwRc = DelRecord((pCondHead)? pCondHead->m_condition : 0, 
                        pCondPara, dwCondParaCount, 
                        pCondData, dwCondDataLen, 
                        &sessionHead);
            break;
        case DCOP_CTRL_SET:
            pCondData = GetMsgParaData(pCondParaData, 
                        dwCondParaCount, dwCondDataLen, 
                        pCondPara);
            dwRc = EditRecord((pCondHead)? pCondHead->m_condition : 0, 
                        pCondPara, dwCondParaCount, 
                        pCondData, dwCondDataLen, 
                        pReqPara, dwReqParaCount, 
                        pReqData, dwReqDataLen, 
                        &sessionHead);
            break;
        case DCOP_CTRL_GET:
            pCondData = GetMsgParaData(pCondParaData, 
                        dwCondParaCount, dwCondDataLen, 
                        pCondPara);
            if (IsCountReq(pReqPara, dwReqParaCount))
            {
                dwRc = pData->CountRecord(GetDataHandle(), 
                        (pCondHead)? pCondHead->m_condition : 0, 
                        pCondPara, dwCondParaCount, 
                        pCondData, dwCondDataLen, 
                        rspHead.m_recordCount);
            }
            else
            {
                dwRc = pData->QueryRecord(GetDataHandle(), 
                        (pCondHead)? pCondHead->m_condition : 0, 
                        pCondPara, dwCondParaCount, 
                        pCondData, dwCondDataLen, 
                        pReqPara, dwReqParaCount, 
                        pRspPara, dwRspParaCount, 
                        aRspData);
            }
            break;
        default:
            dwRc = FAILURE;
            break;
    }

    /// 响应处理结果
    rspHead.m_retCode = dwRc;
    Notify((DCOP_MSG_HEAD *)&rspHead, aRspData, (DCOP_SESSION_HEAD *)&sessionHead, pMsg->GetSrcID(), pRspPara, dwRspParaCount);
    if (pRspPara) DCOP_Free(pRspPara);
}

/*******************************************************
  函 数 名: CObjectMember<IData*>::Create
  描    述: Create
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CObjectMember<IData*>::Create(const DCOP_SESSION_HEAD *pSessionHead)
{
    CAttribute *pAttr = GetAttribute();
    if (!pAttr || (pAttr->GetType() != IModel::TYPE_DATA))
    {
        return FAILURE;
    }

    IData *pData = GetData();
    if (!pData)
    {
        return FAILURE;
    }

    DWORD dwRc = pData->Create(GetDataType(), pAttr->GetID(), GetOwner(), m_hData);
    if (dwRc != SUCCESS)
    {
        return dwRc;
    }

    /// 上报事件
    CAttribute::PACK_SESS_HEAD sessHead(pSessionHead);
    sessHead.m_ctrl = DCOP_CTRL_CREATE;
    CAttribute::PACK_EVT_HEAD evtHead;
    CDArray aEvtData;

    Notify((DCOP_MSG_HEAD *)&evtHead, aEvtData, &sessHead);

    return SUCCESS;
}

/*******************************************************
  函 数 名: CObjectMember<IData*>::Destroy
  描    述: Destroy
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CObjectMember<IData*>::Destroy(const DCOP_SESSION_HEAD *pSessionHead)
{
    CAttribute *pAttr = GetAttribute();
    if (!pAttr || (pAttr->GetType() != IModel::TYPE_DATA))
    {
        return FAILURE;
    }

    IData *pData = GetData();
    if (!pData)
    {
        return FAILURE;
    }

    DWORD dwRc = pData->Destroy(pAttr->GetID());
    if (dwRc != SUCCESS)
    {
        return dwRc;
    }

    /// 上报事件
    CAttribute::PACK_SESS_HEAD sessHead(pSessionHead);
    sessHead.m_ctrl = DCOP_CTRL_DESTROY;
    CAttribute::PACK_EVT_HEAD evtHead;
    CDArray aEvtData;

    Notify((DCOP_MSG_HEAD *)&evtHead, aEvtData, &sessHead);

    return SUCCESS;
}

/*******************************************************
  函 数 名: CObjectMember<IData*>::AddRecord
  描    述: AddRecord
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CObjectMember<IData*>::AddRecord(DCOP_PARA_NODE *pReqPara, 
                        DWORD dwReqParaCount, 
                        void *pReqData, 
                        DWORD dwReqDataLen, 
                        const DCOP_SESSION_HEAD *pSessionHead)
{
    IData *pData = GetData();
    if (!pData)
    {
        return FAILURE;
    }

    /// 准备输出事件
    CAttribute::PACK_SESS_HEAD sessHead(pSessionHead);
    sessHead.m_ctrl = DCOP_CTRL_ADD;
    CAttribute::PACK_EVT_HEAD evtHead;
    DCOP_PARA_NODE *pEvtPara = NULL;
    DWORD dwEvtParaCount = 0;
    CDArray aEvtData;

    DWORD dwRc = pData->AddRecord(GetDataHandle(), 
                        pReqPara, dwReqParaCount, 
                        pReqData, dwReqDataLen, 
                        &pEvtPara, &dwEvtParaCount, 
                        &aEvtData);
    if (dwRc != SUCCESS)
    {
        return dwRc;
    }

    /// 上报事件
    Notify((DCOP_MSG_HEAD *)&evtHead, aEvtData, &sessHead, 0, pEvtPara, dwEvtParaCount);
    if (pEvtPara) DCOP_Free(pEvtPara);

    return SUCCESS;
}

/*******************************************************
  函 数 名: CObjectMember<IData*>::DelRecord
  描    述: DelRecord
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CObjectMember<IData*>::DelRecord(BYTE byCond, 
                        DCOP_PARA_NODE *pCondPara, 
                        DWORD dwCondParaCount, 
                        void *pCondData, 
                        DWORD dwCondDataLen, 
                        const DCOP_SESSION_HEAD *pSessionHead)
{
    IData *pData = GetData();
    if (!pData)
    {
        return FAILURE;
    }

    /// 准备输出事件
    CAttribute::PACK_SESS_HEAD sessHead(pSessionHead);
    sessHead.m_ctrl = DCOP_CTRL_DEL;
    CAttribute::PACK_EVT_HEAD evtHead;
    DCOP_PARA_NODE *pEvtPara = NULL;
    DWORD dwEvtParaCount = 0;
    CDArray aEvtData;

    DWORD dwRc = pData->DelRecord(GetDataHandle(), 
                        byCond, 
                        pCondPara, dwCondParaCount, 
                        pCondData, dwCondDataLen, 
                        &pEvtPara, &dwEvtParaCount, 
                        &aEvtData);
    if (dwRc != SUCCESS)
    {
        return dwRc;
    }

    /// 上报事件
    Notify((DCOP_MSG_HEAD *)&evtHead, aEvtData, &sessHead, 0, pEvtPara, dwEvtParaCount);
    if (pEvtPara) DCOP_Free(pEvtPara);

    return SUCCESS;
}

/*******************************************************
  函 数 名: CObjectMember<IData*>::EditRecord
  描    述: EditRecord
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CObjectMember<IData*>::EditRecord(BYTE byCond, 
                        DCOP_PARA_NODE *pCondPara, 
                        DWORD dwCondParaCount, 
                        void *pCondData, 
                        DWORD dwCondDataLen, 
                        DCOP_PARA_NODE *pReqPara, 
                        DWORD dwReqParaCount, 
                        void *pReqData, 
                        DWORD dwReqDataLen, 
                        const DCOP_SESSION_HEAD *pSessionHead)
{
    IData *pData = GetData();
    if (!pData)
    {
        return FAILURE;
    }

    /// 准备输出事件
    CAttribute::PACK_SESS_HEAD sessHead(pSessionHead);
    sessHead.m_ctrl = DCOP_CTRL_SET;
    CAttribute::PACK_EVT_HEAD evtHead;
    DCOP_PARA_NODE *pEvtPara = NULL;
    DWORD dwEvtParaCount = 0;
    CDArray aEvtData;

    DWORD dwRc = pData->EditRecord(GetDataHandle(), 
                        byCond, 
                        pCondPara, dwCondParaCount, 
                        pCondData, dwCondDataLen, 
                        pReqPara, dwReqParaCount, 
                        pReqData, dwReqDataLen, 
                        &pEvtPara, &dwEvtParaCount, 
                        &aEvtData);
    if (dwRc != SUCCESS)
    {
        return dwRc;
    }

    /// 上报事件
    Notify((DCOP_MSG_HEAD *)&evtHead, aEvtData, &sessHead, 0, pEvtPara, dwEvtParaCount);
    if (pEvtPara) DCOP_Free(pEvtPara);

    return SUCCESS;
}

/*******************************************************
  函 数 名: CObjectMember<CMethod>::Action
  描    述: Action
  输    入: pMsg            - 原始消息
            sessionHead     - 从原始消息中解析的会话消息头
            aCondHeads      - 从原始消息中解析的条件消息头
            pReqPara        - 从原始消息中解析的请求参数列表
            dwReqParaCount  - 从原始消息中解析的请求参数个数
            pReqData        - 从原始消息中解析的请求数据
            dwReqDataLen    - 从原始消息中解析的请求数据长度
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CObjectMember<CMethod>::Action(objMsg *pMsg, 
                        const DCOP_SESSION_HEAD &sessionHead, 
                        const CDArray &aCondHeads, 
                        DCOP_PARA_NODE *pReqPara, 
                        DWORD dwReqParaCount, 
                        void *pReqData, 
                        DWORD dwReqDataLen)
{
    CMethod::ACTION action = m_method.GetAction();
    if (!action)
    {
        IObjectMember::Action(pMsg, sessionHead, aCondHeads, pReqPara, dwReqParaCount, pReqData, dwReqDataLen);
        return;
    }

    (*action)(GetOwner(), pMsg, sessionHead, aCondHeads, pReqPara, dwReqParaCount, pReqData, dwReqDataLen);
}

/*******************************************************
  函 数 名: CObjectMemberIndex::CObjectMemberIndex
  描    述: CObjectMemberIndex构造
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CObjectMemberIndex::CObjectMemberIndex()
{
    m_ppIndexTable = 0;
    m_dwIndexCount = 0;
}

/*******************************************************
  函 数 名: CObjectMemberIndex::CObjectMemberIndex
  描    述: CObjectMemberIndex析构
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CObjectMemberIndex::~CObjectMemberIndex()
{
    if (m_ppIndexTable)
    {
        DCOP_Free(m_ppIndexTable);
        m_ppIndexTable = 0;
    }
    m_dwIndexCount = 0;
}

/*******************************************************
  函 数 名: CObjectMemberIndex::Init
  描    述: CObjectMemberIndex初始化
  输    入: ppMembers
            dwCount
            pDispatch
            pNotifyPool
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CObjectMemberIndex::Init(IObjectMember **ppMembers, DWORD dwCount, IDispatch *pDispatch, INotify::IPool *pNotifyPool)
{
    IObjectMember **ppTmp = (IObjectMember **)DCOP_Malloc(sizeof(IObjectMember *) * dwCount);
    if (!ppTmp)
    {
        return FAILURE;
    }

    (void)memcpy(ppTmp, ppMembers, sizeof(IObjectMember *) * dwCount);

    for (DWORD i = 0; i < dwCount; ++i)
    {
        if (ppTmp[i] == NULL)
        {
            DCOP_Free(ppTmp);
            return FAILURE;
        }

        ppTmp[i]->SetDispatch(pDispatch);
        ppTmp[i]->SetNotifyPool(pNotifyPool);
    }

    m_ppIndexTable = ppTmp;
    m_dwIndexCount = dwCount;

    return SUCCESS;
}

/*******************************************************
  函 数 名: CObjectMemberIndex::Dispatch
  描    述: 分发消息
  输    入: msg
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CObjectMemberIndex::Dispatch(objMsg *pMsg)
{
    if (!m_ppIndexTable || !m_dwIndexCount)
    {
        return;
    }

    if (!pMsg)
    {
        return;
    }

    /// 获取消息头
    CDArray aSessHeads;
    IObjectMember::GetMsgHead(pMsg->GetDataBuf(), pMsg->GetDataLen(), &aSessHeads, 0, 0, 0, 0);
    for (DWORD i = 0; i < aSessHeads.Count(); ++i)
    {
        /// 获取会话头
        DCOP_SESSION_HEAD *pSessionHead = (DCOP_SESSION_HEAD *)aSessHeads.Pos(i);
        if (!pSessionHead)
        {
            continue;
        }

        /// 获取属性值
        DWORD dwAttrID = pSessionHead->m_attribute;
        if (!dwAttrID)
        {
            continue;
        }

        /// 只处理请求
        if (pSessionHead->m_ack != DCOP_REQ)
        {
            continue;
        }

        /// 获取会话数据
        void *pSessionData = *(void **)(pSessionHead + 1);

        /// 获取请求参数
        CDArray aCondHeads;
        CDArray aReqHeads;
        IObjectMember::GetMsgHead(pSessionData, pSessionHead->m_type.m_valueLen, 0, &aCondHeads, &aReqHeads, 0, 0);
        DCOP_REQUEST_HEAD *pReqHead = (DCOP_REQUEST_HEAD *)aReqHeads.Pos(0);
        void *pReqParaData = (pReqHead)? *(void **)(pReqHead + 1) : NULL;
        DWORD dwReqParaCount = (pReqHead)? pReqHead->m_paraCount : 0;
        DWORD dwReqDataLen = (pReqHead)? pReqHead->m_paraLen : 0;
        CDStream sReqPara(dwReqParaCount * sizeof(DCOP_PARA_NODE));
        DCOP_PARA_NODE *pReqPara = (DCOP_PARA_NODE *)sReqPara.Buffer();
        void *pReqData = IObjectMember::GetMsgParaData(pReqParaData, dwReqParaCount, dwReqDataLen, pReqPara);

        /// 分发到指定属性
        for (DWORD i = 0; i < m_dwIndexCount; ++i)
        {
            /// 对应的属性索引表必须有值
            if (!(m_ppIndexTable[i]))
            {
                continue;
            }

            /// 对应的属性值必须匹配
            CAttribute *pAttr = m_ppIndexTable[i]->GetAttribute();
            if (!pAttr || (pAttr->GetID() != dwAttrID))
            {
                continue;
            }

            /// 调用处理接口进行处理
            m_ppIndexTable[i]->Action(pMsg, *pSessionHead, aCondHeads, pReqPara, dwReqParaCount, pReqData, dwReqDataLen);
            break;
        }
    }
}

/*******************************************************
  函 数 名: ParseMsg
  描    述: 解析消息
  输    入: pBuf
            dwLen
            rdwOffset
  输    出: rdwOffset
  返    回: 
  修改记录: 
 *******************************************************/
DCOP_MSG_HEAD *ParseMsg(void *pBuf, DWORD dwLen, DWORD &rdwOffset)
{
    if (!pBuf)
    {
        return NULL;
    }

    if (!dwLen || (dwLen <= rdwOffset))
    {
        return NULL;
    }

    if (!CheckMsgHead((BYTE *)pBuf + rdwOffset, dwLen - rdwOffset))
    {
        return NULL;
    }

    DCOP_MSG_HEAD *pHead = (DCOP_MSG_HEAD *)((BYTE *)pBuf + rdwOffset);
    rdwOffset += pHead->m_headSize + Bytes_GetWord((BYTE *)&(pHead->m_valueLen));

    return pHead;
}

/*******************************************************
  函 数 名: CheckMsgHead
  描    述: 检查消息头
  输    入: pBuf
            dwLen
  输    出: 
  返    回: 是否消息头
  修改记录: 
 *******************************************************/
bool CheckMsgHead(void *pBuf, DWORD dwLen)
{
    if (dwLen < sizeof(DCOP_MSG_HEAD))
    {
        return false;
    }

    DCOP_MSG_HEAD *pHead = (DCOP_MSG_HEAD *)pBuf;
    if (!pHead)
    {
        return false;
    }

    if (pHead->m_headType >= DCOP_MSG_HEAD_COUNT)
    {
        return false;
    }

    if (pHead->m_headSize != DCOP_MSG_HEAD_SIZE[pHead->m_headType])
    {
        return false;
    }

    if (dwLen < (DWORD)(pHead->m_headSize + Bytes_GetWord((BYTE *)&(pHead->m_valueLen))))
    {
        return false;
    }

    return true;
}

/*******************************************************
  函 数 名: IObjectMember::GetMsgHead
  描    述: 获取指定类型的消息头
  输    入: pBuf        - 缓冲区指针
            dwLen       - 缓冲区长度
  输    出: aHeads      - 消息头数组
  返    回: 
  修改记录: 
 *******************************************************/
void IObjectMember::GetMsgHead(void *pBuf, DWORD dwLen, 
                        CDArray *paSessHeads, 
                        CDArray *paCondHeads, 
                        CDArray *paReqHeads, 
                        CDArray *paRspHeads, 
                        CDArray *paEvtHeads)
{
    DWORD dwOffset = 0;
    DCOP_MSG_HEAD *pHead = NULL;
    while ((pHead = ParseMsg(pBuf, dwLen, dwOffset)) != NULL)
    {
        /// 分类进行输出消息头
        CDArray *paHeads = NULL;
        switch (pHead->m_headType)
        {
            case DCOP_MSG_HEAD_SESSION:
                if (paSessHeads) paHeads = paSessHeads;
                break;
            case DCOP_MSG_HEAD_CONDITION:
                if (paCondHeads) paHeads = paCondHeads;
                break;
            case DCOP_MSG_HEAD_REQUEST:
                if (paReqHeads) paHeads = paReqHeads;
                break;
            case DCOP_MSG_HEAD_RESPONSE:
                if (paRspHeads) paHeads = paRspHeads;
                break;
            case DCOP_MSG_HEAD_EVENT:
                if (paEvtHeads) paHeads = paEvtHeads;
                break;
            default:
                break;
        }

        /// 动态数组中的节点是由'消息头'+'参数指针'组成
        /// '消息头'是直接拷贝整个消息头，已经经过字节转换
        /// '参数指针'是直接指向原始消息区的消息头后面的参数和数据区
        if (paHeads)
        {
            /// 写入'消息头'到动态数组最后
            paHeads->SetNodeSize(pHead->m_headSize + sizeof(void *));
            if (paHeads->Append(pHead, pHead->m_headSize) != SUCCESS) continue;

            /// 获取动态数组最后节点
            void *pNode = paHeads->Pos(paHeads->Count() - 1);
            if (!pNode) continue;

            /// 转换字节序
            ChangeMsgHeadOrder((DCOP_MSG_HEAD *)pNode);

            /// 把消息头后面'参数指针'填入到动态数组节点最后
            if (pHead->m_valueLen)
            {
                *(void **)((BYTE *)pNode + pHead->m_headSize) = (BYTE *)pHead + pHead->m_headSize;
            }
        }
    }
}

/*******************************************************
  函 数 名: IObjectMember::GetMsgParaData
  描    述: 获取消息参数和数据
  输    入: pMsgParaData    - 消息参数
            dwParaCount     - 参数个数
            dwDataLen       - 参数长度
            pPara           - 参数内存
  输    出: pPara           - 输出参数
  返    回: 输出数据
  修改记录: 
 *******************************************************/
void *IObjectMember::GetMsgParaData(void *pMsgParaData, 
                        DWORD dwParaCount, 
                        DWORD dwDataLen, 
                        DCOP_PARA_NODE *pPara)
{
    if (!pMsgParaData)
    {
        return NULL;
    }

    if (pPara && dwParaCount)
    {
        (void)memcpy(pPara, pMsgParaData, dwParaCount * sizeof(DCOP_PARA_NODE));
        BYTES_CHANGE_PARA_NODE_ORDER(pPara, dwParaCount);
    }

    return (dwDataLen)? ((BYTE *)pMsgParaData + dwParaCount * sizeof(DCOP_PARA_NODE)) : NULL;
}

/*******************************************************
  函 数 名: ChangeMsgHeadOrder
  描    述: 改变消息头字节序
  输    入: pHead       - 消息头
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void ChangeMsgHeadOrder(DCOP_MSG_HEAD *pHead)
{
    if (!pHead) return;

    switch (pHead->m_headType)
    {
        case DCOP_MSG_HEAD_SESSION:
        {
            BYTES_CHANGE_SESSION_HEAD_ORDER(pHead);
        }
            break;
        case DCOP_MSG_HEAD_CONDITION:
        {
            BYTES_CHANGE_CONDITION_HEAD_ORDER(pHead);
        }
            break;
        case DCOP_MSG_HEAD_REQUEST:
        {
            BYTES_CHANGE_REQUEST_HEAD_ORDER(pHead);
        }
            break;
        case DCOP_MSG_HEAD_RESPONSE:
        {
            BYTES_CHANGE_RESPONSE_HEAD_ORDER(pHead);
        }
            break;
        case DCOP_MSG_HEAD_EVENT:
        {
            BYTES_CHANGE_EVENT_HEAD_ORDER(pHead);
        }
            break;
        default:
            break;
    }
}

/*******************************************************
  函 数 名: AddMsgParaData
  描    述: 设置消息参数和数据
  输    入: pHead       - 消息头
            dwParaCount - 参数个数
            dwDataLen   - 参数长度
            pPara       - 参数
            pData       - 数据
  输    出: 成功或者失败的错误码
  返    回: 
  修改记录: 
 *******************************************************/
void AddMsgParaData(DCOP_MSG_HEAD *pHead, 
                DWORD dwParaCount, DWORD dwDataLen, 
                DCOP_PARA_NODE *pPara, void *pData)
{
    if (!pHead) return;

    if ((pHead->m_headType == DCOP_MSG_HEAD_SESSION) || 
        (pHead->m_headType >= DCOP_MSG_HEAD_COUNT))
    {
        return;
    }

    if (!pPara) dwParaCount = 0;
    if (!pData) dwDataLen = 0;

    if (pHead->m_valueLen < (dwParaCount * sizeof(DCOP_PARA_NODE) + dwDataLen))
    {
        return;
    }

    BYTE *pbyBuf = (BYTE *)pHead + pHead->m_headSize;
    DWORD dwOffset = 0;

    /// 根据参数数量添加参数
    DCOP_PARA_NODE *pParaWritten = (dwParaCount)? (DCOP_PARA_NODE *)pbyBuf : NULL;
    if (pParaWritten)
    {
        (void)memcpy(pParaWritten, pPara, dwParaCount * sizeof(DCOP_PARA_NODE));
        BYTES_CHANGE_PARA_NODE_ORDER(pParaWritten, dwParaCount);
        dwOffset += dwParaCount * sizeof(DCOP_PARA_NODE);
    }

    /// 根据数据长度添加数据
    void *pDataWritten = (dwDataLen)? (pbyBuf + dwOffset) : NULL;
    if (pDataWritten)
    {
        (void)memcpy(pDataWritten, pData, dwDataLen);
        dwOffset += dwDataLen;
    }

    switch (pHead->m_headType)
    {
        case DCOP_MSG_HEAD_CONDITION:
        {
            DCOP_CONDITION_HEAD *pCondHead = (DCOP_CONDITION_HEAD *)pHead;
            pCondHead->m_paraCount = (BYTE)dwParaCount;
            pCondHead->m_paraLen = (WORD)dwDataLen;
        }
            break;
        case DCOP_MSG_HEAD_REQUEST:
        {
            DCOP_REQUEST_HEAD *pReqHead = (DCOP_REQUEST_HEAD *)pHead;
            pReqHead->m_paraCount = (WORD)dwParaCount;
            pReqHead->m_paraLen = (WORD)dwDataLen;
        }
            break;
        case DCOP_MSG_HEAD_RESPONSE:
        {
            DCOP_RESPONSE_HEAD *pRspHead = (DCOP_RESPONSE_HEAD *)pHead;
            pRspHead->m_paraCount = (WORD)dwParaCount;
            pRspHead->m_paraLen = (WORD)dwDataLen;
        }
            break;
        case DCOP_MSG_HEAD_EVENT:
        {
            DCOP_EVENT_HEAD *pEvtHead = (DCOP_EVENT_HEAD *)pHead;
            pEvtHead->m_paraCount = (WORD)dwParaCount;
            pEvtHead->m_paraLen = (WORD)dwDataLen;
        }
            break;
        default:
            break;
    }

    pHead->m_valueLen = (WORD)dwOffset;
}

/*******************************************************
  函 数 名: IsCountReq
  描    述: 是否请求数量
            (如果返回true,则表明是在进行记录数的统计)
  输    入: pReqPara        - 请求字段
            dwReqParaCount  - 字段数量
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
bool IsCountReq(DCOP_PARA_NODE *pReqPara, 
                        DWORD dwReqParaCount)
{
    if (!pReqPara)
    {
        return false;
    }

    for (DWORD i = 0; i < dwReqParaCount; ++i)
    {
        if (pReqPara[i].m_paraID == DCOP_SPECPARA_COUNT)
        {
            return true;
        }
    }

    return false;
}

/*******************************************************
  函 数 名: InitMsgHead
  描    述: 初始化指定类型的消息头
  输    入: pBuf        - 未初始化的Buf
            dwLen       - Buf长度
            byHeadType  - 需要初始化的头部类型
            pHeadCopy   - 需要拷贝的消息头
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void InitMsgHead(void *pBuf, DWORD dwLen, BYTE byHeadType, const DCOP_MSG_HEAD *pHeadCopy)
{
    if (!pBuf || (byHeadType >= DCOP_MSG_HEAD_COUNT) || 
        (dwLen < sizeof(DCOP_MSG_HEAD)) || 
        (dwLen < DCOP_MSG_HEAD_SIZE[byHeadType]))
    {
        return;
    }

    (void)memset(pBuf, 0, dwLen);

    DCOP_MSG_HEAD *pHead = (DCOP_MSG_HEAD *)pBuf;
    pHead->m_headType = byHeadType;
    pHead->m_headSize = DCOP_MSG_HEAD_SIZE[byHeadType];
    pHead->m_valueLen = (WORD)(dwLen - pHead->m_headSize);
    if (pHeadCopy)
    {
        (void)memcpy(pHead + 1, pHeadCopy + 1, 
                        pHead->m_headSize - sizeof(DCOP_MSG_HEAD));
    }
}

/*******************************************************
  函 数 名: IObjectMember::PackMsg
  描    述: 打包消息
  输    入: pDispatch       - 分发器
            dwSrcID         - 源地址
            dwDstID         - 目的地址
            dwMsgType       - 消息类型
            pSessionHead    - 会话消息头
            pMsgHead        - 具体消息头
            pMsgPara        - 消息参数描述
            dwMsgParaCount  - 消息参数个数
            aMsgParaData    - 消息数据列表
            dwSendTryTimes  - 发送重试次数
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD IObjectMember::PackMsg(IDispatch *pDispatch, 
                        DWORD dwSrcID, 
                        DWORD dwDstID, 
                        DWORD dwMsgType, 
                        DCOP_SESSION_HEAD *pSessionHead, 
                        DCOP_MSG_HEAD *pMsgHead, 
                        DCOP_PARA_NODE *pMsgPara, 
                        DWORD dwMsgParaCount, 
                        const CDArray &aMsgParaData, 
                        DWORD dwSendTryTimes)
{
    if (!pDispatch || !dwDstID || !pSessionHead || !pMsgHead)
    {
        /// 输入参数错误
        return FAILURE;
    }

    if (!pMsgPara) dwMsgParaCount = 0;

    /////////////////////////////////////////////////
    /// 创建第一包消息包，空间最大长度不能超过MTU
    /////////////////////////////////////////////////
    DWORD dwMTU = pDispatch->GetMTU();
    DWORD dwMsgParaLen = (aMsgParaData.Count())? aMsgParaData.Size() : 0;
    DWORD dwLen = pSessionHead->m_type.m_headSize + 
                        pMsgHead->m_headSize * ((aMsgParaData.Count())? aMsgParaData.Count() : 1) + 
                        dwMsgParaCount * sizeof(DCOP_PARA_NODE) + 
                        dwMsgParaLen * aMsgParaData.Count();
    if (dwLen > dwMTU) dwLen = dwMTU;

    if ((pSessionHead->m_type.m_headSize + 
            pMsgHead->m_headSize + 
            dwMsgParaCount * sizeof(DCOP_PARA_NODE) + 
            dwMsgParaLen) > dwLen)
    {
        /// 放不下一条记录
        return FAILURE;
    }

    objMsg *pMsg = DCOP_CreateMsg(dwLen, dwMsgType, dwSrcID);
    if (!pMsg) return FAILURE;

    DCOP_SESSION_HEAD *pBuf = (DCOP_SESSION_HEAD *)pMsg->GetDataBuf();
    if (!pBuf)
    {
        delete pMsg;
        return FAILURE;
    }

    InitMsgHead(pBuf, dwLen, DCOP_MSG_HEAD_SESSION, (DCOP_MSG_HEAD *)pSessionHead);
    DWORD dwRc = SUCCESS;

    /////////////////////////////////////////////////
    /// 空包数据立刻发送
    /////////////////////////////////////////////////
    if (!aMsgParaData.Count())
    {
        DCOP_MSG_HEAD *pHead = (DCOP_MSG_HEAD *)(pBuf + 1);
        InitMsgHead(pHead, dwLen - pSessionHead->m_type.m_headSize, pMsgHead->m_headType, pMsgHead);
        AddMsgParaData(pHead, dwMsgParaCount, 0, pMsgPara, 0);
        ChangeMsgHeadOrder(pHead);

        if (DCOP_RSP(pBuf->m_ack)) pBuf->m_ack = DCOP_RSP_END;
        pBuf->m_count = 1;
        pBuf->m_type.m_valueLen = (WORD)(pHead->m_headSize);
        BYTES_CHANGE_SESSION_HEAD_ORDER(pBuf);
        pMsg->MsgHead().m_dwDataLen = sizeof(DCOP_SESSION_HEAD) + pHead->m_headSize;
        dwRc = DispatchMsg(pDispatch, dwDstID, pMsg, dwSendTryTimes);
        if (dwRc) delete pMsg;
        return dwRc;
    }

    /////////////////////////////////////////////////
    /// 多包数据循环添后至消息包满时发送
    /////////////////////////////////////////////////
    objMsg *pMsgSend = NULL;
    DCOP_SESSION_HEAD *pBufSend = NULL;
    DCOP_MSG_HEAD *pHeadSend = NULL;
    DWORD dwOffset = 0;
    DWORD dwSendCount = 0;
    for (DWORD i = 0; i < aMsgParaData.Count(); ++i)
    {
        /// 剩下的数据无法放下一条记录，只有进行发送
        if ((dwOffset + pMsgHead->m_headSize + dwMsgParaLen) > dwLen)
        {
            if (DCOP_RSP(pBufSend->m_ack)) pBufSend->m_ack = DCOP_RSP_CON;
            pBufSend->m_count = (WORD)dwSendCount;
            pBufSend->m_type.m_valueLen = (WORD)(dwOffset - sizeof(DCOP_SESSION_HEAD));
            BYTES_CHANGE_SESSION_HEAD_ORDER(pBufSend);
            pMsgSend->MsgHead().m_dwDataLen = dwOffset;
            dwRc = DispatchMsg(pDispatch, dwDstID, pMsgSend, dwSendTryTimes);
            if (dwRc) delete pMsgSend;
            pMsgSend = 0;
        }

        /// 没有消息就从原始消息复制
        if (!pMsgSend)
        {
            pMsgSend = DCOP_CloneMsg(pMsg);
            if (!pMsgSend)
            {
                dwRc = FAILURE;
                break;
            }

            /// 所有的记录都是相同类型的，因此会话头是一样的
            pBufSend = (DCOP_SESSION_HEAD *)pMsgSend->GetDataBuf();
            if (!pBufSend)
            {
                delete pMsgSend;
                dwRc = FAILURE;
                break;
            }

            dwOffset = sizeof(DCOP_SESSION_HEAD);
            dwSendCount = 0;
        }

        /// 初始化消息头
        pHeadSend = (DCOP_MSG_HEAD *)((BYTE *)pBufSend + dwOffset);
        InitMsgHead(pHeadSend, pMsgHead->m_headSize + 
                        ((!dwSendCount)? dwMsgParaCount : 0) * sizeof(DCOP_PARA_NODE) + 
                        dwMsgParaLen, pMsgHead->m_headType, pMsgHead);

        /// 添加参数
        AddMsgParaData((DCOP_MSG_HEAD *)pHeadSend, 
                        (!dwSendCount)? dwMsgParaCount : 0, 
                        dwMsgParaLen, 
                        (!dwSendCount)? pMsgPara : NULL, 
                        aMsgParaData.Pos(i));

        /// 如果是响应或者事件，需要记录多包的索引
        if (pMsgHead->m_headType == DCOP_MSG_HEAD_RESPONSE)
        {
            DCOP_RESPONSE_HEAD *pRspHead = (DCOP_RESPONSE_HEAD *)pHeadSend;
            pRspHead->m_recordCount = aMsgParaData.Count();
            pRspHead->m_recordIndex = i;
        }
        if (pMsgHead->m_headType == DCOP_MSG_HEAD_EVENT)
        {
            DCOP_EVENT_HEAD *pEvtHead = (DCOP_EVENT_HEAD *)pHeadSend;
            pEvtHead->m_recordCount = aMsgParaData.Count();
            pEvtHead->m_recordIndex = i;
        }

        dwOffset += pHeadSend->m_headSize + pHeadSend->m_valueLen;
        ChangeMsgHeadOrder(pHeadSend);
        ++dwSendCount;
    }

    /////////////////////////////////////////////////
    /// 发送最后的数据
    /////////////////////////////////////////////////
    if (pMsgSend)
    {
        if (DCOP_RSP(pBufSend->m_ack)) pBufSend->m_ack = DCOP_RSP_END;
        pBufSend->m_count = (WORD)dwSendCount;
        pBufSend->m_type.m_valueLen = (WORD)(dwOffset - sizeof(DCOP_SESSION_HEAD));
        BYTES_CHANGE_SESSION_HEAD_ORDER(pBufSend);
        pMsgSend->MsgHead().m_dwDataLen = dwOffset;
        dwRc = DispatchMsg(pDispatch, dwDstID, pMsgSend, dwSendTryTimes);
        if (dwRc) delete pMsgSend;
    }

    delete pMsg;
    return dwRc;
}

/*******************************************************
  函 数 名: IObjectMember::PackMsg
  描    述: 打包消息
  输    入: pDispatch       - 分发器
            dwSrcID         - 源地址
            dwDstID         - 目的地址
            dwMsgType       - 消息类型
            pSessionHead    - 会话消息头
            pPackNode       - 组包节点指针
            dwPackNodeCount - 组包节点个数
            pReqPool        - 请求缓冲池
            dwRspMsgType    - 响应消息类型
            dwTimeout       - 响应超时时间
            dwSendTryTimes  - 发送重试次数
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD IObjectMember::PackMsg(IDispatch *pDispatch, 
                        objMsg **ppOutMsg, 
                        DWORD dwSrcID, 
                        DWORD dwDstID, 
                        DWORD dwMsgType, 
                        DCOP_SESSION_HEAD *pSessionHead, 
                        CAttribute::PACK_MSG_NODE *pPackNode, 
                        DWORD dwPackNodeCount, 
                        IResponse::IPool *pReqPool, 
                        DWORD dwRspMsgType, 
                        DWORD dwTimeout, 
                        DWORD dwSendTryTimes)
{
    if (!pDispatch && !ppOutMsg) return FAILURE;
    if (!pSessionHead) return FAILURE;
    if (dwPackNodeCount && !pPackNode) return FAILURE;

    /////////////////////////////////////////////////
    /// 获取组包节点的总长度
    /////////////////////////////////////////////////
    DWORD dwPackNodeTotalLen = 0;
    for (DWORD i = 0; i < dwPackNodeCount; ++i)
    {
        /// 节点头不能为空
        if (!(pPackNode[i].m_pMsgHead)) continue;
        if (!(pPackNode[i].m_dwMsgParaCount) && !(pPackNode[i].m_sMsgParaData.Length()))
        {
            continue;
        }

        /// 计算进总长度
        dwPackNodeTotalLen += pPackNode[i].m_pMsgHead->m_headSize + 
                        pPackNode[i].m_dwMsgParaCount * sizeof(DCOP_PARA_NODE) + 
                        pPackNode[i].m_sMsgParaData.Length();
    }

    /////////////////////////////////////////////////
    /// 创建消息包，空间最大长度不能超过MTU
    /////////////////////////////////////////////////
    DWORD dwLen = pSessionHead->m_type.m_headSize + dwPackNodeTotalLen;
    if (pDispatch && (dwLen > pDispatch->GetMTU()))
    {
        return FAILURE;
    }

    objMsg *pMsg = DCOP_CreateMsg(dwLen, dwMsgType, dwSrcID);
    if (!pMsg) return FAILURE;

    DCOP_SESSION_HEAD *pBuf = (DCOP_SESSION_HEAD *)pMsg->GetDataBuf();
    if (!pBuf)
    {
        delete pMsg;
        return FAILURE;
    }

    InitMsgHead(pBuf, dwLen, DCOP_MSG_HEAD_SESSION, (DCOP_MSG_HEAD *)pSessionHead);
    DWORD dwRc = SUCCESS;

    /// 空包立刻发送
    if (!dwPackNodeTotalLen)
    {
        pBuf->m_count = 0;
        pBuf->m_type.m_valueLen = 0;
        BYTES_CHANGE_SESSION_HEAD_ORDER(pBuf);
        pMsg->MsgHead().m_dwDataLen = sizeof(DCOP_SESSION_HEAD);
        pMsg->MsgHead().m_dwDstID = dwDstID;

        if (pDispatch)
        {
            dwRc = DispatchMsg(pDispatch, dwDstID, pMsg, dwSendTryTimes, 
                        pReqPool, dwRspMsgType, dwTimeout);
            if (dwRc) delete pMsg;
            return dwRc;
        }

        *ppOutMsg = pMsg;
        return SUCCESS;
    }

    /////////////////////////////////////////////////
    /// 将参数加入到消息包中
    /////////////////////////////////////////////////
    DCOP_MSG_HEAD *pHead = (DCOP_MSG_HEAD *)(pBuf + 1);
    DWORD dwOffset = sizeof(DCOP_SESSION_HEAD);
    DWORD dwSendCount = 0;
    for (DWORD i = 0; i < dwPackNodeCount; ++i)
    {
        /// 节点头不能为空
        if (!(pPackNode[i].m_pMsgHead)) continue;
        if (!(pPackNode[i].m_dwMsgParaCount) && !(pPackNode[i].m_sMsgParaData.Length()))
        {
            continue;
        }

        DWORD dwPackNodeLen = pPackNode[i].m_pMsgHead->m_headSize + 
                        pPackNode[i].m_dwMsgParaCount * sizeof(DCOP_PARA_NODE) + 
                        pPackNode[i].m_sMsgParaData.Length();

        InitMsgHead(pHead, dwPackNodeLen, pPackNode[i].m_pMsgHead->m_headType, pPackNode[i].m_pMsgHead);
        AddMsgParaData(pHead, pPackNode[i].m_dwMsgParaCount, pPackNode[i].m_sMsgParaData.Length(), 
                        pPackNode[i].m_pMsgPara, pPackNode[i].m_sMsgParaData.Buffer());

        ChangeMsgHeadOrder(pHead);
        pHead = (DCOP_MSG_HEAD *)((BYTE *)pHead + dwPackNodeLen);
        dwOffset += dwPackNodeLen;
        ++dwSendCount;
    }

    #if 0
    PrintBuffer(STR_FORMAT("<Send Req Msg> len:%d type:0x%x src:%d, dst:%d", 
                        pMsg->GetDataLen(), pMsg->GetMsgType(), pMsg->GetSrcID(), pMsg->GetDstID()), 
                        pMsg->GetDataBuf(), pMsg->GetDataLen(), printf, 0);
    #endif

    pBuf->m_count = (WORD)dwSendCount;
    pBuf->m_type.m_valueLen = (WORD)(dwOffset - sizeof(DCOP_SESSION_HEAD));
    BYTES_CHANGE_SESSION_HEAD_ORDER(pBuf);
    pMsg->MsgHead().m_dwDataLen = dwOffset;
    pMsg->MsgHead().m_dwDstID = dwDstID;
    
    if (pDispatch)
    {
        
        dwRc = DispatchMsg(pDispatch, dwDstID, pMsg, dwSendTryTimes, 
                        pReqPool, dwRspMsgType, dwTimeout);
        if (dwRc) delete pMsg;
        return dwRc;
    }

    *ppOutMsg = pMsg;
    return SUCCESS;
}

/*******************************************************
  函 数 名: CAttribute::PACK_SESS_HEAD
  描    述: 构造
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CAttribute::PACK_SESS_HEAD::PACK_SESS_HEAD(const DCOP_SESSION_HEAD *pSessHead)
{
    InitMsgHead(this, sizeof(DCOP_SESSION_HEAD), DCOP_MSG_HEAD_SESSION, (DCOP_MSG_HEAD *)pSessHead);
    m_ver = DCOP_SESSION_VER;
}

/*******************************************************
  函 数 名: CAttribute::PACK_COND_HEAD
  描    述: 构造
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CAttribute::PACK_COND_HEAD::PACK_COND_HEAD()
{
    InitMsgHead(this, sizeof(DCOP_CONDITION_HEAD), DCOP_MSG_HEAD_CONDITION);
}

/*******************************************************
  函 数 名: CAttribute::PACK_REQ_HEAD
  描    述: 构造
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CAttribute::PACK_REQ_HEAD::PACK_REQ_HEAD()
{
    InitMsgHead(this, sizeof(DCOP_REQUEST_HEAD), DCOP_MSG_HEAD_REQUEST);
}

/*******************************************************
  函 数 名: CAttribute::PACK_RSP_HEAD
  描    述: 构造
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CAttribute::PACK_RSP_HEAD::PACK_RSP_HEAD()
{
    InitMsgHead(this, sizeof(DCOP_RESPONSE_HEAD), DCOP_MSG_HEAD_RESPONSE);
}

/*******************************************************
  函 数 名: CAttribute::PACK_EVT_HEAD
  描    述: 构造
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CAttribute::PACK_EVT_HEAD::PACK_EVT_HEAD()
{
    InitMsgHead(this, sizeof(DCOP_EVENT_HEAD), DCOP_MSG_HEAD_EVENT);
}

/*******************************************************
  函 数 名: DispatchMsg
  描    述: 分发消息
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD DispatchMsg(IDispatch *pDispatch, DWORD dwDstID, objMsg *pMsg, 
                        DWORD dwSendTryTimes, IResponse::IPool *pReqPool, 
                        DWORD dwRspMsgType, DWORD dwTimeout)
{
    if (!pMsg) return FAILURE;

    if (!dwSendTryTimes) dwSendTryTimes = SEND_TRY_TIMES_DEFAULT;

    DWORD dwRc = FAILURE;
    DWORD dwHaveTryTimes = 0;
    while ((dwRc != SUCCESS) && (dwHaveTryTimes++ < dwSendTryTimes))
    {
        if (pReqPool)
        {
            DCOP_SESSION_HEAD *pBuf = (DCOP_SESSION_HEAD *)pMsg->GetDataBuf();
            if (!pBuf)
            {
                return FAILURE;
            }

            if (pBuf->m_ack == DCOP_REQ)
            {
                if (!dwRspMsgType) dwRspMsgType = DCOP_MSG_OBJECT_RESPONSE;
                if (!dwTimeout) dwTimeout = SEND_TIMEOUT_DEFAULT;
                dwRc = pReqPool->OnReq(pBuf, pMsg->GetMsgType(), pMsg->GetSrcID(), 
                        dwDstID, dwRspMsgType, dwTimeout, dwSendTryTimes);
                if (dwRc)
                {
                    continue;
                }
            }
        }

        pMsg->MsgHead().m_dwDstID = dwDstID;
        dwRc = pDispatch->Send(pMsg);
    }

    return dwRc;
}
