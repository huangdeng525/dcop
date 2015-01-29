/// -------------------------------------------------
/// ObjDispatch_main.h : 消息分发器对象私有头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _OBJDISPATCH_MAIN_H_
#define _OBJDISPATCH_MAIN_H_

#include "ObjDispatch_if.h"
#include "ObjAttribute_if.h"
#include "ObjSchedule_if.h"
#include "cfg/argcfg.h"


class CDispatch : public IDispatch
{
public:
    CDispatch(Instance *piParent, int argc, char **argv);
    ~CDispatch();

    DCOP_DECLARE_INSTANCE;
    DCOP_DECLARE_IOBJECT;

    DWORD Init(IObject *root, int argc, void **argv);

    void Fini();

    void Dump(LOG_PRINT logPrint, LOG_PARA logPara, int argc, void **argv);

    DWORD GetMTU();

    DWORD Send(objMsg *message);

    DWORD SendAndWait(objMsg *request, objMsg **response, DWORD waittime);

    void Hook(objMsg *message);

    DECLARE_ATTRIBUTE_INDEX(StackLayer);
    DECLARE_ATTRIBUTE(IObject*, pInfLayer);                     // 接口层 : 使用调度对象

    /// 分布式协议栈对外接口
    static DWORD StackRecvMsg(
                    DWORD dwSrcNodeID, 
                    void *pMsgBuf, 
                    DWORD dwBufLength, 
                    void *pUserArg
                    );

private:
    CArgCfgTable m_cfgTable;
    DECLARE_CONFIG_ITEM(DWORD, hookFlag);
    DECLARE_CONFIG_ITEM(DWORD, lenMax);
    DECLARE_CONFIG_ITEM(DWORD, srcID);
    DECLARE_CONFIG_ITEM(DWORD, dstID);
    DECLARE_CONFIG_ITEM(DWORD, logParaLen);
    LOG_PRINT m_logPrint;
    LOG_PARA  m_logPara;
};


#endif // _OBJDISPATCH_MAIN_H_

