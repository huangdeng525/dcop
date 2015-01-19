/// -------------------------------------------------
/// msgType.h : msgQ封装类公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _MSGTYPE_H_
#define _MSGTYPE_H_

#define INC_DEQUE

#include "../osBase.h"
#include "msg.h"
#include "sem.h"
#include "list/tailq.h"


/// 消息实现
class CMsgBase : public objMsg, private osBase
{
public:
    CMsgBase(DWORD dwDataLen,
                DWORD dwMsgType,
                DWORD dwLocalID,
                const void *cpData = 0,
                DWORD dwCopyLen = 0);
    CMsgBase(void *pFrameBuf,
                DWORD dwFrameLen);
    ~CMsgBase();

    DWORD GetOffset();

    DWORD GetDataLen();

    DWORD GetMsgType();

    DWORD GetSrcID();

    DWORD GetDstID();

    objMsg *Clone(const char *file, int line);

    void *Pack(DWORD &rdwLen, const char *file, int line);

    OSMsgHeader &MsgHead();

    void *GetDataBuf();

    void *SetCtrl(void *pBuf, DWORD dwLen);

    void *GetCtrl(DWORD &rdwLen);

    void *AddData(void *pBuf, DWORD dwLen);

public:
    /// 消息包附加格式说明
    ////////////////////////////////////////////////////
    /// m_header后面还有空间，后面是真正的数据区;
    /// m_header前面的几个结构是在使用过程中的临时缓存:
    /// (最后需要使用Pack()整理或者Clone()一个新的Msg)
    ///     [1] m_setCtrlBuf - 额外设置的控制信息
    ///     [1] m_setCtrlLen - 控制信息长度[不能超过offset-sizeof(OSMsgHeader)]
    ///     [2] m_addDataLst - 附加的数据列表
    ///     [2] m_addDataLen - 附加的数据总长度
    ////////////////////////////////////////////////////
    struct AddDataNode
    {
        TAILQ_ENTRY(AddDataNode) m_field;
        DWORD m_dwDataLen;
    };

private:
    void *m_setCtrlBuf;
    DWORD m_setCtrlLen;
    TAILQ_HEAD(AddDataNode) m_addDataLst;
    DWORD m_addDataLen;
    OSMsgHeader m_header;
};


/// 消息队列实现
class CMsgQueBase : public objMsgQue, private osBase
{
public:
    typedef std::deque<objMsg *> MSGQUE;
    typedef MSGQUE::iterator IT_MSGQUE;

public:
    CMsgQueBase(DWORD dwMaxSize);
    ~CMsgQueBase();

    DWORD Create();
    void Destroy();

    DWORD Send(objMsg *message, DWORD sendPrio = OSMSGQUE_SENDPRIO_NORMAL);
    DWORD Recv(objMsg *&message);
    DWORD Wait(DWORD waitMilliseconds);

    DWORD Size();

private:
    DWORD               m_dwMaxSize;
    MSGQUE              m_msgQues;
    objLock *           m_pLock;
    objEvent *          m_pEvent;

};


#endif // #ifndef _MSGTYPE_H_

