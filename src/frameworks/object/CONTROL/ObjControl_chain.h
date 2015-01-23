/// -------------------------------------------------
/// ObjControl_chain.h : 控制链对象私有头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _OBJCONTROL_CHAIN_H_
#define _OBJCONTROL_CHAIN_H_

#include "ObjControl_if.h"
#include "ObjAttribute_if.h"
#include "array/darray.h"


class CControlChain : public IControl::IChain
{
public:
    struct Node : public IControl::Node
    {
        IObject *m_ctrler;
    };

public:
    CControlChain();
    ~CControlChain();

    void Dump(LOG_PRINT logPrint, LOG_PARA logPara, int argc, void **argv);

    DWORD OnCtrl(objMsg *pInput,
                        objMsg *&pOutput,
                        bool &bContinue);

    void SetCtrlee(IObject *piCtrlee) {m_piCtrlee = piCtrlee;}
    IObject *GetCtrlee() {return m_piCtrlee;}

    DWORD RegCtrlNode(IObject *ctrler,
                        IControl::Node *ctrls,
                        DWORD count);

private:
    void GetCtrlNode(objMsg *pMsg, CDArray &aNodeList);
    bool IsCtrlNode(Node *pNode, DCOP_SESSION_HEAD *pSessionHead);

private:
    CDArray m_aNodeList;
    IObject *m_piCtrlee;
};


#endif // #ifndef _OBJCONTROL_CHAIN_H_
