/// -------------------------------------------------
/// ObjControl_main.h : 控制器对象私有头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _OBJCONTROL_MAIN_H_
#define _OBJCONTROL_MAIN_H_

#define INC_MAP

#include "ObjControl_chain.h"


class CControl : public IControl
{
public:
    typedef std::map<DWORD, CControlChain> MAP_CHAIN;
    typedef MAP_CHAIN::iterator IT_CHAIN;

public:
    CControl(Instance *piParent, int argc, char **argv);
    ~CControl();

    DCOP_DECLARE_INSTANCE;
    DCOP_DECLARE_IOBJECT;

    DWORD Init(IObject *root, int argc, void **argv);
    void Fini();
    void Proc(objMsg *msg);
    void Dump(LOG_PRINT logPrint, LOG_PARA logPara, int argc, void **argv);

    IChain *CreateChain(IObject *ctrlee);

    void DestroyChain(IChain *chain);

    DWORD RegCtrlNode(IObject *ctrler,
                        DWORD ctrlee,
                        Node *ctrls,
                        DWORD count);

private:
    MAP_CHAIN m_chains;
};


#endif // #ifndef _OBJCONTROL_MAIN_H_

