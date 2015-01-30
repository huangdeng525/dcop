/// -------------------------------------------------
/// ObjAttribute_if.h : 对象属性公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _OBJATTRIBUTE_IF_H_
#define _OBJATTRIBUTE_IF_H_

#include "Object_if.h"
#include "Factory_if.h"
#include "BaseMessage.h"
#include "ObjModel_if.h"
#include "ObjData_if.h"
#include "ObjNotify_if.h"
#include "ObjDispatch_if.h"
#include "ObjResponse_if.h"
#include "ObjControl_if.h"
#include "stream/dstream.h"


/// -------------------------------------------------
/// 属性类
/// -------------------------------------------------
class CAttribute
{
public:
    enum TYPE
    {
        TYPE_VAR = IModel::TYPE_EVENT + 1,      // 变量值
        TYPE_BUF,                               // 缓冲区
        TYPE_STR,                               // 字符串
        TYPE_OBJ                                // 子对象
    };

public:
    CAttribute()
    {
        m_attrName = NULL;
        m_attrID   = 0;
    }
    CAttribute(const char *attrName, DWORD attrID, DWORD attrType)
    {
        m_attrName = attrName;
        m_attrID   = attrID;
        m_attrType = attrType;
    }
    ~CAttribute() {}

    void SetName(const char *attrName) {m_attrName = attrName;}
    void SetID(DWORD attrID) {m_attrID = attrID;}
    void SetType(DWORD attrType) {m_attrType = attrType;}

    const char *GetName() {return m_attrName;}
    DWORD GetID() {return m_attrID;}
    DWORD GetType() {return m_attrType;}

public:
    /// 组装会话消息头
    struct PACK_SESS_HEAD : public tagDCOP_SESSION_HEAD
    {
        PACK_SESS_HEAD(const DCOP_SESSION_HEAD *pSessHead = 0);
    };

    /// 组装条件消息头
    struct PACK_COND_HEAD : public tagDCOP_CONDITION_HEAD
    {
        PACK_COND_HEAD();
    };

    /// 组装请求消息头
    struct PACK_REQ_HEAD : public tagDCOP_REQUEST_HEAD
    {
        PACK_REQ_HEAD();
    };

    /// 组装响应消息头
    struct PACK_RSP_HEAD : public tagDCOP_RESPONSE_HEAD
    {
        PACK_RSP_HEAD();
    };

    /// 组装事件消息头
    struct PACK_EVT_HEAD : public tagDCOP_EVENT_HEAD
    {
        PACK_EVT_HEAD();
    };

    /// 组装消息节点
    struct PACK_MSG_NODE
    {
        DCOP_MSG_HEAD *m_pMsgHead;
        DCOP_PARA_NODE *m_pMsgPara;
        DWORD m_dwMsgParaCount;
        const CDStream &m_sMsgParaData;
    };

private:
    const char *m_attrName;                     // 属性名字
    DWORD m_attrID;                             // 属性ID
    DWORD m_attrType;                           // 属性类型
};


/// -------------------------------------------------
/// 对象成员基类
/// -------------------------------------------------
class IObjectMember
{
public:
    IObjectMember()
    {
        m_pOwner = NULL;
        m_pAttribute = NULL;
        m_pDispatch = NULL;
        m_pNotifyPool = NULL;
    }
    virtual ~IObjectMember() {}

    void SetOwner(IObject *pOwner) {m_pOwner = pOwner;}
    IObject *GetOwner() {return m_pOwner;}

    void SetAttribute(CAttribute *pAttr) {m_pAttribute = pAttr;}
    CAttribute *GetAttribute() {return m_pAttribute;}

    void SetDispatch(IDispatch *pDispatch) {m_pDispatch = pDispatch;}
    IDispatch *GetDispatch() {return m_pDispatch;}

    void SetNotifyPool(INotify::IPool *pNotifyPool) {m_pNotifyPool = pNotifyPool;}
    INotify::IPool *GetNotifyPool() {return m_pNotifyPool;}

    virtual IObjectMember *Init(IObject *pOwner, CAttribute *pAttr)
    {
        SetOwner(pOwner);
        SetAttribute(pAttr);

        return (IObjectMember *)this;
    }

    virtual DWORD Set(void *pData, DWORD dwSize) {return SUCCESS;}
    virtual void *Get(DWORD &rdwSize) {rdwSize = 0; return NULL;}

    virtual void Action(objMsg *pMsg, const DCOP_SESSION_HEAD &sessionHead, const CDArray &aCondHeads, DCOP_PARA_NODE *pReqPara, DWORD dwReqParaCount, void *pReqData, DWORD dwReqDataLen);
    virtual void Notify(DCOP_MSG_HEAD *pHead, const CDArray &aData, DCOP_SESSION_HEAD *pSessionHead = 0, DWORD dwDstID = 0, DCOP_PARA_NODE *pPara = 0, DWORD dwParaCount = 0);

public:
    /// 获取指定的消息头
    static void GetMsgHead(void *pBuf, DWORD dwLen, 
                        CDArray *paSessHeads, 
                        CDArray *paCondHeads, 
                        CDArray *paReqHeads, 
                        CDArray *paRspHeads, 
                        CDArray *paEvtHeads);

    /// 获取消息参数和数据
    static void *GetMsgParaData(void *pMsgParaData, 
                        DWORD dwParaCount, 
                        DWORD dwDataLen, 
                        DCOP_PARA_NODE *pPara = 0);

    /// 组装消息并进行分发 - 适用于相同类型的多条记录(响应和事件)
    /// [最少可发出'一个SessionHead + 一个空包MsgHead']
    static DWORD PackMsg(IDispatch *pDispatch, 
                        DWORD dwSrcID, 
                        DWORD dwDstID, 
                        DWORD dwMsgType, 
                        DCOP_SESSION_HEAD *pSessionHead, 
                        DCOP_MSG_HEAD *pMsgHead, 
                        DCOP_PARA_NODE *pMsgPara, 
                        DWORD dwMsgParaCount, 
                        const CDArray &aMsgParaData, 
                        DWORD dwSendTryTimes = 0);

    /// 组装消息并进行分发 - 适用于不同类型的单条记录(请求)
    /// [最少可发出'一个SessionHead']
    static DWORD PackMsg(IDispatch *pDispatch, 
                        objMsg **ppOutMsg, 
                        DWORD dwSrcID, 
                        DWORD dwDstID, 
                        DWORD dwMsgType, 
                        DCOP_SESSION_HEAD *pSessionHead, 
                        CAttribute::PACK_MSG_NODE *pPackNode, 
                        DWORD dwPackNodeCount, 
                        IResponse::IPool *pReqPool = 0, 
                        DWORD dwRspMsgType = 0, 
                        DWORD dwTimeout = 0, 
                        DWORD dwSendTryTimes = 0);

protected:
    void OnChanged()
    {
        DWORD dwSize = 0;
        void *pAddr = Get(dwSize);
        if (!pAddr || !dwSize) return;

        CDArray aEvtData;
        aEvtData.SetNodeSize(dwSize);
        if (aEvtData.Append(pAddr) != SUCCESS)
        {
            return;
        }

        CAttribute::PACK_EVT_HEAD evtHead;
        evtHead.m_recordCount = 1;
        Notify((DCOP_MSG_HEAD *)&evtHead, aEvtData);
    }

private:
    IObject *m_pOwner;                          // 属性所属的对象
    CAttribute *m_pAttribute;                   // 成员对应的属性
    IDispatch *m_pDispatch;                     // 消息分发器
    INotify::IPool *m_pNotifyPool;              // 订阅发布器缓冲池
};


/// -------------------------------------------------
/// 对象成员类
/// -------------------------------------------------
template<class T>
class CObjectMember : public IObjectMember
{
public:
    CObjectMember()
    {
        m_var = 0;
    }
    CObjectMember(T t)
    {
        m_var = t;
    }
    ~CObjectMember() {}

    CObjectMember& operator=(T t)
    {
        if (m_var != t)
        {            
            m_var = t;
            OnChanged();
        }
        
        return *this;
    }
    operator T()
    {
        return m_var;
    }

    DWORD Set(void *pData, DWORD dwSize)
    {
        if (!pData || !dwSize)
            return FAILURE;

        *this = (T)Bytes_GetDwordValue((BYTE *)pData, dwSize);
        return SUCCESS;
    }
    void *Get(DWORD &rdwSize) {rdwSize = sizeof(m_var); return &m_var;}

private:
    T m_var;
};


/// -------------------------------------------------
/// 对象成员类(偏特化: 针对指针)
/// -------------------------------------------------
template<class T>
class CObjectMember<T*> : public IObjectMember
{
public:
    CObjectMember()
    {
        m_ptr = NULL;
        m_dwCount = 0;
    }
    CObjectMember(T* p, DWORD dwCount = 0)
    {
        m_ptr = NULL;
        m_dwCount = dwCount;
        (void)Set(p, dwCount * sizeof(T));
    }
    ~CObjectMember() {if (m_ptr) DCOP_Free(m_ptr);}

    CObjectMember& operator=(T *p)
    {
        (void)Set(p, m_dwCount * sizeof(T));
        return *this;
    }
    operator T*()
    {
        return m_ptr;
    }

    DWORD Set(void *pData, DWORD dwSize)
    {
        if (!pData)
            return FAILURE;

        if (!m_dwCount)
        {
            /// 如果m_dwCount为零，说明是直接使用指针
            m_ptr = pData;
            return SUCCESS;
        }

        DWORD dwLen = m_dwCount * sizeof(T);
        if (dwSize != dwLen)
            return FAILURE;

        if (m_ptr && !memcmp(m_ptr, pData, dwSize))
            return SUCCESS;

        if (!m_ptr)
        {
            void *pBuf = DCOP_Malloc(dwLen);
            if (!pBuf) return FAILURE;
            (void)memset(pBuf, 0, dwLen);
            m_ptr = (T *)pBuf;
        }

        (void)memcpy(m_ptr, pData, dwSize);
        OnChanged();
        return SUCCESS;
    }
    void *Get(DWORD &rdwSize) {rdwSize = m_dwCount * sizeof(m_dwCount); return m_ptr;}

private:
    T *m_ptr;
    DWORD m_dwCount;
};


/// -------------------------------------------------
/// 对象成员类(特化: 针对字符串类型)
/// -------------------------------------------------
template<>
class CObjectMember<char*> : public IObjectMember
{
public:
    CObjectMember()
    {
        m_str = NULL;
    }
    CObjectMember(const char *pStr)
    {
        m_str = NULL;
        (void)Set((void *)pStr, (DWORD)strlen(pStr) + 1);
    }
    ~CObjectMember() {if (m_str) DCOP_Free(m_str);}

    CObjectMember& operator=(const char *pStr)
    {
        (void)Set((void *)pStr, (DWORD)strlen(pStr) + 1);
        return *this;
    }
    operator const char*()
    {
        return m_str;
    }

    DWORD Set(void *pData, DWORD dwSize)
    {
        if (!pData || !dwSize)
            return FAILURE;

        char *pStr = (char *)DCOP_Malloc(dwSize);
        if (!pStr) return FAILURE;
        (void)snprintf(pStr, dwSize, "%s", (char *)pData);
        pStr[dwSize - 1] = '\0';
        if (m_str) DCOP_Free(m_str);
        m_str = pStr;

        OnChanged();
        return SUCCESS;
    }
    void *Get(DWORD &rdwSize) {rdwSize = (DWORD)strlen(m_str) + 1; return m_str;}

private:
    char *m_str;
};


/// -------------------------------------------------
/// 对象成员类(特化: 针对对象类型)
/// -------------------------------------------------
template<>
class CObjectMember<IObject*> : public IObjectMember
{
public:
    CObjectMember()
    {
        m_child = NULL;
    }
    ~CObjectMember() {DCOP_RELEASE_INSTANCE_REFER(GetOwner(), m_child);}

    CObjectMember &operator=(IObject *p)
    {
        m_child = p;
        return *this;
    }
    operator IObject*()
    {
        return m_child;
    }

    void Action(objMsg *pMsg, const DCOP_SESSION_HEAD &sessionHead, const CDArray &aCondHeads, DCOP_PARA_NODE *pReqPara, DWORD dwReqParaCount, void *pReqData, DWORD dwReqDataLen)
    {
        if (m_child) m_child->Proc(pMsg);
    }

private:
    IObject *m_child;
};


/// -------------------------------------------------
/// 对象成员类(特化: 针对数据类型)
/// -------------------------------------------------
template<>
class CObjectMember<IData*> : public IObjectMember
{
public:
    CObjectMember()
    {
        m_pData = NULL;
        m_hData = NULL;
        m_tData = IData::TYPE_MEM;
    }
    CObjectMember(IData::TYPE tData)
    {
        m_pData = NULL;
        m_hData = NULL;
        m_tData = tData;
    }
    ~CObjectMember() {}

    void SetDataObj(IData *pData)
    {
        m_pData = pData;
    }

    void SetDataType(IData::TYPE tData)
    {
        m_tData = tData;
    }

    operator IData::Handle&()
    {
        return m_hData;
    }

    IData *GetData() {return m_pData;}
    IData::Handle GetDataHandle() {return m_hData;}
    IData::TYPE GetDataType() {return m_tData;}
    IModel::TYPE GetModelType() {return IModel::TYPE_DATA;}

    void Action(objMsg *pMsg, const DCOP_SESSION_HEAD &sessionHead, const CDArray &aCondHeads, DCOP_PARA_NODE *pReqPara, DWORD dwReqParaCount, void *pReqData, DWORD dwReqDataLen);

    DWORD Create(const DCOP_SESSION_HEAD *pSessionHead = 0);

    DWORD Destroy(const DCOP_SESSION_HEAD *pSessionHead = 0);

    DWORD AddRecord(DCOP_PARA_NODE *pReqPara, 
                DWORD dwReqParaCount, 
                void *pReqData, 
                DWORD dwReqDataLen, 
                const DCOP_SESSION_HEAD *pSessionHead = 0);

    DWORD DelRecord(BYTE byCond, 
                DCOP_PARA_NODE *pCondPara, 
                DWORD dwCondParaCount, 
                void *pCondData, 
                DWORD dwCondDataLen, 
                const DCOP_SESSION_HEAD *pSessionHead = 0);

    DWORD EditRecord(BYTE byCond, 
                DCOP_PARA_NODE *pCondPara, 
                DWORD dwCondParaCount, 
                void *pCondData, 
                DWORD dwCondDataLen, 
                DCOP_PARA_NODE *pReqPara, 
                DWORD dwReqParaCount, 
                void *pReqData, 
                DWORD dwReqDataLen, 
                const DCOP_SESSION_HEAD *pSessionHead = 0);

private:
    IData *m_pData;
    IData::Handle m_hData;
    IData::TYPE m_tData;
};


/// -------------------------------------------------
/// 对象方法类
/// -------------------------------------------------
class CMethod
{
public:
    typedef void (*ACTION)(IObject *pOwner, objMsg *pMsg, 
                        const DCOP_SESSION_HEAD &sessionHead, 
                        const CDArray &aCondHeads, 
                        DCOP_PARA_NODE *pReqPara, 
                        DWORD dwReqParaCount, 
                        void *pReqData, 
                        DWORD dwReqDataLen);

public:
    CMethod() {m_action = NULL;}
    ~CMethod() {}

    void SetProc(ACTION action) {m_action = action;}
    ACTION GetAction() {return m_action;}

private:
    ACTION m_action;
};


/// -------------------------------------------------
/// 对象成员类(特化: 针对方法类型)
/// -------------------------------------------------
template<>
class CObjectMember<CMethod> : public IObjectMember
{
public:
    CObjectMember() {}
    ~CObjectMember() {}

    CObjectMember &operator=(CMethod::ACTION action)
    {
        m_method.SetProc(action);
        return *this;
    }

    void Action(objMsg *pMsg, const DCOP_SESSION_HEAD &sessionHead, const CDArray &aCondHeads, DCOP_PARA_NODE *pReqPara, DWORD dwReqParaCount, void *pReqData, DWORD dwReqDataLen);

private:
    CMethod m_method;
};


/// -------------------------------------------------
/// 对象成员索引类
/// -------------------------------------------------
class CObjectMemberIndex
{
public:
    CObjectMemberIndex();
    ~CObjectMemberIndex();

    DWORD Init(IObjectMember **ppMembers, DWORD dwCount, IDispatch *pDispatch, INotify::IPool *pNotifyPool);

    void Dispatch(objMsg *pMsg);

private:
    IObjectMember **m_ppIndexTable;
    DWORD m_dwIndexCount;
};


/// -------------------------------------------------
/// 声明属性索引
/// -------------------------------------------------
#define DECLARE_ATTRIBUTE_INDEX(Index)              \
    CObjectMemberIndex m_attribute_##Index


/// -------------------------------------------------
/// 声明属性
/// -------------------------------------------------
#define DECLARE_ATTRIBUTE(Type, Member)             \
    CObjectMember<Type> m_##Member;                 \
    static CAttribute m_attribute_##Member


/// -------------------------------------------------
/// 实现属性
/// -------------------------------------------------
#define IMPLEMENT_ATTRIBUTE(CMyClass, Member, ID, Type) \
    CAttribute CMyClass::m_attribute_##Member(#Member, ID, Type);


/// -------------------------------------------------
/// 初始化属性开始
/// -------------------------------------------------
#define INIT_ATTRIBUTE_START(Index, iDispatch, pNotifyPool) \
    CObjectMemberIndex *__memberIndex = &m_attribute_##Index; \
    IDispatch *__memberDispatch = iDispatch;        \
    INotify::IPool *__memberNotifyPool = pNotifyPool; \
    IObjectMember *__memberTable[] = {


/// -------------------------------------------------
/// 初始化属性成员
/// -------------------------------------------------
#define INIT_ATTRIBUTE_MEMBER(Member)               \
    m_##Member.Init(this, &m_attribute_##Member),


/// -------------------------------------------------
/// 初始化属性结束
/// -------------------------------------------------
#define INIT_ATTRIBUTE_END                          \
    };                                              \
    DWORD __rcIndexInit = __memberIndex->Init(      \
                        __memberTable,              \
                        ARRAY_SIZE(__memberTable),  \
                        __memberDispatch,           \
                        __memberNotifyPool);        \
    if (__rcIndexInit != SUCCESS) return __rcIndexInit;


/// -------------------------------------------------
/// 挂接实现类到IObject模板 - 配置数据类型
/// -------------------------------------------------
#define IMPLEMENT_CONFIG_DATATYPE(Config, Member)   \
    char __type##Member[16] = {0, };                \
    DCOP_IMPLEMENT_CONFIG_STRING(Config, __type##Member) \
    if (!strcmp(__type##Member, "mem"))             \
    {                                               \
        m_##Member.SetDataType(IData::TYPE_MEM);    \
    }                                               \
    else if (!strcmp(__type##Member, "file"))       \
    {                                               \
        m_##Member.SetDataType(IData::TYPE_FILE);   \
    }                                               \
    else if (!strcmp(__type##Member, "mysql"))      \
    {                                               \
        m_##Member.SetDataType(IData::TYPE_MYSQL);  \
    }                                               \
    else                                            \
    {                                               \
    }


/// -------------------------------------------------
/// 挂接实现类到IObject消息处理模板 - 挂接表项
/// -------------------------------------------------
#define IMPLEMENT_ATTRIBUTE_MSG_PROC(Index)         \
            case DCOP_MSG_OBJECT_REQUEST:           \
                m_attribute_##Index.Dispatch(msg);  \
                break;


#endif // #ifndef _OBJATTRIBUTE_IF_H_

