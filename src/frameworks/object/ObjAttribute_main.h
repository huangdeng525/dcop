/// -------------------------------------------------
/// ObjAttribute_main.h : 对象属性私有头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _OBJATTRIBUTE_MAIN_H_
#define _OBJATTRIBUTE_MAIN_H_

#include "ObjAttribute_if.h"
#include "BaseMessage.h"


/// -------------------------------------------------
/// 默认重发次数、默认超时时间
/// -------------------------------------------------
const DWORD SEND_TRY_TIMES_DEFAULT = 3;
const DWORD SEND_TIMEOUT_DEFAULT = 5000;

/// -------------------------------------------------
/// 解析消息
/// -------------------------------------------------
DCOP_MSG_HEAD *ParseMsg(void *pBuf, DWORD dwLen, DWORD &rdwOffset);

/// -------------------------------------------------
/// 检查消息头
/// -------------------------------------------------
bool CheckMsgHead(void *pBuf, DWORD dwLen);

/// -------------------------------------------------
/// 改变消息头字节序
/// -------------------------------------------------
void ChangeMsgHeadOrder(DCOP_MSG_HEAD *pHead);

/// -------------------------------------------------
/// 设置消息参数和数据
/// -------------------------------------------------
void AddMsgParaData(DCOP_MSG_HEAD *pHead, 
                        DWORD dwParaCount, 
                        DWORD dwDataLen, 
                        DCOP_PARA_NODE *pPara, 
                        void *pData);

/// -------------------------------------------------
/// 是否请求数量
/// -------------------------------------------------
bool IsCountReq(DCOP_PARA_NODE *pReqPara, 
                        DWORD dwReqParaCount);

/// -------------------------------------------------
/// 初始化消息头
/// -------------------------------------------------
void InitMsgHead(void *pBuf, DWORD dwLen, BYTE byHeadType, const DCOP_MSG_HEAD *pHeadCopy = NULL);

/// -------------------------------------------------
/// 分发消息
/// -------------------------------------------------
DWORD DispatchMsg(IDispatch *pDispatch, DWORD dwDstID, objMsg *pMsg, DWORD dwSendTryTimes, 
                        IResponse::IPool *pReqPool = 0, 
                        DWORD dwRspMsgType = 0, 
                        DWORD dwTimeout  = 0);


#endif // #ifndef _OBJATTRIBUTE_MAIN_H_

