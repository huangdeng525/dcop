/// -------------------------------------------------
/// msg.h : 消息队列定义公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _MSG_H_
#define _MSG_H_

#include "dcop.h"


/// -------------------------------------------------
/// 消息报文的格式如下:
/// +----------------------+------------------------+
/// |  OSMsgHeader  (Ctrl) |          Data          |
/// +----------------------+------------------------+
/// 
///  <OSMsgHeader> 格式如下:
/// +-----------+-----------+-----------+-----------+
/// |     'D'   |    'c'    |    'o'    |   Offset  |
/// +-----------+-----------+-----------+-----------+
/// |                    DataLen                    |
/// +-----------+-----------+-----------+-----------+
/// |                    MsgType                    |
/// +-----------+-----------+-----------+-----------+
/// |                     SrcID                     |
/// +-----------+-----------+-----------+-----------+
/// |                     DstID                     |
/// +-----------+-----------+-----------+-----------+



/// 消息报文头部格式
class OSMsgHeader
{
public:
    OSMsgHeader();
    ~OSMsgHeader();

    /// 获取消息头部大小
    static BYTE GetHeaderSize() {return sizeof(OSMsgHeader);}

    /// 判断BUF开头是否是正确的帧格式
    ////////////////////////////////////////////////////
    /// 判断数据包的边界, 并: 
    ///     [1] 返回<0: 收到的长度不够判断数据帧(<8Bytes)
    ///     [2] 返回=0: 不是一个正确的数据帧的开头
    ///     [3] 返回>0: 是正确的数据帧并且返回整个数据帧的长度
    /// [说明]pBuf使用的是网络字节序(会按照本地字节序读取)
    ////////////////////////////////////////////////////
    static int bFrame(void *pBuf, DWORD dwLen);

    /// 把数据填充到BUF中
    static OSMsgHeader *Format(void *pBuf,
                        DWORD dwBufLen,
                        DWORD dwMsgType = 0,
                        DWORD dwLocalID = 0,
                        const void *cpData = 0,
                        DWORD dwDataLen = 0
                        );

    /// 按头部格式转换一段pBuf的字节序
    /// 必须保证pBuf的大小是等于消息报文的帧头
    /// 一般用在刚收到远端的数据后或者马上要发送到远端前
    static void ChangeBytesOrder(void *pBuf, DWORD dwLen);

public:
    BYTE    m_byMagicWord0;                                     // 头部魔术字(固定为'D')
    BYTE    m_byMagicWord1;                                     // 头部魔术字(固定为'c')
    BYTE    m_byMagicWord2;                                     // 头部魔术字(固定为'o')
    BYTE    m_byOffset;                                         // 头部长度(HeaderSize==sizeof(OSMsgHeader))
    DWORD   m_dwDataLen;                                        // 数据长度(除头部外的数据长度)
    DWORD   m_dwMsgType;                                        // 消息类型(用于分流消息的处理)
    DWORD   m_dwSrcID;                                          // 源(发送者ID)
    DWORD   m_dwDstID;                                          // 宿(接收者ID)
};


/// 创建消息
#define DCOP_CreateMsg(dwDataLen, dwMsgType, dwLocalID) \
    objMsg::CreateInstance(dwDataLen, dwMsgType, dwLocalID, __FILE__, __LINE__)


/// 创建并加载消息
#define DCOP_LoadMsg(dwDataLen, dwMsgType, dwLocalID, cpData, dwCopyLen) \
    objMsg::CreateInstance(dwDataLen, dwMsgType, dwLocalID, __FILE__, __LINE__, cpData, dwCopyLen)


/// 解析消息
#define DCOP_ParseMsg(pBuf, dwLen) \
    objMsg::Parse(pBuf, dwLen, __FILE__, __LINE__);


/// 复制消息
#define DCOP_CloneMsg(pMsg) \
    ((!pMsg) ? NULL : (pMsg)->Clone(__FILE__, __LINE__))


/// 整理消息
#define DCOP_PackMsg(pMsg, rdwLen) \
    ((!pMsg) ? NULL : (pMsg)->Pack(rdwLen, __FILE__, __LINE__))


/// 消息对象(用于系统中传递消息报文的载体)
class objMsg
{
public:
    static objMsg *CreateInstance(DWORD dwDataLen,
                        DWORD dwMsgType,
                        DWORD dwLocalID,
                        const char *file,
                        int line,
                        const void *cpData = 0,
                        DWORD dwCopyLen = 0);

    static objMsg *Parse(void *pFrameBuf,
                        DWORD dwFrameLen,
                        const char *file,
                        int line);

    virtual ~objMsg() = 0;

    /// 获取头部长度
    virtual DWORD GetOffset() = 0;

    /// 获取数据长度
    virtual DWORD GetDataLen() = 0;

    /// 获取消息类型
    virtual DWORD GetMsgType() = 0;

    /// 获取消息源地址
    virtual DWORD GetSrcID() = 0;

    /// 获取消息目的地址
    virtual DWORD GetDstID() = 0;

    /// 复制一个新的消息
    virtual objMsg *Clone(const char *file, int line) = 0;

    /// 整理消息成一个缓冲区
    virtual void *Pack(DWORD &rdwLen, const char *file, int line) = 0;

    /// 获取消息头
    virtual OSMsgHeader &MsgHead() = 0;

    /// 获取消息地址
    virtual void *GetDataBuf() = 0;

    /// 设置控制信息
    virtual void *SetCtrl(void *pBuf, DWORD dwLen) = 0;

    /// 获取控制信息
    virtual void *GetCtrl(DWORD &rdwLen) = 0;

    /// 添加数据信息
    virtual void *AddData(void *pBuf, DWORD dwLen) = 0;
};


/// 创建消息队列的宏
#define DCOP_CreateMsgQue(dwMaxSize) \
    objMsgQue::CreateInstance(dwMaxSize, __FILE__, __LINE__)


/// 消息队列对象(用于系统中缓存消息报文的载体)
class objMsgQue
{
public:
    typedef enum tagOSMSGQUE_SENDPRIO                           // 发送优先级
    {
        OSMSGQUE_SENDPRIO_NORMAL,                               // 普通发送
        OSMSGQUE_SENDPRIO_URGENT                                // 紧急发送
    }OSMSGQUE_SENDPRIO;

public:
    static objMsgQue *CreateInstance(DWORD dwMaxSize,
                            const char *file,
                            int line);
    virtual ~objMsgQue() = 0;

    virtual DWORD Send(
                    objMsg *message, 
                    DWORD sendPrio = OSMSGQUE_SENDPRIO_NORMAL
                    ) = 0;

    virtual DWORD Recv(
                    objMsg *&message
                    ) = 0;

    virtual DWORD Wait(
                    DWORD waitMilliseconds
                    ) = 0;

    virtual DWORD Size() = 0;

};


#endif // #ifndef _MSG_H_

