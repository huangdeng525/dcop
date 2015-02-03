/// -------------------------------------------------
/// secure_main.h : 安全管理私有头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _SECURE_MAIN_H_
#define _SECURE_MAIN_H_

#include "secure_if.h"
#include "ObjAttribute_if.h"
#include "ObjControl_if.h"


/// -------------------------------------------------
/// 安全管理实现类
/// -------------------------------------------------
class CSecure : public ISecure
{
public:
    CSecure(Instance *piParent, int argc, char **argv);
    ~CSecure();

    DCOP_DECLARE_INSTANCE;
    DCOP_DECLARE_IOBJECT;

    DCOP_DECLARE_IOBJECT_MSG_HANDLE;

    DWORD Init(IObject *root, int argc, void **argv);
    void Fini();

    void Dump(LOG_PRINT logPrint, LOG_PARA logPara, int argc, void **argv);

    void OnStart(objMsg *msg);
    void OnFinish(objMsg *msg);

private:
    static DWORD InputCtrl(objMsg *pInput,
                        objMsg *&pOutput,
                        bool &bContinue,
                        IObject *piCtrler);

private:
    IControl *m_piControl;
};


#endif // #ifndef _SECURE_MAIN_H_

